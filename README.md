This project provides an ha_switch component for ESP-IDF which implements Home Assistant's [MQTT Switches](https://www.home-assistant.io/integrations/switch.mqtt/) and [MQTT Device Triggers](https://www.home-assistant.io/integrations/device_trigger.mqtt/) integrations. Classes for these integrations are abstracted by objects of HaSwitch class.

## Getting started

1. Clone Home Assistant Control project:
        `$ git clone https://github.com/vinRocha/homeassistant-control.git`

2. `idf.py menuconfig` to configure your WiFi and MQTT Credentials.

3. `idf.py -p <SERIAL_DEVICE> build flash monitor`


