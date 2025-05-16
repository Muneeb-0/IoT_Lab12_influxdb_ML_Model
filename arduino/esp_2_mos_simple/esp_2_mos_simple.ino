#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 4        // GPIO pin connected to DHT11
#define DHTTYPE DHT11   // DHT 11 (not DHT22)
#define WIFI_SSID "TampleDiago"
#define WIFI_PASSWORD "12345699"
#define MQTT_SERVER "192.168.48.78"  // Replace with your Windows PC's IP address on LAN
#define MQTT_PORT 1883

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 5000;  // Send every 5 seconds

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give it a second to stabilize
  Serial.println("✅ ESP32 Boot Complete!");

  setup_wifi();         // Initialize WiFi
  client.setServer(MQTT_SERVER, MQTT_PORT);  // Set MQTT server
  dht.begin();          // Initialize DHT sensor
}

void setup_wifi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    String tempStr = String(temperature, 2);
    String humStr = String(humidity, 2);

    client.publish("esp32/dht/temp", tempStr.c_str());
    client.publish("esp32/dht/hum", humStr.c_str());

    Serial.print("Published Temperature: ");
    Serial.println(tempStr);
    Serial.print("Published Humidity: ");
    Serial.println(humStr);
    Serial.println("waiting next value");
  }
}