#include "NexusBLE.hpp"
#include "esp_log.h"
#include "esp_mac.h"
#include "secrets.hpp"

static const char* TAG = "NexusBLE";
static NexusBLE* s_instance = nullptr;

// Lowercase UUIDs to match Web Bluetooth standards
#define SVC_UUID  "0000ff01-0000-1000-8000-00805f9b34fb"
#define DATA_UUID "0000ff02-0000-1000-8000-00805f9b34fb"
#define CTRL_UUID "0000ff03-0000-1000-8000-00805f9b34fb"

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

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    
    // Set Name in advertising data
    NimBLEAdvertisementData advData;
    advData.setName(m_deviceName);
    pAdvertising->setAdvertisementData(advData);
    
    // Add Service UUID to ensure filtering works
    pAdvertising->addServiceUUID(SVC_UUID);
    pAdvertising->start();

    ESP_LOGI(TAG, "BLE initialized as %s", m_deviceName.c_str());
}

void NexusBLE::setToggleCallback(ToggleCallback cb) {
    m_toggleCb = cb;
}

// Security logic uses NEXUS_SECRET_BYTE from secrets.hpp

void NexusBLE::updatePowerData(float voltage, float current, float power, uint8_t batteryPct, bool isPowerOn, uint8_t statusCode) {
    if (m_pServer->getConnectedCount() == 0) return;

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);

    uint8_t packet[19]; // Expanded for state, status, and signature
    memcpy(packet, &voltage, 4);
    memcpy(packet + 4, &current, 4);
    memcpy(packet + 8, &power, 4);
    packet[12] = batteryPct;
    packet[13] = mac[3];
    packet[14] = mac[4];
    packet[15] = mac[5];
    
    // Generate Security Signature: (MAC bytes sum) XOR Secret Key
    packet[16] = (mac[3] + mac[4] + mac[5]) ^ NEXUS_SECRET_BYTE;
    
    packet[17] = isPowerOn ? 1 : 0;
    packet[18] = statusCode;

    m_pDataChar->setValue(packet, 19);
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
        ESP_LOGI("NexusBLE", "Toggle Command Received: %s", on ? "ON" : "OFF");
        if (s_instance->m_toggleCb) s_instance->m_toggleCb(on);
    }
}
