#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define MAX_ROUNDS 10
#define ENTRY_TIME_LIMIT 4000
#define RED 0
#define GREEN 1
#define BLUE 2
#define YELLOW 3
#define BUZZER_PIN 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const byte ledPins[] = {10, 3, 13, 5};
const byte buttonPins[] = {9, 2, 12, 6};
const int tones[] = {294, 370, 440, 523};
byte sequence[MAX_ROUNDS];
byte currentRound = 0;
int score = 0;

void setup() {
  Serial.begin(9600);
  
  for (byte i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(BUZZER_PIN, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("Display non trovato.");
    while (1);
  }

  display.clearDisplay();
  displayCenteredText("Ciao!", 20, 2);
  display.display();
  delay(1500);
}

void loop() {
  attractMode();

  display.clearDisplay();
  displayCenteredText("Inizia!", 20, 2);
  display.display();
  delay(1000);

  currentRound = 1;
  score = 0;

  while (currentRound <= MAX_ROUNDS) {
    addMove();
    playSequence();

    for (byte i = 0; i < currentRound; i++) {
      int playerInput = waitForButton();
      
      if (playerInput != sequence[i]) {
        playLoser();
        delay(2000);
        return;
      }
    }

    score++;
    display.clearDisplay();
    displayCenteredText("Corretto!", 20, 2);
    displayCenteredText("+1 Punto", 40, 1);
    display.display();
    currentRound++;
    delay(1200);
  }

  playWinner();
  delay(2000);
}

void displayCenteredText(String text, int y, int size = 1) {
  display.setTextSize(size);
  display.setTextColor(WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, y);
  display.println(text);
}

void displayGameInfo() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Round: ");
  display.print(currentRound);
  display.setCursor(0, 8);
  display.print("Punti: ");
  display.print(score);
}

void showCountdown(unsigned long msLeft) {
  display.clearDisplay();
  displayGameInfo();
  displayCenteredText(String((msLeft + 999) / 1000), 32, 2);
  display.display();
}

bool isButtonPressed(byte pin) {
  if (digitalRead(pin) == LOW) {
    delay(50);
    return digitalRead(pin) == LOW;
  }
  return false;
}

void visualFeedback(byte index, bool on) {
  digitalWrite(ledPins[index], on);
}

void toneFeedback(byte index) {
  visualFeedback(index, HIGH);
  tone(BUZZER_PIN, tones[index], 100);
  delay(100);
  noTone(BUZZER_PIN);
  delay(200);
  visualFeedback(index, LOW);
  delay(100);
}

int waitForButton() {
  long start = millis();
  while (millis() - start < ENTRY_TIME_LIMIT) {
    showCountdown(ENTRY_TIME_LIMIT - (millis() - start));
    for (byte i = 0; i < 4; i++) {
      if (isButtonPressed(buttonPins[i])) {
        toneFeedback(i);
        return i;
      }
    }
  }
  return -1;
}

void addMove() {
  sequence[currentRound - 1] = random(0, 4);
}

void playSequence() {
  display.clearDisplay();
  displayGameInfo();
  displayCenteredText("Memorizza!", 20);
  display.display();
  delay(750);

  for (byte i = 0; i < currentRound; i++) {
    toneFeedback(sequence[i]);
    delay(400);
  }

  display.clearDisplay();
  displayGameInfo();
  displayCenteredText("Tocca!", 36);
  display.display();
}

void playWinner() {
  display.clearDisplay();
  displayCenteredText("Bravo!", 16, 2);
  displayCenteredText("Punteggio: " + String(score), 40, 1);
  display.display();
  
  for (int i = 0; i < 4; i++) {
    visualFeedback(i, HIGH);
    toneFeedback(i);
    visualFeedback(i, LOW);
    delay(150);
  }
  delay(2000);
}

void playLoser() {
  display.clearDisplay();
  displayCenteredText("Sbagliato!", 16, 2);
  displayCenteredText("Punteggio: " + String(score), 40, 1);
  display.display();
  
  for (int i = 0; i < 2; i++) {
    visualFeedback(RED, HIGH);
    visualFeedback(YELLOW, HIGH);
    toneFeedback(RED);
    toneFeedback(YELLOW);
    visualFeedback(RED, LOW);
    visualFeedback(YELLOW, LOW);
    delay(250);
  }
  delay(2000);
}

void attractMode() {
  display.clearDisplay();
  displayCenteredText("MEMORY", 10, 2);
  displayCenteredText("GAME", 35, 2);
  displayCenteredText("Premi il tasto verde", 55, 1);
  display.display();

  score = 0;

  while (true) {
    for (byte i = 0; i < 4; i++) {
      visualFeedback(i, HIGH);  
      delay(150);  
      if (isButtonPressed(buttonPins[i])) {
        visualFeedback(i, LOW);
        delay(200);
        return;
      }
      visualFeedback(i, LOW);
      delay(150);
    }
    delay(200);
  }
}
