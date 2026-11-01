// Keypad Music Player
// Arduino Uno + 4x4 keypad + passive buzzer + 7 LEDs (one per note C4-B4)
// Plays 4 built-in songs or single notes, with pause, next/previous and speed control.

#include <Keypad.h>

#define BUZZER 9
#define RESET_SWITCH 12

int ledPins[7] = {2,3,4,5,6,7,8};
int notes[7] = {262, 294, 330, 349, 392, 440, 494};   // C4 D4 E4 F4 G4 A4 B4

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {A4, A5, 10, 11};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Songs (note indices: 0=C 1=D 2=E 3=F 4=G 5=A 6=B)
const byte songA[] = {0,0,4,4,5,5,4, 3,3,2,2,1,1,0, 4,4,3,3,2,2,1, 4,4,3,3,2,2,1,
                      0,0,4,4,5,5,4, 3,3,2,2,1,1,0};
// Happy Birthday spans more than one octave, so it is approximated within C4-B4
const byte songB[] = {0,0,1,0,3,2, 0,0,1,0,4,3, 0,0,6,5,3,2,1, 5,5,4,3,4,3};
const byte songC[] = {2,2,2,2,2,2,2,4,0,1,2, 3,3,3,3,3,2,2,2,2, 2,1,1,2,1,4,
                      2,2,2,2,2,2,2,4,0,1,2, 3,3,3,3,3,2,2,2,2, 4,4,3,1,0};
const byte songD[] = {2,2,3,4,4,3,2,1,0,0,1,2,2,1,1, 2,2,3,4,4,3,2,1,0,0,1,2,1,0,0};

#define LEN(a) (sizeof(a) / sizeof(a[0]))

const byte* const songs[] = {songA, songB, songC, songD};
const int songLength[] = {LEN(songA), LEN(songB), LEN(songC), LEN(songD)};
const int NUM_SONGS = LEN(songs);

// Song names
const char* songNames[] = {
  "Twinkle Twinkle Little Star",
  "Happy Birthday",
  "Jingle Bells",
  "Ode to Joy"
};

// Player state
int currentSong = 0;
bool isPlaying = false;
int speedDelay = 400;

int noteIndex = 0;
bool notePlaying = false;
bool inGap = false;

unsigned long noteStartTime = 0;

// Reset tracking
bool lastResetState = HIGH;

// Print control
bool songJustStarted = false;

// ====================== MAIN ======================

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(RESET_SWITCH, INPUT_PULLUP);
  for(int i = 0; i < 7; i++) pinMode(ledPins[i], OUTPUT);

  Serial.begin(9600);
}

void playManual(int n){
  isPlaying = false;
  notePlaying = false;
  inGap = false;

  noTone(BUZZER);
  tone(BUZZER, notes[n]);
  digitalWrite(ledPins[n], HIGH);
  delay(150);
  noTone(BUZZER);
  digitalWrite(ledPins[n], LOW);
}

void loop() {

  // RESET
  bool currentResetState = digitalRead(RESET_SWITCH);

  if(lastResetState == HIGH && currentResetState == LOW){
    speedDelay = 400;
    Serial.println("RESET PRESSED -> speed = 400");
  }

  lastResetState = currentResetState;

  char key = keypad.getKey();

  if(key){
    // Manual play
    if(key >= '1' && key <= '7'){
      playManual(key - '1');
    }

    // Select song
    if(key >= 'A' && key <= 'D'){
      currentSong = key - 'A';
      noteIndex = 0;
      isPlaying = true;
      notePlaying = false;
      inGap = false;
      noTone(BUZZER);

      songJustStarted = true;
    }

    // Pause
    if(key == '0'){
      isPlaying = !isPlaying;
      if(!isPlaying){
        noTone(BUZZER);
        notePlaying = false;
        inGap = false;
      }
    }

    // Next
    if(key == '#'){
      currentSong = (currentSong + 1) % NUM_SONGS;
      noteIndex = 0;
      notePlaying = false;
      inGap = false;
      noTone(BUZZER);

      songJustStarted = true;
    }

    // Previous
    if(key == '*'){
      currentSong = (currentSong + NUM_SONGS - 1) % NUM_SONGS;
      noteIndex = 0;
      notePlaying = false;
      inGap = false;
      noTone(BUZZER);

      songJustStarted = true;
    }

    // Slow
    if(key == '8'){
      speedDelay *= 1.5;
      if(speedDelay > 2000) speedDelay = 2000;
      Serial.print("SLOW -> ");
      Serial.println(speedDelay);
    }

    // Fast
    if(key == '9'){
      speedDelay /= 1.5;
      if(speedDelay < 20) speedDelay = 20;
      Serial.print("FAST -> ");
      Serial.println(speedDelay);
    }
  }

  playSong();
}

void playSong() {
  if (!isPlaying) return;

  // Print song name ONCE
  if (songJustStarted) {
    Serial.print("Now Playing: ");
    Serial.print(songNames[currentSong]);
    Serial.print(" | Speed: ");
    Serial.println(speedDelay);
    songJustStarted = false;
  }

  unsigned long currentMillis = millis();

  // Start note
  if (!notePlaying && noteIndex < songLength[currentSong]) {
    int n = songs[currentSong][noteIndex];

    tone(BUZZER, notes[n]);
    allLedsOff();
    digitalWrite(ledPins[n], HIGH);

    noteStartTime = currentMillis;
    notePlaying = true;
    inGap = false;
  }

  if (notePlaying) {

    float playRatio = 0.85;
    float gapRatio  = 0.15;

    unsigned long noteDuration = speedDelay * playRatio;
    unsigned long gapDuration  = speedDelay * gapRatio;

    unsigned long elapsed = currentMillis - noteStartTime;

    if (!inGap && elapsed >= noteDuration) {
      noTone(BUZZER);
      allLedsOff();
      noteStartTime = currentMillis;
      inGap = true;
    }
    else if (inGap && elapsed >= gapDuration) {
      notePlaying = false;
      noteIndex++;
    }
  }

  // End song
  if (noteIndex >= songLength[currentSong]) {
    isPlaying = false;
    noteIndex = 0;
    notePlaying = false;
    inGap = false;
    noTone(BUZZER);
    allLedsOff();
  }
}

void allLedsOff() {
  for(int i = 0; i < 7; i++) digitalWrite(ledPins[i], LOW);
}
