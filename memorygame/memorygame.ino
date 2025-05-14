#include <Wire.h>              // Libreria per la comunicazione I2C
#include <Adafruit_GFX.h>       // Libreria base per la grafica di Adafruit
#include <Adafruit_SSD1306.h>   // Libreria specifica per il display OLED SSD1306

// Definizione delle dimensioni dello schermo OLED
#define SCREEN_WIDTH 128        // Larghezza dello schermo OLED
#define SCREEN_HEIGHT 64        // Altezza dello schermo OLED

// Definizione dell'indirizzo I2C del display OLED
#define OLED_ADDR 0x3C          // Indirizzo I2C del display OLED

// Inizializzazione dell'oggetto display OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Definizione del numero massimo di round del gioco
#define MAX_ROUNDS 10           // Numero massimo di round

// Definizione del limite di tempo in millisecondi per l'inserimento di un input
#define ENTRY_TIME_LIMIT 4000   // Tempo limite per inserire l'input (in millisecondi)

// Definizione di costanti per i colori/indici dei LED
#define RED 0                    // Indice per il LED rosso
#define GREEN 1                  // Indice per il LED verde
#define BLUE 2                   // Indice per il LED blu
#define YELLOW 3                 // Indice per il LED giallo

// Array contenente i pin digitali collegati ai LED
const byte ledPins[] = {10, 3, 13, 5};  // Pin dei LED

// Array contenente i pin digitali collegati ai pulsanti (con resistenza di pull-up interna)
const byte buttonPins[] = {9, 2, 12, 6};  // Pin dei pulsanti

// Array contenente le frequenze sonore (in Hz) associate a ciascun colore/LED
const int tones[] = {294, 370, 440, 523};  // Frequenze sonore

// Definizione del pin digitale collegato al buzzer
#define BUZZER_PIN 4            // Pin del buzzer

// Array per memorizzare la sequenza di colori/LED da indovinare
byte sequence[MAX_ROUNDS];  // Sequenza da indovinare

// Variabile per tenere traccia del round corrente
byte currentRound = 0;      // Round attuale

// Variabile per memorizzare il punteggio del giocatore
int score = 0;              // Punteggio del giocatore

// Funzione per verificare se un pulsante è stato premuto (debounce incluso)
bool isButtonPressed(byte pin) {
  if (digitalRead(pin) == LOW) {  // Se il pulsante è premuto (LOW con INPUT_PULLUP)
    delay(50);  // Ritardo per il debounce
    return digitalRead(pin) == LOW;  // Verifica che il pulsante sia ancora premuto
  }
  return false;  // Se il pulsante non è premuto
}

// Funzione per fornire un feedback visivo accendendo o spegnendo un LED
void visualFeedback(byte index, bool on) {
  digitalWrite(ledPins[index], on);  // Accende o spegne il LED
}

// Funzione per fornire un feedback sonoro e visivo quando un pulsante viene premuto
void toneFeedback(byte index) {
  visualFeedback(index, HIGH);        // Accende il LED corrispondente
  tone(BUZZER_PIN, tones[index], 100);  // Emette il suono per 100 ms
  delay(100);                         // Ritardo per il suono
  noTone(BUZZER_PIN);                 // Ferma il suono
  delay(200);                         // Ritardo tra i feedback
  visualFeedback(index, LOW);         // Spegne il LED
  delay(100);                         // Ritardo finale
}

// Funzione per visualizzare un testo centrato sul display OLED
void displayCenteredText(String text, int y, int size = 1) {
  display.setTextSize(size);          // Imposta la dimensione del testo
  display.setTextColor(WHITE);        // Imposta il colore del testo (bianco)
  int16_t x1, y1;                    // Variabili per i limiti del testo
  uint16_t w, h;                     // Variabili per la larghezza e l'altezza del testo
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);  // Ottiene le dimensioni del testo
  display.setCursor((SCREEN_WIDTH - w) / 2, y);  // Imposta il cursore per centrare il testo
  display.println(text);  // Stampa il testo sul display
}

