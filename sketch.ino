#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 4
#define DHTTYPE DHT22

#define SOILPIN 34

#define RELAY 18
#define LEDMERAH 5
#define LEDHIJAU 19

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// API Key ThingSpeak
String apiKey = "F071T3RXPF1A0FD8";

// Server ThingSpeak
const char* server = "https://api.thingspeak.com/update";

void setup() {

  Serial.begin(115200);

  pinMode(RELAY, OUTPUT);
  pinMode(LEDMERAH, OUTPUT);
  pinMode(LEDHIJAU, OUTPUT);

  digitalWrite(RELAY, LOW);
  digitalWrite(LEDMERAH, LOW);
  digitalWrite(LEDHIJAU, LOW);

  dht.begin();

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("Monitoring");
  lcd.setCursor(0,1);
  lcd.print("Tanaman");

  delay(2000);
  lcd.clear();

  Serial.print("Menghubungkan WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
}

void loop() {

  float suhu = dht.readTemperature();
  float hum = dht.readHumidity();

  int soil = analogRead(SOILPIN);

  // Konversi menjadi persen
  int persen = map(soil, 0, 4095, 100, 0);

  Serial.println("--------------------------");
  Serial.print("Suhu          : ");
  Serial.print(suhu);
  Serial.println(" C");

  Serial.print("Kelembapan    : ");
  Serial.print(hum);
  Serial.println(" %");

  Serial.print("Soil Moisture : ");
  Serial.print(persen);
  Serial.println(" %");

  // LCD
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(suhu,1);
  lcd.print("C");

  lcd.setCursor(9,0);
  lcd.print("H:");
  lcd.print(hum,0);

  lcd.setCursor(0,1);
  lcd.print("Soil:");
  lcd.print(persen);
  lcd.print("%");

  // Kontrol Relay dan LED
  if (persen < 40) {

    digitalWrite(RELAY, HIGH);
    digitalWrite(LEDMERAH, HIGH);
    digitalWrite(LEDHIJAU, LOW);

    Serial.println("Pompa : ON");

  } else {

    digitalWrite(RELAY, LOW);
    digitalWrite(LEDMERAH, LOW);
    digitalWrite(LEDHIJAU, HIGH);

    Serial.println("Pompa : OFF");
  }

  // Kirim data ke ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String url = String(server);
    url += "?api_key=" + apiKey;
    url += "&field1=" + String(suhu);
    url += "&field2=" + String(hum);
    url += "&field3=" + String(persen);

    Serial.println(url);

    http.begin(url);

    int httpCode = http.GET();

    Serial.print("HTTP Response Code : ");
    Serial.println(httpCode);

    http.end();
  }

  // ThingSpeak Free minimal 15 detik
  delay(20000);
}