const int ph_pin = 32;
float PH_step;
float Po;
float teganganPh;

// Kalibrasi tegangan pada pH4 dan pH7
const float pH4 = 3.099;  // Tegangan pada pH 4
const float pH7 = 2.600;  // Tegangan pada pH 7
const float VREF = 3.3; // Tegangan referensi ESP32
const int ADC_RESOLUTION = 4095.0; // Resolusi ADC 12-bit

void setup() {
  pinMode(ph_pin, INPUT);
  Serial.begin(9600);

  // Hitung langkah pH (konstanta) berdasarkan kalibrasi
  PH_step = (pH4 - pH7) / 3.0; // Rentang pH (4-7 = 3 unit)
}

void loop() {
  // Baca nilai ADC dari sensor pH
  int nilai_analog_PH = analogRead(ph_pin);

  // Konversi nilai ADC menjadi tegangan
  teganganPh = VREF / ADC_RESOLUTION * nilai_analog_PH;

  // Hitung nilai pH berdasarkan tegangan yang dibaca
  Po = 7.00 + ((pH7 - teganganPh) / PH_step);

  // Tampilkan hasil pada Serial Monitor
  Serial.print("Nilai ADC pH: ");
  Serial.print(nilai_analog_PH);
  Serial.print("\tTegangan pH: ");
  Serial.print(teganganPh, 3); // Tampilkan 3 angka desimal
  Serial.print(" V\tNilai pH Cairan: ");
  Serial.println(Po, 2); // Tampilkan 2 angka desimal

  delay(1000); // Tunggu 1 detik sebelum pembacaan berikutnya
}
