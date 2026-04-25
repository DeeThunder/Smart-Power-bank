#include "PowerController.hpp"
#include "Ina219Sensor.hpp"
#include "NexusBLE.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"

#define CONTROL_GPIO GPIO_NUM_10
#define READ_INTERVAL_MS 1000

// Battery Calibration (3S Li-ion)
#define BATT_MAX_V 12.6f
#define BATT_MIN_V 9.0f

static PowerController powerCtrl(CONTROL_GPIO);
static Ina219Sensor ina219(4, 5);
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
    // Immediate log to see if we reach app_main
    printf("\n\n[SYSTEM] BOOT START...\n");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // 1. Initialize NVS (required for BLE)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("MAIN", "Initializing Hardware...");
    powerCtrl.init();

    if (ina219.init() != ESP_OK) {
        ESP_LOGE("MAIN", "INA219 Init Failed!");
    }

    ESP_LOGI("MAIN", "Initializing BLE...");
    nexusBle.init("Nexus Power");
    nexusBle.setToggleCallback(onBleToggle);

    // --- Nexus Power Splash Screen ---
    uint8_t m[6];
    esp_read_mac(m, ESP_MAC_BT);
    uint8_t sig = (m[3] + m[4] + m[5]) ^ 0xAC;
    
    printf("\n==========================================\n");
    printf("       NEXUS POWER - SMART SYSTEM         \n");
    printf("==========================================\n");
    printf(" STATUS:   BLE Advertising Active         \n");
    printf(" DEVICE:   Nexus Power                    \n");
    printf(" SERIAL:   NX-%02X%02X%02X                 \n", m[3], m[4], m[5]);
    printf(" REG_KEY:  %02X%02X%02X%02X               \n", m[3], m[4], m[5], sig);
    printf(" DASHBOARD: https://deethunder.github.io/Smart-Power-bank/ \n");
    printf("==========================================\n\n");
    fflush(stdout);

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
