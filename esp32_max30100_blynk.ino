#include <Wire.h>
#include "MAX30105.h"
#include <BlynkSimpleEsp32_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include "heartRate.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>


#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT
#define DHTPIN 4
#define DHTTYPE DHT11

char auth[] = "c-HgFsEu8BcZq3quB0_vLir6_6i-Pxkw";
int pin=15;
float val;
float beatsuma=0.0;
float beatprom = 0;
int cont;
const int intervalo=1000;
unsigned long milisactuales = millis();
unsigned long anteriores=0;
int pin1Value = 0;
const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0; 
float beatsPerMinute;
int beatAvg;

MAX30105 particleSensor;
DHT dht(DHTPIN, DHTTYPE);

BLYNK_WRITE(V1)
{
  pin1Value = param.asInt(); // assigning incoming value from pin V1 to a variable
}

void setup()
{
  Serial.begin(9600);
  dht.begin();
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    while (1);
  }

  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A); 
  particleSensor.setPulseAmplitudeGreen(0); 

  Blynk.setDeviceName("Blynk");

  Blynk.begin(auth);
}

void loop()
{
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
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
