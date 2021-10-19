/*
 * Version Multiuser
 * Creator: Aaron Gensetter & M. J.
 * M242, C. J.
 * Date: 03.2021
*/

// Include Librarys
#include <SPI.h> // (RFID)
#include <MFRC522.h> // (download from arduino library) (RFID)
#include <Wire.h> // (LCD)
#include <LiquidCrystal_I2C.h> // (download from arduino library) (LCD)
#include <Servo.h> // (download from arduino library) (Servo)

// Define Constats
#define SS_PIN 10 // Set SS PIN (RFID reader)
#define RST_PIN 9 // Set RST PIN (RFID reader)
#define SERVO_PIN 7 // Set Servo PIN
#define BTN1_PIN 5 // Set Button PIN
#define LED_RED 3 // Set RED LED PIN
#define LED_GREEN 2 // Set GREEN LED PIN
#define POT_PIN A0 // Set Poti Pin
#define SPEAKER_PIN 4 // Set Speaker PIN

Servo servo; // define Servo
LiquidCrystal_I2C lcd(0x27, 16, 2); // create the Variable for the Display // (com), char, rows
MFRC522 mfrc522(SS_PIN, RST_PIN); // define the RFID reader pin positions

String cards[2][2] = {{"AB601728", "User1"}, {"B6ADA628", "User2"}}; // define Cards and Usernames

void setup() {
  pinMode (LED_GREEN, OUTPUT); // Set the GEERN LED PIN to Output mode
  pinMode (LED_RED, OUTPUT); // Set the RED LED PIN to Output mode
  pinMode (BTN1_PIN, INPUT_PULLUP); // Set BTN PIN to INPUT (PULLUP (prevent ghost input))
  
  Serial.begin(9600); // Start Serial for monitoring and Debuging (baud = 9600)
  SPI.begin(); // Start the SPI session for the RFID reader
  
  mfrc522.PCD_Init(); // Initialize the RFID reader
  lcd.init(); // Initialize the LCD Display
  lcd.backlight(); // turn on Backlight from the lcd Display
  servo.attach(SERVO_PIN); // Initialize Servo on PIN 7
  
  Serial.println("Power: Startup..."); // print Debug message in Serial console

  // DEBUG/TEST START:
    // make a tone on startup
    tone(SPEAKER_PIN, 2000, 300); // PIN, tune, length
    // flash LEDs on Startup
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, HIGH);
    delay(300);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
  // DEBUG/TEST END

  openDoor(0, "debug"); // open door on startup to make shure the door is closed)
}

void loop() {
  int waittime = (floor((analogRead(POT_PIN) / 100)) + 1); // Check Poti Status and make a single digit (1 - 11)
  int bypass = !digitalRead(BTN1_PIN); // check bypass btn status
  String cardID = "";

  delay(30); // make a break (30ms) to prevent a random system crash
  
  lcd.clear(); // clear the LCD screen
  lcd.setCursor(0, 0); // set Cursor on LCD to the first positon on the first row
  lcd.print("Please Scan");
  lcd.setCursor(0, 1); // set Cursor on LCD to the first positon on the second row
  lcd.print("a Card");

  // 0 = unpressed, 1 = pressed
  if(bypass == 1) {
    // Check if the Bypass Button is pressed
    Serial.println("Bypass: btn pressed"); // print Debug message in Serial console
    openDoor(waittime, "User"); // call function openDoor
  }
  
  if(!mfrc522.PICC_IsNewCardPresent()) { // check if a new RFID chip/card is detected
    // if no new card cas detected, return
    return;
  }
  if(!mfrc522.PICC_ReadCardSerial()) { // select a card
    return;
  }
   
  for(byte i = 0; i < mfrc522.uid.size; i++) {
    cardID.concat(String(mfrc522.uid.uidByte[i], HEX)); // get the CardID in HEX format
  }
  cardID.toUpperCase(); // set the cardID to Uppercase

  // Iterate trought all the pre-saved cards
  for(int i = 0; i < (sizeof(cards) / sizeof(cards[0])); i++) {
    if(cardID == cards[i][0]) {
      // go on if card is the right
      Serial.println("Access: granted with cardID: " + cardID); // print Debug message in Serial console
      openDoor(waittime, cards[i][1]); // call function rejectAccess
      return;
    }
  }
  // if no valid card was found, go on
  Serial.println("Access: denied with cardID: " + cardID); // print Debug message in Serial console
  rejectAccess(); // call function rejectAccess
}

// Function Reject Access
void rejectAccess() {
    digitalWrite(LED_RED, HIGH); // turn on the red LED
    // Create Error Tone
    tone(SPEAKER_PIN, 1000, 200); // PIN, tune, length
    delay(300); 
    tone(SPEAKER_PIN, 1000, 200); // PIN, tune, length
    // create message
    lcd.clear();
    lcd.setCursor(0, 0); // set Cursor on LCD to the first positon on the first row
    lcd.print("Access Denied");
    digitalWrite(LED_RED, LOW); // turn off the red LED
    delay(1000); // make a little break to prevent a second activation from the same card (smaller then the Success one)
    digitalWrite(LED_RED, HIGH);
    delay(100);
    digitalWrite(LED_RED, LOW); // flash the led for 200ms to indicate that you can scan again
    Serial.println("Reset: reset"); // print Debug message in Serial console
}

// Function Open Door
void openDoor(int waittime, String uname) {
    Serial.println("Door: open"); // print Debug message in Serial console
    servo.write(90); // Open Door
    digitalWrite(LED_GREEN, HIGH); // turn on the green LED
    // Create Success Tone
    tone(SPEAKER_PIN, 5000, 250); // PIN, tune, length
    // create message
    lcd.clear();
    lcd.setCursor(0, 0); // set Cursor on LCD to the first positon on the first row
    lcd.print("Welcome ");
    lcd.print(uname);
    lcd.setCursor(0, 1); // set Cursor on LCD to the first positon on the second row
    lcd.print("Door Open:");
      
    // create a timer for the LCD display
    for(int i = 0; i < waittime; i++) {
      int n = waittime - i;
      lcd.setCursor(11, 1); // set Cursor on LCD to the 11th positon on the second row
      if(n < 10) {
        // If number is under 10, add a zero (01,02...09)
        lcd.print("0");
        lcd.print(n);
      } else {
        // else print number, (10, 11)
        lcd.print(n);
      }
      // print Debug message in Serial console
      Serial.print("Timer: ");
      Serial.print(n);
      Serial.println("");
      delay(1000);
    }
    Serial.println("Door: close"); // print Debug message in Serial console
    servo.write(-90); // Close Door
    digitalWrite(LED_GREEN, LOW); // turn off the green LEDcard
    digitalWrite(LED_GREEN, HIGH);
    delay(100);
    digitalWrite(LED_GREEN, LOW); // flash the led for 200ms to indicate that you can scan again
    Serial.println("Reset: reset"); // print Debug message in Serial console
}
/* 
 *  QUELLEN: 
 *  https://create.arduino.cc/projecthub/Raushancpr/diy-idea-with-rfid-89e41d
 */

