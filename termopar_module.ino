
#include <SPI.h>                //http://arduino.cc/en/Reference/SPI
#include <LiquidCrystal.h>      //http://arduino.cc/en/Reference/LiquidCrystal
#include <Thermocouple.h>       //http://github.com/JChristensen/Thermocouple
#include <virtuabotixRTC.h>     //http://github.com/chrisfryer78/ArduinoRTClibrary

//Connect as follows:
//SO    Arduino pin 12    master in slave out
//SCK   Arduino pin 13    serial clock
#define CS     3         //CS2 Arduino pin
#define RELAY  2         //RLY Arduino pin
#define CLK   A1         //CLK DS1302 Arduino pin
#define DAT   A2         //DAT DS1302 Arduino pin
#define RST   A3         //RST DS1302 Arduino pin

#define ON    true
#define OFF   false

//Default set values
int decarb_temp = 122;     //Decarbonization temperature [50,150]
int decarb_time_h = 0;     //Decarbonization time in hours [0,2]
int decarb_time_min = 27;  //Decarbonization time in minutes [0,59]
int infusion_temp = 90;    //Infusion temperature [50,150]
int infusion_time_h = 2;   //Infusion time in hours [0,10]
int infusion_time_min = 0; //Infusion time in minutes [0,59]
int temp_ripple = 0;       //Temperature ripple [6,20]

bool state;
int button;
int temp;
int RTC_time_sum;
int screen = 0;
int cycle = 0;
int minute_total = 0;
int hour_total = 0;

byte newChar1[8] = { B00000, B00000, B01110, B01110, B01110, B01110, B00000, B00000 }; // Caracter stop
byte newChar2[8] = { B00000, B01000, B01100, B01110, B01100, B01000, B00000, B00000 }; // Caracter play


LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
Thermocouple tc = Thermocouple(CS);
virtuabotixRTC myRTC(CLK, DAT, RST);

void relayDrive (bool action) {
  if (action == ON) {
    digitalWrite(RELAY, HIGH);
    state = true;
  }
  else if (action == OFF) {
    digitalWrite(RELAY, LOW);
    state = false;
  }
}

void printTemperature(int data) {
  if (data < 10)
    lcd.print("  ");
  else if (data < 100)
    lcd.print(" ");
  lcd.print(data);
}

void printTime(int data) {
  if (data < 10)
    lcd.print("0");
  lcd.print(data);
}

void programSetScreen(int data) { // data values: 1 for stop and 2 for play
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("D:");
  printTemperature(decarb_temp);
  lcd.print("\337C  I:");
  printTemperature(infusion_temp);
  lcd.print("\337C");
  lcd.setCursor(0,1);
  lcd.write(data);
  lcd.setCursor(2,1);
  printTime(decarb_time_h);
  lcd.print(":");
  printTime(decarb_time_min);
  lcd.setCursor(11,1);
  printTime(infusion_time_h);
  lcd.print(":");
  lcd.setCursor(14,1);
  printTime(infusion_time_min); 
}

int readKeypad() {
  int state = 0;
  int x = analogRead (0);
  if (x < 100) // right
    state = 1;
  else if (x < 200) // up
    state = 3;
  else if (x < 400) // down
    state = 4;
  else if (x < 600) // left
    state = 2;
  else if (x < 800) // select
    state = 5;
  return(state);
}

void cursorChange(int pos) {
  switch (pos) {
    case 0:
      lcd.setCursor(4,0);
      break;
    case 1:
      lcd.setCursor(3,1);
      break;
    case 2:
      lcd.setCursor(6,1);
      break;
    case 3:
      lcd.setCursor(13,0);
      break;
    case 4:
      lcd.setCursor(12,1);
      break;
    case 5:
      lcd.setCursor(15,1);
      break;
  }
}

