#define PIN 13        // пин DI
#define NUM_LEDS 50   // число диодов
#include "Adafruit_NeoPixel.h"

int led;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
void setup() {
  Serial.begin(9600);
  Serial.println("start");
  strip.begin();
  strip.setBrightness(50);    // яркость, от 0 до 255
  strip.clear();                          // очистить
  strip.show();                           // отправить на ленту
  led = 0;
}
void loop() {
  strip.setPixelColor(led, 0xffffff);
  led = (led + 1) % NUM_LEDS;
  Serial.println(led);
  strip.show();
  delay(500);
  strip.clear();
  strip.show();
//  // заливаем трёмя цветами плавно
//  for (int i = 0; i < NUM_LEDS / 3; i++ ) {   // от 0 до первой трети
//    strip.setPixelColor(i, 0xff0000);     // залить красным
//    strip.show();                         // отправить на ленту
//    delay(100);
//  }
//  for (int i = NUM_LEDS / 3; i < NUM_LEDS * 2 / 3; i++ ) {   // от 1/3 до 2/3
//    strip.setPixelColor(i, 0x00ff00);     // залить зелёным
//    strip.show();                         // отправить на ленту
//    delay(100);
//  }
//  for (int i = NUM_LEDS * 2 / 3; i < NUM_LEDS; i++ ) {   // от 2/3 до конца
//    strip.setPixelColor(i, 0x0000ff);     // залить синим
//    strip.show();                         // отправить на ленту
//    delay(100);
//  }
//  delay(1000);
//  // заливаем белым
//  for (int i = 0; i < NUM_LEDS; i++ ) {   // всю ленту
//    strip.setPixelColor(i, 0xffffff);     // залить белым
//    strip.show();                         // отправить на ленту
//    delay(10);
//  }
//  delay(1000);
//  // заливаем чёрным
//  for (int i = 0; i < NUM_LEDS; i++ ) {   // всю ленту
//    strip.setPixelColor(i, 0x000000);     // залить чёрным
//    strip.show();                         // отправить на ленту
//    delay(10);
//  }
//  delay(1000);
//  // включаем случайные диоды жёлтым
//  for (int i = 0; i < 50; i++ ) {         // 50 раз
//    strip.setPixelColor(random(0, NUM_LEDS), 0xffff00);     // залить жёлтым
//    strip.show();                         // отправить на ленту
//    delay(500);
//  }
}
