/* 
 Traffic Light Controller Build 1.00
 
 Jim Windgassen
 12/6/20
  
   This sketch reads a HC-SR04 ultrasonic rangefinder and returns the
   distance to the closest object in range. To do this, it sends a pulse
   to the sensor to initiate a reading, then listens for a pulse 
   to return.  The length of the returning pulse is proportional to 
   the distance of the object from the sensor.
   
   In the main loop of the code, the trqaffic lights cycle between green, red, and yellow with delays between them.
   When a car is detected by the ultrasonic rangefinder, it immediately switches the state of the lights to green.
     
   The circuit:
	* VCC connection of the sensor attached to +5V
	* GND connection of the sensor attached to ground
	* TRIG connection of the sensor attached to Arduino digital pin 5 which is pin 11 on the microcontroller
	* ECHO connection of the sensor attached to Arduino digital pin 7 which is pin 13 on the microcontroller

        * Green light SSR connected to Arduino digital pin 2 which is pin 4 on the microcontroller
        * Yellow light SSR connected to Arduino digital pin 3 which is pin 5 on the microcontroller
        * Red light SSR connected to Arduino digital pin 4 which is pin 6 on the microcontroller
 */

//Pin Definitions

const int GREEN = 2;            //Green light SSR
const int YELLOW = 3;           //Yellow light SSR
const int RED = 4;              //Red light SSR
const int trigPin = 5;          //Trigger pin for ultrasonic rangefinder
const int echoPin = 7;          //Echo pin for ultrasonic rangefinder

//State Machine States
enum states{GREEN_STATE, YELLOW_STATE, RED_STATE};
/*
#define GREEN_STATE 1          //State machine state counts
#define YELLOW_STATE 2
#define RED_STATE 3
*/

//Operational Parameters

const int MIN_RANGE = 3;       //minimum detected range in inches for valid car detection.  If no return comes back from sensor, duration variable will return 0, so MIN_RANGE should be 3 inches or greater
const int MAX_RANGE = 48;      //maximum detected range in inches for valid car detection

const int GREEN_MS = 5000;    //Time that green light is on
const int YELLOW_MS = 3000;   //Time that yellow light is on  
const int RED_MS = 5000;      //Time that red light is on

const int CYCLE_DELAY_MS = 100;  //time to wait in ms before next rangefinding cycle


void setup() {
  
  pinMode(trigPin, OUTPUT);  //Set up trigger pin as a digital output pin
  pinMode(echoPin, INPUT);   //Set up echo pin as an input pin
  
  digitalWrite(RED, LOW);    //Initialize SSR outputs to off
  digitalWrite(YELLOW, LOW); 
  digitalWrite(GREEN, LOW); 
  
  // initialize serial communication:
  Serial.begin(9600);
  
  Serial.print("Traffic Light + Car Sensor Firmware Build 1.00 - 6 December 2020");
  Serial.println();  //Do a line feed and carriage return
  Serial.print("Written by Jim Windgassen");
  Serial.println();  //Do a line feed and carriage return
  delay(1000); 
}

void loop()
{
 
  // establish variables for duration of the ping, 
  // and the distance result in inches and centimeters:
  unsigned long duration, inches, cm, current_ms_count;
  static unsigned long last_ms_count = 0;
  static unsigned long state_time = GREEN_MS;  //initialize state_time to green on time
  static unsigned int state = GREEN_STATE;    //initialize state to green
  boolean car_detected = 0;

//Begin ultrasonic rangefinder reading

  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  //pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  //pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches(duration);
//  cm = microsecondsToCentimeters(duration);

  
//Light state machine indexer
//This indexer uses the millis() function in Arduino.  The code in this section takes into account the rollover that happe3ns with millis() every 50 days or so.
//See https://oshlab.com/handle-millis-overflow-arduino/

  current_ms_count = millis();  
  if(current_ms_count - last_ms_count >= state_time)
    {
      last_ms_count = millis();
      if(state < sizeof(states))
        state++;
      else
        state = 0;
    }


//Normal light state machine operation superceded by detection of car turning light green

  if(inches >=  MIN_RANGE && inches <= MAX_RANGE)
    {
      state = GREEN_STATE;
      car_detected = 1;
    }
  else
   car_detected = 0; 


  
//Light state machine
  switch(state){
    
    case GREEN_STATE:
      digitalWrite(GREEN, HIGH);
      digitalWrite(YELLOW, LOW);
      digitalWrite(RED, LOW);
      state_time = GREEN_MS;
      Serial.print("Green\t");
    break;
      
    case YELLOW_STATE:
      digitalWrite(GREEN, LOW);
      digitalWrite(YELLOW, HIGH);
      digitalWrite(RED, LOW);
      state_time = YELLOW_MS;
      Serial.print("Yellow\t");
    break;  
  
    case RED_STATE:
      digitalWrite(GREEN, LOW);
      digitalWrite(YELLOW, LOW);
      digitalWrite(RED, HIGH);
      state_time = RED_MS;
      Serial.print("Red\t");
    break;
  
    default:
      Serial.print("Invalid State !!"); 
  } 

//Output status information via UART for debugging  
  Serial.print(inches);
  Serial.print("in\t");
//  Serial.print(cm);
//  Serial.print("cm");

  if(car_detected)
    Serial.print("Car Detected");
  else
    Serial.print("NO Car Detected");
   
  Serial.println();  //Do a line feed and carriage return

//delay 100ms before next ultrasonic sensor reading 
  delay(CYCLE_DELAY_MS); 
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  //return microseconds / 74 / 2;
  return microseconds / 60 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  //return microseconds / 29 / 2;
  return microseconds / 25 / 2;
}
