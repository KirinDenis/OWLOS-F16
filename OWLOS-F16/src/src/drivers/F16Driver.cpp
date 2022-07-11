/* ----------------------------------------------------------------------------
Ready IoT Solution - OWLOS
Copyright 2022 by:
- Denis Kirin (deniskirinacs@gmail.com)

This file is part of Ready IoT Solution - OWLOS

OWLOS is free software : you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

OWLOS is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with OWLOS. If not, see < https://www.gnu.org/licenses/>.

GitHub: https://github.com/KirinDenis/owlos

(Этот файл — часть Ready IoT Solution - OWLOS.

OWLOS - свободная программа: вы можете перераспространять ее и/или изменять
ее на условиях Стандартной общественной лицензии GNU в том виде, в каком она
была опубликована Фондом свободного программного обеспечения; версии 3
лицензии, любой более поздней версии.

OWLOS распространяется в надежде, что она будет полезной, но БЕЗО ВСЯКИХ
ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА или ПРИГОДНОСТИ ДЛЯ
ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ.
Подробнее см.в Стандартной общественной лицензии GNU.

Вы должны были получить копию Стандартной общественной лицензии GNU вместе с
этой программой. Если это не так, см. <https://www.gnu.org/licenses/>.)
--------------------------------------------------------------------------------------*/

#include "F16Driver.h"
#ifdef USE_F16_DRIVER
#define DRIVER_MASK "F16Driver"
#define DRIVER_ID "F16"

#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include <Ticker.h>

void IRAM_ATTR onMotionDetectHandler();

volatile int prevM = -1;

#define LIGHT_OFF_PWM 1024
#define LIGHT_ON_PWM 0
#define LIGHT_OFF_CHANGE_STEP_PWM 1
#define LIGHT_ON_CHANGE_STEP_PWM 10
#define NO_MOTION_LIGHT 3

#define LIGHT_SENSOR_BRIGHT 830
#define LIGHT_SENSOR_DARK 1024
#define LIGHT_SENSOR_RANGE (LIGHT_SENSOR_DARK - LIGHT_SENSOR_BRIGHT)

#define ANTI_LIGHT_BOUNCE 100

// https://github.com/khoih-prog/ESP8266TimerInterrupt/blob/master/src/ESP8266TimerInterrupt.h
#define TIM_DIV1_CLOCK (80000000UL) // 80000000 / 1   = 80.0  MHz
#define TIM_DIV16_CLOCK (5000000UL) // 80000000 / 16  = 5.0   MHz
#define TIM_DIV256_CLOCK (312500UL) // 80000000 / 256 = 312.5 KHz

#define MOTION_INTERVAL 5

volatile int currentLightPWM = LIGHT_ON_PWM;
volatile int lightPWMHistory[4];
volatile int pwm = 0;
volatile int lastLightSensor = 0;
volatile int lightBalance = 0;
volatile bool beep = true;

volatile unsigned long lastMotionChange = 0;

//-------------------
volatile byte mLightBright = 50;
volatile byte nmLightBright = 0;
volatile byte mLightDark = 100;
volatile byte nmLightDark = 20;

Ticker ticker;

// Percent to PWM
// LIGHT_ON_PWM  = 100% (default value 0)
// LIGHT_OFF_PWM = 0%  (default value 1024)
uint16_t PercentToPWM(uint8_t percent)
{
  if (percent > 100)
  {
    percent = 100;
  }
  return LIGHT_OFF_PWM - (LIGHT_OFF_PWM / 100 * percent);
}

// Light sensor value(data) to percent
// LIGHT_SENSOR_BRIGHT - 100% (default value 830)
// LIGHT_SENSOR_DARK   - 0%   (default value 1024)
uint8_t LStoPercent(uint16_t value)
{
  if (value > LIGHT_SENSOR_DARK)
  {
    value = LIGHT_SENSOR_DARK;
  }
  else if (value < LIGHT_SENSOR_BRIGHT)
  {
    value = LIGHT_SENSOR_BRIGHT;
  }

  return 100 - ((value - LIGHT_SENSOR_BRIGHT) / (LIGHT_SENSOR_RANGE / 100));
}

void IRAM_ATTR onMotionDetectHandler()
{
  lastMotionChange = millis();
}

void IRAM_ATTR onTimerHandler()
{

  noInterrupts();

  uint8_t LSPercent = LStoPercent(analogRead(A0) - LIGHT_SENSOR_DARK) / 10 * 10; // 10% sensor bounce
  
  // if motion detected
  uint16_t PWM;
  if (lastMotionChange + MOTION_INTERVAL * 1000 > millis())
  {
    PWM = PercentToPWM(100 - LSPercent * ((mLightDark - mLightBright) / 100));
  }
  else
  {
    PWM = PercentToPWM(100 - LSPercent * ((nmLightDark - nmLightBright) / 100));
  }

  if (currentLightPWM <= PWM)
  {
    currentLightPWM += LIGHT_OFF_CHANGE_STEP_PWM;
    analogWrite(D1, currentLightPWM);
  }
  else
  {
    currentLightPWM -= LIGHT_OFF_CHANGE_STEP_PWM;
    analogWrite(D1, currentLightPWM);
  }

  interrupts();
}

bool F16Driver::init()
{
  if (id.length() == 0)
    id = DRIVER_ID;
  BaseDriver::init(id);

  analogWriteRange(LIGHT_OFF_PWM); // Range from 0 to 1000
  analogWriteFreq(4000);           // 4000 Gz
  pinMode(D7, INPUT);
  pinMode(A0, INPUT);
  pinMode(D1, OUTPUT);
  pinMode(D6, OUTPUT);
  digitalWrite(D6, LOW);

  attachInterrupt(digitalPinToInterrupt(D7), onMotionDetectHandler, CHANGE);

  ticker.attach(0.01, onTimerHandler);

  return true;
}

bool F16Driver::begin(String _topic)
{
  BaseDriver::begin(_topic);
  setType(ACTUATOR_DRIVER_TYPE);
  setAvailable(available);
  return true;
}

bool F16Driver::query()
{
  return false;
};

String F16Driver::getAllProperties()
{
  return BaseDriver::getAllProperties() +
         "analog=" + String(pwm) + "//br\n"
                                   "data=" +
         String(LIGHT_SENSOR_BRIGHT) + "//is\n";
}

bool F16Driver::publish()
{
  if (BaseDriver::publish())
  {
    return true;
  }
  return false;
};

String F16Driver::onMessage(String route, String _payload, int8_t transportMask)
{
  String result = BaseDriver::onMessage(route, _payload, transportMask);
  return result;
  // if (!result.equals(WRONG_PROPERTY_NAME))
  //	return result;
};

#endif
