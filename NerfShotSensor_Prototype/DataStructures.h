#include <Arduino.h>
struct ShotDetector {
  byte previousState;
  byte currentState;
  long lastStateReadTime;
  int readPin;
  int hitLedPin;
  int hitCount;
  long shotDetectedTime;
  bool displayingShotDetected;
  int playerNumber;
};

