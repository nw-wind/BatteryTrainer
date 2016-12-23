/*
** Реле на пин 2
** Кнопка на пин 3
** Меряем напряжение на аналоговом пине 0
** Амперметр на аралоговый порт 1
** Переключаем на заряд при напряжении 10.8В
** Не хотим работать при напряжении меньше 5В
** 
** При включении работает зарядка. 
** Кнопка переключает режим заряд/разряд.
** Разряд завершается при достижении 10.8B и начинается заряд.
*/


/*
   Todo:
   Считать время заряда и разряда.
   Если лампа перегорает -- на зарядку.
*/


#define RELAYPIN  2
#define BUTTONPIN 3
#define POWERPIN  0
#define AMPERPIN  1
#define LIMIT     10.8f
#define UPLIMIT   14.5f
#define BADPOWER  5.0f
#define CIRCLES   3

#define SWITCH_LAMP     0
#define SWITCH_CHARGER  1

// Секунд на вывод инфы
#define TICKS     (60*5)

void setup() {
  pinMode(RELAYPIN, OUTPUT);
  pinMode(BUTTONPIN, INPUT_PULLUP);
  digitalWrite(RELAYPIN, HIGH);
  Serial.begin(9600);
  Serial.println("Start...");
}

int mode = SWITCH_CHARGER;
int pressed = 0;
int tick = 0;
unsigned long start = 0;
int circle = 1;

float chargeAh = 0.0;
unsigned long chargeMs = 0;
float dischargeAh = 0.0;
unsigned long dischargeMs = 0;

void loop() {
  // Закончили
  if (circle > CIRCLES) return;
  // Проверяем напряжение.
  float v = analogRead(POWERPIN) / 1023.0f * 3.0f * 5.0f;
  // Измеряем ток
  float a = ((analogRead(AMPERPIN) * (5.0 / 1024.0)) - 2.5) / 0.1;
  // Заряжаем, если что-то не так с зарядкой.
  if (v > BADPOWER) {
    // Идёт зарядка, ожидаем кнопку.
    if (mode == SWITCH_CHARGER) {
      chargeAh += a * (millis() - chargeMs) / 3600000.0;
      chargeMs = millis();
      if (digitalRead(BUTTONPIN) == LOW || v > UPLIMIT && circle <= CIRCLES) {
        mode = SWITCH_LAMP;
        digitalWrite(RELAYPIN, LOW); // Переключаем на лампочку.
        dischargeMs = millis();
        Serial.print("Try "); Serial.println(circle);
        Serial.print("Charge was "); Serial.print(chargeAh); Serial.println("Ah");
        dischargeAh = 0.0;
        Serial.println("LETS LIGHT!");
        delay(100);
        pressed = 1;
        circle++;
        if (circle > CIRCLES) Serial.println("Thats enough!");
      }
    }
    // Идёт разряд, ожидаем падение напряжения.
    if (mode == SWITCH_LAMP) {
      dischargeAh += a * (millis() - dischargeMs) / 3600000.0;
      dischargeMs = millis();
      // Достигли дна по напряжению.
      if (v < LIMIT) {
        mode = SWITCH_CHARGER;
        digitalWrite(RELAYPIN, HIGH); // Переключаем на зарядник.
        start = millis();
        Serial.println("TIME TO SUCK ELECTRICITY!");
        unsigned long msec = millis() - start;
        Serial.print("Discharge time ");
        Serial.print(msec / 3600000); Serial.print("h ");
        Serial.print((msec % 3600000) / 60000); Serial.print("m ");
        Serial.print((msec % 60000) / 1000); Serial.print("s ");
        Serial.println();
        Serial.print("Appoximate capacity "); Serial.print(55 / 12);
        Serial.println("Ah");
        Serial.print("Disharge was "); Serial.print(dischargeAh); Serial.println("Ah");
        chargeAh = 0.0;
      }
      // Переключить на зарядку принудительно.
      if (pressed == 0 && digitalRead(BUTTONPIN) == LOW) {
        mode = SWITCH_CHARGER;
        digitalWrite(RELAYPIN, HIGH); // Переключаем на зарядник.
        chargeMs = millis();
        Serial.println("SUCK THE ELECTRICITY!");
        Serial.print("Disharge was "); Serial.print(-dischargeAh); Serial.println("Ah");
        chargeAh = 0.0;
        if (circle > CIRCLES) Serial.println("LAST CHARGING!");
        delay(100);
      }
    }
  } else {
    Serial.println("BAD POWER!");
  }
  if (tick == 0) {
    Serial.print(" Disharge = "); Serial.print(-dischargeAh);
    Serial.print(" Charge = "); Serial.print(chargeAh);
    Serial.print(" Current = "); Serial.print(a);
    Serial.print(" Voltage = "); Serial.println(v);
  }
  pressed = 0;
  tick = ++tick % TICKS;
  delay(1000); // Не спешим
}
