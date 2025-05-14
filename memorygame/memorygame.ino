#include <Wire.h>  // Libreria per la comunicazione I2C (necessaria per il display OLED)
#include <Adafruit_GFX.h>  // Libreria grafica generica Adafruit (necessaria per gestire scritte e grafica)
#include <Adafruit_SSD1306.h>  // Libreria specifica per il display OLED SSD1306

#define SCREEN_WIDTH 128  // Larghezza del display OLED in pixel
#define SCREEN_HEIGHT 64  // Altezza del display OLED in pixel
#define OLED_ADDR 0x3C  // Indirizzo I2C standard del display OLED

// Creazione dell’oggetto display con le dimensioni specificate
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define MAX_ROUNDS 10  // Numero massimo di turni del gioco
#define ENTRY_TIME_LIMIT 4000  // Tempo massimo (in millisecondi) per rispondere a ogni input

// Definizione simbolica dei colori (indici per array)
#define RED 0
#define GREEN 1
#define BLUE 2
#define YELLOW 3

// Array con i pin collegati ai LED colorati
const byte ledPins[] = {10, 3, 13, 5};
// Array con i pin collegati ai pulsanti (associati ai colori)
const byte buttonPins[] = {9, 2, 12, 6};
// Toni corrispondenti a ciascun colore (in Hz) per il buzzer
const int tones[] = {294, 370, 440, 523};

#define BUZZER_PIN 4  // Pin a cui è collegato il buzzer

byte sequence[MAX_ROUNDS];  // Array per memorizzare la sequenza da ripetere
byte currentRound = 0;  // Tiene traccia del round attuale
int score = 0;  // Punteggio dell'utente

// Funzione per verificare se un pulsante è premuto
bool isButtonPressed(byte pin) {
  if (digitalRead(pin) == LOW) {  // I pulsanti sono attivi LOW (premuti = LOW)
    delay(50);  // Ritardo per evitare rimbalzi (debounce)
    return digitalRead(pin) == LOW;  // Controlla di nuovo per confermare la pressione
  }
  return false;  // Se non è stato premuto
}

// Accende o spegne il LED indicato (on = true/false)
void visualFeedback(byte index, bool on) {
  digitalWrite(ledPins[index], on);  // Imposta lo stato del pin LED
}

// Emette un suono e accende brevemente il LED corrispondente
void toneFeedback(byte index) {
  visualFeedback(index, HIGH);  // Accende il LED
  tone(BUZZER_PIN, tones[index], 100);  // Suona la nota per 100 ms
  delay(100);  // Attende mentre suona
  noTone(BUZZER_PIN);  // Ferma il suono
  delay(200);  // Pausa prima di spegnere il LED
  visualFeedback(index, LOW);  // Spegne il LED
  delay(100);  // Ulteriore pausa prima di continuare
}

// Mostra un testo centrato sul display OLED
void displayCenteredText(String text, int y, int size = 1) {
  display.setTextSize(size);  // Imposta la dimensione del testo
  display.setTextColor(WHITE);  // Imposta il colore del testo
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);  // Calcola la dimensione del testo
  display.setCursor((SCREEN_WIDTH - w) / 2, y);  // Posiziona il cursore orizzontalmente centrato
  display.println(text);  // Stampa il testo sul display
}

// Mostra informazioni di gioco in alto (round e punteggio)
void displayGameInfo() {
  display.setTextSize(1);  // Testo piccolo
  display.setTextColor(WHITE);  // Colore testo
  display.setCursor(0, 0);  // Posizione in alto a sinistra
  display.print("Round: ");  // Scrive "Round:"
  display.print(currentRound);  // Mostra il numero del round
  display.setCursor(0, 8);  // Riga successiva
  display.print("Punti: ");  // Scrive "Punti:"
  display.print(score);  // Mostra il punteggio
}

// Mostra un countdown con secondi rimanenti per l'input
void showCountdown(unsigned long msLeft) {
  display.clearDisplay();  // Pulisce il display
  displayGameInfo();  // Mostra round e punteggio
  displayCenteredText(String((msLeft + 999) / 1000), 32, 2);  // Mostra secondi rimanenti (arrotondati in alto)
  display.display();  // Aggiorna lo schermo
}

// Aspetta che l’utente prema un pulsante, con timeout
int waitForButton() {
  long start = millis();  // Salva l'ora iniziale
  while (millis() - start < ENTRY_TIME_LIMIT) {  // Finché non scade il tempo
    showCountdown(ENTRY_TIME_LIMIT - (millis() - start));  // Mostra il conto alla rovescia
    for (byte i = 0; i < 4; i++) {  // Scorre tutti i pulsanti
      if (isButtonPressed(buttonPins[i])) {  // Se il pulsante è premuto
        toneFeedback(i);  // Suono e LED
        return i;  // Ritorna il colore premuto
      }
    }
  }
  return -1;  // Nessun pulsante premuto entro il tempo
}

