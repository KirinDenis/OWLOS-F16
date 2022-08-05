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

(Цей файл — частина Ready IoT Solution — OWLOS.

OWLOS — безкоштовна програма, ви можете ділитися нею та/або змінювати її на
умовах Стандартної суспільної ліцензії GNU в тому вигляді, в якому вона була
опублікована Фондом вільного програмного забезпечення; версія 3 ліцензія,
будь-яка старша версія.

OWLOS поширюється з надією, що вона буде корисною, але БЕЗ БУДЬ-ЯКИХ ГАРАНТІЙ;
навіть без неявної гарантії ТОВАРНОГО ВИДУ чи ПРИДАТНОСТІ ДЛЯ ПЕВНИХ ЦІЛЕЙ.
Див. детальніше у Стандартній громадській ліцензії GNU.

Ви повинні отримати копію Стандартної суспільної ліцензії GNU разом із цією програмою.
Якщо цього не сталося, див. <https://www.gnu.org/licenses/>.)
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

              16th floor. IoT smart light driver with supporting motion and light sensors. ESP8266 friendly.

-------------------------------------------------------------------------------------------------------------------------
Device configuration #1:

[Motion Sensor 1 (D7)]  [Motion Sensor 2(D8)]  [Light Sensor (A0)]  [Beeper (D6)]
 ^                       ^                      ^                    ^
 |                       |                      |                    |
[PWM 1 (D1)]-------------+----------------------+---------------------
                         |                      |
                        [PWM 2 (D2)] ------------
*/

#include "F16Driver.h"
#ifdef USE_F16_DRIVER
#define DRIVER_MASK "F16Driver"
#define DRIVER_ID "F16"

#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include <Ticker.h>

#define PWM_1_PIN D1
#define PWM_2_PIN D2
#define MOTION_SENSOR_1_PIN D7
#define MOTION_SENSOR_2_PIN D8
#define LIGHT_SENSOR_PIN A0
#define BEEPER_PIN D6

#define LIGHT_OFF_PWM 1024
#define LIGHT_ON_PWM 0
#define LIGHT_OFF_CHANGE_STEP_PWM 1
#define LIGHT_ON_CHANGE_STEP_PWM 10
#define NO_MOTION_LIGHT 3

#define LIGHT_SENSOR_BRIGHT 830
#define LIGHT_SENSOR_DARK 1024
#define LIGHT_SENSOR_RANGE (LIGHT_SENSOR_DARK - LIGHT_SENSOR_BRIGHT)

#define LIGHT_MODE_OFF 0
#define LIGHT_MODE_ON 1
#define LIGHT_MODE_AUTO 2
#define LIGHT_MODE_AUTO_MOTION 3

#define MOTION_INTERVAL 5
#define TIKER_INTERVAL 0.01

void IRAM_ATTR onMotion1DetectHandler();
void IRAM_ATTR onMotion2DetectHandler();

volatile uint8_t LSPercent;

volatile unsigned long lastMotion1Change = 0;
volatile unsigned long lastMotion2Change = 0;

volatile bool motion1Detect = false;
volatile bool motion2Detect = false;

volatile uint16_t current1LightPWM = LIGHT_ON_PWM;
volatile uint16_t current2LightPWM = LIGHT_ON_PWM;

volatile uint8 mLightBright = 0;
volatile uint8 nmLightBright = 0;
volatile uint8 mLightDark = 100;
volatile uint8 nmLightDark = 20;

volatile uint16 mTime = MOTION_INTERVAL;

volatile uint8 light1Mode = LIGHT_MODE_AUTO_MOTION;
volatile uint8 light2Mode = LIGHT_MODE_AUTO_MOTION;

volatile bool beepMode = 1; // 1 enabled, 0 disabled
volatile uint8 beepTime = 4;
volatile bool beep = true;

Ticker ticker;

// Percent to PWM
// LIGHT_ON_PWM  = 100% (default value 0)
// LIGHT_OFF_PWM = 0%  (default value 1024)
uint16_t PercentToPWM(uint8_t percent)
{
  if (percent <= 0)
  {
    return LIGHT_OFF_PWM;
  }
  else if (percent >= 100)
  {
    return LIGHT_ON_PWM;
  }
  return LIGHT_OFF_PWM - ((float)LIGHT_OFF_PWM / 100.0f * (float)percent);
}

// PWM to Percent 
// LIGHT_ON_PWM  = 100% (default value 0)
// LIGHT_OFF_PWM = 0%  (default value 1024)
uint16_t PWMToPercent(uint16_t PWM)
{
  if (PWM >= 1024)
  {
    return 0;
  }
  PWM = (1024 - PWM) / (1024.0f / 100.0f);
  if (PWM > 100)
  {
    PWM = 100;
  }
  return PWM;
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
  return 100.0f - ((value - (float)LIGHT_SENSOR_BRIGHT) / ((float)LIGHT_SENSOR_RANGE / 100.0f));
}

//-------------------------------------------------------------------------------------------------------
// Motion sensor 1 interupt handler
void IRAM_ATTR onMotion1DetectHandler()
{
  lastMotion1Change = millis();    
  debugOut("motion 1", "detect");
}

//-------------------------------------------------------------------------------------------------------
// Motion sensor 2 interupt handler
void IRAM_ATTR onMotion2DetectHandler()
{
  lastMotion2Change = millis();  
  debugOut("motion 2", "detect");
}

//-------------------------------------------------------------------------------------------------------
// Set PWM
uint16_t setPWM(uint8 lightMode, uint8_t PWMPin, unsigned long lastMotionChange, uint16_t currentLightPWM)
{
  if (lightMode == LIGHT_MODE_OFF)
  {
    analogWrite(PWMPin, LIGHT_OFF_PWM);
  }
  else if (lightMode == LIGHT_MODE_AUTO || lightMode == LIGHT_MODE_AUTO_MOTION)
  {

    LSPercent = LStoPercent(analogRead(LIGHT_SENSOR_PIN)) / 10 * 10; // 10% sensor bounce

    // if motion detected
    uint16_t PWM;    
    if (lastMotionChange + mTime * 1000 > millis() || lightMode == LIGHT_MODE_AUTO)
    {
      PWM = PercentToPWM((100.0f - (float)LSPercent) / (float)(100.0f / (mLightDark - mLightBright)));
      if (PWMPin == PWM_1_PIN)
      {
        motion1Detect = true;
      }
      else 
      {
        motion2Detect = true;
      }
    }
    else
    {
      PWM = PercentToPWM((100.0f - (float)LSPercent) / (100.0f / (float)(nmLightDark - nmLightBright)));
      if (PWMPin == PWM_1_PIN)
      {
        motion1Detect = false;
      }
      else 
      {
        motion2Detect = false;
      }
    }

    if (currentLightPWM != PWM)
    {

      if (currentLightPWM < PWM)
      {
        currentLightPWM += LIGHT_OFF_CHANGE_STEP_PWM;
        analogWrite(PWMPin, currentLightPWM);
      }
      else
      {
        currentLightPWM -= LIGHT_OFF_CHANGE_STEP_PWM;
        analogWrite(PWMPin, currentLightPWM);
      }
    }
  }
  else // LIGHT_MODE_ON and any others valies of lightMode - light is ON
  {
    analogWrite(PWMPin, LIGHT_ON_PWM);
  }
  return currentLightPWM;
}
//-------------------------------------------------------------------------------------------------------
// Timer interupt handler
void IRAM_ATTR onTimerHandler()
{
  noInterrupts();

  current1LightPWM = setPWM(light1Mode, PWM_1_PIN, lastMotion1Change, current1LightPWM);
  current2LightPWM = setPWM(light2Mode, PWM_2_PIN, lastMotion2Change, current2LightPWM);

  if (beepMode && lastMotion1Change + beepTime * 1000 > millis())
  {
    beep = !beep;
    if (beep)
    {      
      digitalWrite(BEEPER_PIN, HIGH);
    }
    else
    {
      digitalWrite(BEEPER_PIN, LOW);
    }
  }
  else
  {
    digitalWrite(BEEPER_PIN, LOW);
  }

  interrupts();
}

//-------------------------------------------------------------------------------------------------------
// OWLOS driver initialize
bool F16Driver::init()
{
  if (id.length() == 0)
    id = DRIVER_ID;
  BaseDriver::init(id);

  analogWriteRange(LIGHT_OFF_PWM); // Range from 0 to 1000
  analogWriteFreq(4000);           // 4000 Gz

  pinMode(PWM_1_PIN, OUTPUT);
  pinMode(PWM_2_PIN, OUTPUT);

  pinMode(MOTION_SENSOR_1_PIN, INPUT);
  pinMode(MOTION_SENSOR_2_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);

  pinMode(BEEPER_PIN, OUTPUT);
  digitalWrite(BEEPER_PIN, LOW);

  attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR_1_PIN), onMotion1DetectHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR_2_PIN), onMotion2DetectHandler, CHANGE);

  ticker.attach(TIKER_INTERVAL, onTimerHandler);

  return true;
}