void increaseValue(int pos) {
  switch (pos) {
    case 0:
      decarb_temp = decarb_temp + 1;
      if (decarb_temp > 150)
        decarb_temp = 50;
      lcd.setCursor(2,0);
      printTemperature(decarb_temp);
      lcd.setCursor(4,0);
      break;
    case 1:
      decarb_time_h = decarb_time_h + 1;
      if (decarb_time_h > 2)
        decarb_time_h = 0;
      lcd.setCursor(2,1);
      printTime(decarb_time_h);
      lcd.setCursor(3,1);
      break;
    case 2:
      decarb_time_min = decarb_time_min + 1;
      if (decarb_time_min > 59)
        decarb_time_min = 0;
      lcd.setCursor(5,1);
      printTime(decarb_time_min);
      lcd.setCursor(6,1);
      break;
    case 3:
      infusion_temp = infusion_temp + 1;
      if (infusion_temp > 150)
        infusion_temp = 50;
      lcd.setCursor(11,0);
      printTemperature(infusion_temp);
      lcd.setCursor(13,0);
      break;
    case 4:
      infusion_time_h = infusion_time_h + 1;
      if (infusion_time_h > 10)
        infusion_time_h = 0;
      lcd.setCursor(11,1);
      printTime(infusion_time_h);
      lcd.setCursor(12,1);
      break;
    case 5:
      infusion_time_min = infusion_time_min + 1;
      if (infusion_time_min > 59)
       infusion_time_min = 0;
      lcd.setCursor(14,1);
      printTime(infusion_time_min);
      lcd.setCursor(15,1);
      break;
  }
}

void decreaseValue(int pos) {
  switch (pos) {
    case 0:
      decarb_temp = decarb_temp - 1;
      if (decarb_temp < 50)
        decarb_temp = 150;
      lcd.setCursor(2,0);
      printTemperature(decarb_temp);
      lcd.setCursor(4,0);
      break;
    case 1:
      decarb_time_h = decarb_time_h - 1;
      if (decarb_time_h < 0)
        decarb_time_h = 2;
      lcd.setCursor(2,1);
      printTime(decarb_time_h);
      lcd.setCursor(3,1);
      break;
    case 2:
      decarb_time_min = decarb_time_min - 1;
      if (decarb_time_min < 0)
        decarb_time_min = 59;
      lcd.setCursor(5,1);
      printTime(decarb_time_min);
      lcd.setCursor(6,1);
      break;
    case 3:
      infusion_temp = infusion_temp - 1;
      if (infusion_temp < 50)
        infusion_temp = 150;
      lcd.setCursor(11,0);
      printTemperature(infusion_temp);
      lcd.setCursor(13,0);
      break;
    case 4:
      infusion_time_h = infusion_time_h - 1;
      if (infusion_time_h < 0)
        infusion_time_h = 10;
      lcd.setCursor(11,1);
      printTime(infusion_time_h);
      lcd.setCursor(12,1);
      break;
    case 5:
      infusion_time_min = infusion_time_min - 1;
      if (infusion_time_min < 0)
       infusion_time_min = 59;
      lcd.setCursor(14,1);
      printTime(infusion_time_min);
      lcd.setCursor(15,1);
      break;
  }
}

void programSet() {
  int key = 0;
  int cur_pos = 0;
  programSetScreen(1);
  lcd.cursor();
  cursorChange(cur_pos);
  while (key != 5) {
    key = readKeypad();
    if (key == 1) {
      cur_pos = cur_pos + 1;
      if (cur_pos > 5)
        cur_pos = 0;
      cursorChange(cur_pos);
      delay(500);
    }
    else if (key == 2) {
      cur_pos = cur_pos - 1;
      if (cur_pos < 0)
        cur_pos = 5;
      cursorChange(cur_pos);
      delay(500);
    }
    else if (key == 3) {
      increaseValue(cur_pos);
      delay(200);
    }
    else if (key == 4) {
      decreaseValue(cur_pos);
      delay(200);
    }
  }
  lcd.noCursor();
}

void totalTime() {
  minute_total = decarb_time_min + infusion_time_min;
  if (minute_total > 59) {
    minute_total = minute_total - 60;
    hour_total = 1;
  }
  hour_total = hour_total + decarb_time_h + infusion_time_h;
}

void heatingScreen() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Heating");
  lcd.setCursor(1, 1);
  printTemperature(temp);
  lcd.print("\337C => ");
  printTemperature(decarb_temp);
  lcd.print("\337C");
}

void heating() {
  cycle = 1;
  temp = tc.readC();
  if (temp < decarb_temp) {
    heatingScreen();
    relayDrive(ON);
    while (temp <= decarb_temp) {
      if (screen != 0) {
        lcd.setCursor(1, 1);
        printTemperature(temp);
      }
      temp = tc.readC();
      delay(200);
    }
  }
}

