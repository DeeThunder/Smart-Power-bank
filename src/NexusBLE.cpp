#include "NexusBLE.hpp"
#include "esp_log.h"

static const char* TAG = "NexusBLE";
static NexusBLE* s_instance = nullptr;

// UUIDs
#define SVC_UUID  "0000FF01-0000-1000-8000-00805F9B34FB"
#define DATA_UUID "0000FF02-0000-1000-8000-00805F9B34FB"
#define CTRL_UUID "0000FF03-0000-1000-8000-00805F9B34FB"

NexusBLE::NexusBLE() : m_deviceName("Nexus Power"), m_toggleCb(nullptr), m_pServer(nullptr) {
    s_instance = this;
}

void NexusBLE::init(const std::string& deviceName) {
    m_deviceName = deviceName;
    NimBLEDevice::init(m_deviceName);

    m_pServer = NimBLEDevice::createServer();
    m_pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pService = m_pServer->createService(SVC_UUID);

    m_pDataChar = pService->createCharacteristic(DATA_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    m_pCtrlChar = pService->createCharacteristic(CTRL_UUID, NIMBLE_PROPERTY::WRITE);
    m_pCtrlChar->setCallbacks(new ControlCallbacks());

    // pService->start() is deprecated in newer NimBLE, it starts with the server.

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SVC_UUID);
    pAdvertising->start();

    ESP_LOGI(TAG, "BLE initialized as %s", m_deviceName.c_str());
}

void NexusBLE::setToggleCallback(ToggleCallback cb) {
    m_toggleCb = cb;
}

void NexusBLE::updatePowerData(float voltage, float current, float power, uint8_t batteryPct) {
    if (m_pServer->getConnectedCount() == 0) return;

    uint8_t packet[13];
    memcpy(packet, &voltage, 4);
    memcpy(packet + 4, &current, 4);
    memcpy(packet + 8, &power, 4);
    packet[12] = batteryPct;

    m_pDataChar->setValue(packet, 13);
    m_pDataChar->notify();
}

void NexusBLE::ServerCallbacks::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    ESP_LOGI(TAG, "Device connected");
}

void NexusBLE::ServerCallbacks::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    ESP_LOGI(TAG, "Device disconnected. Restarting advertising...");
    NimBLEDevice::startAdvertising();
}

void NexusBLE::ControlCallbacks::onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
        bool on = (value[0] == 1);
        if (s_instance->m_toggleCb) s_instance->m_toggleCb(on);
    }
}
