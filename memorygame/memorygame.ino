#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C  // Cambia in 0x3D se necessario

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define MAX_ROUNDS 13
#define ENTRY_TIME_LIMIT 5000

const byte ledPins[4] = {10, 3, 13, 5};     // Rosso, Verde, Blu, Giallo
const byte buttonPins[4] = {9, 2, 12, 6};   // Rosso, Verde, Blu, Giallo

#define BUZZER1 4
#define BUZZER2 7

byte sequence[MAX_ROUNDS];
byte currentRound = 0;

void setup() {
  Serial.begin(9600);

  for (byte i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  pinMode(BUZZER1, OUTPUT);
  pinMode(BUZZER2, OUTPUT);

  // Inizializza OLED con messaggio di debug
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("Display non trovato");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Avvio...");
  display.display();
  delay(1000);

  displayMessage("Memory game", "");
  delay(2000);
}

void loop() {
  attractMode();

  displayMessage("Inizia il Gioco", "");
  delay(1000);
  currentRound = 1;

  while (currentRound <= MAX_ROUNDS) {
    addMove();
    playSequence();

    for (byte i = 0; i < currentRound; i++) {
      int userInput = waitForButton();
      if (userInput < 0 || userInput != sequence[i]) {
        displayMessage("Hai PERSO G", "GET  OUT!");
        playLoser();
        delay(3000);
        return;
      }
    }

    currentRound++;
    delay(1000);
  }

  displayMessage("Hai vinto!", "Congratulazioni!");
  playWinner();
  delay(3000);
}

// Aggiunge una nuova mossa casuale
void addMove() {
  sequence[currentRound - 1] = random(0, 4);
}

// Riproduce la sequenza attuale
void playSequence() {
  for (byte i = 0; i < currentRound; i++) {
    toneFeedback(sequence[i]);
    delay(250);
  }
}

// Accende LED + suono
void toneFeedback(byte index) {
  digitalWrite(ledPins[index], HIGH);
  buzz(index);
  delay(150);
  digitalWrite(ledPins[index], LOW);
}

// Suono associato al colore
void buzz(byte index) {
  int freq[] = {1136, 568, 851, 638}; // Frequenze per Rosso, Verde, Blu, Giallo
  long duration = 150000L;
  while (duration > freq[index] * 2) {
    duration -= freq[index] * 2;
    digitalWrite(BUZZER1, LOW);  digitalWrite(BUZZER2, HIGH);
    delayMicroseconds(freq[index]);
    digitalWrite(BUZZER1, HIGH); digitalWrite(BUZZER2, LOW);
    delayMicroseconds(freq[index]);
  }
}

// Aspetta il bottone premuto entro tempo limite
int waitForButton() {
  long start = millis();
  while (millis() - start < ENTRY_TIME_LIMIT) {
    showCountdown(ENTRY_TIME_LIMIT - (millis() - start));
    for (byte i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        toneFeedback(i);
        while (digitalRead(buttonPins[i]) == LOW); // Attesa rilascio
        delay(50);
        return i;
      }
    }
  }
  return -1; // Timeout
}

// Mostra tempo rimanente sul display
void showCountdown(unsigned long msLeft) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Repeat:");
  display.setCursor(0, 20);
  display.setTextSize(2);
  display.print((msLeft + 999) / 1000);
  display.setTextSize(1);
  display.display();
}

// Mostra messaggi sullo schermo
void displayMessage(String line1, String line2) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(line1);
  display.println(line2);
  display.display();
}

// Animazione vittoria
void playWinner() {
  for (byte i = 0; i < 3; i++) {
    digitalWrite(ledPins[1], HIGH);
    digitalWrite(ledPins[2], HIGH);
    buzz(1); buzz(2);
    digitalWrite(ledPins[1], LOW);
    digitalWrite(ledPins[2], LOW);
  }
}

// Animazione sconfitta
void playLoser() {
  for (byte i = 0; i < 2; i++) {
    digitalWrite(ledPins[0], HIGH);
    digitalWrite(ledPins[3], HIGH);
    buzz(0); buzz(3);
    digitalWrite(ledPins[0], LOW);
    digitalWrite(ledPins[3], LOW);
  }
}

// Modalità standby: lampeggia LED finché non si preme un bottone
void attractMode() {
  while (true) {
    for (byte i = 0; i < 4; i++) {
      digitalWrite(ledPins[i], HIGH);
      delay(100);
      if (digitalRead(buttonPins[i]) == LOW) {
        digitalWrite(ledPins[i], LOW);
        delay(300);
        return;
      }
      digitalWrite(ledPins[i], LOW);
    }
  }
}