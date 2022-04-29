// http://developer.alexanderklimov.ru/arduino/rfid_rc522.phphttp://developer.alexanderklimov.ru/arduino/rfid_rc522.php
// https://esp32io.com/tutorials/esp32-rfid-nfchttps://esp32io.com/tutorials/esp32-rfid-nfc
// 5v vs 3.3v ???

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  5  // ESP32 pin GIOP5 
#define RST_PIN 27 // ESP32 pin GIOP27 

// Создание экземпляра объекта MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Создание экземпляра MFRC522

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // инициализация MFRC522
  mfrc522.PCD_Init();
  // выводим номер версии прошивки ридера
  mfrc522.PCD_DumpVersionToSerial();
}

void loop() {
  // Ожидание
  if ( !mfrc522.PICC_IsNewCardPresent())
    return;

  // чтение
  if ( !mfrc522.PICC_ReadCardSerial())
    return;

  // вывод данных
  Serial.print("UID = ");
  view_data(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print("type = ");
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.print(mfrc522.PICC_GetTypeName(piccType));
  Serial.println();
  delay(1000);
}

// преобразование в HEX
void view_data (byte *buf, byte size) {
  for (byte j = 0; j < size; j++) {
    Serial.print(buf [j]);
    Serial.print(buf [j], HEX);
  }
}
