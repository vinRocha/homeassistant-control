idf_component_register(SRCS "ha_switch.cpp" "ha_virtual_switch.cpp" "mqtt_device_trigger.cpp" "mqtt_switch.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES mqtt_manager)