//-------------------------------------------------------------------------------------------------------
// OWLOS driver begin
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
  light1Mode = getProperty("light1mode", light1Mode);
  light2Mode = getProperty("light2mode", light2Mode);
  beepMode = getProperty("beepmode", beepMode);
  beepTime = getProperty("beeptime", beepTime);
  return true;
}

//-------------------------------------------------------------------------------------------------------
// OWLOS driver query
bool F16Driver::query()
{
  return false;
};

String F16Driver::getAllProperties()
{
  
  return BaseDriver::getAllProperties() +
         "lspercent=" + String(LSPercent) + "//ir\n"
         "pwm1=" + String(PWMToPercent(current1LightPWM)) + "//ir\n"
         "pwm2=" + String(PWMToPercent(current2LightPWM)) + "//ir\n"
         "motion1detect=" + String(motion1Detect) + "//br\n"         
         "motion2detect=" + String(motion2Detect) + "//br\n"         
         "mlightb=" + getProperty("mlightb", mLightBright) + "//i\n"
                                                             "nmlightb=" +
         getProperty("nmlightb", nmLightBright) + "//i\n"
                                                  "mlightd=" +
         getProperty("mlightd", mLightDark) + "//i\n"
                                              "nmlightd=" +
         getProperty("nmlightd", nmLightDark) + "//i\n"
                                                "mtime=" +
         getProperty("mtime", mTime) + "//i\n"
                                       "light1mode=" +
         getProperty("light1mode", light1Mode) + "//i\n"
                                                 "light2mode=" +
         getProperty("light2mode", light2Mode) + "//i\n"
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


  if (matchRoute(route, topic, "/getlspercent"))
  {
    result = String(LSPercent);
  }
  else 
  if (matchRoute(route, topic, "/getpwm1"))
  {
    result = String(PWMToPercent(current1LightPWM));
  }
  else   
  if (matchRoute(route, topic, "/getpwm2"))
  {
    result = String(PWMToPercent(current2LightPWM));
  }
  else   
  if (matchRoute(route, topic, "/getmotion1detect"))
  {
    result = String(motion1Detect);
  }
  else 
  if (matchRoute(route, topic, "/getmotion2detect"))
  {
    result = String(motion2Detect);
  }
  else 
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
  else if (matchRoute(route, topic, "/getlight1mode"))
  {
    result = String(getProperty("light1mode", light1Mode));
  }
  else if (matchRoute(route, topic, "/setlight1mode"))
  {
    light1Mode = setProperty("light1mode", atoi(_payload.c_str()), true);
    result = "1";
  }
  else if (matchRoute(route, topic, "/getlight2mode"))
  {
    result = String(getProperty("light2mode", light2Mode));
  }
  else if (matchRoute(route, topic, "/setlight2mode"))
  {
    light2Mode = setProperty("light2mode", atoi(_payload.c_str()), true);
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
