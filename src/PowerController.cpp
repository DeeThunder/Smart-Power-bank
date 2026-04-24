#include "PowerController.hpp"
#include "esp_log.h"

static const char* TAG = "PowerController";

PowerController::PowerController(gpio_num_t gpio_num) 
    : m_gpio_num(gpio_num), m_is_on(false) {}

void PowerController::init() {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << m_gpio_num);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // Pull up to ensure it stays OFF during boot
    
    esp_err_t err = gpio_config(&io_conf);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "GPIO %d initialized (Active Low)", m_gpio_num);
    }
    
    // Ensure it starts OFF (High for Active Low)
    setPower(false);
}

void PowerController::setPower(bool on) {
    m_is_on = on;
    // Inverted Logic: ON = 0, OFF = 1
    gpio_set_level(m_gpio_num, on ? 0 : 1);
    ESP_LOGI(TAG, "Power set to %s (GPIO %d set to %d)", on ? "ON" : "OFF", m_gpio_num, on ? 0 : 1);
}

bool PowerController::isOn() const {
    return m_is_on;
}

void PowerController::toggle() {
    setPower(!isOn());
}
