/* own header files */
#include "XdkAppInfo.h"
#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_APP_CONTROLLER

/* own header files */
#include "AppController.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */
#include "BCDS_BSP_Board.h"
#include "BCDS_CmdProcessor.h"
#include "BCDS_WlanNetworkConfig.h"
#include "BCDS_WlanNetworkConnect.h"
#include "BCDS_Assert.h"
#include "XDK_Utils.h"
#include "XDK_WLAN.h"
#include "XDK_MQTT.h"
#include "XDK_Sensor.h"
#include "XDK_SNTP.h"
#include "XDK_ServalPAL.h"
#include "FreeRTOS.h"
#include "task.h"

/* constant definitions ***************************************************** */

#define MQTT_CONNECT_TIMEOUT_IN_MS                  UINT32_C(60000)/**< Macro for MQTT connection timeout in milli-second */

#define MQTT_PUBLISH_TIMEOUT_IN_MS                  UINT32_C(20000)/**< Macro for MQTT publication timeout in milli-second */

#define APP_TEMPERATURE_OFFSET_CORRECTION               (-3459)/**< Macro for static temperature offset correction. Self heating, temperature correction factor */
#define APP_MQTT_DATA_BUFFER_SIZE                   UINT32_C(256)/**< macro for data size of incoming subscribed and published messages */

/* local variables ********************************************************** */

static WLAN_Setup_T WLANSetupInfo =
        {
                .IsEnterprise = false,
                .IsHostPgmEnabled = false,
                .SSID = WLAN_SSID,
                .Username = WLAN_PSK,
                .Password = WLAN_PSK,
                .IsStatic = WLAN_STATIC_IP,
                .IpAddr = WLAN_IP_ADDR,
                .GwAddr = WLAN_GW_ADDR,
                .DnsAddr = WLAN_DNS_ADDR,
                .Mask = WLAN_MASK,
        };/**< WLAN setup parameters */

static Sensor_Setup_T SensorSetup =
        {
                .CmdProcessorHandle = NULL,
                .Enable =
                        {
                                .Accel = false,
                                .Mag = false,
                                .Gyro = false,
                                .Humidity = true,
                                .Temp = true,
                                .Pressure = true,
                                .Light = true,
                                .Noise = false,
                        },
                .Config =
                        {
                                .Accel =
                                        {
                                                .Type = SENSOR_ACCEL_BMA280,
                                                .IsRawData = false,
                                                .IsInteruptEnabled = false,
                                                .Callback = NULL,
                                        },
                                .Gyro =
                                        {
                                                .Type = SENSOR_GYRO_BMG160,
                                                .IsRawData = false,
                                        },
                                .Mag =
                                        {
                                                .IsRawData = false
                                        },
                                .Light =
                                        {
                                                .IsInteruptEnabled = false,
                                                .Callback = NULL,
                                        },
                                .Temp =
                                        {
                                                .OffsetCorrection = APP_TEMPERATURE_OFFSET_CORRECTION,
                                        },
                        },
        };/**< Sensor setup parameters */

static MQTT_Setup_T MqttSetupInfo =
        {
                .MqttType = MQTT_TYPE_SERVALSTACK,
                .IsSecure = APP_MQTT_SECURE_ENABLE,
        };/**< MQTT setup parameters */

static MQTT_Connect_T MqttConnectInfo =
        {
                .ClientId = APP_MQTT_CLIENT_ID,
                .BrokerURL = APP_MQTT_BROKER_HOST_URL,
                .BrokerPort = APP_MQTT_BROKER_HOST_PORT,
                .CleanSession = true,
                .KeepAliveInterval = 100,
        };/**< MQTT connect parameters */

static MQTT_Publish_T MqttPublishInfo =
        {
                .Topic = APP_MQTT_TOPIC,
                .QoS = 1UL,
                .Payload = NULL,
                .PayloadLength = 0UL,
        };/**< MQTT publish parameters */

static CmdProcessor_T * AppCmdProcessor;/**< Handle to store the main Command processor handle to be used by run-time event driven threads */

static xTaskHandle AppControllerHandle = NULL;/**< OS thread handle for Application controller to be used by run-time blocking threads */

/* global variables ********************************************************* */

/* inline functions ********************************************************* */

/* local functions ********************************************************** */

/**
 * @brief This will validate the WLAN network connectivity
 *
 * If there is no connectivity then it will scan for the given network and try to reconnect
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T AppControllerValidateWLANConnectivity(void)
{
    Retcode_T retcode = RETCODE_OK;
    WlanNetworkConnect_IpStatus_T nwStatus;

    nwStatus = WlanNetworkConnect_GetIpStatus();
    if (WLANNWCT_IPSTATUS_CT_AQRD != nwStatus)
    {
        retcode = WLAN_Reconnect();
        if (RETCODE_OK == retcode)
        {
            retcode = MQTT_ConnectToBroker(&MqttConnectInfo, MQTT_CONNECT_TIMEOUT_IN_MS);
            if (RETCODE_OK != retcode)
            {
                printf("AppControllerFire : MQTT connection to the broker failed \n\r");
            }
        }
    }
    return retcode;
}

/**
 * @brief Responsible for controlling the send data over MQTT application control flow.
 *
 * - Synchronize SNTP time stamp for the system if MQTT communication is secure
 * - Connect to MQTT broker
 * - Subscribe to MQTT topic
 * - Read environmental sensor data
 * - Publish data periodically for a MQTT topic
 *
 * @param[in] pvParameters
 * Unused
 */
