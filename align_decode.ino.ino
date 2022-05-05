#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
int bright_inc = 0;             //auto-increasing value that goes from 0 to 4095 (approx) in steps of 100
int pwm_channel = 4;
int freq = 100;
int resolution = 12;
const int LOOP_PERIOD = 50;     //speed of main loop
const uint8_t AlIGN = 0; //example definition
const uint8_t DECODE = 1; //example...
int loop_timer;                 //used for loop timing
const int BUTTON = 45; //pin connected to button 
uint8_t state;
#define RXD1 9
#define TXD1 10

byte byte1_read;
byte byte2_read;

int i;
int chunk_count;
const int num_chunks = 16;

const int HEIGHT = 160;
const int WIDTH = 128;
const int size = 2*WIDTH;

const int BUFFER_MAX = 32;
uint16_t image[BUFFER_MAX] = { 0 };



int test_bit(byte b, int pos) {
  // adapted from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
  unsigned int flag = 1; // flag = 000.....00001
  flag = flag << pos; // shifted k positions
  if (b & flag) {
    return 1;
  } else {
    return 0;
  }
};

byte reverse(byte b) {
  // from https://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

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
//  delay(100); 
  //TFT initializtation stuff:
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_WHITE); //fill background
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font
  tft.setCursor(0, 0, 2); //set cursor
  loop_timer = millis();
  state = ALIGN;
  pinMode(BUTTON, INPUT_PULLUP);
  i = 0;
  chunk_count = 0;
}


void loop() {
  align_encode();
}

void align_decode() {
  switch(state){
    case ALIGN:
    if (digitalRead(BUTTON)==0){
      tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  
    Serial.begin(9600, SERIAL_8N1); // initialize UART with baud rate of 9600 bps
    Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);
    Serial1.flush();
      state = ENCODE;
    }
    float vout0 = analogRead(8)*(3.3/4096.0); // middle photosensor
    float vout1 = analogRead(6)*(3.3/4096.0); // left2 photosensor
    float vout2 = analogRead(7)*(3.3/4096.0); // left1 photosensor
    float vout3 = analogRead(9)*(3.3/4096.0); // right1 photosensor
    float vout4 = analogRead(11)*(3.3/4096.0); // right2 photosensor
  
    float lux0 = brightnessExtractor(vout0);
    float lux1 = brightnessExtractor(vout1);
    float lux2 = brightnessExtractor(vout2);
    float lux3 = brightnessExtractor(vout3);
    float lux4 = brightnessExtractor(vout4);
    
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
      break;
    case DECODE:
      if (Serial1.available() > 2){ 
        byte1_read = reverse(Serial1.read());
        byte2_read = reverse(Serial1.read());
        // Serial.println(byte_read, HEX);
        // Serial.print("0b");
        // for (int i = 0; i < 8; i++) {
        //   Serial.print(test_bit(byte_read,i));
        // }
        // Serial.println();
        uint16_t pixel = (byte1_read << 8) + (uint16_t)byte2_read;
        image[i] = pixel;
        i++;
    
      } 
    
      if (i >= BUFFER_MAX) {
        tft.pushImage((int32_t)(chunk_count%4), (int32_t)(chunk_count/4), (int32_t)32, (int32_t)1, image);
        chunk_count += 1;
        i = 0;
    
      }
      break;
  }
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
