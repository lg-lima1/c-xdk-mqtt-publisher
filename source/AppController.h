/* header definition ******************************************************** */
#ifndef APPCONTROLLER_H_
#define APPCONTROLLER_H_

/* local interface declaration ********************************************** */
#include "XDK_Utils.h"

/* local type and macro definitions */

/* WLAN configurations ****************************************************** */

/**
 * WLAN_SSID is the WIFI network name where user wants connect the XDK device.
 * Make sure to update the WLAN_PSK constant according to your required WIFI network.
 */
#define WLAN_SSID                           "Bat Caverna-2.4GHz"

/**
 * WLAN_PSK is the WIFI router WPA/WPA2 password used at the Wifi network connection.
 * Make sure to update the WLAN_PSK constant according to your router password.
 */
#define WLAN_PSK                            "Warcraft7.0)"

/**
 * WLAN_STATIC_IP is a boolean. If "true" then static IP will be assigned and if "false" then DHCP is used.
 */
#define WLAN_STATIC_IP                      false

/**
 * WLAN_IP_ADDR is the WIFI router WPA/WPA2 static IPv4 IP address (unused if WLAN_STATIC_IP is false)
 * Make sure to update the WLAN_IP_ADDR constant according to your required WIFI network,
 * if WLAN_STATIC_IP is "true".
 */
#define WLAN_IP_ADDR                        XDK_NETWORK_IPV4(0, 0, 0, 0)

/**
 * WLAN_GW_ADDR is the WIFI router WPA/WPA2 static IPv4 gateway address (unused if WLAN_STATIC_IP is false)
 * Make sure to update the WLAN_GW_ADDR constant according to your required WIFI network,
 * if WLAN_STATIC_IP is "true".
 */
#define WLAN_GW_ADDR                        XDK_NETWORK_IPV4(0, 0, 0, 0)

/**
 * WLAN_DNS_ADDR is the WIFI router WPA/WPA2 static IPv4 DNS address (unused if WLAN_STATIC_IP is false)
 * Make sure to update the WLAN_DNS_ADDR constant according to your required WIFI network,
 * if WLAN_STATIC_IP is "true".
 */
#define WLAN_DNS_ADDR                       XDK_NETWORK_IPV4(0, 0, 0, 0)

/**
 * WLAN_MASK is the WIFI router WPA/WPA2 static IPv4 mask address (unused if WLAN_STATIC_IP is false)
 * Make sure to update the WLAN_MASK constant according to your required WIFI network,
 * if WLAN_STATIC_IP is "true".
 */
#define WLAN_MASK                           XDK_NETWORK_IPV4(0, 0, 0, 0)

/* SNTP configurations ****************************************************** */

/**
 * SNTP_SERVER_URL is the SNTP server URL.
 */
#define SNTP_SERVER_URL                     "YourSNTPServerURL"

/**
 * SNTP_SERVER_PORT is the SNTP server port number.
 */
#define SNTP_SERVER_PORT                    UINT16_C(123)

/* MQTT server configurations *********************************************** */

/**
 * APP_MQTT_BROKER_HOST_URL is the MQTT broker host address URL.
 */
#define APP_MQTT_BROKER_HOST_URL            "192.168.15.25"

/**
 * APP_MQTT_BROKER_HOST_PORT is the MQTT broker host port.
 */
#define APP_MQTT_BROKER_HOST_PORT           UINT16_C(1883)

/**
 * APP_MQTT_CLIENT_ID is the device name
 */
#define APP_MQTT_CLIENT_ID                  "XDK-5E-AC"

/**
 * APP_MQTT_TOPIC is the topic to subscribe and publish
 */
#define APP_MQTT_TOPIC                      "XDK"

/**
 * APP_MQTT_SECURE_ENABLE is a macro to enable MQTT with security
 */
#define APP_MQTT_SECURE_ENABLE              0

/**
 * APP_MQTT_DATA_PUBLISH_PERIODICITY is time for MQTT to publish the sensor data
 */
#define APP_MQTT_DATA_PUBLISH_PERIODICITY   UINT32_C(1000)

/* local function prototype declarations */

/* local module global variable declarations */

/* local inline function definitions */

/**
 * @brief Gives control to the Application controller.
 *
 * @param[in] cmdProcessorHandle
 * Handle of the main command processor which shall be used based on the application needs
 *
 * @param[in] param2
 * Unused
 */
void AppController_Init(void * cmdProcessorHandle, uint32_t param2);

#endif /* APPCONTROLLER_H_ */

/** ************************************************************************* */
