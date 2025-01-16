// Pin turbidity sensor
const int turbidityPin = 34; // Input analog sensor turbidity pada GPIO34

// Variabel untuk kalibrasi
const float voltageClear = 2.55; // Tegangan untuk air bersih (0 NTU)
const float voltageDirty = 1.9; // Tegangan untuk air kotor (maksimal NTU)
const float maxNTU = 100.0;     // Nilai NTU maksimum untuk air sangat kotor
const float VREF = 3.3;         // Tegangan referensi ESP32
const int ADC_RESOLUTION = 4095; // Resolusi ADC 12-bit ESP32

void setup() {
  Serial.begin(9600);
  pinMode(turbidityPin, INPUT);
}

void loop() {
  // Membaca nilai ADC dari sensor turbidity
  int adcValue = analogRead(turbidityPin);

  // Konversi nilai ADC ke tegangan
  float voltage = adcValue * (VREF / ADC_RESOLUTION);

  // Menghitung nilai NTU berdasarkan kalibrasi
  float ntu;
  if (voltage > voltageClear) {
    ntu = 0.0; // Jika tegangan lebih tinggi dari tegangan air bersih, NTU adalah 0
  } else if (voltage < voltageDirty) {
    ntu = maxNTU; // Jika tegangan lebih rendah dari tegangan air kotor, NTU adalah maksimum
  } else {
    // Linear mapping dari tegangan ke NTU
    ntu = (voltageClear - voltage) / (voltageClear - voltageDirty) * maxNTU;
  }

  // Menampilkan hasil pada Serial Monitor
  Serial.print("Nilai ADC: ");
  Serial.print(adcValue);
  Serial.print("\tTegangan: ");
  Serial.print(voltage, 3);
  Serial.print(" V\tNTU: ");
  Serial.println(ntu, 2); // Tampilkan dengan 2 angka desimal

  delay(1000); // Tunggu 1 detik sebelum pembacaan berikutnya
}
