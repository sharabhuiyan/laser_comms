#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


#define RXD1 9
#define TXD1 10


int i;
int chunk_count;
const int num_chunks = 16;

const int HEIGHT = 160;
const int WIDTH = 128;
const int size = 2*WIDTH;

const int BUFFER_MAX = 16;
//uint16_t image[BUFFER_MAX] = { 0 };

const int SONG_LENGTH = 840; // length of notes list being sent down (chnages from song to song)
float curr_sounds[SONG_LENGTH];
float song_note_period = 150; //changes from song to song but 150 should be decent for most sounds (can easily look up for other songs)

//pins for LCD and AUDIO CONTROL
uint8_t LCD_CONTROL = 21;
uint8_t AUDIO_TRANSDUCER = 17;

//PWM Channels. The LCD will still be controlled by channel 0, we'll use channel 1 for audio generation
uint8_t LCD_PWM = 0;
uint8_t AUDIO_PWM = 1;


//global variables to help your code remember what the last note was to prevent double-playing a note which can cause audible clicking
float new_note = 0;
float old_note = 0;
int curr_note_num = 0;

void play_song() {
 
  if (curr_note_num < (SONG_LENGTH-1)){
    float curr_note = curr_sounds[curr_note_num];
    ledcWriteTone(AUDIO_PWM, curr_note);
    delay(song_note_period);
    Serial.println(curr_note);
    old_note = new_note;
    curr_note_num += 1; 
  }
  else{
    curr_note_num = 0;
  }
}

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



void setup() {
  // put your setup code here, to run once:
  // Set up TFT screen
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  Serial.begin(2400, SERIAL_8N1); // initialize UART with baud rate of 9600 bps
  Serial1.begin(2400, SERIAL_8N1, RXD1, TXD1);
  Serial1.flush();

  i = 0;

  pinMode(AUDIO_TRANSDUCER, OUTPUT);
  pinMode(LCD_CONTROL, OUTPUT);

  //set up AUDIO_PWM which we will control in this lab for music:
  ledcSetup(AUDIO_PWM, 200, 12);//12 bits of PWM precision
  ledcWrite(AUDIO_PWM, 0); //0 is a 0% duty cycle for the NFET
  ledcAttachPin(AUDIO_TRANSDUCER, AUDIO_PWM);

  //set up the LCD PWM and set it to 
  pinMode(LCD_CONTROL, OUTPUT);
  ledcSetup(LCD_PWM, 100, 12);//12 bits of PWM precision
  ledcWrite(LCD_PWM, 0); //0 is a 0% duty cycle for the PFET...increase if you'd like to dim the LCD.
  ledcAttachPin(LCD_CONTROL, LCD_PWM);

}

bool getting_song = true;

union {
   byte b[4];
   float fval;
} sample;

void loop() {
  // put your main code here, to run repeatedly:
  if(getting_song) {
    if (Serial1.available() > 4){ 
      sample.b[3] = Serial1.read();
      sample.b[2] = Serial1.read();
      sample.b[1] = Serial1.read();
      sample.b[2] = Serial1.read();
      // Serial.print("0b");
      // for (int i = 0; i < 8; i++) {
      //   Serial.print(test_bit(byte_read,i));
      // }
      // Serial.println();
      curr_sounds[i] = sample.fval;
      i++;
    }
    if(i >= SONG_LENGTH) {
      getting_song = false;
    }
  }
  else {
    play_song();
  }
  

}
