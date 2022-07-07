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

#define LIGHT_SENSOR_UP 830
#define LIGHT_SENSOR_DOWN 1024
#define LIGHT_SENSOR_RANGE (LIGHT_SENSOR_DOWN - LIGHT_SENSOR_UP)

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



Ticker ticker;

void IRAM_ATTR onMotionDetectHandler()
{
  lastMotionChange = millis();
}

void IRAM_ATTR onTimerHandler()
{

  noInterrupts();

    
    int LSn = (analogRead(A0) - LIGHT_SENSOR_UP) / 30 * 30; 
    float PL = LIGHT_OFF_PWM / LIGHT_SENSOR_RANGE; //PWM to Light Sensor
    int pwm = (LIGHT_SENSOR_RANGE - LSn) * PL;
    //Anti light sensor bounce 


    
    if ((pwm > LIGHT_OFF_PWM - ANTI_LIGHT_BOUNCE * 2))
    {
        pwm = LIGHT_OFF_PWM;
    }
    else       
    if ((pwm < LIGHT_ON_PWM + ANTI_LIGHT_BOUNCE * 2) )
    {
        pwm = LIGHT_ON_PWM;
    }
/*
    lightPWMHistory[0] = lightPWMHistory[1] = lightPWMHistory[2] = lightPWMHistory[3] = pwm; 
    lightPWMHistory[3] = pwm = (lightPWMHistory[0] + lightPWMHistory[1] + lightPWMHistory[2] + lightPWMHistory[3]) / 4;
   //lastLishtPWM = pwm;
   Serial.println("---");
   Serial.println(pwm);
   Serial.println(lightPWMHistory[0]);
   Serial.println(lightPWMHistory[1]);
   Serial.println(lightPWMHistory[2]);
   Serial.println(lightPWMHistory[3]);   
   Serial.println("-----");

    /*
    else 
    {
      pwm = pwm / ANTI_LIGHT_BOUNCE * ANTI_LIGHT_BOUNCE;
    }
    */

  if (lastMotionChange + MOTION_INTERVAL * 1000 < millis())
  {
    if (beep)
    {
      beep = false;
      digitalWrite(D6, beep);
    }    
    // light off
    if (currentLightPWM <= (pwm + LIGHT_OFF_PWM / 2))
    {
      currentLightPWM += LIGHT_OFF_CHANGE_STEP_PWM;
      analogWrite(D1, currentLightPWM);       
    }
    else 
    {
      currentLightPWM -= LIGHT_OFF_CHANGE_STEP_PWM;
      analogWrite(D1, currentLightPWM);       
    }
  }
  else
  {
    // light on    
    if (currentLightPWM >= pwm)
    {
      currentLightPWM -= LIGHT_ON_CHANGE_STEP_PWM;
      analogWrite(D1, currentLightPWM);      
      beep = !beep;      
      digitalWrite(D6, beep);
    }
    else 
    {
      currentLightPWM += LIGHT_ON_CHANGE_STEP_PWM;
      analogWrite(D1, currentLightPWM);      
      digitalWrite(D6, LOW);
    }
  }

  interrupts();
}

void setup1()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("OWLOS-F16");

  analogWriteRange(LIGHT_OFF_PWM); // Range from 0 to 1000
  analogWriteFreq(4000);           // 4000 Gz
  pinMode(D7, INPUT);
  pinMode(A0, INPUT);
  pinMode(D1, OUTPUT);  
  pinMode(D6, OUTPUT);
  digitalWrite(D6, LOW);

  attachInterrupt(digitalPinToInterrupt(D7), onMotionDetectHandler, CHANGE);

  ticker.attach(0.01, onTimerHandler);
}
void loop1()
{
  // PWM Pin debug
  if (Serial.available())
  {
    int i = (Serial.readString()).toInt();
    analogWrite(D1, i);
  }

  // int a = analogRead(A0); //FFR

  delay(1000);
}