# XDK MQTT Publisher
A simple MQTT Publisher using the XDK110 from Bosch. It acquires data from the built-in environment sensors.

## Features
- Publishing period of 1 second
- Temperature, pressure, humidity and light intensity

## Flashing
Just drag&drop the `.bin` file into the XDK Device when using the XDK Workbench and it should work

## Preparation
- Modify the `AppController.h` WLAN and MQTT related macros to your needs
- You will need an running MQTT broker. Could either run local in your machine, or be any from the internet
- Also, you will need an instance of the InfluxDB (if you want to use NodeRED and Grafana pre-configured files)

## Operation
1. Just turn on the XDK and it will automatically connect to the configured WLAN. Then it will search for the configured MQTT broker, and when this is all ok, it will publish the data in the configured MQTT Topic.
2. Just import the configurations on either NodeRED and Grafana
3. Happy hacking