// Funzione per visualizzare le informazioni di gioco (round corrente e punteggio)
void displayGameInfo() {
  display.setTextSize(1);          // Imposta la dimensione del testo
  display.setTextColor(WHITE);     // Imposta il colore del testo
  display.setCursor(0, 0);         // Imposta il cursore nell'angolo in alto a sinistra
  display.print("Round: ");        // Stampa "Round:"
  display.print(currentRound);     // Stampa il round corrente
  display.setCursor(0, 8);         // Sposta il cursore alla riga successiva
  display.print("Punti: ");        // Stampa "Punti:"
  display.print(score);            // Stampa il punteggio
}

// Funzione per mostrare un conto alla rovescia sul display
void showCountdown(unsigned long msLeft) {
  display.clearDisplay();               // Pulisce il display
  displayGameInfo();                     // Mostra le informazioni di gioco
  displayCenteredText(String((msLeft + 999) / 1000), 32, 2);  // Mostra il tempo rimanente
  display.display();                     // Aggiorna il display
}

// Funzione per attendere la pressione di un pulsante entro un limite di tempo
int waitForButton() {
  long start = millis();  // Registra il momento di inizio dell'attesa
  while (millis() - start < ENTRY_TIME_LIMIT) {  // Controlla finché non scade il tempo
    showCountdown(ENTRY_TIME_LIMIT - (millis() - start));  // Mostra il conto alla rovescia
    for (byte i = 0; i < 4; i++) {  // Controlla i pulsanti
      if (isButtonPressed(buttonPins[i])) {  // Se il pulsante è premuto
        toneFeedback(i);  // Fornisce feedback sonoro e visivo
        return i;          // Restituisce l'indice del pulsante premuto
      }
    }
  }
  return -1;  // Se nessun pulsante è premuto entro il tempo limite
}

// Funzione per riprodurre la sequenza di colori/LED
void playSequence() {
  display.clearDisplay();   // Pulisce il display
  displayGameInfo();         // Mostra le informazioni di gioco
  displayCenteredText("Memorizza!", 20);  // Mostra il messaggio "Memorizza!"
  display.display();         // Aggiorna il display
  delay(750);                // Ritardo prima di iniziare la sequenza

  for (byte i = 0; i < currentRound; i++) {  // Scorre la sequenza
    toneFeedback(sequence[i]);  // Fornisce feedback sonoro e visivo per ogni elemento
    delay(400);                  // Ritardo tra gli elementi
  }

  display.clearDisplay();   // Pulisce il display
  displayGameInfo();         // Mostra le informazioni di gioco
  displayCenteredText("Tocca!", 36);  // Mostra il messaggio "Tocca!"
  display.display();         // Aggiorna il display
}

// Funzione per aggiungere una nuova mossa casuale alla sequenza
void addMove() {
  sequence[currentRound - 1] = random(0, 4);  // Aggiunge un movimento casuale alla sequenza
}

// Funzione per l'animazione di vittoria
void playWinner() {
  display.clearDisplay();  // Pulisce il display
  displayCenteredText("Bravo!", 16, 2);  // Mostra "Bravo!"
  displayCenteredText("Punteggio: " + String(score), 40, 1);  // Mostra il punteggio
  display.display();  // Aggiorna il display
  for (int i = 0; i < 4; i++) {  // Ciclo per eseguire il feedback visivo e sonoro
    visualFeedback(i, HIGH);
    toneFeedback(i);
    visualFeedback(i, LOW);
    delay(150);
  }
  delay(2000);  // Ritardo finale
}

