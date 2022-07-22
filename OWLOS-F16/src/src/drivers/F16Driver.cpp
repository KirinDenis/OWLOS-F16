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

-------------------------------------------------------------------------------------------------------------------------

    _,.---._            ,-.-.             _,.---._      ,-,--.             _,---.                ,-----.--.      .,-.  
  ,-.' , -  `. ,-..-.-./  \==\  _.-.    ,-.' , -  `.  ,-.'-  _\         .-`.' ,  \              /` ` - /==/     / \==\ 
 /==/_,  ,  - \|, \=/\=|- |==|.-,.'|   /==/_,  ,  - \/==/_ ,_.'        /==/_  _.-' ,--.--------.`-'-. -|==|    / -/==/ 
|==|   .=.     |- |/ |/ , /==/==|, |  |==|   .=.     \==\  \          /==/-  '..-./==/,  -   , -\   | `|==|   /- /==/  
|==|_ : ;=:  - |\, ,     _|==|==|- |  |==|_ : ;=:  - |\==\ -\         |==|_ ,    /\==\.-.  - ,-./   | -|==|  /  /==/   
|==| , '='     || -  -  , |==|==|, |  |==| , '='     |_\==\ ,\        |==|   .--'  `--`--------`    | `|==| /. / \==\  
 \==\ -    ,_ /  \  ,  - /==/|==|- `-._\==\ -    ,_ //==/\/ _ |       |==|-  |                    .-','|==|| _ \_/\==\ 
  '.='. -   .'   |-  /\ /==/ /==/ - , ,/'.='. -   .' \==\ - , /       /==/   \                   /     \==\\ . -  /==/ 
    `--`--''     `--`  `--`  `--`-----'   `--`--''    `--`---'        `--`---'                   `-----`---`'----`--`  

              16th floor. IoT smart lamp driver with motion and light sensors. ESP8266 friendly.

-------------------------------------------------------------------------------------------------------------------------*/

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

volatile unsigned long lastMotionChange = 0;

//-------------------
volatile uint8 mLightBright = 0;
volatile uint8 nmLightBright = 0;
volatile uint8 mLightDark = 100;
volatile uint8 nmLightDark = 20;

volatile uint16 mTime = MOTION_INTERVAL;

#define LIGHT_MODE_OFF 0
#define LIGHT_MODE_ON 1
#define LIGHT_MODE_AUTO 2
#define LIGHT_MODE_AUTO_MOTION 3

volatile uint8 lightMode = LIGHT_MODE_AUTO_MOTION;

volatile bool beepMode = 1; // 1 enabled, 0 disabled
volatile uint8 beepTime = 4;
volatile bool beep = true;

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
  return LIGHT_OFF_PWM - ((float)LIGHT_OFF_PWM / 100.0f * (float)percent);
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
  return 100 - ((value - (float)LIGHT_SENSOR_BRIGHT) / ((float)LIGHT_SENSOR_RANGE / 100.0f));
}

void IRAM_ATTR onMotionDetectHandler()
{
  lastMotionChange = millis();
}

