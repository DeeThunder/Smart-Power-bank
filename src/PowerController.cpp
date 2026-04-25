#include "PowerController.hpp"
#include "esp_log.h"

static const char* TAG = "PowerController";

PowerController::PowerController(gpio_num_t gpio_num) 
    : m_gpio_num(gpio_num), m_is_on(false) {}

#include "nvs_flash.h"
#include "nvs.h"

void PowerController::init() {
    // Load last state from NVS BEFORE configuring GPIO
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READWRITE, &handle) == ESP_OK) {
        uint8_t state = 0;
        if (nvs_get_u8(handle, "pwr_state", &state) == ESP_OK) {
            m_is_on = (state == 1);
            ESP_LOGI(TAG, "Restored last state from memory: %s", m_is_on ? "ON" : "OFF");
        }
        nvs_close(handle);
    }

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << m_gpio_num);
    // Since it's high level logic, pull down to keep it OFF by default if not driven
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    
    // Apply the restored or default state immediately
    setPower(m_is_on);
}

void PowerController::setPower(bool on) {
    m_is_on = on;
    // High level logic: 1 = ON, 0 = OFF
    gpio_set_level(m_gpio_num, on ? 1 : 0);
    
    // Save to NVS
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READWRITE, &handle) == ESP_OK) {
        nvs_set_u8(handle, "pwr_state", on ? 1 : 0);
        nvs_commit(handle);
        nvs_close(handle);
    }
    
    ESP_LOGI(TAG, "Power set to %s (GPIO %d)", on ? "ON" : "OFF", m_gpio_num);
}

bool PowerController::isOn() const {
    return m_is_on;
}

void PowerController::toggle() {
    setPower(!isOn());
}
