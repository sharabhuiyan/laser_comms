//#include <string.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <stdio.h>
#include <WiFi.h>
#include <ArduinoJson.h>
//#include "dome_image.h"

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

#define RXD1 9
#define TXD1 10
/*
INIT CODE FROM IMAGE DISPLAY
*/
uint8_t channel = 11; //network channel on 2.4 GHz
byte bssid[] = {0xD4, 0x20, 0xB0, 0x56, 0xB3, 0xE3}; //6 byte MAC address of AP you're targeting.

const int SONG_LENGTH = 840; // length of notes list being sent down (chnages from song to song)
float curr_sounds[SONG_LENGTH];
float song_note_period = 150; //changes from song to song but 150 should be decent for most sounds (can easily look up for other songs)
bool new_song = true;

const char network[] = "EECS_Labs";
const char password[] = "";

const uint8_t LOOP_PERIOD = 10; //milliseconds
uint32_t primary_timer = 0;

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 10000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
uint16_t *subArr;

bool sending = false;
int chunk = 0;

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

/*
INIT CODE FROM ENCODE
*/

int x = 0;
uint16_t timer;

const int HEIGHT = 160;
const int WIDTH = 128;
const int size = 2*WIDTH;
const int message_size = size;
uint8_t message[message_size] = { 0 };
uint8_t data[64] = {0};
int bit_count;
uint8_t PROGMEM image_chunk[size] = { 0 };
int buffer_start;
int buffer_end;
int chunk_count;

enum transmission_state{GET_AUDIO, DELAY1, TRANSMIT, DELAY2};
enum transmission_state state;

void setup(void) {

  Serial.begin(2400, SERIAL_8N1);
  Serial1.begin(2400, SERIAL_8N1, RXD1, TXD1);
  while (!Serial); // wait for Serial to show up
  delay(50); //pause to make sure comms get set up

  analogReadResolution(12);
  pinMode(11, OUTPUT); 

  tft.init();
  tft.setSwapBytes(true);
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  bit_count = 0;
  timer = millis();

  buffer_start = 0;
  buffer_end = 16;
  chunk_count = 0;
  
  state = GET_AUDIO;

 int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
      uint8_t* cc = WiFi.BSSID(i);
      for (int k = 0; k < 6; k++) {
        Serial.print(*cc, HEX);
        if (k != 5) Serial.print(":");
        cc++;
      }
      Serial.println("");
    }
  }
  delay(100); //wait a bit (100 ms)

  //if using regular connection use line below:
  WiFi.begin(network, password);
  //if using channel/mac specification for crowded bands use the following:
  //WiFi.begin(network, password, channel, bssid);


  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }

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


void loop() {

  // tft.pushImage(0, 0, 160, 128, image_bitmap);
  // tft.pushImage(20, 30, 160, 50, test_map);
  // tft.pushImage(0, 0, 160, 50, test_map);
  if(state == GET_AUDIO){

    if(new_song){
      memset(curr_sounds, 0.0, SONG_LENGTH);
      memset(request_buffer, 0, IN_BUFFER_SIZE);
      sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/send_songs_pwm.py?song HTTP/1.1\r\n");
      strcat(request_buffer, "Host: 608dev-2.net\r\n");
      strcat(request_buffer,"\r\n"); //new line from header to body
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
      //Serial.println("NEW IMAGE RES");
      Serial.println(response_buffer);
      new_song = false;
      char* trimmed_res = response_buffer;
    // strcpy(curr, doc["right_vals"]); 
      while (*trimmed_res != '['){
        trimmed_res++;
      }
      char* end_of_res = response_buffer + strlen(response_buffer);
      while(*end_of_res != ']'){
        *end_of_res =0;
        end_of_res--;
      }
      float_extractor(trimmed_res, curr_sounds, ',');
      new_song = false;
      //using_chunks = true;
    }

    state = DELAY1;
    chunk = 0;
    //IMP
  }
  else if (state == DELAY1) {
//    delay(300);
    state = TRANSMIT;
  }
  else if (state == TRANSMIT){
    uint8_t data[64];
    if(chunk*16 < SONG_LENGTH) {
      for(int i = 0; i < 16; i++){
        union {
          byte b[4];
          float fval;
        } sample;
        if(chunk*16+i < SONG_LENGTH){
          sample.fval = curr_sounds[chunk*16+i];

          for(int j = 0; j < 4; j++){
            data[4*i+j] = sample.b[j];
          } 
        }    
      }
      //Serial.println(data);
      Serial1.write(data, 64);
      chunk += 1;            
    }
    else {
      chunk = 0;
      state = DELAY2;
    }

  }
  else if (state == DELAY2){
//    delay(300);
    state = GET_AUDIO;
  }
}

  // if(height_count>=0 && height_count<128){ // LOCAL Version
  //   uint16_t subArr[160];
  //   get_subarray(&test_map[height_count*160], subArr, 160);
  //   tft.pushImage(0, (int32_t)height_count, (int32_t)160, (int32_t)128, subArr);
  //   for(int j =0; j<160; j++){
  //   Serial.print(subArr[j]);
  //   Serial.print(" ");
  //   }
  //   height_count++;
  //   Serial.println("------------------");
  // }
  // else{
  //   // tft.fillScreen(TFT_BLACK);
  //   height_count = 0;
  // }

void get_subarray(uint16_t originalArray[], uint16_t subArray[], int n)
{
    for (int i = 0; i < n; i++)
    {
        subArray[i] = originalArray[i];
    }
}
