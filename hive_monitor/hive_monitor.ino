#include <DHT.h>

/*****************************************************************
XBee_Serial_Passthrough.ino

Set up a software serial port to pass data between an XBee Shield
and the serial monitor.

Hardware Hookup:
  The XBee Shield makes all of the connections you'll need
  between Arduino and XBee. If you have the shield make
  sure the SWITCH IS IN THE "DLINE" POSITION. That will connect
  the XBee's DOUT and DIN pins to Arduino pins 2 and 3.

*****************************************************************/
// We'll use SoftwareSerial to communicate with the XBee:
#include <SoftwareSerial.h>
// XBee's DOUT (TX) is connected to pin 2 (Arduino's Software RX)
// XBee's DIN (RX) is connected to pin 3 (Arduino's Software TX)
SoftwareSerial XBee(2, 3); // RX, TX

#include <math.h>
#include "HX711.h"

#define SCALE_DAT  6
#define SCALE_CLK  5
#define DHTPIN 4
#define DHTTYPE DHT22

HX711 scale(SCALE_DAT, SCALE_CLK);

float calibration_factor = -22930; //-7050 worked for my 440lb max scale setup

DHT dht(DHTPIN,DHTTYPE);

float humidity_readings[60];
float tempc_readings[60];
float weight_readings[60];
float sound_readings[60];
int pos;
unsigned long last_read;
char my_sensor_id;

void print_all_readings(boolean transmit) {
  if (transmit) {
    XBee.print("[remote] ");
    XBee.print(average_readings(humidity_readings, 60));
    XBee.print("% Humidity / ");
    XBee.print(average_readings(tempc_readings, 60));
    XBee.print(" degrees C / ");
    XBee.print(average_readings(weight_readings, 60));
    XBee.print(" kg / ");
    XBee.print(average_readings(sound_readings, 60));
    XBee.println(" noise");
  } else {
    Serial.print("[local] ");
    Serial.print(average_readings(humidity_readings, 60));
    Serial.print("% Humidity / ");
    Serial.print(average_readings(tempc_readings, 60));
    Serial.print(" degrees C / ");
    Serial.print(average_readings(weight_readings, 60));
    Serial.print(" kg / ");
    Serial.print(average_readings(sound_readings, 60));
    Serial.println(" noise");
  }
  
}

float average_readings(float arr[], int len) {
  int i;
  float sum = 0;
  float retval = 0;
  int num_counted = 0;
  
  for (i = 0; i < len; i++) {
    if (!isnan(arr[i]) &&  arr[i] != -999) {
      sum += arr[i];
      num_counted++;
    }
  }
  retval = sum / num_counted;
  
  return retval;
}

void setup()
{
  my_sensor_id = '1';
  // Set up both ports at 9600 baud. This value is most important
  // for the XBee. Make sure the baud rate matches the config
  // setting of your XBee.
  XBee.begin(9600);
  Serial.begin(9600);

  pos = 0;
  last_read = 0;
  
  // pre-fill the readings array with bad values
  for (int i = 0; i < 60; i++) {
    humidity_readings[i] = -999;
    weight_readings[i] = -999;
    tempc_readings[i] = -999;
    sound_readings[i] = -999;
  }

  scale.set_scale();
  scale.tare();  //Reset the scale to 0
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  
  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
  
}

void loop()
{
  unsigned long current_time = millis();

  if (XBee.available()) {
    char input = XBee.read();
    Serial.print("Got input from XBee: '");
    Serial.print(input);
    Serial.println("'");
    if (input == my_sensor_id)
      XBee.print("[response] ");
      print_all_readings(true);
  }
  // if it's been more than a second since we last read, do another reading
  // and update the last_read time
  if (current_time - 1000 >= last_read) {
    humidity_readings[pos] = dht.readHumidity();
    tempc_readings[pos] = dht.readTemperature();
    weight_readings[pos] = scale.get_units();
    sound_readings[pos] = analogRead(0);
    last_read = current_time;
    pos++;
  }

  // we're averaging the last 60 readings, so when we get to 59,
  // wrap the array position back around to 0
  if (pos >= 59) {
//    XBee.print("[passive] ");
    print_all_readings(true);
    print_all_readings(false);
    pos = 0;
  }
 
}

