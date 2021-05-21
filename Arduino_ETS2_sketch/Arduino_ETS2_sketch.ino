#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Servo.h>

//-------------Номери пінів-------------------
#define LEFT_TURN_PIN     28 //Лівий покажчик повороту
#define RIGHT_TURN_PIN    30 //Правий покажчик повороту
#define HIGHBEAM_PIN      32 //Покажчик дального світла фар
#define HANDBRAKE_PIN     34 //Покажчик ручного гальма
#define CHECK_ENGINE_PIN  36 //Покажчик несправності двигуна
#define LOWBEAM_PIN       38 //Покажчик ближнього світла фар
#define BATTERY_LOW_PIN   40 //Покажчик розрядженого акумулятора
#define OIL_LOW_PIN       42 //Покажчик низького рівня мастила
#define AIR_LOW_PIN       44 //Покажчик низкього тиску в гальмах
#define SPEEDOMETER_PIN   A1 //Пін для спідометру
#define TACHOMETER_PIN    A0 //Пін для тахометра
#define POTENTIOMETER_PIN A4
//--------------------------------------------

Servo speedometer;
Servo tachometer;

//-----{ Display }-----
//enum for checking current page
enum Pages{
  PAGE_SPEED,
  PAGE_TEMPS,
  PAGE_PRES,
  PAGE_CONS,
  MAX_PAGES
};

LiquidCrystal_I2C m_lcd(0x27, 20, 4);
Pages m_currentPage;
int potent; 
//---------------------

//-----Константи та змінні для роботи с СОМ-портом і пакетами даних-----
const uint8_t PACKET_SIZE       { 18 };   //Розмір пакету даних
const uint8_t PACKET_SYNC_FIRST { 126 };  //Перший розряд синхронізації
const uint8_t PACKET_SYNC_SECOND{ 0 };    //Другий розряд синхронізації
const uint8_t MAX_SPEED         { 160 };  //Максимальна швидкість
const uint16_t MAX_RPM          { 400 };  //Максимальні оберти двигуна
byte packet[PACKET_SIZE];                 //Пакет даних

//Перерахування для ітерації по пакету
enum PacketInfo {
    SYNC_FIRST,
    FLAGS_FIRST,
    FLAGS_SECOND,
    GEAR,
    AIR_PRESSURE_WHOLE,
    OIL_PRESSURE_WHOLE,
    OIL_TEMPERATURE,
    WATER_TEMPERATURE,
    SPEED,
    RPM_HIGH,
    RPM_LOW,
    FUEL_INSTANT_CONS_WHOLE,
    FUEL_INSTANT_CONS_FRACTIONAL,
    FUEL_AVG_CONS_WHOLE,
    FUEL_AVG_CONS_FRACTIONAL
  };
  //Прапори першого байту прапорів
  enum FirstByteFlags {
    ELECTRIC_ENABLED = 1,
    ENGINE_ENABLED = 2,
    LEFT_TURN = 4,
    RIGHT_TURN = 8,
    LIGHTS_BEAM_LOW = 16,
    LIGHTS_BEAM_HIGH = 32,
    BATTERY_VOLTAGE_WARNING = 64,
    AIR_PRESSURE_WARNING = 128
  };

  //Прапори другого байту прапорів
  enum SecondByteFlags {
    AIR_PRESSURE_EMERGENCY = 1,
    OIL_PRESSURE_WARNING = 2,
    WATER_TEMPERATURE_WARNING = 4,
    GEAR_SIGN = 8,
    HANDBRAKE = 16,
    CRUISE_ENABLED = 32
  };
//--------------------------------------------

//функція конвертації цілої і дробової частини у float
float to_float(uint8_t wholePart, uint8_t fractionalPart){
  float ret = wholePart;
  return ret + (fractionalPart / 10);
}
//TODO: check this code
uint16_t to_int16(uint8_t firstPart, uint8_t secondPart){
  return (firstPart << 8) + secondPart;
}

void writeSpeed(uint8_t speed){
  constrain(speed, 0, MAX_SPEED);
  speedometer.write(map(MAX_SPEED - speed, 0, MAX_SPEED, 0, 180));
}

