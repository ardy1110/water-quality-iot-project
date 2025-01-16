#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>

// Inisialisasi RTC dan Servo
RTC_DS3231 rtc;
Servo myServo;

// Pin untuk servo
int servoPin = 33;

// Pin turbidity sensor
const int turbidityPin = 34; // Input analog sensor turbidity pada GPIO34

// Variabel untuk kalibrasi
const float voltageClear = 2.55; // Tegangan untuk air bersih (0 NTU)
const float voltageDirty = 1.9; // Tegangan untuk air kotor (maksimal NTU)
const float maxNTU = 100.0;     // Nilai NTU maksimum untuk air sangat kotor
const float VREF = 3.3;         // Tegangan referensi ESP32
const int ADC_RESOLUTION = 4095; // Resolusi ADC 12-bit ESP32

// Pin untuk sensor pH
const int pHSensorPin = 32; // GPIO32 sebagai input analog untuk pH

// Variabel kalibrasi pH
const float pH4 = 3.099;  // Tegangan pada pH 4
const float pH7 = 2.600;  // Tegangan pada pH 7
float PH_step;            // Langkah perubahan pH
float Po;                 // Nilai pH
float teganganPh;         // Tegangan dari sensor pH

// Batas aman untuk pH
const float PH_MIN = 6.0;
const float PH_MAX = 7.5;

// Inisialisasi LCD dengan alamat I2C 0x27
LiquidCrystal_I2C lcd(0x27, 20, 4);

// WiFi credentials
const char* ssid = "PLR 1";
const char* password = "minimalbayar";

// Inisialisasi server HTTP
WebServer server(80);

// Alamat IP komputer server
const char* ipAdrr = "192.168.1.12";

// Variabel untuk waktu kontrol servo
unsigned long previousMillis = 0;
long interval = 10000; // Default 10 detik (10.000 milidetik)

