#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// Wi-Fi Credentials
const char* ssid = "PLR 1";
const char* password = "minimalbayar";

// Web Server
WebServer server(80);

// Servo
Servo servoMotor;
int interval = 10; // Default interval 4 jam
unsigned long previousMillis = 0;
const long hourMillis = 1000; // 1 jam dalam milidetik

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Kontrol Pakan Otomatis</title>
    </head>
    <body>
      <h1>Pengaturan Pakan Otomatis</h1>
      <form action="/set-interval" method="GET">
        <label for="interval">Pilih Interval:</label>
        <select id="interval" name="interval">
          <option value="10">10 detik</option>
          <option value="20">20 detik</option>
          <option value="30">30 detik</option>
        </select>
        <button type="submit">Simpan</button>
      </form>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void handleSetInterval() {
  if (server.hasArg("interval")) {
    interval = server.arg("interval").toInt();
    if (interval == 10 || interval == 20 || interval == 30) {
      server.send(200, "text/plain", "Interval updated to " + String(interval) + " hours.");
    } else {
      server.send(400, "text/plain", "Invalid interval!");
    }
  } else {
    server.send(400, "text/plain", "Interval not set!");
  }
}

void feedFish() {
  servoMotor.write(90);  // Buka pakan
  delay(1000);           // Tunggu 1 detik
  servoMotor.write(0);   // Kembali ke posisi awal
}

void setup() {
  Serial.begin(9600);
  servoMotor.attach(33); // Servo di pin GPIO 13
  servoMotor.write(0);   // Posisi awal servo

  // Koneksi Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Routing server
  server.on("/", handleRoot);
  server.on("/set-interval", handleSetInterval);
  server.begin();
}

void loop() {
  server.handleClient();

  // Kontrol servo berdasarkan interval
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval * hourMillis) {
    previousMillis = currentMillis;
    feedFish();
  }
}