void cycleScreen(int program) {
  lcd.clear();
  lcd.setCursor(0, 0);
  switch (program) {
    case 2:
      lcd.print("Decarb");
      break;
    case 3:
      lcd.print("Infusion");
      break;
    case 4:
      lcd.print("Finished");
      break;
  }
  lcd.setCursor(11, 0);
  printTemperature(temp);
  lcd.print("\337C");
  lcd.setCursor(0, 1);
  printTime(myRTC.hours);
  lcd.print(":");
  printTime(myRTC.minutes);
  lcd.print(":");
  printTime(myRTC.seconds);
  lcd.setCursor(11, 1);
  printTime(hour_total);
  lcd.print(":");
  printTime(minute_total);
}

void decarb() {
  cycle = 2;
  myRTC.setDS1302Time(0, 0, 0, 6, 23, 1, 2016);
  delay(500);
  myRTC.updateTime();
  RTC_time_sum = 60 * myRTC.hours + myRTC.minutes;
  int temp_high = decarb_temp + temp_ripple/2;
  int temp_low = decarb_temp - temp_ripple/2;
  int decarb_time_sum = 60 * decarb_time_h + decarb_time_min;
  cycleScreen(cycle);
  while (RTC_time_sum < decarb_time_sum) {
    if (temp > temp_high && state == true)
      relayDrive(OFF);
    else if (temp < temp_low && state == false)
      relayDrive(ON);
    myRTC.updateTime();
    RTC_time_sum = 60 * myRTC.hours + myRTC.minutes;
    temp = tc.readC();
    if (screen != 0) {
      lcd.setCursor(0, 1);
      printTime(myRTC.hours);
      lcd.setCursor(3, 1);
      printTime(myRTC.minutes);
      lcd.setCursor(6, 1);
      printTime(myRTC.seconds);
      lcd.setCursor(11, 0);
      printTemperature(temp);
    }
    delay(500);
  }
}

void infusion() {
  cycle = 3;
  int temp_high = infusion_temp + temp_ripple/2;
  int temp_low = infusion_temp - temp_ripple/2;
  int time_total_sum = 60 * hour_total + minute_total;
  cycleScreen(cycle);
  while (RTC_time_sum < time_total_sum) {
    if (temp > temp_high && state == true)
      relayDrive(OFF);
    else if (temp < temp_low && state == false)
      relayDrive(ON);
    myRTC.updateTime();
    RTC_time_sum = 60 * myRTC.hours + myRTC.minutes;
    temp = tc.readC();
    if (screen != 0) {
      lcd.setCursor(0, 1);
      printTime(myRTC.hours);
      lcd.setCursor(3, 1);
      printTime(myRTC.minutes);
      lcd.setCursor(6, 1);
      printTime(myRTC.seconds);
      lcd.setCursor(11, 0);
      printTemperature(temp);
    }
    delay(500);
  }
}

void finished() {
  cycle = 4;
  cycleScreen(cycle);
  relayDrive(OFF);
  while (true) {
    temp = tc.readC();
    if (screen != 0) {
      lcd.setCursor(11, 0);
      printTemperature(temp);
    }
    delay(200);
  }
}

ISR(TIMER1_COMPA_vect) {
  //interrupt commands for TIMER 1 here
  screen = screen + 1;
  if (screen < 4){
    switch (cycle) {
      case 1:
        heatingScreen();
        break;
      case 2:
        cycleScreen(2);
        break;
      case 3:
        cycleScreen(3);
        break;
      case 4:
        cycleScreen(4);
        break;
    }
  }
  else {
    if (cycle == 4)
      programSetScreen(1);
    else
      programSetScreen(2);
  screen = 0;
  }
}

void setup() {
  pinMode(RELAY, OUTPUT);
  relayDrive(OFF);
  lcd.begin(16, 2);
  lcd.createChar(1, newChar1); // Caracter stop
  lcd.createChar(2, newChar2); // Caracter play
  delay(500);
  programSet();
  totalTime();
  
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 0.5 Hz increments
  OCR1A = 62499; // = 16000000 / (1024 * 0.5) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts
}

void loop() {
  heating();
  decarb();
  infusion();
  finished();
}
