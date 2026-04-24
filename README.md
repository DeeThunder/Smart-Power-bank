# 🔋 Nexus Power | Smart Power Bank System

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Framework](https://img.shields.io/badge/Framework-ESP--IDF-orange.svg)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/index.html)

**Nexus Power** is a professional-grade, bare-metal firmware for the **ESP32-C3** that transforms a standard battery pack into a smart, connected power station. Designed for precision, security, and efficiency.

---

## 🚀 Core Features

- **Real-Time Telemetry**: Precision monitoring of Voltage, Current, and Power via INA219.
- **Smart Control**: Low-latency power toggling via BLE (Bluetooth Low Energy).
- **Persistent Memory**: Intelligent state recovery—remembers your ON/OFF status even after power loss (© 2026).
- **Hardware Authenticity**: Cryptographic signature system to detect and block counterfeit hardware.
- **Cloud-Free Dashboard**: A high-performance Web Bluetooth dashboard (HTTPS) for mobile and desktop.
- **Device Registry**: Integrated serial number verification for brand protection.

## 🛠 Hardware Interface (ESP32-C3)

| Component | Pin | Description |
|-----------|-----|-------------|
| **I2C SDA** | `GPIO 4` | Data line for INA219 Sensor |
| **I2C SCL** | `GPIO 5` | Clock line for INA219 Sensor |
| **Power Toggle**| `GPIO 10` | Output for Relay/MOSFET (Active Low) |
| **Status LED** | Internal | BLE / System Status Indicators |

## 💻 Software & Dashboard

Access your power bank settings directly from your browser (Chrome/Edge/Bluefy):
👉 **[Nexus Dashboard](https://deethunder.github.io/Smart-Power-bank/)**

## 🔐 Brand Protection & Registry

To maintain a secure ecosystem, Nexus Power uses a Hardware Signature system.
1. **Flash Firmware**: Note the unique Serial ID in the Serial Monitor.
2. **Register**: Add the Serial to `docs/devices.json`.
3. **Verify**: Connect via the Dashboard to see the **✅ VERIFIED GENUINE** badge.

## 📦 Getting Started

### Prerequisites
- [PlatformIO IDE](https://platformio.org/)
- ESP32-C3 DevKit (or custom Nexus hardware)

### Build & Flash
```bash
# Build and Upload
pio run -t upload

# Monitor Logs
pio run -t monitor
```

## 📜 License
Licensed under the **Apache License 2.0**. You are free to use, modify, and distribute this software for commercial purposes. See the [LICENSE](LICENSE) file for more details.

---
**Disclaimer**: This project involves high-power electrical components. Always use caution and proper insulation when building hardware.
