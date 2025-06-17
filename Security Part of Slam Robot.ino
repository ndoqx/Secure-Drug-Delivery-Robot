#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Buzzer.h>

// I2C LCD Setup
#define I2C_ADDR 0x27
LiquidCrystal_I2C lcd(I2C_ADDR, 16, 2);

// Relay & Buzzer
#define RELAY1_PIN 5  // Main relay (controls unlock/lock)
#define BUZZER_PIN 4
Buzzer buzzer(BUZZER_PIN);

// Keypad Setup
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {32, 33, 25, 26}; // Updated row pin assignments
byte colPins[COLS] = {27, 14, 12, 13}; // Updated column pin assignments

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Password logic
String keyinput = "";
String password = "0000";
bool isLocked = true;
bool isBuzzerActive = false;

void playWrongPasswordSound() {
  tone(BUZZER_PIN, 262, 500); // Single beep for incorrect password
  delay(500);
  noTone(BUZZER_PIN);
}

void playSuccessSound() {
  int melody[] = {988, 1319};
  int durations[] = {150, 300};  

  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, melody[i], durations[i]);
    delay(durations[i] * 1.3);
  }
  noTone(BUZZER_PIN);
}


void playBreakingSound() {
  int melody[] = {1000, 1500, 1200, 900, 600, 400, 300, 200, 300, 500, 700};  
  int durations[] = {50, 70, 80, 90, 100, 110, 120, 150, 120, 100, 80};  

  for (int i = 0; i < 11; i++) {
    tone(BUZZER_PIN, melody[i], durations[i]);
    delay(durations[i] * 1.2);  
  }

  noTone(BUZZER_PIN);
}

// Display Utility
void updateDisplay(const String &line1, const String &line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  Serial.println(line1);
  Serial.println(line2);
}

// Function to update relay based on lock status
void updateRelay() {
  digitalWrite(RELAY1_PIN, isLocked ? LOW : HIGH);
}

// Setup
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  
  pinMode(RELAY1_PIN, OUTPUT);
  
  isLocked = true;  // Start in locked state
  updateRelay();
  updateDisplay(">>> LOCKED <<<", "Enter Password:");
}

// Main Loop
void loop() {
  char key = customKeypad.getKey();

  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);

    // Handle buzzer reset if needed
    if (isBuzzerActive && key == '#') {
      buzzer.end(0);
      isBuzzerActive = false;
    }

    // Handle Locking when Unlocked
    if (!isLocked && key == '#') {
      isLocked = true;
      updateRelay();
      Serial.println("Relay is locked.");
      updateDisplay(">>> LOCKED <<<", "Enter Password:");
      keyinput = "";
      playBreakingSound();
      return;
    }

    // Handle Password Entry when Locked
    if (isLocked) {
      if (key == '#') { 
        // Reset the password input
        keyinput = "";
        updateDisplay(">>> LOCKED <<<", "Enter Password:");
        playBreakingSound();
        return;
      }

      keyinput += key;
      updateDisplay(">>> LOCKED <<<", "PASSWORD: " + keyinput);

      // When 4 digits are entered, check password
      if (keyinput.length() >= 4) {
        if (keyinput == password) {
          isLocked = false;
          updateRelay();
          Serial.println("Password correct, Relay is unlocked.");
          updateDisplay("*** UNLOCKED ***", "Press '#' to lock");
          playSuccessSound();
        } else {
          Serial.println("Incorrect password, try again.");
          updateDisplay("Incorrect Pass", "word Try again.");
          playWrongPasswordSound();
        }
        keyinput = "";
      }
    }
  }
}
