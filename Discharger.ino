/*
** Реле на пин 2
** Кнопка на пин 3
** Меряем напряжение на аналоговом пине 0
** Переключаем на заряд при напряжении 10.8В
** Не хотим работать при напряжении меньше 5В
** 
** При включении работает зарядка. 
** Кнопка переключает режим заряд/разряд.
** Разряд завершается при достижении 10.8B и начинается заряд.
*/


#define RELAYPIN  2
#define BUTTONPIN 3
#define POWERPIN  0
#define LIMIT     10.8f
#define BADPOWER  5.0f

#define SWITCH_LAMP     0
#define SWITCH_CHARGER  1

void setup() {
  pinMode(RELAYPIN, OUTPUT);
  pinMode(BUTTONPIN, INPUT_PULLUP);
  digitalWrite(RELAYPIN, HIGH);
  Serial.begin(9600);
  Serial.println("START!");
}

int mode = SWITCH_CHARGER;
int pressed=0;

void loop() {
  // Проверяем напряжение.
  float v = analogRead(POWERPIN) / 1023.0f * 3.0f * 5.0f;
  // Заряжаем, если что-то не так с зарядкой.
  if (v > BADPOWER) {
    // Идёт зарядка, ожидаем кнопку.
    if (mode == SWITCH_CHARGER) {
      if (digitalRead(BUTTONPIN) == LOW) {
        mode = SWITCH_LAMP;
        digitalWrite(RELAYPIN, LOW); // Переключаем на лампочку.
        Serial.println("LETS LIGHT!");
        delay(100);
        pressed=1;
      }
    }
    // Идёт разряд, ожидаем падение напряжения.
    if (mode == SWITCH_LAMP) {
      // Достигли дна по напряжению.
      if (v < LIMIT) {
        mode = SWITCH_CHARGER;
        digitalWrite(RELAYPIN, HIGH); // Переключаем на зарядник.
        Serial.println("TIME TO SUCK ELECTRICITY!");
      }
      // Переключить на зарядку принудительно.
      if (pressed==0 && digitalRead(BUTTONPIN) == LOW) {
        mode = SWITCH_CHARGER;
        digitalWrite(RELAYPIN, HIGH); // Переключаем на зарядник.
        Serial.println("SUCK THE ELECTRICITY!");
        delay(100);
      }
    }
  } else {
    Serial.println("BAD POWER!");
  }
  Serial.println(v);
  pressed=0;
  delay(1000); // Не спешим
}
