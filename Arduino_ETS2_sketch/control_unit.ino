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
const int kAcceleratorPin = 0; // Not connected yet
const int kBrakePin = 0;       // Not connected yet
const int kClutchPin = 0;      // Not connected yet
// --------------------

void setup()
{
    pinMode(kbLeftTurnPin, INPUT_PULLUP);
    pinMode(kbRightTurnPin, INPUT_PULLUP);
    pinMode(kbHighbeamSignalPin, INPUT_PULLUP);

    pinMode(kEncoderPinA, INPUT_PULLUP); // Пины в режим приема INPUT
    pinMode(kEncoderPinB, INPUT_PULLUP);
    digitalWrite(kEncoderPinA, HIGH);
    digitalWrite(kEncoderPinB, HIGH); // Пины в режим приема INPUT

    attachInterrupt(1, checkEncoderPinA, CHANGE); // Настраиваем обработчик прерываний по изменению сигнала
    attachInterrupt(0, checkEncoderPinB, CHANGE); // Настраиваем обработчик прерываний по изменению сигнала
    attachInterrupt(4, highBeamButton, CHANGE);

#ifdef DEBUG
    Serial.begin(9600); // Включаем Serial
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

    gEncoderCount = constrain(gEncoderCount, -900, 900);
    Gamepad.xAxis(map(gEncoderCount, -900, 900, -32768, 32767));
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
    
    //Gamepad.write();
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
        return; // Если с момента последнего изменения состояния не прошло
    // достаточно времени - выходим из прерывания
    kEncoderPinAValue = digitalRead(kEncoderPinA); // Получаем состояние пинов A и B
    kEncoderPinBValue = digitalRead(kEncoderPinB);

    cli(); // Запрещаем обработку прерываний, чтобы не отвлекаться
    if (gEncoderTurnState == 0 && !kEncoderPinAValue && kEncoderPinBValue || gEncoderTurnState == 2 && kEncoderPinAValue && !kEncoderPinBValue)
    {
        ++gEncoderTurnState; // Если выполняется условие, наращиваем переменную gEncoderTurnState
        gEncoderLastTurnTime = micros();
    }
    if (gEncoderTurnState == -1 && !kEncoderPinAValue && !kEncoderPinBValue || gEncoderTurnState == -3 && kEncoderPinAValue && kEncoderPinBValue)
    {
        --gEncoderTurnState; // Если выполняется условие, наращиваем в минус переменную gEncoderTurnState
        gEncoderLastTurnTime = micros();
    }
    setCount(); // Проверяем не было ли полного шага из 4 изменений сигналов (2 импульсов)
    sei();      // Разрешаем обработку прерываний

    if (kEncoderPinAValue && kEncoderPinBValue && gEncoderTurnState != 0)
        gEncoderTurnState = 0; // Если что-то пошло не так, возвращаем статус в исходное состояние
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
        ++gEncoderTurnState; // Если выполняется условие, наращиваем переменную gEncoderTurnState
        gEncoderLastTurnTime = micros();
    }
    if (gEncoderTurnState == 0 && kEncoderPinAValue && !kEncoderPinBValue || gEncoderTurnState == -2 && !kEncoderPinAValue && kEncoderPinBValue)
    {
        --gEncoderTurnState; // Если выполняется условие, наращиваем в минус переменную gEncoderTurnState
        gEncoderLastTurnTime = micros();
    }
    setCount(); // Проверяем не было ли полного шага из 4 изменений сигналов (2 импульсов)
    sei();

    if (kEncoderPinAValue && kEncoderPinBValue && gEncoderTurnState != 0)
        gEncoderTurnState = 0; // Если что-то пошло не так, возвращаем статус в исходное состояние
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