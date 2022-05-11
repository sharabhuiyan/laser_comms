#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

// Serial pins
#define RXD1 9
#define TXD1 10

// ---------------
// FOR DOME BITMAP
// ---------------
// byte byte_read;

// int i;
// int chunk_count;
// const int IMAGE_SIZE = 16 * 128;
// const int num_chunks = 16;
// const int BUFFER_MAX = 16 * num_chunks;
// uint8_t PROGMEM image[BUFFER_MAX] = { 0 };


// ---------------
// FOR COLOR IMAGE
// ---------------
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


// ---------
// FOR AUDIO
// ---------

// for audio
uint8_t AUDIO_TRANSDUCER = 17;

//PWM Channels. The LCD will still be controlled by channel 0, we'll use channel 1 for audio generation
uint8_t LCD_PWM = 0;
uint8_t AUDIO_PWM = 1;

//global variables to help your code remember what the last note was to prevent double-playing a note which can cause audible clicking
float new_note = 0;
float old_note = 0;
int curr_note_num = 0;
const int SONG_LENGTH = 447; // length of notes list being sent down (chnages from song to song)
float song_note_period = 100;//150; //changes from song to song but 150 should be decent for most sounds (can easily look up for other songs)
float song[SONG_LENGTH] = { 0 };

// for converting from float --> 4 bytes
union {
  float f;
  byte b[4];
} sample;

// ---------------
// STATE VARIABLES
// ---------------
enum alignment_state{ALIGN, DECODE};
enum alignment_state align_state;

const byte starting_sequence[] = { 0b11111111, 0b00000000 };
// const byte IMAGE_SYNCH_SEQ[] = { 0b00000000, 0b11111111 };

int started;
bool getting_song = true;
bool is_image;

int bright_inc = 0;             //auto-increasing value that goes from 0 to 4095 (approx) in steps of 100
int pwm_channel = 4;
int freq = 100;
int resolution = 12;

const int BUTTON = 45; //pin connected to button 
const int LOOP_PERIOD = 50;     //speed of main loop
int loop_timer;                 //used for loop timing


// ---------------------------------------------
// ---------------------------------------------
// ***************** FOR AUDIO *****************
// ---------------------------------------------
// ---------------------------------------------

void play_song() {
 
  if (curr_note_num < (SONG_LENGTH-1)){
    float curr_note = song[curr_note_num];
    ledcWriteTone(AUDIO_PWM, curr_note);
    delay(song_note_period);
    Serial.println(curr_note);
    tft.setCursor(0,0,4);
    if (old_note>curr_note){
      tft.fillScreen(TFT_BLUE);
    }
    else{
      tft.fillScreen(TFT_ORANGE);
    }
    tft.println(curr_note);
    old_note = new_note;
    curr_note_num += 1; 
  }
  else{
    curr_note_num = 0;
  }
}

// ---------------------------------------------
// ---------------------------------------------
// ************** FOR ALIGNMENT ****************
// ---------------------------------------------
// ---------------------------------------------

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


void align_decode() {
  
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


// int test_bit(byte b, int pos) {
//   // adapted from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
//   unsigned int flag = 1; // flag = 000.....00001
//   flag = flag << pos; // shifted k positions
//   if (b & flag) {
//     return 1;
//   } else {
//     return 0;
//   }
// };

// byte reverse(byte b) {
//   // from https://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
//    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
//    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
//    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
//    return b;
// }

// ---------------------------------------------
// ---------------------------------------------
// ***************** SETUP *********************
// ---------------------------------------------
// ---------------------------------------------

void setup() {
  // put your setup code here, to run once:
  // Set up TFT screen
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  Serial.begin(38400, SERIAL_8N1); // initialize UART with baud rate of 9600 bps
  Serial1.begin(38400, SERIAL_8N1, RXD1, TXD1);
  Serial1.flush();

  i = 0;
  chunk_count = 0;
  started = 0;

  //set up AUDIO_PWM which we will control in this lab for music:
  ledcSetup(AUDIO_PWM, 200, 12);//12 bits of PWM precision
  ledcWrite(AUDIO_PWM, 0); //0 is a 0% duty cycle for the NFET
  ledcAttachPin(AUDIO_TRANSDUCER, AUDIO_PWM);

  memset(image, 0, sizeof image);
  memset(song, 0, sizeof song);

  pinMode(BUTTON, INPUT_PULLUP);

  align_state = ALIGN;
  is_image = true;
  loop_timer = millis();

}


// ---------------------------------------------
// ---------------------------------------------
// *************** MAIN LOOP *******************
// ---------------------------------------------
// ---------------------------------------------

void loop() {

  if (is_image) {

    if (Serial1.available() > 2) {
      // if (started){
      byte2_read = Serial1.read();
      byte1_read = Serial1.read();
      // Serial.print("0b");
      // for (int i = 0; i < 8; i++) {
      //   Serial.print(test_bit(byte_read,i));
      // }
      // Serial.println();
      uint16_t pixel = ((((uint16_t)byte1_read) << 8) & 0xFF00) + ((uint16_t)byte2_read & 0x00FF);
      // Serial.println(pixel, HEX);
      image[i] = pixel;
      i++;
      // } else {
      //   // started = 1;
      //   byte b1 = Serial1.read();
      //   byte b2 = Serial1.read();
      //   // Serial.print(byte2_read, HEX);
      //   // Serial.print(", ");
      //   // Serial.print(byte1_read, HEX);
      //   // Serial.println();
      //   if (b1 == IMAGE_SYNCH_SEQ[0] && b2 == IMAGE_SYNCH_SEQ[1] ) {
      //     started = 1;
      //     Serial.println("STARTED");
      // }
    } 

    if (i >= BUFFER_MAX) {
      tft.pushImage((int32_t)(32*(chunk_count%4)), (int32_t)(chunk_count/4), (int32_t)32, (int32_t)1, image);
      chunk_count += 1;
      i = 0;
      memset(image, 0, sizeof image);
    }

  } else {

    // ~~~~~~~~~~~~~
    // ~~~ AUDIO ~~~
    // ~~~~~~~~~~~~~


    if (getting_song) {
      if (Serial1.available() > 4){ 

        if (started) {

          sample.b[0] = Serial1.read();
          sample.b[1] = Serial1.read();
          sample.b[2] = Serial1.read();
          sample.b[3] = Serial1.read();
          // Serial.print(sample.b[3]);
          // Serial.print(" , ");
          // Serial.print(sample.b[2]);
          // Serial.print(" , ");
          // Serial.print(sample.b[1]);
          // Serial.print(" , ");
          // Serial.println(sample.b[0]);
          // Serial.print("0b");
          // for (int i = 0; i < 8; i++) {
          //   Serial.print(test_bit(byte_read,i));
          // }
          // Serial.println();
          // Serial.println(sample.f);
          song[i] = sample.f;
          i++;
        } else { // IDENTIFY STARTING SEQUENCE
          byte byte1_read = Serial1.read();
          byte byte2_read = Serial1.read();
          // Serial.print(byte2_read, HEX);
          // Serial.print(", ");
          // Serial.print(byte1_read, HEX);
          // Serial.println();
          if (byte1_read == starting_sequence[0] && byte2_read == starting_sequence[1]) {
            started = 1;
            Serial.println("STARTED");
          }
        }
      }
      if(i >= SONG_LENGTH) {
        getting_song = false;
      }
    } else {
      play_song();
    }

  }

}


