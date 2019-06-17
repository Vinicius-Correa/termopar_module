#include <SPI.h>                //http://arduino.cc/en/Reference/SPI
#include <Thermocouple.h>       //http://github.com/JChristensen/Thermocouple

//Connect as follows:
//SO    Arduino pin 12          //master in slave out
//SCK   Arduino pin 13          //serial clock
#define CS1 9                   //CS1 Arduino pin
#define CS2 10                  //CS2 Arduino pin
#define RELAY 3                 //RLY Arduino pin

float Temp1;
float Temp2;
bool status = false;

Thermocouple tc1 = Thermocouple(CS1);    //instantiate the thermocouple objects
Thermocouple tc2 = Thermocouple(CS2);

void setup() {
    pinMode(RELAY, OUTPUT);    //set relay port
    digitalWrite(RELAY, LOW);
    Serial.begin(9600);
    delay(500);
    Serial.println("tc1;tc2");
}

void loop() {
    Temp1 = tc1.readC();     //read the 1st TC
    Temp2 = tc2.readC();     //read the 2nd TC
    Serial.print(Temp1);
    Serial.print(";");
    Serial.println(Temp2);
    
    //you code start here
    digitalWrite(RELAY, HIGH);   // turn the OUTPUT on (voltage level)
    delay(1000);
    digitalWrite(RELAY, LOW);    // turn the OUTPUT off
    delay(1000);
}