void setup() {
  Wire.begin(21, 22);  // SDA pada GPIO 21, SCL pada GPIO 22
  if (!rtc.begin()) {
    Serial.println("RTC tidak terdeteksi!");
    while (1);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  myServo.attach(servoPin);
  myServo.write(0);

  lcd.init();
  lcd.begin(20, 4);
  lcd.backlight();
  lcd.clear();

  pinMode(turbidityPin, INPUT);
  pinMode(pHSensorPin, INPUT);

  Serial.begin(9600);

  // Hitung langkah pH (konstanta) berdasarkan kalibrasi
  PH_step = (pH4 - pH7) / 3.0; // Rentang pH (4-7 = 3 unit)

  // Setup HTTP Routes
  server.on("/setServo", HTTP_GET, handleSetServo);

  // Start the server
  server.begin();
}

void loop() {
  // Handle incoming client requests
  server.handleClient();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    myServo.write(90);
    delay(1000);
    myServo.write(0);
  }

  // Membaca nilai turbidity
  int adcValue = analogRead(turbidityPin);
  float voltage = adcValue * (VREF / ADC_RESOLUTION);
  float turbidity;
  if (voltage > voltageClear) {
    turbidity = 0.0; // Jika tegangan lebih tinggi dari tegangan air bersih, NTU adalah 0
  } else if (voltage < voltageDirty) {
    turbidity = maxNTU; // Jika tegangan lebih rendah dari tegangan air kotor, NTU adalah maksimum
  } else {
    turbidity = (voltageClear - voltage) / (voltageClear - voltageDirty) * maxNTU;
  }

  // Membaca nilai pH
  int nilai_analog_PH = analogRead(pHSensorPin);
  teganganPh = VREF / ADC_RESOLUTION * nilai_analog_PH;
  Po = 7.00 + ((pH7 - teganganPh) / PH_step);
  Po = constrain(Po, 4.0, 9.0);

  // Menampilkan nilai pH dan turbidity di Serial Monitor
  Serial.print("Tegangan pH: ");
  Serial.print(teganganPh, 3);
  Serial.print(" V, pH Air: ");
  Serial.println(Po, 2);
  Serial.print("Tegangan Turbidity: ");
  Serial.print(voltage, 3);
  Serial.print(" V, NTU: ");
  Serial.println(turbidity, 2);

  // Logika notifikasi dan pesan Telegram
  if ((Po < PH_MIN || Po > PH_MAX) && turbidity > 30.0) {
    String message = "Peringatan! Nilai pH: " + String(Po, 2) + " dan Kekeruhan: " + String(turbidity, 2) + " tidak sesuai batas aman.";
    kirimDataTelegram(message);
  } else if (Po < PH_MIN || Po > PH_MAX) {
    String message = "Peringatan! Nilai pH: " + String(Po, 2) + " tidak sesuai batas aman. Mohon lakukan tindakan.";
    kirimDataTelegram(message);
  } else if (turbidity > 30.0) {
    String message = "Peringatan! Nilai Kekeruhan: " + String(turbidity, 2) + " tidak sesuai batas aman. Mohon cek kondisi.";
    kirimDataTelegram(message);
  }

  // Koneksi ke server dan kirim data ke database
  WiFiClient client;
  const int httpPort = 80;

  if (!client.connect(ipAdrr, httpPort)) {
    Serial.println("Koneksi gagal");
    return;
  }

  // URL untuk kirim data ke server
  HTTPClient http;
  String url = String("http://") + ipAdrr + "/water-quality/receive_data.php?turbidity=" + String(turbidity, 2) + "&ph=" + String(Po);
  http.begin(client, url);

  // Eksekusi GET request dan tampilkan respons
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    String respon = http.getString();
    Serial.println(respon);
  } else {
    Serial.println("Error on HTTP request");
  }
  http.end();

  // Menampilkan data pada LCD
  lcd.setCursor(0, 0);
  lcd.print("Turbidity: ");
  lcd.print(turbidity, 2);
  lcd.print("   ");

  lcd.setCursor(0, 1);
  if (turbidity < 30) {
    lcd.print("Status: Bersih       ");
  } else if (turbidity >= 30 && turbidity < 50) {
    lcd.print("Status: Keruh        ");
  } else {
    lcd.print("Status: Sangat Keruh ");
  }

  lcd.setCursor(0, 2);
  lcd.print("pH Water: ");
  lcd.print(Po, 2);
  lcd.print("   ");

  DateTime now = rtc.now();
  lcd.setCursor(0, 3);
  lcd.print("Time: ");
  if (now.hour() < 10) lcd.print('0');
  lcd.print(now.hour());
  lcd.print(':');
  if (now.minute() < 10) lcd.print('0');
  lcd.print(now.minute());
  lcd.print(':');
  if (now.second() < 10) lcd.print('0');
  lcd.print(now.second());

  delay(1000);
}

// Fungsi untuk mengirim pesan Telegram dengan parameter pesan
void kirimDataTelegram(String message) {
  String token = "8117170440:AAGzi6cBqs2eHm-zPIVlhUnvczJ3wGJonVU";  // Ganti dengan token bot Telegram Anda
  String chat_id = "1275203394"; 
  String url = "https://api.telegram.org/bot" + token + "/sendMessage?chat_id=" + chat_id + "&text=" + message;

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.println("Pesan terkirim ke Telegram.");
    } else {
      Serial.print("Error dalam pengiriman pesan: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi tidak tersambung. Gagal mengirim pesan ke Telegram.");
  }
}

void handleSetServo() {
  // Get the servo interval from the query parameter
  String intervalStr = server.arg("interval");
  if (intervalStr.length() > 0) {
    interval = intervalStr.toInt() * 1000; // Convert to milliseconds
    // Menambahkan header CORS
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    Serial.print("Servo interval set to: ");
    Serial.println(interval);
  }

  // Respond to the web request
  server.send(200, "text/plain", "Servo interval updated");
}
