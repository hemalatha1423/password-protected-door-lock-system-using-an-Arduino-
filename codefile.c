#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define BUZZER_PIN 11
#define SERVO_PIN 10

Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---- Password configuration ----
const char correctPassword[] = "0123"; // 4-digit password
const byte PASSWORD_LENGTH = 4;

char entered[PASSWORD_LENGTH + 1]; // +1 for null terminator
byte enteredLen = 0;

// ---- Keypad setup ----
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'},
};
byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---- UI positions ----
byte starCol = 5; // where the stars start printing (like your original)

bool isLocked = true; // track door state (true = locked)

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  servo.attach(SERVO_PIN);

  // Lock the door at start
  lockDoor();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(3, 0); lcd.print("WELCOME TO");
  lcd.setCursor(0, 1); lcd.print("DOOR LOCK SYSTEM");
  delay(2000);
  lcd.clear();

  resetEntry();
}

void loop() {
  lcd.setCursor(1, 0);
  lcd.print("ENTER PASSWORD ");

  char key = keypad.getKey();
  if (key == NO_KEY) return;

  delay(60); // debounce

  if (key == 'C') {
    resetEntry();
    lcd.clear();
  } else if (key >= '0' && key <= '9') {
    if (enteredLen < PASSWORD_LENGTH) {
      entered[enteredLen++] = key;
      entered[enteredLen] = '\0';
      lcd.setCursor(starCol + enteredLen - 1, 1);
      lcd.print('*');
    }
    if (enteredLen == PASSWORD_LENGTH) {
      evaluateEntry();
    }
  }
}

void resetEntry() {
  enteredLen = 0;
  entered[0] = '\0';
  for (byte i = 0; i < PASSWORD_LENGTH; ++i) {
    lcd.setCursor(starCol + i, 1);
    lcd.print(' ');
  }
  lcd.setCursor(starCol, 1);
}

void evaluateEntry() {
  lcd.clear();
  if (strcmp(entered, correctPassword) == 0) {
    // correct password
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);

    if (isLocked) {
      openDoor(); // if locked, open it
      lcd.setCursor(0, 0); lcd.print("CORRECT PASSWORD");
      lcd.setCursor(0, 1); lcd.print("DOOR OPENED");
    } else {
      lockDoor(); // if open, lock it
      lcd.setCursor(0, 0); lcd.print("CORRECT PASSWORD");
      lcd.setCursor(0, 1); lcd.print("DOOR LOCKED");
    }

  } else {
    // wrong password
    for (int i = 0; i < 3; ++i) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(150);
      digitalWrite(BUZZER_PIN, LOW);
      delay(150);
    }
    lcd.setCursor(0, 0); lcd.print("WRONG PASSWORD!");
    lcd.setCursor(0, 1); lcd.print("PLEASE TRY AGAIN");
  }

  delay(1500);
  lcd.clear();
  resetEntry();
}

void openDoor() {
  servo.write(50);  // open position (adjust if needed)
  isLocked = false;
}

void lockDoor() {
  servo.write(110); // locked position (adjust if needed)
  isLocked = true;
}
