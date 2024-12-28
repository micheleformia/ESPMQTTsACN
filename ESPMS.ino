#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESPAsyncE131.h>

// Wi-Fi Configuration
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// MQTT Configuration
const char* mqtt_server = "Your_MQTT_Broker_IP";
const char* mqtt_username = "Your_MQTT_Username";
const char* mqtt_password = "Your_MQTT_Password";
const char* mqtt_topic = "your/mqtt/topic";
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// sACN Configuration
#define UNIVERSE 1
ESPAsyncE131 e131;

// Pin Definitions
#define RELAY_PIN D4  // Relay pin
#define BUTTON_PIN D1 // Button pin for manual control

// Relay state
bool relayState = LOW;

// MQTT Callback Function
void callback(char* topic, byte* payload, unsigned int length) {
  String message;

  // Construct the received message
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Debug: Print received message
  Serial.print("Message received on MQTT [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  // Change relay state only if needed
  if (message == "ON" && relayState == LOW) {
    relayState = HIGH;
    digitalWrite(RELAY_PIN, relayState);
    Serial.println("Relay TURNED ON (MQTT Command)");
  } else if (message == "OFF" && relayState == HIGH) {
    relayState = LOW;
    digitalWrite(RELAY_PIN, relayState);
    Serial.println("Relay TURNED OFF (MQTT Command)");
  } else {
    Serial.println("Relay state unchanged.");
  }
}

// Setup Function
void setup() {
  Serial.begin(115200);
  Serial.println("Initializing ESP8266...");

  // Configure pins
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Turn off relay at startup
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Button pin with pull-up resistor

  // Wi-Fi Setup
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected");
  Serial.print("Assigned IP Address: ");
  Serial.println(WiFi.localIP());

  // MQTT Setup
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);

  // sACN Setup
  if (e131.begin(E131_UNICAST)) {
    Serial.println("sACN Initialized");
    Serial.print("Listening on Universe: ");
    Serial.println(UNIVERSE);
  }
}

// MQTT Reconnection Function
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      mqttClient.subscribe(mqtt_topic);
      Serial.println("MQTT Connected and Subscribed to Topic");
    } else {
      Serial.print("MQTT Connection Failed, Status: ");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

// Main Loop
void loop() {
  // Handle MQTT
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // Handle sACN
  if (e131.isEmpty() == false) {
    e131_packet_t packet;
    e131.pull(&packet);

    // Check DMX channel value (0-255)
    bool newRelayState = packet.property_values[1] > 127 ? HIGH : LOW;

    // Change relay state only if necessary
    if (newRelayState != relayState) {
      relayState = newRelayState;
      digitalWrite(RELAY_PIN, relayState);
      Serial.print("Relay ");
      Serial.print((relayState == HIGH) ? "TURNED ON" : "TURNED OFF");
      Serial.println(" (sACN Command)");
    }
  }

  // Handle button press
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (currentButtonState == LOW && lastButtonState == HIGH) { // Button pressed
    relayState = !relayState; // Toggle relay state
    digitalWrite(RELAY_PIN, relayState);
    Serial.print("Relay ");
    Serial.println((relayState == HIGH) ? "TURNED ON (Button Press)" : "TURNED OFF (Button Press)");
  }

  lastButtonState = currentButtonState;

  // Debug Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected! Attempting to reconnect...");
    WiFi.begin(ssid, password);
  }
}
