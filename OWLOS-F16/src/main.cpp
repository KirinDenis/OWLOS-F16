#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include <Ticker.h>

void IRAM_ATTR onMotionDetectHandler();

volatile int prevM = -1;

#define LIGHT_OFF_PWM 1024
#define LIGHT_ON_PWM 0
#define LIGHT_OFF_CHANGE_STEP_PWM 1
#define LIGHT_ON_CHANGE_STEP_PWM 10

// https://github.com/khoih-prog/ESP8266TimerInterrupt/blob/master/src/ESP8266TimerInterrupt.h
#define TIM_DIV1_CLOCK (80000000UL) // 80000000 / 1   = 80.0  MHz
#define TIM_DIV16_CLOCK (5000000UL) // 80000000 / 16  = 5.0   MHz
#define TIM_DIV256_CLOCK (312500UL) // 80000000 / 256 = 312.5 KHz

#define MOTION_INTERVAL 5

volatile int currentLightPWM = LIGHT_ON_PWM;

volatile unsigned long lastMotionChange = 0;

Ticker ticker;

void IRAM_ATTR onMotionDetectHandler()
{

  lastMotionChange = millis();
}

void IRAM_ATTR onTimerHandler()
{

  noInterrupts();

  if (lastMotionChange + MOTION_INTERVAL * 1000 < millis())
  {
    // light off
    if (currentLightPWM < LIGHT_OFF_PWM)
    {
      currentLightPWM += LIGHT_OFF_CHANGE_STEP_PWM;
      analogWrite(D1, currentLightPWM);
    }
  }
  else
  {
    // light on
    if (currentLightPWM > LIGHT_ON_PWM)
    {
      currentLightPWM -= LIGHT_ON_CHANGE_STEP_PWM;
      analogWrite(D1, currentLightPWM);
    }
  }

  interrupts();
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("OWLOS-F16");

  analogWriteRange(1000); // Range from 0 to 1000
  analogWriteFreq(4000);  // 4000 Gz
  pinMode(D7, INPUT);
  pinMode(A0, INPUT);
  pinMode(D1, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(D7), onMotionDetectHandler, CHANGE);
  ticker.attach(0.01, onTimerHandler);
}
void loop()
{
  //PWM Pin debug
  if (Serial.available())
  {
    int i = (Serial.readString()).toInt();
    analogWrite(D1, i);
  }

  // int a = analogRead(A0); //FFR

  delay(1000);
}