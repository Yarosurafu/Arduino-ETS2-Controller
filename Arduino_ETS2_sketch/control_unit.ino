#include <HID-Project.h>
#include <HID-Settings.h>

//#define DEBUG

#ifdef DEBUG
int gLeftTurnState = 0;
int gRightTurnState = 0;
int gHighBeamSignalState = 0;
#endif

// -----{ Control buttons }-----
// b- prefix means BUTTON
// s- prefix means SWITCH
const int kbLeftTurnPin = 9;
const int kbRightTurnPin = 10;
const int kbStartEnginePin = 0;         // Not connected yet
const int kbHighbeamSignalPin = 7;
const int kbEmergencyButtonPin = 0;     // Not connected yet
const int ksWipersPin = 11;
const int ksLowBeamPin = 0;             // Not connected yet
const int ksHighBeamPin = 0;            // Not connected yet

enum Buttons {
    LEFT_TURN_BUTTON = 1,
    RIGHT_TURN_BUTTON,
    START_ENGINE_BUTTON,
    HIGHBEAM_BUTTON,
    LOWBEAM_BUTTON,
    HIGHBEAM_SIGNAL_BUTTON,
    EMERGENCY_BUTTON,
    WIPERS_BUTTON
};
// -----------------------------

// -----{ Encoder data }-----
const int kEncoderMin = -750;
const int kEncoderMax = 750;
const int kEncoderPinA = 2;                     // Interrupt pin
const int kEncoderPinB = 3;                     // Interrupt pin
volatile const long kEncoderPauseMicros = 50;   // Pause for avoiding bounce
volatile long gEncoderLastTurnTime = 0;         // Last change in micros
volatile int gEncoderCount = 0;                 // Turn counter
volatile int gEncoderTurnState = 0; // Turn state - [0;4] right, [-4;0] left
volatile int kEncoderPinAValue = 0; // 'A' bus state
volatile int kEncoderPinBValue = 0; // 'B' bus state

volatile unsigned long gResetLastPress = 0;
volatile bool gIsResetPressed = false;
// ----------------------------

// -----{ Pedals }-----
const int kAcceleratorPin = A0;
int gAcceleratorValue = 0;
const int kBrakePin = A1;
int gBrakeValue = 0;
const int kClutchPin = 0;      // Not connected yet
// --------------------

void setup()
{
    pinMode(kbLeftTurnPin, INPUT_PULLUP);
    pinMode(kbRightTurnPin, INPUT_PULLUP);
    pinMode(kbHighbeamSignalPin, INPUT_PULLUP);
    pinMode(kAcceleratorPin, INPUT);
    pinMode(kBrakePin, INPUT);

    pinMode(kEncoderPinA, INPUT_PULLUP);
    pinMode(kEncoderPinB, INPUT_PULLUP);
    digitalWrite(kEncoderPinA, HIGH);
    digitalWrite(kEncoderPinB, HIGH);

    attachInterrupt(1, checkEncoderPinA, CHANGE);
    attachInterrupt(0, checkEncoderPinB, CHANGE);
    attachInterrupt(4, highBeamButton, CHANGE);

#ifdef DEBUG
    Serial.begin(9600);
#endif

    Gamepad.begin();
}

