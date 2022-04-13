#include<string.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "dome_image.h"
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
const int BUTTON = 45; //connect to pin 45
const uint8_t PAUSE = 60;

uint8_t draw_state;  //used for remembering state
uint8_t previous_value;  //used for remembering previous button
 
void setup(){
    Serial.begin(115200); //initialize serial!
    tft.init(); //initialize the screen
    tft.setRotation(2); //set rotation for our layout
    pinMode(BUTTON,INPUT_PULLUP); //sets IO pin 45 as an input which defaults to a 3.3V signal when unconnected and 0V when the switch is pushed
    draw_state = 0; //initialize to 0
    previous_value = 1; //initialize to 1
    tft.begin();
    tft.setSwapBytes(true);
    tft.setRotation(2);
    tft.pushImage(0,0, dome_608_width, dome_608_height, test_map);
}


void loop(){
  uint8_t value = digitalRead(BUTTON); //get reading
}