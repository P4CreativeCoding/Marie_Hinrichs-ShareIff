//includes for RFID
#include <SPI.h>
#include <MFRC522.h>

//RFID Reader Pins
#define RST_PIN         9
#define SS_PIN          10

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

//includes for LCD Display
#include <LiquidCrystal.h>

//Define LCD Pins
int LCD_RS = 3;
int LCD_E = 4;
int LCD_D4 = 5;
int LCD_D5 = 6;
int LCD_D6 = 7;
int LCD_D7 = 8;

//Setup LCD Screen
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

//includes for Servo Motor
#include <Servo.h>

//Servo Motor setup
Servo servomotor1; //servo Motor object 1
Servo servomotor2; //servo Motor object 2
//initital motor position
int pos = 20;
//Define Servo D-Pins
int servo1Pin = 15;
int servo2Pin = 2;

//Define Button Pins
int LEFT_BUTTON = 0;
int RIGHT_BUTTON = 14;
//Pressed bool for button logic
bool pressed = false;

//Timer Variables
int timePassed = 0;
int timeLeft = 10;

//Authentication Variable
bool authenticated = false;

//Process Step, for Locker Selection
int processStep = 1;

//Open Locker Function
void openLocker1() {
  for (pos = 20; pos <= 100; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servomotor1.write(pos);              // tell servo to go to position in variable 'pos'
    delay(1);                       // waits 1 ms for the servo to reach the position
  }
}
//Open Locker Function
void openLocker2() {
  for (pos = 20; pos <= 100; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servomotor2.write(pos);              // tell servo to go to position in variable 'pos'
    delay(1);                       // waits 1 ms for the servo to reach the position
  }
}
//Close Locker Function
void closeLocker1() {
  for (pos = 100; pos >= 20; pos -= 1) { // goes from 180 degrees to 0 degrees
    servomotor1.write(pos);              // tell servo to go to position in variable 'pos'
    delay(1);                       // waits 1 ms for the servo to reach the position
  }
}
//Close Locker Function
void closeLocker2() {
  for (pos = 100; pos >= 20; pos -= 1) { // goes from 180 degrees to 0 degrees
    servomotor2.write(pos);              // tell servo to go to position in variable 'pos'
    delay(1);                       // waits 1 ms for the servo to reach the position
  }
}

//Init Move Servo Motors, to reduce vibrations of motors
void initMoveServos() {
  for (pos = 0; pos <= 90; pos += 1) { // goes from 180 degrees to 0 degrees
    servomotor1.write(pos);
    servomotor2.write(pos);              // tell servo to go to position in variable 'pos'
    delay(1);                       // waits 1 ms for the servo to reach the position
  }
  for (pos = 90; pos >= 20; pos -= 1) { // goes from 180 degrees to 0 degrees
    servomotor1.write(pos);
    servomotor2.write(pos);              // tell servo to go to position in variable 'pos'
    delay(1);                       // waits 1 ms for the servo to reach the position
  }
}

//Helper Function to jump in second row of LCD Screen
void br() {
  lcd.setCursor(0, 1);
}

//Helper Function, for Session End Scenarios
void endSession() {
  lcd.clear();
  lcd.print("Vielen Dank fuer");
  br();
  lcd.print("die Nutzung");
  delay(2000);
  lcd.clear();
  lcd.print("die Nutzung");
  br();
  lcd.print("von ShareIff.");
  delay(3000);
  timePassed = 9; //sets timePassed to 9, so the next loop exits the session and removes auth Tag
}

//Helper Function, to write a two row Text, to the LCD Screen
void printText(String FirstRow, String SecondRow) {
  lcd.clear();
  lcd.print(FirstRow);
  br();
  lcd.print(SecondRow);
}

void setup() {
  // Initialize serial communications with the PC
	Serial.begin(9600);
  Serial.write("\nSerial started");

  //LCD Setup
  lcd.begin(16, 2);
  printText("Bitte mit Chip", "authentifizieren");

  //set pinmode to input for buttons
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  Serial.write("\nButtons Pin Mode defined");

	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	// Serial.println(F("Ready for RFID Chip"));
  // Serial.print("\nLooking for RFID UID : D5 4B E8 75");
  Serial.write("\nRFID started");

  //attach Servos to Pins
  servomotor1.attach(servo1Pin);
  servomotor1.write(20); //set servo position to start position
  servomotor2.attach(servo2Pin);
  servomotor2.write(20); //set servo position to start position
  initMoveServos();
  Serial.write("\nServo Motors Setup");
}

