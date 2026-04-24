#include "PowerController.hpp"
#include "Ina219Sensor.hpp"
#include "NexusBLE.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define CONTROL_GPIO GPIO_NUM_2
#define READ_INTERVAL_MS 1000

// Battery Calibration (3S Li-ion)
#define BATT_MAX_V 12.6f
#define BATT_MIN_V 9.0f

static PowerController powerCtrl(CONTROL_GPIO);
static Ina219Sensor ina219(8, 9);
static NexusBLE nexusBle;

/**
 * @brief Calculate battery percentage based on voltage
 */
uint8_t calculateBatteryPct(float voltage) {
    if (voltage >= BATT_MAX_V) return 100;
    if (voltage <= BATT_MIN_V) return 0;
    return (uint8_t)((voltage - BATT_MIN_V) / (BATT_MAX_V - BATT_MIN_V) * 100.0f);
}

/**
 * @brief Callback for BLE toggle command
 */
void onBleToggle(bool on) {
    ESP_LOGI("MAIN", "BLE Command: Turn Power %s", on ? "ON" : "OFF");
    powerCtrl.setPower(on);
}

extern "C" void app_main() {
    // 1. Initialize NVS (required for BLE)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize Hardware
    powerCtrl.init();
    if (ina219.init() != ESP_OK) {
        ESP_LOGE("MAIN", "INA219 Init Failed!");
    }

    // 3. Initialize BLE
    nexusBle.init("Nexus Power");
    nexusBle.setToggleCallback(onBleToggle);

    // --- Nexus Power Splash Screen ---
    printf("\n==========================================\n");
    printf("       NEXUS POWER - SMART SYSTEM         \n");
    printf("==========================================\n");
    printf(" STATUS:   BLE Advertising Active         \n");
    printf(" DEVICE:   Nexus Power                    \n");
    printf(" DASHBOARD: software/dashboard.html       \n");
    printf(" BATTERY:  12.6V System (3S Li-ion)       \n");
    printf("==========================================\n\n");

    while (true) {
        float v = ina219.getBusVoltage_V();
        float i = ina219.getCurrent_mA();
        float p = ina219.getPower_mW();
        uint8_t pct = calculateBatteryPct(v);

        // Notify BLE Dashboard
        nexusBle.updatePowerData(v, i, p, pct);

        // Safety Cutoff
        if (v < BATT_MIN_V && powerCtrl.isOn()) {
            ESP_LOGW("MAIN", "Critical Battery! Shutting down.");
            powerCtrl.setPower(false);
        }

        vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL_MS));
    }
}
