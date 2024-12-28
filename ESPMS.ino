#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESPAsyncE131.h>

// Wi-Fi Configuration
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// MQTT Configuration
const char* mqtt_server = "Your_MQTT_Broker_IP";
const char* mqtt_username = "Your_MQTT_Username"; // MQTT Username
const char* mqtt_password = "Your_MQTT_Password"; // MQTT Password
const char* mqtt_topic = "your/mqtt/topic";       // MQTT topic to turn the relay ON/OFF
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// sACN Configuration
#define UNIVERSE 1         // DMX Universe to listen to
ESPAsyncE131 e131;

// Relay Pin
int relayPin = D4; // Update this with the pin connected to your relay

// Current relay state
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

  // Change the relay state only if needed
  if (message == "ON" && relayState == LOW) {
    relayState = HIGH;
    digitalWrite(relayPin, relayState);
    Serial.println("Relay TURNED ON (MQTT Command)");
  } else if (message == "OFF" && relayState == HIGH) {
    relayState = LOW;
    digitalWrite(relayPin, relayState);
    Serial.println("Relay TURNED OFF (MQTT Command)");
  } else {
    Serial.println("Relay state unchanged.");
  }
}

// Initial Setup
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Initializing ESP8266...");

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Turn off the relay at startup

  // Wi-Fi Setup
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Wi-Fi Connected");
  Serial.print("Assigned IP Address: ");
  Serial.println(WiFi.localIP());

  // MQTT Setup
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);

  // sACN Setup
  if (e131.begin(E131_UNICAST)) { // You can also use E131_MULTICAST if needed
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
    e131.pull(&packet); // Fetch sACN packet

    // Check the first DMX channel value (0-255)
    bool newRelayState = packet.property_values[1] > 127 ? HIGH : LOW;

    // Change the relay state only if necessary
    if (newRelayState != relayState) {
      relayState = newRelayState;
      digitalWrite(relayPin, relayState); // Update the relay state
      Serial.print("Relay ");
      Serial.print((relayState == HIGH) ? "TURNED ON" : "TURNED OFF");
      Serial.println(" (sACN Command)");
    }
  }

  // General Debugging
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected! Attempting to reconnect...");
    WiFi.begin(ssid, password);
  }
}
