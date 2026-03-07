#include <Servo.h>
#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ─── OLED ────────────────────────────────────────────────────────────────────
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ─── Password ────────────────────────────────────────────────────────────────
#define Password_Length 7
char Data[Password_Length];
char Master[Password_Length] = "6769BA";
byte data_count = 0;
char customKey;

// ─── Keypad ──────────────────────────────────────────────────────────────────
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {39, 41, 43, 45};
byte colPins[COLS]  = {31, 33, 35, 37};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// ─── RGB LED Pins ────────────────────────────────────────────────────────────
int pinR = 5;  // Red Pin
int pinG = 3;  // Green Pin
int pinB = 4;  // Blue Pin (Used for Yellow/Process)
int pinbuzz = 28;
Servo Servo1;

#define OPEN_SECONDS 15 

// ─── RGB Helper Function ─────────────────────────────────────────────────────
// Call this to change colors easily: 1 = ON, 0 = OFF
void setColor(int r, int g, int b) {
  digitalWrite(pinR, r);
  digitalWrite(pinG, g);
  digitalWrite(pinB, b);
}

// ─── OLED helpers (unchanged) ────────────────────────────────────────────────
void drawLock(bool locked) {
  if (locked) {
    display.drawCircle(64, 22, 10, SSD1306_WHITE);
    display.fillRect(54, 22, 21, 4, SSD1306_BLACK);
  } else {
    display.drawCircle(64, 18, 10, SSD1306_WHITE);
    display.fillRect(54, 18, 21, 12, SSD1306_BLACK);
    display.drawFastVLine(54, 18, 12, SSD1306_WHITE);
  }
  display.fillRoundRect(52, 32, 25, 18, 3, SSD1306_WHITE);
  display.fillCircle(64, 38, 3, SSD1306_BLACK);
  display.fillRect(63, 40, 3, 5, SSD1306_BLACK);
}

void oledReady() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  drawLock(true);
  display.setCursor(28, 54);
  display.print(F("Inserire codice"));
  display.display();
}

void oledPassword() {
  display.clearDisplay();
  drawLock(true);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(16, 54);
  for (byte i = 0; i < data_count; i++) display.print('*');
  display.display();
}

void oledStatus(const char* line1, const char* line2, bool unlocked) {
  display.clearDisplay();
  drawLock(!unlocked);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 52);
  display.print(line1);
  display.setCursor(0, 60);
  display.print(line2);
  display.display();
}

void oledCountdown(int secondi) {
  display.clearDisplay();
  drawLock(false); 
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 52);
  display.print(F("Chiusura in: "));
  display.print(secondi);
  display.print(F("s"));
  display.display();
}

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinbuzz, OUTPUT);

  Servo1.attach(9);

  // Initial State: Locked (RED)
  setColor(1, 0, 0); 
  Servo1.write(90);

  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("OLED non trovato!"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.print(F("SISTEMA"));
  display.setCursor(22, 42);
  display.print(F("PRONTO"));
  display.display();
  delay(1500);

  Serial.println(F("Sistema Pronto. Inserire Password:"));
  oledReady();
}

void loop() {
  customKey = customKeypad.getKey();
  if (customKey) {
    if (customKey == '*') {
      clearData();
      return;
    }
    Data[data_count] = customKey;
    data_count++;
    digitalWrite(pinbuzz, HIGH);
    delay(50);
    digitalWrite(pinbuzz, LOW);
    oledPassword();
  }

  if (data_count == Password_Length - 1) {
    Data[data_count] = '\0';
    if (strcmp(Data, Master) == 0) {
      oledStatus("Password OK!", "Accesso consentito", true);
      delay(800);
      unlock();
    } else {
      oledStatus("Password errata!", "Accesso negato", false);
      Errore();
    }
    clearData();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
void unlock() {
  // APERTURA
  setColor(0, 0, 0); // Off
  Servo1.write(180); 
  blinkRGB(5, 1, 2, 0); // Blink Yellow (Red + Green)
  
  setColor(0, 1, 0); // Solid Green
  Serial.println(F("Porta APERTA"));

  // COUNTDOWN
  for (int i = OPEN_SECONDS; i > 0; i--) {
    oledCountdown(i);
    delay(1000);
  }

  // CHIUSURA
  setColor(0, 0, 0);
  blinkRGB(3, 1, 1, 0); // Blink Yellow

  Servo1.write(90); 
  setColor(1, 0, 0); // Solid Red
  Serial.println(F("Porta CHIUSA"));
}

void clearData() {
  while (data_count != 0) Data[data_count--] = 0;
  data_count = 0;
  oledReady();
}

// Flexible blinker: choose the color components to blink
void blinkRGB(int times, int r, int g, int b) {
  for (int i = 0; i < times; i++) {
    setColor(r, g, b); delay(200);
    setColor(0, 0, 0); delay(200);
  }
}

void Errore() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(pinbuzz, HIGH);
    setColor(0, 0, 0);
    delay(150);
    digitalWrite(pinbuzz, LOW);
    setColor(1, 0, 0); // Flash Red
    delay(150);
  }
}
