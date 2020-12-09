/*
  Optical Heart Rate Detection (PBA Algorithm) using the MAX30105 Breakout
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  This is a demo to show the reading of heart rate or beats per minute (BPM) using
  a Penpheral Beat Amplitude (PBA) algorithm.

  It is best to attach the sensor to your finger using a rubber band or other tightening
  device. Humans are generally bad at applying constant pressure to a thing. When you
  press your finger against the sensor it varies enough to cause the blood in your
  finger to flow differently which causes the sensor readings to go wonky.

  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected

  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.
*/

#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"


#define BLYNK_PRINT Serial

#define BLYNK_USE_DIRECT_CONNECT

#include <BlynkSimpleEsp32_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>
char auth[] = "c-HgFsEu8BcZq3quB0_vLir6_6i-Pxkw";
int pin=15;
float val;
//JoseDefines
float beatsuma=0.0;
float beatprom = 0;
int cont;
const int intervalo=1000;
unsigned long milisactuales = millis();
unsigned long anteriores=0;
int pin1Value = 0;

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

#include <Adafruit_Sensor.h>

#include <DHT.h>
 
// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 4
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11
 
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

BLYNK_WRITE(V1)
{
  pin1Value = param.asInt(); // assigning incoming value from pin V1 to a variable
}

void setup()
{
  Serial.begin(9600);
    dht.begin();
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    while (1);
  }

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  Blynk.setDeviceName("Blynk");

  Blynk.begin(auth);
}

void loop()
{
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 10000.0);
  Serial.print("Delta: ");
  Serial.println(delta);
  }

  while(pin1Value == 1)
  {
      Blynk.run();
    irValue = particleSensor.getIR();

    if (checkForBeat(irValue) == true)
    {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 10000.0);
  Serial.print("Deltas: ");
  Serial.println(delta);
    Serial.print("Val: ");
  Serial.println(pin1Value);
  }
    anteriores=millis();
    if (milisactuales - anteriores >= intervalo)
    {
      beatsuma=beatsPerMinute+beatsuma;
      cont++;
    } 
  }
  beatprom=beatsuma/cont;
  cont=0;
  Serial.print("Beatprom: ");
  Serial.println(beatprom);
  if(beatprom>54 && beatprom<60)
  {
    Blynk.virtualWrite(V6, 1000);
    Blynk.virtualWrite(V5, 0);
    Blynk.virtualWrite(V4, 1000);
  }
  else if(beatprom>46 && beatprom<=54)
  {
    Blynk.virtualWrite(V5, 1000);
    Blynk.virtualWrite(V6, 0);
    Blynk.virtualWrite(V4, 1000);
  }
  else if(beatprom>100)
  {
    Blynk.virtualWrite(V4, 1000);
    Blynk.virtualWrite(V5, 0);
    Blynk.virtualWrite(V6, 0);
  }  
  else
  {
    Blynk.virtualWrite(V4, 0);
    Blynk.virtualWrite(V5, 0);
    Blynk.virtualWrite(V6, 0);
  }

  float t = dht.readTemperature();
  float hum = dht.readHumidity();
  Blynk.run();
  Blynk.virtualWrite(V2, hum);
  Blynk.virtualWrite(V7, beatsPerMinute);
  Blynk.virtualWrite(V3, irValue);
  Blynk.virtualWrite(V0, t);
}