void IRAM_ATTR onTimerHandler()
{
  noInterrupts();

  if (lightMode == LIGHT_MODE_OFF)
  {
    analogWrite(D2, LIGHT_OFF_PWM);
  }
  else if (lightMode == LIGHT_MODE_AUTO || lightMode == LIGHT_MODE_AUTO_MOTION)
  {

    uint8_t LSPercent = LStoPercent(analogRead(A0)) / 10 * 10; // 10% sensor bounce

    // if motion detected
    uint16_t PWM;
    if (lastMotionChange + mTime * 1000 > millis() || lightMode == LIGHT_MODE_AUTO)
    {
      PWM = PercentToPWM((100.0f - (float)LSPercent) / (float)(100.0f / (mLightDark - mLightBright)));
    }
    else
    {
      PWM = PercentToPWM((100.0f - (float)LSPercent) / (100.0f / (float)(nmLightDark - nmLightBright)));
    }

    if (currentLightPWM != PWM)
    {

      if (currentLightPWM < PWM)
      {
        currentLightPWM += LIGHT_OFF_CHANGE_STEP_PWM;
        analogWrite(D2, currentLightPWM);
      }
      else
      {
        currentLightPWM -= LIGHT_OFF_CHANGE_STEP_PWM;
        analogWrite(D2, currentLightPWM);
      }
    }
  }
  else // LIGHT_MODE_ON and any others valies of lightMode - light is ON
  {
    analogWrite(D2, LIGHT_ON_PWM);
  }

  if (beepMode && lastMotionChange + beepTime * 1000 > millis())
  {
    beep = !beep;
    if (beep)
    {
      digitalWrite(D6, HIGH);
    }
    else
    {
      digitalWrite(D6, LOW);
    }
  }
  else
  {
    digitalWrite(D6, LOW);
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
  pinMode(D2, OUTPUT);
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

  mLightBright = getProperty("mlightb", mLightBright);
  nmLightBright = getProperty("nmlightb", nmLightBright);
  mLightDark = getProperty("mlightd", mLightDark);
  nmLightDark = getProperty("nmlightd", nmLightDark);
  mTime = getProperty("mtime", mTime);
  lightMode = getProperty("lightmode", lightMode);
  beepMode = getProperty("beepmode", beepMode);
  beepTime = getProperty("beeptime", beepTime);
  return true;
}

bool F16Driver::query()
{
  return false;
};

String F16Driver::getAllProperties()
{
  return BaseDriver::getAllProperties() +
         "mlightb=" + getProperty("mlightb", mLightBright) + "//i\n"
                                                             "nmlightb=" +
         getProperty("nmlightb", nmLightBright) + "//i\n"
                                                  "mlightd=" +
         getProperty("mlightd", mLightDark) + "//i\n"
                                              "nmlightd=" +
         getProperty("nmlightd", nmLightDark) + "//i\n"
                                                "mtime=" +
         getProperty("mtime", mTime) + "//i\n"
                                       "lightmode=" +
         getProperty("lightmode", lightMode) + "//i\n"
                                               "beepmode=" +
         getProperty("beepmode", beepMode) + "//b\n"
                                             "beeptime=" +
         getProperty("beeptime", beepTime) + "//i\n";
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

  if (!result.equals(WRONG_PROPERTY_NAME))
    return result;

  if (matchRoute(route, topic, "/getmlightb"))
  {
    result = String(getProperty("mlightb", mLightBright));
  }
  else if (matchRoute(route, topic, "/setmlightb"))
  {
    mLightBright = setProperty("mlightb", atoi(_payload.c_str()), true);
    result = "1";
  }
  else if (matchRoute(route, topic, "/getnmlightb"))
  {
    result = String(getProperty("nmlightb", nmLightBright));
  }
  else if (matchRoute(route, topic, "/setnmlightb"))
  {
    nmLightBright = setProperty("nmlightb", atoi(_payload.c_str()), true);
    result = "1";
  }
  else if (matchRoute(route, topic, "/getmlightd"))
  {
    result = String(getProperty("mlightd", mLightDark));
  }
  else if (matchRoute(route, topic, "/setmlightd"))
  {
    mLightDark = setProperty("mlightd", atoi(_payload.c_str()), true);
    result = "1";
  }
  else if (matchRoute(route, topic, "/getnmlightd"))
  {
    result = String(getProperty("nmlightd", nmLightDark));
  }
  else if (matchRoute(route, topic, "/setnmlightd"))
  {
    nmLightDark = setProperty("nmlightd", atoi(_payload.c_str()), true);
    result = "1";
  }
  else if (matchRoute(route, topic, "/getmtime"))
  {
    result = String(getProperty("mtime", mTime));
  }
  else if (matchRoute(route, topic, "/setmtime"))
  {
    mTime = setProperty("mtime", atoi(_payload.c_str()), true);
    result = "1";
  }
  else if (matchRoute(route, topic, "/getlightmode"))
  {
    result = String(getProperty("lightmode", lightMode));
  }
  else if (matchRoute(route, topic, "/setlightmode"))
  {
    lightMode = setProperty("lightmode", atoi(_payload.c_str()), true);
    result = "1";
  }
  else if (matchRoute(route, topic, "/getbeepmode"))
  {
    result = String(getProperty("beepmode", beepMode));
  }
  else if (matchRoute(route, topic, "/setbeepmode"))
  {
    beepMode = setProperty("beepmode", atoi(_payload.c_str()), true);
    result = "1";
  }
  else if (matchRoute(route, topic, "/getbeeptime"))
  {
    result = String(getProperty("beeptime", beepTime));
  }
  else if (matchRoute(route, topic, "/setbeeptime"))
  {
    beepTime = setProperty("beeptime", atoi(_payload.c_str()), true);
    result = "1";
  }
  else
  {
    result = WRONG_PROPERTY_NAME;
  }

  return result;
};

int F16Driver::getProperty(String name, int value)
{
  int result = value;
  if (filesExists(id + "." + name))
  {
    result = filesReadInt(id + "." + name);
  }
  else
  { // initial set data
    setProperty(name, value, false);
  }

  return result;
}

int F16Driver::setProperty(String name, int value, bool doEvent)
{
  filesWriteInt(id + "." + name, value);
  if (doEvent)
  {
    onInsideChange(name, String(value));
  }
  return value;
};

#endif
