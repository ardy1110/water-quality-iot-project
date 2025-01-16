#include <WiFi.h>
const char* ssid = "PLR 1";       // Replace with your WiFi SSID
const char* password = "minimalbayar"; // Replace with your WiFi Password
void setup() {
  Serial.begin(115200);// Start the Serial communication  at115200 baud
  WiFi.begin(ssid, password);    // Connect to Wi-Fi network
  Serial.print("Connecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // Print the IP address
}

void loop() {
  // Your code here
}