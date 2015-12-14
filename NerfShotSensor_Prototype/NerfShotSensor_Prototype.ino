
#include "DataStructures.h"

const byte shotDetectorStateActivated = 0x01;
const byte shotDetectorStateDeactivated = 0x02;

ShotDetector *shotDetector1;
ShotDetector* shotDetectors[2];

const int playerCount = 2;

void setup() {

  //iterate all of the players
  for (int player = 0; player < 2; player++)
  {
    initializeShotDetector(player);
    initializeShotDetector(player);
  }
  
  Serial.begin(9600);
  Serial.println("Ready for multiplayer game!");
}

void initializeShotDetector(int playerIndex) {
  shotDetectors[playerIndex] = new ShotDetector();
  shotDetectors[playerIndex]->readPin = (3 * playerIndex) + 2;  //Read it on pin 2
  shotDetectors[playerIndex]->hitLedPin = (3 * playerIndex) + 3;  //Read it on pin 12
  shotDetectors[playerIndex]->previousState = shotDetectorStateDeactivated;
  shotDetectors[playerIndex]->currentState = shotDetectorStateDeactivated;
  shotDetectors[playerIndex]->lastStateReadTime = millis();
  shotDetectors[playerIndex]->playerNumber = playerIndex + 1;
  
  //Setup the shot detector pin
  pinMode(shotDetectors[playerIndex]->readPin, INPUT);
  pinMode(shotDetectors[playerIndex]->hitLedPin, OUTPUT);
}

bool gameOver = false;
ShotDetector *winner;
bool displayingWinner = false;


void loop() {
  
  if (!gameOver)
  {
     for (int playerIndex = 0; playerIndex < 2; playerIndex++)
     {
       HandleShotDetection(shotDetectors[playerIndex]);
       HandleShotDisplay(shotDetectors[playerIndex]);
     }
  }
  else
  {
    if (!displayingWinner) {
      
      //set displaying winner to true
      //It can be set to false during the handle shot display loop
      displayingWinner = true;
      
      for (int playerIndex = 0; playerIndex < 2; playerIndex++)
      {
         HandleShotDisplay(shotDetectors[playerIndex]);
         if (shotDetectors[playerIndex]->currentState =! shotDetectorStateDeactivated)
         {
           displayingWinner = false;
         }
      }
    } else {
        //Display the winner by blinking the winning led
        digitalWrite(winner->hitLedPin, HIGH);  
        delay(250);              
        digitalWrite(winner->hitLedPin, LOW);
        delay(250);
    }
  }

  
}

void HandleGameOver(ShotDetector *shotDetector) {
  if (shotDetector->hitCount >= 2) {
       //Display winner
       gameOver = true;  
       winner = shotDetector;
  }
}

void HandleShotDetection(ShotDetector *shotDetector) {
    //Determine if a shot was detected
    if (WasShotDetected (shotDetector)) {
      Serial.println("Shot detected.");
      
      shotDetector->hitCount += 1;
      //turn on the shot detection light
      digitalWrite(shotDetector->hitLedPin, HIGH);
      Serial.println("Turned on Hit LED on pin " + String(shotDetector->hitLedPin, DEC));
  
      //set the display shot detected flag so the light will come on
      shotDetector->displayingShotDetected = true;
      
      //Set the shot detetected time so that we can compare during the loop
      shotDetector->shotDetectedTime = millis();
      
      if (shotDetector->hitCount >= 3)
      {
      Serial.println("Found winner with hit count " + String(shotDetector->hitLedPin, DEC));
        //winnerDetected = true;
        winner = shotDetector;
        gameOver = true;
      }
    }
}

void HandleShotDisplay(ShotDetector *shotDetector) {
    //Determine if we're displaying the shot detected, if so, determine when we can shut it off
    if (shotDetector->displayingShotDetected)
    {
      //Serial.println("Displaying shot detected." + String(millis(), DEC) + "> " + String(shotDetectedTime, DEC));
      if (millis() > shotDetector->shotDetectedTime + 500)
      {
        digitalWrite(shotDetector->hitLedPin, LOW);
        shotDetector->displayingShotDetected = false;
      }
    }
}

//Determine if a shot was detected by reading and setting a detectors active state
bool WasShotDetected (ShotDetector *shotDetector) {
  
  //Wait 50 milliseconds in order to debouce so we don't get noise (duplicate reads)
  if (shotDetector->currentState == shotDetectorStateDeactivated || (shotDetector->currentState == shotDetectorStateActivated && millis() > shotDetector->lastStateReadTime + 50)) {
     
      //Currently deactivated, or activated and waited longer than 50 milliseonds, so take another sample of the dector pin
      int shotDetectorPinState = digitalRead(shotDetector->readPin);
      
      if (shotDetectorPinState == HIGH)   {  
        Serial.println("Hit [" + String(shotDetectorPinState, DEC) + "] at + " + String(millis(), DEC)); 
      }
      //Mark right now as the last read time of the detector
      shotDetector->lastStateReadTime = millis();
      
      //Check to see if the shot detector pin state
      if (shotDetectorPinState == HIGH)   {       
        shotDetector->currentState = shotDetectorStateActivated;
      } else {
        shotDetector->currentState = shotDetectorStateDeactivated;
      }
      
      //Check to see if the current state is different then the previous state
      if (shotDetector->currentState != shotDetector->previousState) {
        //It is different, so check to see if it now deactivated, meaning it was previously activated, and now not, so an activation occurred (i.e. Shot Detected)
        if (shotDetector->currentState == shotDetectorStateDeactivated) {
          shotDetector->previousState = shotDetector->currentState;
          return true;
        } else {
          shotDetector->previousState = shotDetector->currentState;
        }
      }
  }
  
  //Got this far in code with no shot detected, so return with no shot detected.
  return false;
}
