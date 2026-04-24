# The Component Manager will automatically find esp-nimble-cpp
idf_component_register(
    SRCS "main.cpp" "Ina219Sensor.cpp" "NexusBLE.cpp" "PowerController.cpp"
    INCLUDE_DIRS "." "../include"
    REQUIRES nvs_flash bt driver esp_adc
)
