#pragma once

#include <string>
#include <vector>
#include "NimBLEDevice.h"

/**
 * @brief Class to handle BLE communication for the Nexus Power Bank.
 * Uses esp-nimble-cpp for a clean, stable implementation.
 */
class NexusBLE {
public:
    NexusBLE();
    void init(const std::string& deviceName);
    void updatePowerData(float voltage, float current, float power, uint8_t batteryPct, bool isPowerOn, uint8_t statusCode);

    using ToggleCallback = void (*)(bool on);
    void setToggleCallback(ToggleCallback cb);

private:
    std::string m_deviceName;
    ToggleCallback m_toggleCb;

    NimBLEServer* m_pServer;
    NimBLECharacteristic* m_pDataChar;
    NimBLECharacteristic* m_pCtrlChar;

    class ServerCallbacks : public NimBLEServerCallbacks {
    public:
        void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
        void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
    };

    class ControlCallbacks : public NimBLECharacteristicCallbacks {
    public:
        void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
    };
};
