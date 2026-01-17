# ESPRelayBridge

**ESPRelayBridge** is a project for controlling a relay connected to an ESP8266 using both MQTT and sACN (Streaming ACN). This dual-protocol setup allows seamless integration into home automation systems like Home Assistant and DMX lighting control applications such as Vixen Light.

---

## Features

- **MQTT Control:** Send `ON` and `OFF` commands via MQTT to toggle the relay.
- **sACN/DMX Support:** Use DMX channel values via sACN to control the relay (channel 1 by default).
- **Dual Protocols:** MQTT and sACN coexist, with sACN taking priority when active.
- **Serial Debugging:** Clear debug messages for easier troubleshooting.

---

## Hardware Requirements

- **ESP8266-based board:** e.g., NodeMCU, Shelly 1.
- **Relay Module:** Compatible with ESP8266 GPIO pins.
- **Optional Button:** To manually toggle the relay.

---

## Libraries Used

Ensure you have the following libraries installed in your Arduino IDE:
1. [`PubSubClient`](https://github.com/knolleary/pubsubclient) - For MQTT communication.
2. [`ESPAsyncE131`](https://github.com/forkineye/ESPAsyncE131) - For sACN/DMX support.

---

## Pin Configuration

| Component      | GPIO Pin | Description             |
|-----------------|----------|-------------------------|
| Relay           | GPIO2 (D4) | Controls the relay module |
| Optional Button | GPIO5 (D5) | Toggles the relay manually |

---

## Getting Started

### 1. Update Wi-Fi and MQTT Configuration

Modify the following lines in the code to match your setup:

*// Wi-Fi Configuration
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

*// MQTT Configuration
const char* mqtt_server = "Your_MQTT_Broker_IP";
const char* mqtt_username = "Your_MQTT_Username";
const char* mqtt_password = "Your_MQTT_Password";
const char* mqtt_topic = "your/mqtt/topic";

### 2.Configure DMX Universe for sACN
// Configurazione sACN
#define UNIVERSE 1 // DMX Universe to listen on

________________________________________________________________________________________________
Home Assistant Integration

Add the following configuration to your configuration.yaml file to control the relay via MQTT:

mqtt:
  light:
    - name: "ESP Relay"
      command_topic: "your/mqtt/topic"
      payload_on: "ON"
      payload_off: "OFF"
      qos: 0
      retain: false
