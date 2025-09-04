This project provides a ha_switch component for ESP-IDF which implements Home Assistant's [MQTT Switches](https://www.home-assistant.io/integrations/switch.mqtt/) and [MQTT Device Triggers](https://www.home-assistant.io/integrations/device_trigger.mqtt/). These objects are abstracted by HaSwitch class.

Objects of HaSwitch allow you to easily integrate your ESP-IDF projects with a Home Assistant instalation through MQTT.

## Getting started

1. Clone Home Assistant Control project:
        `$ git clone https://github.com/vinRocha/homeassistant-control.git`

2. `idf.py menuconfig` to configure your WiFi and MQTT Credentials.

3. `idf.py -p <SERIAL_DEVICE> build flash monitor`


