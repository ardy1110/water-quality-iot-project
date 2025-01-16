const int turbidityPin = 34;  // Pin analog untuk sensor turbidity
const float VREF = 3.3;       // Tegangan referensi ESP32
const int ADC_RESOLUTION = 4095; // Resolusi ADC 12-bit

void setup() {
  Serial.begin(9600);
  pinMode(turbidityPin, INPUT);
}

void loop() {
  int adcValue = analogRead(turbidityPin); // Baca nilai ADC
  float voltage = adcValue * VREF / ADC_RESOLUTION; // Konversi ke tegangan
  Serial.print("ADC Value: ");
  Serial.print(adcValue);
  Serial.print("\tVoltage: ");
  Serial.print(voltage, 3); // Tampilkan 3 angka desimal
  Serial.println(" V");
  delay(1000); // Baca setiap 1 detik
}
