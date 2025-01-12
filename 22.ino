Maria Clarence Casipe
#include <LiquidCrystal_I2C.h>
#include <Servo.h>  // Include the Servo library

LiquidCrystal_I2C lcd(0x27, 16, 2); // Initialize LCD with I2C address 0x27

const int pumpPin = 2;     // Relay module connected to pin 2
const int buzzerPin = 3;   // Buzzer connected to pin 3
const int highMoistureLedPin = 4;  // LED for high moisture connected to pin 4
const int lowMoistureLedPin = 5;   // LED for low moisture connected to pin 5
const int servoPin = 6;    // Servo motor connected to pin 6

unsigned long moistureLowStartTime = 0;  // Time when moisture became low
unsigned long servoLastMoveTime = 0;     // Time for last servo movement
bool moistureLow = false;  // Flag to track if moisture is low
bool buzzerTriggered = false;  // Flag to check if buzzer has been triggered
bool servoMovingForward = true;  // Direction of servo movement

Servo irrigationServo;  // Create a Servo object for the irrigation valve

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(pumpPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(highMoistureLedPin, OUTPUT);  // Set the High Moisture LED pin as output
  pinMode(lowMoistureLedPin, OUTPUT);   // Set the Low Moisture LED pin as output

  digitalWrite(pumpPin, HIGH);  // Ensure relay is OFF initially
  digitalWrite(buzzerPin, LOW); // Ensure buzzer is OFF initially
  digitalWrite(highMoistureLedPin, LOW);  // Turn off the high moisture LED initially
  digitalWrite(lowMoistureLedPin, LOW);   // Turn off the low moisture LED initially

  // Initialize the servo motor
  irrigationServo.attach(servoPin);  // Attach the servo motor to pin 6
  irrigationServo.write(0);  // Set the servo to initial position (closed valve)

  // Display system startup message
  lcd.setCursor(0, 0);
  lcd.print("IRRIGATION");
  lcd.setCursor(0, 1);
  lcd.print("SYSTEM IS ON");
  delay(3000);
  lcd.clear();
}

void loop() {
  int value = analogRead(A0);  // Read soil moisture sensor value
  Serial.println(value);       // Print moisture value for debugging

  // Check moisture level and control the pump and servo
  if (value > 500) {  // Moisture level is high (no need for watering)
    digitalWrite(pumpPin, HIGH);  // Turn off the pump
    lcd.setCursor(0, 0);
    lcd.print("Water Pump OFF ");
    lcd.setCursor(0, 1);
    lcd.print("Moisture : HIGH");

    // Turn on high moisture LED and turn off low moisture LED
    digitalWrite(highMoistureLedPin, HIGH);  // Turn on high moisture LED
    digitalWrite(lowMoistureLedPin, LOW);    // Turn off low moisture LED

    moistureLow = false;  // Reset low moisture flag
    moistureLowStartTime = 0;  // Reset timer
    buzzerTriggered = false;   // Reset buzzer flag

    irrigationServo.write(0);  // Close the irrigation valve (servo at 0 degrees)
  } else {  // Moisture level is low (watering needed)
    digitalWrite(pumpPin, LOW);  // Turn on the pump
    lcd.setCursor(0, 0);
    lcd.print("Water Pump ON  ");
    lcd.setCursor(0, 1);
    lcd.print("Moisture : LOW ");

    // Turn on low moisture LED and turn off high moisture LED
    digitalWrite(highMoistureLedPin, LOW);   // Turn off high moisture LED
    digitalWrite(lowMoistureLedPin, HIGH);   // Turn on low moisture LED

    if (!moistureLow) {  // Start tracking time if moisture just became low
      moistureLow = true;
      moistureLowStartTime = millis();  // Record the time when moisture became low
    }

    // Make the servo move dynamically when moisture is low
    if (millis() - servoLastMoveTime >= 1000) {  // Move every 1 second
      if (servoMovingForward) {
        irrigationServo.write(90);  // Move servo to 90 degrees
      } else {
        irrigationServo.write(45);  // Move servo to 45 degrees
      }
      servoMovingForward = !servoMovingForward;  // Toggle direction
      servoLastMoveTime = millis();  // Update the last move time
    }
  }

  // Check if the moisture has been low for more than 10 seconds (10,000 milliseconds)
  if (moistureLow && !buzzerTriggered && (millis() - moistureLowStartTime >= 10000)) {
    triggerBuzzer();  // Trigger the buzzer after 10 seconds
    buzzerTriggered = true;  // Ensure the buzzer only triggers once
  }

  delay(100);  // Short delay for stability
}

// Function to trigger the buzzer
void triggerBuzzer() {
  lcd.clear();  // Clear the screen to display alert
  lcd.setCursor(0, 0);
  lcd.print("ALERT: LOW SOIL");
  lcd.setCursor(0, 1);
  lcd.print("Moisture!!!     ");

  digitalWrite(buzzerPin, HIGH);  // Turn on the buzzer
  delay(5000);  // Buzzer beep duration (5 seconds)
  digitalWrite(buzzerPin, LOW);   // Turn off the buzzer
}
