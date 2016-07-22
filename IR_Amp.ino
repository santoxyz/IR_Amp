#include <Arduino.h>

/* Example program for from IRLib – an Arduino library for infrared encoding and decoding
 * Version 1.3   January 2014
 * Copyright 2014 by Chris Young http://cyborg5.com
 * Based on original example sketch for IRremuote library
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://www.righto.com/
 */
/*
 * IRLib: IRrecvDump - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 */
//#include "<Arduino.h>"
#include <IRLib.h>
#include <elapsedMillis.h>
#include <LiquidCrystal.h>

int RECV_PIN = 4; //TSOP 1836 : PIN1=VCC,PIN2=GND,*PIN3=OUT*
int ONOFF_PIN = 11;
int interval = 32000; //500 *64 fattore moltiplicativo dovuto al cambio di moltiplicatore del timer
int AMP_VOL_PIN = 5;

int LEVELS=32;
int cur_vol;

IRrecv My_Receiver(RECV_PIN);

IRdecode My_Decoder;
unsigned int Buffer[RAWBUF];
elapsedMillis timeElapsed; //declare global if you don't want it reset every time loop runs
elapsedMillis backlight_timeout;
unsigned long long int backlight_on_ms = 320000; //5000 *64 fattore moltiplicativo dovuto al cambio di moltiplicatore del timer

// LCD Connections:
// rs (LCD pin 4) to Arduino pin 12
// rw (LCD pin 5) to Arduino pin 13
// enable (LCD pin 6) to Arduino pin A0
// LCD pin 15 to Arduino pin 13
// LCD pins d4, d5, d6, d7 to Arduino pins 9, 8, 7, 6
LiquidCrystal lcd(12, 13, A0, 9, 8, 7, 6);

int backLight = 10;    // pin 10 will control the backlight


byte full[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

void show_vol(){
  digitalWrite(backLight, HIGH); // turn backlight on. Replace 'HIGH' with 'LOW' to turn it off.
  backlight_timeout = 0;
  lcd.clear();                  // start with a blank screen
  lcd.setCursor(0,0);           // set cursor to column 0, row 0 (the first row)

  String s_volume = "";
  if(digitalRead(ONOFF_PIN)){
    s_volume = "ON       ";
  } else {
    s_volume = "OFF      ";
  }
  
  s_volume +="VOL ";
  s_volume += cur_vol;
  lcd.print(s_volume);    // change text to whatever you like. keep it clean!
  lcd.setCursor(0,1);     // set cursor to column 0, row 1
  String s_bars = "";
  int n = cur_vol/(256/16);
  for(int i=0;i<n;i++){
    s_bars+="#";
    lcd.write(byte(0));
  }
  //lcd.print(s_bars);

}

void setup()
{

  TCCR0B = ((TCCR0B & B11111000) | B00000001); //Set timer0 freq (ports D5 & D6) to 60KHz //ATTENZIONE: SPUTTANA LA elapsedMillis

  pinMode(ONOFF_PIN, OUTPUT);
  pinMode(13, OUTPUT);

  pinMode(AMP_VOL_PIN, OUTPUT);
  analogWrite(AMP_VOL_PIN, cur_vol);

  Serial.begin(9600);
  delay(2000);while(!Serial);//delay for Leonardo

  Serial.println("sniffing...");

  My_Receiver.enableIRIn(); // Start the receiver
  My_Decoder.UseExtnBuf(Buffer);

  //LCD DISPLAY 
  pinMode(backLight, OUTPUT);
  digitalWrite(backLight, HIGH); // turn backlight on. Replace 'HIGH' with 'LOW' to turn it off.
  lcd.begin(16,2);              // columns, rows.  use 16,2 for a 16x2 LCD, etc.

  lcd.createChar(0, full);

  show_vol();  
}

void toggle_onoff(){
    if (timeElapsed > interval) {
        digitalWrite(ONOFF_PIN, !digitalRead(ONOFF_PIN));
        timeElapsed = 0;

        show_vol();
        
    }
}


void amp_vol(int inc){
  if(cur_vol+inc*256/LEVELS > 255)
    cur_vol = 255;
  else if(cur_vol+inc*256/LEVELS < 0)
    cur_vol = 0;
  else
    cur_vol+=inc*256/LEVELS;
  
  analogWrite(AMP_VOL_PIN, cur_vol);
  String s = "VOL=";
  s += cur_vol;
  Serial.println(s);

}

void loop() {
  if (My_Receiver.GetResults(&My_Decoder)) {
    Serial.println("packet received");
    //Restart the receiver so it can be capturing another code
    //while we are working on decoding this one.
    My_Receiver.resume();
    My_Decoder.decode();

    switch(My_Decoder.value){
      case 0xE0E0D02F:
        Serial.println("VOL-");
        //amp_vol(-1);
        break;
      case 3772833823:
        Serial.println("VOL+");
        //amp_vol(+1);
        break;
      case 3772790473:
        Serial.println("RED(ON-OFF)");
        toggle_onoff();
        break;
      case 3772803223:
        Serial.println("BLUE(VOL+)");
        amp_vol(+1);
        show_vol();
        break;
      case 3772819543:
        Serial.println("YELLOW(VOL-)");
        amp_vol(-1);
        show_vol();
        break;
      default:
        //My_Decoder.DumpResults();
        String s = String(My_Decoder.value);
        Serial.println(s);

        break;
    }

  } //IR Receiver end


    if (backlight_timeout > backlight_on_ms) {
        digitalWrite(backLight, LOW); // turn backlight on. Replace 'HIGH' with 'LOW' to turn it off.
        backlight_timeout = 0;
    } 
    
 
}