void loop()
{
    if(gIsResetPressed && (millis() - gResetLastPress > 10000)) {
        gEncoderCount = 0;
    }

    setButton(LEFT_TURN_BUTTON, !digitalRead(kbLeftTurnPin));
    setButton(RIGHT_TURN_BUTTON, !digitalRead(kbRightTurnPin));
    setButton(HIGHBEAM_SIGNAL_BUTTON, !digitalRead(kbHighbeamSignalPin));

    // Accelerator pedal
    gAcceleratorValue = analogRead(kAcceleratorPin);
    gAcceleratorValue = constrain(gAcceleratorValue, 177, 940);
    Gamepad.zAxis(map(gAcceleratorValue, 177, 940, -128, 127));

    //Brake pedal
    gBrakeValue = analogRead(kBrakePin);
    gBrakeValue = constrain(gBrakeValue, 2, 765);
    gBrakeValue = 767 - gBrakeValue;
    Gamepad.rzAxis(map(gBrakeValue, 2, 765, -128, 127));

    gEncoderCount = constrain(gEncoderCount, kEncoderMin, kEncoderMax);
    Gamepad.xAxis(map(gEncoderCount, kEncoderMin, kEncoderMax, -32768, 32767));
    Gamepad.write();

#ifdef DEBUG
    gLeftTurnState = !digitalRead(kbLeftTurnPin);
    gRightTurnState = !digitalRead(kbRightTurnPin);
    gHighBeamSignalState = !digitalRead(kbHighbeamSignalPin);

    Serial.print("\n---------------------\n");
    Serial.print("gEncoderCount == ");
    Serial.println(gEncoderCount);

    Serial.print("Left turn == ");
    Serial.println(gLeftTurnState);

    Serial.print("Right turn == ");
    Serial.println(gRightTurnState);

    Serial.print("Highbeam signal == ");
    Serial.println(gHighBeamSignalState);
    Serial.print("---------------------");
#endif
}

void setButton(uint8_t button, int signal) {
    if(signal)
        Gamepad.press(button);
    else
        Gamepad.release(button);
}

void highBeamButton() {
    cli();
    if(!digitalRead(kbHighbeamSignalPin)) {
        gResetLastPress = millis();
        gIsResetPressed = true;
    }
    else {
        gIsResetPressed = false;
    }
    sei();
}

void checkEncoderPinA()
{
    if (micros() - gEncoderLastTurnTime < kEncoderPauseMicros)
        return;
    
    kEncoderPinAValue = digitalRead(kEncoderPinA);
    kEncoderPinBValue = digitalRead(kEncoderPinB);

    cli();
    if (gEncoderTurnState == 0 && !kEncoderPinAValue && kEncoderPinBValue || gEncoderTurnState == 2 && kEncoderPinAValue && !kEncoderPinBValue)
    {
        ++gEncoderTurnState;
        gEncoderLastTurnTime = micros();
    }
    if (gEncoderTurnState == -1 && !kEncoderPinAValue && !kEncoderPinBValue || gEncoderTurnState == -3 && kEncoderPinAValue && kEncoderPinBValue)
    {
        --gEncoderTurnState;
        gEncoderLastTurnTime = micros();
    }
    setCount();
    sei();

    if (kEncoderPinAValue && kEncoderPinBValue && gEncoderTurnState != 0)
        gEncoderTurnState = 0;
}
void checkEncoderPinB()
{
    if (micros() - gEncoderLastTurnTime < kEncoderPauseMicros)
        return;
    kEncoderPinAValue = digitalRead(kEncoderPinA);
    kEncoderPinBValue = digitalRead(kEncoderPinB);

    cli();
    if (gEncoderTurnState == 1 && !kEncoderPinAValue && !kEncoderPinBValue || gEncoderTurnState == 3 && kEncoderPinAValue && kEncoderPinBValue)
    {
        ++gEncoderTurnState;
        gEncoderLastTurnTime = micros();
    }
    if (gEncoderTurnState == 0 && kEncoderPinAValue && !kEncoderPinBValue || gEncoderTurnState == -2 && !kEncoderPinAValue && kEncoderPinBValue)
    {
        --gEncoderTurnState;
        gEncoderLastTurnTime = micros();
    }
    setCount();
    sei();

    if (kEncoderPinAValue && kEncoderPinBValue && gEncoderTurnState != 0)
        gEncoderTurnState = 0;
}

/**
 * @brief Set gEncoderCount if rotate-cycle was finished 
 */
void setCount()
{
    if (gEncoderTurnState == 4 || gEncoderTurnState == -4)
    {
        gEncoderCount += (int)(gEncoderTurnState / 4);
        gEncoderLastTurnTime = micros();
    }
}