// Funzione per l'animazione di sconfitta
void playLoser() {
  display.clearDisplay();  // Pulisce il display
  displayCenteredText("Sbagliato!", 16, 2);  // Mostra "Sbagliato!"
  displayCenteredText("Punteggio: " + String(score), 40, 1);  // Mostra il punteggio
  display.display();  // Aggiorna il display
  for (int i = 0; i < 2; i++) {  // Ciclo per eseguire il feedback visivo e sonoro
    visualFeedback(RED, HIGH);
    visualFeedback(YELLOW, HIGH);
    toneFeedback(RED);
    toneFeedback(YELLOW);
    visualFeedback(RED, LOW);
    visualFeedback(YELLOW, LOW);
    delay(250);
  }
  delay(2000);  // Ritardo finale
}

// Funzione per la modalità attrattiva (schermata iniziale)
void attractMode() {
  display.clearDisplay();  // Pulisce il display
  displayCenteredText("MEMORY", 10, 2);  // Mostra "MEMORY"
  displayCenteredText("GAME", 35, 2);    // Mostra "GAME"
  displayCenteredText("Premi il tasto verde", 55, 1);  // Istruzioni
  display.display();  // Aggiorna il display

  score = 0;  // Resetta il punteggio

  while (true) {  // Ciclo infinito per l'animazione
    for (byte i = 0; i < 4; i++) {  // Ciclo attraverso i LED
      visualFeedback(i, HIGH);  
      delay(150);  
      if (isButtonPressed(buttonPins[i])) {  // Se un pulsante è premuto
        visualFeedback(i, LOW);  // Spegne il LED
        delay(200);  // Ritardo
        return;  // Esce dalla modalità attrattiva
      }
      visualFeedback(i, LOW);
      delay(150);  // Ritardo
    }
    delay(200);  // Ritardo tra i cicli
  }
}

// Funzione di setup, eseguita una sola volta all'avvio
void setup() {
  Serial.begin(9600);  // Inizializza la comunicazione seriale per il debug
  for (byte i = 0; i < 4; i++) {  // Configura i pin dei LED come output
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);  // Configura i pin dei pulsanti come input con pull-up
  }
  pinMode(BUZZER_PIN, OUTPUT);  // Configura il pin del buzzer come output

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {  // Inizializza il display OLED
    Serial.println("Display non trovato.");
    while (1);  // Entra in un ciclo infinito se il display non è trovato
  }

  display.clearDisplay();  // Pulisce il display
  displayCenteredText("Ciao!", 20, 2);  // Mostra "Ciao!"
  display.display();  // Aggiorna il display
  delay(1500);  // Ritardo iniziale
}

// Funzione principale del loop, eseguita continuamente
void loop() {
  attractMode();  // Modalità attrattiva

  display.clearDisplay();  // Pulisce il display
  displayCenteredText("Inizia!", 20, 2);  // Mostra "Inizia!"
  display.display();  // Aggiorna il display
  delay(1000);  // Ritardo iniziale

  currentRound = 1;  // Inizializza il round corrente
  score = 0;         // Resetta il punteggio

  while (currentRound <= MAX_ROUNDS) {  // Ciclo principale del gioco
    addMove();  // Aggiunge una nuova mossa alla sequenza
    playSequence();  // Riproduce la sequenza

    for (byte i = 0; i < currentRound; i++) {  // Ciclo per l'input del giocatore
      int playerInput = waitForButton();  // Attende l'input del giocatore
      if (playerInput != sequence[i]) {  // Se l'input è errato
        playLoser();  // Esegue l'animazione di sconfitta
        delay(2000);  // Ritardo
        return;  // Ritorna alla modalità attrattiva
      }
    }

    score++;  // Incrementa il punteggio
    display.clearDisplay();  // Pulisce il display
    displayCenteredText("Corretto!", 20, 2);  // Mostra "Corretto!"
    displayCenteredText("+1 Punto", 40, 1);  // Mostra "+1 Punto"
    display.display();  // Aggiorna il display
    currentRound++;  // Passa al round successivo
    delay(1200);  // Ritardo tra i round
  }

  playWinner();  // Esegue l'animazione di vittoria
  delay(2000);  // Ritardo finale
}
