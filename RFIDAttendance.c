#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h> // Menggunakan hd44780 untuk I2C
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h> // Ganti ESPAsyncWebServer dengan WebServer
#include <ElegantOTA.h> // Library ElegantOTA

// Pin untuk RFID-RC522
#define SS_PIN 5
#define RST_PIN 4

// Pin untuk Buzzer
#define BUZZER_PIN 15

// Inisialisasi RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Inisialisasi LCD 16x2 (I2C) menggunakan hd44780
hd44780_I2Cexp lcd(0x27, 16, 2); // Alamat I2C 0x27, 16 kolom, 2 baris

// Kredensial Wi-Fi
const char* ssid = "SMKN69.NET_2.4G";
const char* password = "@smkn69.sukses";

// URL Google Apps Script
const char* googleScriptURL = "https://script.google.com/macros/s/AKfycbz9yng4O8eZY-q9I1adDM-0lVwV3pQqMiKyQCsau2CQFa0JZnSfnn5FFeqnrLLjSpoWoA/exec";

// Variabel global
String uid; // Untuk menyimpan UID kartu
MFRC522::MIFARE_Key key; // Untuk menyimpan key MIFARE

// Inisialisasi Web Server untuk ElegantOTA
WebServer server(80); // Ganti AsyncWebServer dengan WebServer

void setup() {
  // Inisialisasi Serial
  Serial.begin(115200);

  // Inisialisasi SPI dan RFID
  SPI.begin();
  mfrc522.PCD_Init();

  // Inisialisasi LCD
  lcd.begin(16, 2); // Perbaiki pemanggilan begin() dengan parameter
  lcd.backlight();
  lcd.print("Absen Otomatis");
  lcd.setCursor(0, 1);
  lcd.print("Tempelkan Kartu");

  // Inisialisasi Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Inisialisasi key MIFARE
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF; // Key default FFFFFFFFFFFF
  }

  // Koneksi Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  Serial.println("Terhubung ke WiFi");
  Serial.println("IP Address: " + WiFi.localIP().toString());

  // Konfigurasi ElegantOTA
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/plain", "Silakan kunjungi /update untuk mengupload firmware.");
  });

  ElegantOTA.begin(&server); // Mulai ElegantOTA
  server.begin();
  Serial.println("ElegantOTA siap");
}

void loop() {
  // Handle client requests
  server.handleClient();

  // Cek jika ada kartu RFID yang terdeteksi
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      // Baca UID kartu
      uid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        uid += String(mfrc522.uid.uidByte[i], HEX);
      }
      uid.toUpperCase();

      // Tampilkan UID di Serial Monitor
      Serial.print("UID: ");
      Serial.println(uid);

      // Tampilkan UID di LCD
      lcd.clear();
      lcd.print("UID: " + uid);
      lcd.setCursor(0, 1);
      lcd.print("Mengirim data...");

      // Bunyikan buzzer
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);

      // Kirim data ke Google Spreadsheet
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(googleScriptURL) + "?uid=" + uid;
        http.begin(url);
        int httpCode = http.GET();

        if (httpCode > 0) {
          String payload = http.getString();
          Serial.println("Response: " + payload);
          lcd.setCursor(0, 1);
          lcd.print("Data terkirim!");
        } else {
          Serial.println("Gagal mengirim data. Error code: " + String(httpCode));
          lcd.setCursor(0, 1);
          lcd.print("Gagal mengirim!");
        }
        http.end();
      } else {
        Serial.println("WiFi terputus. Mencoba menghubungkan kembali...");
        WiFi.reconnect();
      }

      // Hentikan pembacaan kartu
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();

      // Tunggu 2 detik sebelum membaca kartu lagi
      delay(2000);
      lcd.clear();
      lcd.print("Tempelkan Kartu");
    }
  }
}