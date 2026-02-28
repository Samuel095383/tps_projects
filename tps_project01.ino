#include <Servo.h>
#include <Keypad.h>

// Definizione lunghezza: 7 caratteri + 1 per il terminatore null
#define Password_Length 7 

char Data[Password_Length]; 
char Master[Password_Length] = "030409"; 
byte data_count = 0;
char customKey;

const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

int rosso = 10, giallo = 11, verde = 12, pinbuzz = 13;
Servo Servo1;

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
  pinMode(rosso, OUTPUT);
  pinMode(giallo, OUTPUT);
  pinMode(verde, OUTPUT);
  pinMode(pinbuzz, OUTPUT);
  
  Servo1.attach(A0); 
  
  // Stato iniziale
  digitalWrite(verde, LOW);
  digitalWrite(giallo, LOW);
  digitalWrite(rosso, HIGH);
  Servo1.write(0);
  
  Serial.begin(9600);
  Serial.println("Sistema Pronto. Inserire Password:");
}

void loop() {
  customKey = customKeypad.getKey();

  if (customKey) {
    Data[data_count] = customKey; 
    Serial.print("*"); // Feedback visivo sul seriale
    data_count++; 
    
    // Suono breve alla pressione del tasto
    digitalWrite(pinbuzz, HIGH);
    delay(50);
    digitalWrite(pinbuzz, LOW);
  }

  // Quando il numero di caratteri inseriti è pari alla password
  if (data_count == Password_Length - 1) {
    Data[data_count] = '\0'; // Chiude la stringa correttamente

    Serial.println(); // Va a capo nel monitor seriale
    
    if (strcmp(Data, Master) == 0) {
      Serial.println("Password Corretta!");
      Apertura();
      delay(3000); // Tempo di attesa porta aperta
      Chiusura();
    } else {
      Serial.println("Password Errata!");
      Errore();
    }

    // RESET TOTALE dopo ogni tentativo
    clearData();
  }
}

void clearData() {
  while (data_count != 0) {
    Data[data_count--] = 0; 
  }
  data_count = 0;
  Serial.println("Inserire Password:");
}

void Lampeggio() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(giallo, HIGH);
    delay(200);
    digitalWrite(giallo, LOW);
    delay(200);
  }
}

void Apertura() {
  digitalWrite(rosso, LOW);
  Lampeggio();
  Servo1.write(180);
  digitalWrite(verde, HIGH);
}

void Chiusura() {
  digitalWrite(verde, LOW);
  Lampeggio();
  Servo1.write(0);
  digitalWrite(rosso, HIGH);
}

void Errore() {
  for(int i=0; i<3; i++){
    digitalWrite(pinbuzz, HIGH);
    digitalWrite(rosso, LOW);
    delay(150);
    digitalWrite(pinbuzz, LOW);
    digitalWrite(rosso, HIGH);
    delay(150);
  }
}