void writeRPM(uint16_t RPM){
  constrain(RPM, 0, MAX_RPM);
  tachometer.write(map(MAX_RPM - RPM, 0, MAX_RPM, 0, 180));
}

void setup() {
  //Ініціалізація
  Serial.begin(115200);
  pinMode(RIGHT_TURN_PIN, OUTPUT);
  pinMode(LEFT_TURN_PIN, OUTPUT);
  pinMode(HIGHBEAM_PIN, OUTPUT);
  pinMode(LOWBEAM_PIN, OUTPUT);
  pinMode(HANDBRAKE_PIN, OUTPUT);
  pinMode(CHECK_ENGINE_PIN, OUTPUT);
  pinMode(BATTERY_LOW_PIN, OUTPUT);
  pinMode(OIL_LOW_PIN, OUTPUT);
  pinMode(AIR_LOW_PIN, OUTPUT);
  pinMode(POTENTIOMETER_PIN, INPUT);
  speedometer.attach(SPEEDOMETER_PIN);
  tachometer.attach(TACHOMETER_PIN);
  writeSpeed(0);
  writeRPM(0);
  digitalWrite(RIGHT_TURN_PIN, HIGH);
  digitalWrite(LEFT_TURN_PIN, HIGH);
  digitalWrite(HIGHBEAM_PIN, HIGH);
  digitalWrite(LOWBEAM_PIN, HIGH);
  digitalWrite(HANDBRAKE_PIN, HIGH);
  digitalWrite(CHECK_ENGINE_PIN, HIGH);
  digitalWrite(BATTERY_LOW_PIN, HIGH);
  digitalWrite(OIL_LOW_PIN, HIGH);
  digitalWrite(AIR_LOW_PIN, HIGH);
  delay(1000);
  writeSpeed(MAX_SPEED);
  writeRPM(MAX_RPM);
  delay(1000);
  writeSpeed(0);
  writeRPM(0);
  delay(100);
  digitalWrite(RIGHT_TURN_PIN, LOW);
  digitalWrite(LEFT_TURN_PIN, LOW);
  digitalWrite(HIGHBEAM_PIN, LOW);
  digitalWrite(LOWBEAM_PIN, LOW);
  digitalWrite(HANDBRAKE_PIN, LOW);
  digitalWrite(CHECK_ENGINE_PIN, LOW);
  digitalWrite(BATTERY_LOW_PIN, LOW);
  digitalWrite(OIL_LOW_PIN, LOW);
  digitalWrite(AIR_LOW_PIN, LOW);
}

void loop() {
}

void serialEvent() {
  if(Serial.available()) {
    Serial.readBytes(packet, PACKET_SIZE);
    digitalWrite(13, HIGH);
    if(packet[SYNC_FIRST] == PACKET_SYNC_FIRST){
        writeSpeed(packet[SPEED]);
        writeRPM(to_int16(packet[RPM_HIGH], packet[RPM_LOW]));
        digitalWrite(LEFT_TURN_PIN, packet[FLAGS_FIRST] & LEFT_TURN);
        digitalWrite(RIGHT_TURN_PIN, packet[FLAGS_FIRST] & RIGHT_TURN);
        digitalWrite(LOWBEAM_PIN, packet[FLAGS_FIRST] & LIGHTS_BEAM_LOW);
        digitalWrite(HIGHBEAM_PIN, packet[FLAGS_FIRST] & LIGHTS_BEAM_HIGH & (LIGHTS_BEAM_LOW << 1));
        digitalWrite(HANDBRAKE_PIN, packet[FLAGS_SECOND] & HANDBRAKE);
        digitalWrite(BATTERY_LOW_PIN, packet[FLAGS_FIRST] & BATTERY_VOLTAGE_WARNING);
        digitalWrite(OIL_LOW_PIN, packet[FLAGS_SECOND] & OIL_PRESSURE_WARNING);
        digitalWrite(AIR_LOW_PIN, packet[FLAGS_FIRST] & AIR_PRESSURE_WARNING);
      }
 }
}