static void AppControllerFire(void* pvParameters)
{
    BCDS_UNUSED(pvParameters);

    Retcode_T retcode = RETCODE_OK;
    Sensor_Value_T sensorValue;
    char publishBuffer[APP_MQTT_DATA_BUFFER_SIZE];

    const char *publishDataFormat = "{\"Humidity\" : %ld, \"Pressure\": %ld, \"Temperature\" : %f, \"Light\": %ld}";

    memset(&sensorValue, 0x00, sizeof(sensorValue));
    if (RETCODE_OK == retcode)
    {
        retcode = MQTT_ConnectToBroker(&MqttConnectInfo, MQTT_CONNECT_TIMEOUT_IN_MS);
        if (RETCODE_OK != retcode)
        {
            printf("AppControllerFire : MQTT connection to the broker failed \n\r");
        }
    }

    /* A function that implements a task must not exit or attempt to return to
     its caller function as there is nothing to return to. */
    while (1)
    {
        /* Resetting / clearing the necessary buffers / variables for re-use */
        retcode = RETCODE_OK;

        /* Check whether the WLAN network connection is available */
        retcode = AppControllerValidateWLANConnectivity();
        if (RETCODE_OK == retcode)
        {
            retcode = Sensor_GetData(&sensorValue);
        }
        if (RETCODE_OK == retcode)
        {
            int32_t length = snprintf((char *) publishBuffer, APP_MQTT_DATA_BUFFER_SIZE, publishDataFormat,
                    (long int) sensorValue.RH, (long int) sensorValue.Pressure,
                    (sensorValue.Temp /= 1000), (long int) sensorValue.Light);

            MqttPublishInfo.Payload = publishBuffer;
            MqttPublishInfo.PayloadLength = length;

            retcode = MQTT_PublishToTopic(&MqttPublishInfo, MQTT_PUBLISH_TIMEOUT_IN_MS);
            if (RETCODE_OK != retcode)
            {
                printf("AppControllerFire : MQTT publish failed \n\r");
            }
        }
        vTaskDelay(APP_MQTT_DATA_PUBLISH_PERIODICITY);
    }
}

/**
 * @brief To enable the necessary modules for the application
 * - WLAN
 * - ServalPAL
 * - SNTP (if secure communication)
 * - MQTT
 * - Sensor
 *
 * @param[in] param1
 * Unused
 *
 * @param[in] param2
 * Unused
 */
static void AppControllerEnable(void * param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);

    Retcode_T retcode = WLAN_Enable();
    if (RETCODE_OK == retcode)
    {
        retcode = ServalPAL_Enable();
    }
    if (RETCODE_OK == retcode)
    {
        retcode = MQTT_Enable();
    }
    if (RETCODE_OK == retcode)
    {
        retcode = Sensor_Enable();
    }
    if (RETCODE_OK == retcode)
    {
        if (pdPASS != xTaskCreate(AppControllerFire, (const char * const ) "AppController", TASK_STACK_SIZE_APP_CONTROLLER, NULL, TASK_PRIO_APP_CONTROLLER, &AppControllerHandle))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
        }
    }
    if (RETCODE_OK != retcode)
    {
        printf("AppControllerEnable : Failed \r\n");
        Retcode_RaiseError(retcode);
        assert(0); /* To provide LED indication for the user */
    }
    Utils_PrintResetCause();
}

/**
 * @brief To setup the necessary modules for the application
 * - WLAN
 * - ServalPAL
 * - SNTP (if secure communication)
 * - MQTT
 * - Sensor
 *
 * @param[in] param1
 * Unused
 *
 * @param[in] param2
 * Unused
 */
static void AppControllerSetup(void * param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);

    Retcode_T retcode = WLAN_Setup(&WLANSetupInfo);
    if (RETCODE_OK == retcode)
    {
        retcode = ServalPAL_Setup(AppCmdProcessor);
    }
    if (RETCODE_OK == retcode)
    {
        retcode = MQTT_Setup(&MqttSetupInfo);
    }
    if (RETCODE_OK == retcode)
    {
        SensorSetup.CmdProcessorHandle = AppCmdProcessor;
        retcode = Sensor_Setup(&SensorSetup);
    }
    if (RETCODE_OK == retcode)
    {
        retcode = CmdProcessor_Enqueue(AppCmdProcessor, AppControllerEnable, NULL, UINT32_C(0));
    }
    if (RETCODE_OK != retcode)
    {
        printf("AppControllerSetup : Failed \r\n");
        Retcode_RaiseError(retcode);
        assert(0); /* To provide LED indication for the user */
    }
}

/* global functions ********************************************************* */

/** Refer interface header for description */
void AppController_Init(void * cmdProcessorHandle, uint32_t param2)
{
    BCDS_UNUSED(param2);

    Retcode_T retcode = RETCODE_OK;

    if (cmdProcessorHandle == NULL)
    {
        printf("AppController_Init : Command processor handle is NULL \r\n");
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        AppCmdProcessor = (CmdProcessor_T *) cmdProcessorHandle;
        retcode = CmdProcessor_Enqueue(AppCmdProcessor, AppControllerSetup, NULL, UINT32_C(0));
    }

    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
        assert(0); /* To provide LED indication for the user */
    }
}

/**@} */
/** ************************************************************************* */
