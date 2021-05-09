#include <HID-Project.h>
#include <HID-Settings.h>

// -----{ Encoder data }-----
<<<<<<< HEAD
const int kEncoderPinA = 2;             // Пины прерываний
const int kEncoderPinB = 3;             // Пины прерываний
volatile long kEncoderPauseMicros = 50; // Пауза для борьбы с дребезгом
volatile long gEncoderLastTurnTime = 0; // Переменная для хранения времени последнего изменения
volatile int gEncoderCount = 0;         // Счетчик оборотов

#ifdef DEBUG
int actualcount = 0; // Временная переменная определяющая изменение основного счетчика
#endif

volatile int gEncoderTurnState = 0; // Статус одного шага - от 0 до 4 в одну сторону, от 0 до -4 - в другую
volatile int kEncoderPinAValue = 0; // Переменные хранящие состояние пина, для экономии времени
volatile int kEncoderPinBValue = 0; // Переменные хранящие состояние пина, для экономии времени
=======
const int       kEncoderPinA = 2; // Пины прерываний
const int       kEncoderPinB = 3; // Пины прерываний
volatile long   kEncoderPauseMicros = 50;   // Пауза для борьбы с дребезгом
volatile long   gEncoderLastTurnTime = 0; // Переменная для хранения времени последнего изменения
volatile int    gEncoderCount = 0; // Счетчик оборотов
int             actualcount = 0;    // Временная переменная определяющая изменение основного счетчика
volatile int    gEncoderTurnState = 0; // Статус одного шага - от 0 до 4 в одну сторону, от 0 до -4 - в другую
volatile int    kEncoderPinAValue = 0; // Переменные хранящие состояние пина, для экономии времени
volatile int    kEncoderPinBValue = 0; // Переменные хранящие состояние пина, для экономии времени
>>>>>>> 58898ab4dbdf4199415e0bdb65b5e42440e8c783
// ----------------------------

void setup()
{
    pinMode(kEncoderPinA, INPUT_PULLUP); // Пины в режим приема INPUT
    pinMode(kEncoderPinB, INPUT_PULLUP);
    digitalWrite(kEncoderPinA, HIGH);
    digitalWrite(kEncoderPinB, HIGH); // Пины в режим приема INPUT

    attachInterrupt(1, checkEncoderPinA, CHANGE); // Настраиваем обработчик прерываний по изменению сигнала
    attachInterrupt(0, checkEncoderPinB, CHANGE); // Настраиваем обработчик прерываний по изменению сигнала

<<<<<<< HEAD
#ifdef DEBUG
    Serial.begin(9600); // Включаем Serial
#endif

=======
    Serial.begin(9600); // Включаем Serial
>>>>>>> 58898ab4dbdf4199415e0bdb65b5e42440e8c783
    Gamepad.begin();
}

void loop()
{
<<<<<<< HEAD

#ifdef DEBUG
    if (actualcount != gEncoderCount)
    {                                // Чтобы не загружать ненужным выводом в Serial, выводим состояние
        actualcount = gEncoderCount; // счетчика только в момент изменения
        Serial.println(actualcount);
    }
#endif

=======
    if (actualcount != gEncoderCount)
    {                        // Чтобы не загружать ненужным выводом в Serial, выводим состояние
        actualcount = gEncoderCount; // счетчика только в момент изменения
        Serial.println(actualcount);
    }
>>>>>>> 58898ab4dbdf4199415e0bdb65b5e42440e8c783
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
<<<<<<< HEAD
    sei();      // Разрешаем обработку прерываний
=======
    sei();           // Разрешаем обработку прерываний
>>>>>>> 58898ab4dbdf4199415e0bdb65b5e42440e8c783

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

void setCount()
{ // Устанавливаем значение счетчика
    if (gEncoderTurnState == 4 || gEncoderTurnState == -4)
<<<<<<< HEAD
    {                                                  // Если переменная gEncoderTurnState приняла заданное значение приращения
=======
    {                              // Если переменная gEncoderTurnState приняла заданное значение приращения
>>>>>>> 58898ab4dbdf4199415e0bdb65b5e42440e8c783
        gEncoderCount += (int)(gEncoderTurnState / 4); // Увеличиваем/уменьшаем счетчик
        gEncoderCount = constrain(gEncoderCount, -900, 900);
        Gamepad.xAxis(map(gEncoderCount, -900, 900, -32768, 32767));
        Gamepad.write();
<<<<<<< HEAD
        gEncoderLastTurnTime = micros(); // Запоминаем последнее изменение
=======
        gEncoderLastTurnTime = micros();       // Запоминаем последнее изменение
>>>>>>> 58898ab4dbdf4199415e0bdb65b5e42440e8c783
    }
}