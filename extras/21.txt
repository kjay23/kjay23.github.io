#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define SS_PIN 10
#define RST_PIN 9

#define LED_G 4      // Green LED for authorized access
#define LED_R 5      // Red LED for unauthorized access
#define Buzzer 6     // Digital pin for buzzer
#define door 3

int lcdRows = 2;

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// List of authorized UIDs
String authorizedUIDs[] = {
  "30 7C 44 59",  // Person 1
  "40 20 EE 14",  // Person 2
  "41 8C C0 10",  // Person 3
  "22 9D 65 55",  // Person 4
  "71 26 C1 10",  // Person 5
  "10 D0 61 59"   // Person 6
};
int numAuthorizedUIDs = sizeof(authorizedUIDs) / sizeof(authorizedUIDs[0]);

void setup() {
  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Your RFID Card!");

  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(Buzzer, OUTPUT);  // Configure buzzer pin as output
  pinMode(door, OUTPUT);

  digitalWrite(door, HIGH);  // Lock door initially
  digitalWrite(Buzzer, LOW); // Ensure buzzer is off initially
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Extract UID
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    content += String(mfrc522.uid.uidByte[i], HEX);
  }
  content.trim();  // Remove leading and trailing spaces
  content.toUpperCase();  // Convert to uppercase for comparison

  Serial.print("UID tag: ");
  Serial.println(content);

  // Check if the scanned UID is in the authorized list
  bool isAuthorized = false;
  for (int i = 0; i < numAuthorizedUIDs; i++) {
    if (content == authorizedUIDs[i]) {
      isAuthorized = true;
      break;
    }
  }

  if (isAuthorized) {
    digitalWrite(LED_G, HIGH);  // Turn on green LED
    digitalWrite(door, LOW);   // Unlock the door

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Your RFID Card: ");
    lcd.setCursor(0, 1);
    lcd.print("CARD Accepted");

    // Play chime melody on buzzer
    playChime();

    delay(2500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Your RFID Card:");

    digitalWrite(LED_G, LOW);  // Turn off green LED
    digitalWrite(door, HIGH); // Lock the door
  } else {  // Unauthorized UID
    digitalWrite(LED_R, HIGH);  // Turn on red LED
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("UNAUTHORIZED");
    lcd.setCursor(0, 1);
    lcd.print("ACCESS");

    // Long beep for unauthorized access
    digitalWrite(Buzzer, HIGH);
    delay(3000);  // Beep for 3 seconds
    digitalWrite(Buzzer, LOW);

    digitalWrite(LED_R, LOW);  // Turn off red LED
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Your RFID Card!");
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// Function to play a chime melody on the buzzer
void playChime() {
  tone(Buzzer, 262, 300); // C note for 300ms
  delay(350);
  tone(Buzzer, 330, 300); // E note for 300ms
  delay(350);
  tone(Buzzer, 392, 300); // G note for 300ms
  delay(350);
  noTone(Buzzer);         // Turn off the buzzer
}
