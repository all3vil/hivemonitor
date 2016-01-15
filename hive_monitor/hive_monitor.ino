// library for reading DHT22 temp/humidity sensor
#include <DHT.h>

// library for reading the scale
#include "HX711.h"

// DAT and CLK pins from scale sensor
#define SCALE_DAT  6
#define SCALE_CLK  5

// DHT22 sensor pin
#define DHTPIN 4
#define DHTTYPE DHT22

// The scale sensor object
HX711 scale(SCALE_DAT, SCALE_CLK);

// calibration factor for scale, differs from scale to scale
float calibration_factor = -22930; //-7050 worked for my 440lb max scale setup

// DHT sensor object
DHT dht(DHTPIN,DHTTYPE);

void setup()
{
  Serial.begin(9600);

  scale.set_scale();
  scale.tare();  //Reset the scale to 0
  scale.set_scale(calibration_factor); //Adjust to this calibration factor

  /*
  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
  */
  
}

void loop()
{
  unsigned long current_time = millis();

  float humidity = dht.readHumidity();
  float temp_c = dht.readTemperature();
  float weight_kg = scale.get_units(10);
  float noise = analogRead(0);

  Serial.print("Hum:");
  Serial.print(humidity);
  Serial.print(" Temp:");
  Serial.print(temp_c);
  Serial.print(" Weight:");
  Serial.print(weight_kg);
  Serial.print(" Noise:");
  Serial.println(noise);

  /* sleep code goes here */

  delay(1000);
  
}

