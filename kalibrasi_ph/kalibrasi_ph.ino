const float VREF = 3.3;  // Tegangan referensi ESP32
int pin = 32;            // Pin ADC yang digunakan (ubah sesuai pin yang Anda pilih)

void setup() {
  Serial.begin(9600);
}

void loop() {
  int adcValue = analogRead(pin);  // Baca nilai ADC dari TO (pH Output)
  float tegangan = adcValue * VREF / 4095.0;  // Konversi ADC ke volt
  Serial.print("Tegangan: ");
  Serial.println(tegangan);
  delay(1000);
}
