/*********************************************************************

  Betto: Fish feeder and maintenance system using Adafruit SHARP 
  Memory Display and DS18B20 temperature sensor.
  
  This software runs on Arduino Yun
  
  Alfredo Rius
  alfredo.rius@gmail.com 

  v1.1   2017-08-20
  Added Feed Button
  
  v1.0   2017-08-18
  Upgraded to Arduino Yun
  Added Feeder
  Show feeder remaining time
  Show elapsed time
  
  v0.1   2017-08-16
  Show temperature
  Show max temperature
  Show min temperature

*********************************************************************/


#define FIRMWARE " v1.1 "


#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <DallasTemperature.h>
#include <TimerThree.h>
#include <Servo.h>


// Hardware
#define SERVO 6
#define BUTTON 2
#define ONE_WIRE_BUS 12
#define SHARP_SCK 5
#define SHARP_MOSI 4
#define SHARP_SS 3

#define SERVO_MAX 85
#define SERVO_MIN 65

// Other constants
#define FEED_PERIOD (long)12*60*60 //Every 12 hours
#define FEED_TIMES 3

#define FREQ_DIV 20 // 20 seconds

#define BLACK 0
#define WHITE 1

Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

Servo servo;
uint8_t feed_state = 0;
long feed_count = 0;
int freq_div = 0;
unsigned long day_count = 0;
boolean command = false;
float temp;
float min_temp = 40;
float max_temp = -20;

float getTemp(void){
  sensors.requestTemperatures(); // Send the command to get temperatures
  temp = sensors.getTempCByIndex(0);
  if(temp > -20 && temp < 40){// realistic temperatures
    if(temp < min_temp){
      min_temp = temp;
    }
    if(temp > max_temp){
      max_temp = temp;
    }
  }
  return temp;
}

void displayAll(){
  command = true;
  display.refresh();

  display.clearDisplay();
    
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(8,15);
    
  display.print(" Betto ");

  display.setCursor(0,42);
  display.print(" ");
  display.print(temp);
  display.println("c");

  display.setTextSize(1);
  display.setTextColor(BLACK);

  
  display.setCursor(1,64);
  display.print("time: ");
  if(day_count<3600){
    if(day_count<60){
      display.print(day_count);
      display.print("sec");
    }else{
      display.print(day_count/60);
      display.print("min");
    }
  }else if(day_count<86400){
    display.print(day_count/3600);
    display.print("hr");
  }else{
    display.print(day_count/86400);
    display.print("days");
  }

  display.setCursor(1,72);
  display.print("feed: ");
  if(feed_count<3600){
    if(feed_count<60){
      display.print(feed_count);
      display.print("sec");
    }else{
      display.print(feed_count/60);
      display.print("min");
    }
  }else{
    display.print(feed_count/3600);
    display.print("hr");
  }
    
  display.setCursor(1,80);
  display.print("min:  ");
  display.print(min_temp);
  display.println("c");
  display.setCursor(1,88);
  display.print("max:  ");
  display.print(max_temp);
  display.println("c");
  command = false;
}

void setFeedState(){
  feed_state = 1;
}

void feed(int times, int d){
  int pos,i;
  
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Reset everything
  feed_state = 0;

  // Feed fish
  for(i=0;i<times;i++){
    for (pos = SERVO_MIN; pos <= SERVO_MAX; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      servo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(d);                       // waits 15ms for the servo to reach the position
    }
    for (pos = SERVO_MAX; pos >= SERVO_MIN; pos -= 1) { // goes from 180 degrees to 0 degrees
      servo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(d);                       // waits 15ms for the servo to reach the position
    }
  }
  
  digitalWrite(LED_BUILTIN, LOW);
  
  if(!command)
      displayAll();
}


void timerCount(void){ // Every 1000 ms
  if(!freq_div){
    
    if(!command)
      displayAll();

    getTemp();
    
    freq_div = FREQ_DIV;
  }
  if(!feed_count){
    feed_state = 1;
    feed_count = FEED_PERIOD;
  }
  day_count++;
  feed_count--;
  freq_div--;
}

void setup(void) 
{  
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(BUTTON,INPUT_PULLUP);

  digitalWrite(LED_BUILTIN, HIGH);


  // Start Servo
  servo.attach(SERVO);
  // Set servo in a defined location
  servo.write(SERVO_MIN);
  
  // Enable Servo
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);


  // start & clear the display
  display.begin();
  display.clearDisplay();

  
  // text display tests
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(8,15);
  display.println(" Betto ");
  display.setCursor(0,35);
  display.println("The Fish");
  display.setCursor(12,55);
  display.println(FIRMWARE);
  display.refresh();
  delay(5000);
  
  Timer3.initialize(1000000);// 1 second
  Timer3.attachInterrupt(timerCount);

  attachInterrupt(digitalPinToInterrupt(BUTTON),setFeedState,FALLING);
  
}

void loop(void) 
{
  if(!command)
    display.refresh();
  if(feed_state){
    feed(FEED_TIMES,20);
  }
  delay(100);
}

