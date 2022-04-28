#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
int bright_inc = 0;             //auto-increasing value that goes from 0 to 4095 (approx) in steps of 100
int pwm_channel = 4;
int freq = 100;
int resolution = 12;
const int LOOP_PERIOD = 50;     //speed of main loop

int loop_timer;                 //used for loop timing
//const uint8_t LEFT2 = 1; //example definition
//const uint8_t LEFT1 = 2; //example...
//const uint8_t MIDDLE = 0; //change if you want!
//const uint8_t RIGHT1 = 3; //example...
//const uint8_t RIGHT2 = 4; //change if you want!


float resistanceExtractor(float vin,float rb,float vout){
    return -(vout*rb)/(vout-vin); 
}

float brightnessExtractor(float vout){
    float R;
    float L;
    R = resistanceExtractor(3.3, 20000, vout);
    L = pow(10, (8.65-log10(R))/2.0);
    return L;
}


void setup() {
  
  analogReadResolution(12);       // initialize the analog resolution
  Serial.begin(115200);           // Set up serial port
  delay(100); 
  //TFT initializtation stuff:
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_WHITE); //fill background
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font
  tft.setCursor(0, 0, 2); //set cursor
  loop_timer = millis();
}


void loop() {
  // CHANGE PIN NUMBERS AFTER HARDWARE SETUP
  
  float vout0 = analogRead(5)*(3.3/4096.0); // middle photosensor
  float vout1 = analogRead(3)*(3.3/4096.0); // left2 photosensor
  float vout2 = analogRead(4)*(3.3/4096.0); // left1 photosensor
  float vout3 = analogRead(6)*(3.3/4096.0); // right1 photosensor
  float vout4 = analogRead(7)*(3.3/4096.0); // right2 photosensor

  float lux0 = brightnessExtractor(vout0);
  float lux1 = brightnessExtractor(vout1);
  float lux2 = brightnessExtractor(vout2);
  float lux3 = brightnessExtractor(vout3);
  float lux4 = brightnessExtractor(vout4);
//  if ((lux0 < 500.0)&(lux2 < 500.0)&(lux3 < 500.0)) {
////    tft.fillScreen(TFT_WHITE); //fill background
////  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font
////    tft.setCursor(0, 0, 2);
////    tft.printf("no laser detected");
//  Serial.println("no laser detected");
//  } else {
//    alignment_check(lux0, lux2, lux3);
//  }
  
  Serial.printf("Current Lux0: %f.  ", (float)lux0);
  Serial.printf("Current Lux3: %f.  ", (float)lux1);
  Serial.printf("Current Lux2: %f.  ", (float)lux2);
  Serial.printf("Current Lux3: %f.  ", (float)lux3);
  Serial.printf("Current Lux3: %f.  ", (float)lux4);
  Serial.printf("time: %d", loop_timer);
  Serial.println("-------");

  
  alignment_check(lux0, lux1, lux2, lux3, lux4);
  
  while (millis()-loop_timer<LOOP_PERIOD);
  loop_timer = millis();
}

void alignment_check(float lux0, float lux1, float lux2, float lux3, float lux4){
 
    if ((lux0 < 500.0)&(lux1 < 500.0)&(lux2 < 500.0)&(lux3 < 500.0)&(lux4 < 500.0)) {
    tft.fillScreen(TFT_WHITE); //fill background
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font
    tft.setCursor(0, 0, 2);
    tft.printf("no laser detected");
  Serial.println("no laser detected");
    } else  if (lux1 > lux0) {
     tft.fillScreen(TFT_WHITE); //fill background
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font
     tft.setCursor(0, 0, 2);
     tft.printf("Move right 2");
  } else if (lux2 > lux0) {
    tft.fillScreen(TFT_WHITE); //fill background
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font
    tft.setCursor(0, 0, 2);
    tft.printf("Move right 1");
  } else if (lux3 > lux0) {
    tft.fillScreen(TFT_WHITE); //fill background
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font
    tft.setCursor(0, 0, 2);
    tft.printf("Move left 1");
  } else if (lux4 > lux0) {
    tft.fillScreen(TFT_WHITE); //fill background
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font
    tft.setCursor(0, 0, 2);
    tft.printf("Move left 2");
  } else if ((lux0 > lux1)&(lux0 > lux2)&(lux0 > lux3)&(lux0 > lux4)) {
    tft.fillScreen(TFT_WHITE); //fill background
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font
    tft.setCursor(0, 0, 2);
    tft.printf("IN THE MIDDLE");
  }
  
}