// Riproduce la sequenza di colori e suoni che l’utente deve memorizzare
void playSequence() {
  display.clearDisplay();  // Pulisce il display
  displayGameInfo();  // Mostra punteggio e round
  displayCenteredText("Memorizza!", 20);  // Messaggio per l’utente
  display.display();  // Aggiorna display
  delay(750);  // Attesa prima di iniziare

  for (byte i = 0; i < currentRound; i++) {  // Scorre la sequenza da riprodurre
    toneFeedback(sequence[i]);  // Suona e illumina il colore della sequenza
    delay(400);  // Pausa tra i segnali
  }

  display.clearDisplay();  // Pulisce il display
  displayGameInfo();  // Mostra info gioco
  displayCenteredText("Tocca!", 36);  // Invita l’utente a toccare
  display.display();  // Aggiorna il display
}

// Aggiunge un nuovo colore casuale alla sequenza
void addMove() {
  sequence[currentRound - 1] = random(0, 4);  // Genera numero tra 0 e 3 (colori)
}

// Mostra schermata di vittoria
void playWinner() {
  display.clearDisplay();
  displayCenteredText("Bravo!", 16, 2);  // Messaggio di vittoria
  displayCenteredText("Punteggio: " + String(score), 40, 1);  // Mostra punteggio
  display.display();
  for (int i = 0; i < 4; i++) {  // Animazione vincita
    visualFeedback(i, HIGH);
    toneFeedback(i);
    visualFeedback(i, LOW);
    delay(150);
  }
  delay(2000);  // Attesa finale
}

// Mostra schermata di errore (sconfitta)
void playLoser() {
  display.clearDisplay();
  displayCenteredText("Sbagliato!", 16, 2);  // Messaggio di errore
  displayCenteredText("Punteggio: " + String(score), 40, 1);  // Mostra punteggio
  display.display();
  for (int i = 0; i < 2; i++) {  // Animazione errore
    visualFeedback(RED, HIGH);
    visualFeedback(YELLOW, HIGH);
    toneFeedback(RED);
    toneFeedback(YELLOW);
    visualFeedback(RED, LOW);
    visualFeedback(YELLOW, LOW);
    delay(250);
  }
  delay(2000);  // Attesa
}

// Modalità "attract mode": il gioco aspetta che un giocatore inizi
void attractMode() {
  display.clearDisplay();
  displayCenteredText("MEMORY", 10, 2);  // Titolo
  displayCenteredText("GAME", 35, 2);  // Sottotitolo
  displayCenteredText("Premi il tasto verde", 55, 1);  // Invito all’utente
  display.display();

  score = 0;  // Reset punteggio

  while (true) {  // Ciclo infinito finché non si preme un pulsante
    for (byte i = 0; i < 4; i++) {
      visualFeedback(i, HIGH);  // Accende LED
      delay(150);
      if (isButtonPressed(buttonPins[i])) {  // Se viene premuto un tasto
        visualFeedback(i, LOW);  // Spegne LED
        delay(200);
        return;  // Esce e inizia il gioco
      }
      visualFeedback(i, LOW);  // Spegne LED
      delay(150);
    }
    delay(200);  // Pausa tra cicli
  }
}

// Setup iniziale (eseguito una sola volta all’accensione)
void setup() {
  Serial.begin(9600);  // Avvia comunicazione seriale
  for (byte i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);  // Imposta LED come output
    pinMode(buttonPins[i], INPUT_PULLUP);  // Pulsanti con resistenza di pull-up interna
  }
  pinMode(BUZZER_PIN, OUTPUT);  // Pin del buzzer come output

  // Inizializza il display OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("Display non trovato.");  // Messaggio di errore
    while (1);  // Blocco infinito
  }

  display.clearDisplay();
  displayCenteredText("Ciao!", 20, 2);  // Messaggio di benvenuto
  display.display();
  delay(1500);  // Pausa
}

// Ciclo principale del programma
void loop() {
  attractMode();  // Attende che l’utente inizi il gioco

  display.clearDisplay();
  displayCenteredText("Inizia!", 20, 2);  // Messaggio di inizio
  display.display();
  delay(1000);

  currentRound = 1;  // Inizia da round 1
  score = 0;  // Reset punteggio

  while (currentRound <= MAX_ROUNDS) {  // Continua finché non si completano i round
    addMove();  // Aggiunge una nuova mossa alla sequenza
    playSequence();  // Mostra la sequenza all’utente

    for (byte i = 0; i < currentRound; i++) {  // Controlla gli input dell’utente
      int playerInput = waitForButton();  // Attende risposta
      if (playerInput != sequence[i]) {  // Se la risposta è sbagliata
        playLoser();  // Mostra errore
        delay(2000);
        return;  // Termina il gioco
      }
    }

    score++;  // Aggiunge punto al punteggio
    display.clearDisplay();
    displayCenteredText("Corretto!", 20, 2);  // Messaggio di conferma
    displayCenteredText("+1 Punto", 40, 1);
    display.display();
    currentRound++;  // Passa al turno successivo
    delay(1200);
  }

  playWinner();  // L’utente ha completato tutti i round
  delay(2000);
}
