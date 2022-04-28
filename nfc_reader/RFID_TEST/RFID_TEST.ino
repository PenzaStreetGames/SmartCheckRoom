#include <SoftwareSerial.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

SoftwareSerial RFID(5,4); // RX and TX
String text;
char c;

const char* ssid = "PickleRick";
const char* password = "14736900147369001473692281337QpyZ0511473692281337QpyZ051";

const char* mqtt_server = "51.250.105.61";
const int mqtt_port = 1884;

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  Serial.begin(9600);
  RFID.begin(9600);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("/server/nfc/0");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

String last = "";

void read_me() {
  while (RFID.available() > 0) {
    delay(5);
    c = RFID.read();
    
    if ((int)c == 2) { // STX
      text = "";
      continue;
    }
    
    if ((int)c == 3) break; // ETX
    
    text += c;
    
    Serial.print((int)c);
    Serial.print(" ");
  }
  Serial.println();
  while (RFID.available() > 0) {
    c = RFID.read();
  }
  /*if (text.length()) { // magic constant
    Serial.println(text);
    client.publish("/server/nfc/0", text.substring(1,13).c_str());
  }*/
  Serial.println(text);
  Serial.println(text.length());
  if (text.length() == 12){// && text != last) {
    last = text;
    client.publish("/server/nfc/0", text.c_str());//text.substring(1,13).c_str());
  }
  
  text = "";
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }

  read_me();
  
  // client.publish("/server/nfc/0", ???);
  
  client.loop();

  delay(3000);
}