void loop() {
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  //Reset content to nothing
  String content = "";

//Concat UID Bytes of RFID Card to one String
	for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  Serial.println("\n");
  content.toUpperCase();

  //Check for correct RFID UID Tag
  if (content.substring(1) == "D5 4B E8 75") {
    Serial.println("Zugang gewährt");
    //actions for correct RFID Tag
    //sets authenticated to true, and is used later on
    authenticated = true;
    printText("Erfolgreich", "authentifiziert");
    delay(3000);
    //Just Text for Put in or Rent decision. The Algorithm is not implemented yet
    printText("Gelb=Einlegen", "Weiss=Mieten");
  } else {
    Serial.println("Zugang nicht gewährt");
    lcd.clear();
    printText("Nutzer nicht", "bekannt");
    delay(3000);
    printText("Bitte mit Chip", "authentifizieren");
  }

 authenticated = true;
  //actions when user is authenticated 
  if (authenticated == true) {
    //check if user ran into time out
    while (timePassed <= 10) { //actions if user didn't ran into time out
      timeLeft = 10 - timePassed;
      //Print time left on LCD Screen and set cursor position
      if (timeLeft <= 9) { //if time < 9, print a 0 before single digit
        lcd.setCursor(14,1);
        lcd.print("0");
      } else {
        lcd.setCursor(14, 1);
      }
      lcd.print(timeLeft);
      //Timeout if no user input
      if (timePassed >= 10 && authenticated == true) { //Actions if user ran into time out
        printText("Kehre zum Start", "zurueck");
        delay(2000);
        authenticated = false;
        printText("Bitte mit Chip", "authentifizieren");
      }

      //on user input
      //Selected left compartment
      if (digitalRead(LEFT_BUTTON) == pressed) {
        timePassed = 0;
        while (digitalRead(LEFT_BUTTON) == pressed) {
          //do nothing, while button is held down
        }
        //Event on release
        printText("Bitte Fach", "auswaehlen");
        //checks if user is on processStep 2, processStep 1 is Put in or Rent decision
        if (processStep >= 2) {
          //Actions on button press and locker selection
          //open left locker
          openLocker1();
          printText("Unteres Fach", "oeffnet sich");
          delay(3000);
          printText(" Bitte Knopf druecken", "          wenn Sie fertig sind");
          //Wait for Buton Press
          while (authenticated == true) {
            lcd.scrollDisplayLeft();
            delay(750);
            //check for any button press, to close the door
            if (digitalRead(LEFT_BUTTON) == pressed || digitalRead(RIGHT_BUTTON) == pressed) {
              while (digitalRead(LEFT_BUTTON) == pressed ||digitalRead(RIGHT_BUTTON) == pressed) {
                //do nothing
              }
              closeLocker1();
              //exits the While Loop
              break;
            }
          }
          //resets the whole Process, returns to start
          endSession();
        }
        //Sets processStep to 2, so the Algorithm enters the next if block, on the next loop
        processStep = 2;
      }

      //Selected right compartment
      if (digitalRead(RIGHT_BUTTON) == pressed) {
        timePassed = 0;
        while (digitalRead(RIGHT_BUTTON) == pressed) {
          //do nothing, while button is held down
        }
        //Event on release
        printText("Bitte Fach", "auswaehlen");
        //checks if user is on processStep 2, processStep 1 is Put in or Rent decision
        if (processStep >= 2) {
          //Actions on button press and locker selection
          openLocker2();
          printText("Oberes Fach", "oeffnet sich");
          delay(3000);
          printText(" Bitte Knopf druecken", "          wenn Sie fertig sind");
          //Wait for Buton Press
          while (authenticated == true) {
            lcd.scrollDisplayLeft();
            delay(750);
            //check for any button press, to close the door
            if (digitalRead(RIGHT_BUTTON) == pressed || digitalRead(LEFT_BUTTON) == pressed) {
              while (digitalRead(RIGHT_BUTTON) == pressed || digitalRead(LEFT_BUTTON) == pressed) {
                //do nothing
              }
              closeLocker2();
              //exits while loop
              break;
            }
          }
          //resets the whole Process, returns to start
          endSession();
        }
        //Sets processStep to 2, so the Algorithm enters the next if block, on the next loop
        processStep = 2;
      }

      //Timeout counter
      timePassed = timePassed + 1;
      Serial.write("\nTime Left: ");
      Serial.print(timeLeft);
      delay(1000);
    }
  }
  //If User isnt authenticated anymore, the loop will start at the start and checks for any NFC Tags
  timePassed = 0;
  processStep = 1;
}
