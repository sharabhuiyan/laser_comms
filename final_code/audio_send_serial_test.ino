#include <TFT_eSPI.h>
#include <SPI.h>
#include <stdio.h>
#include <WiFi.h>
#include <ArduinoJson.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

// ---------------
// FOR SERIAL COMM
// ---------------

#define RXD1 9
#define TXD1 10

// ---------------------
// FOR GET/POST REQUESTS
// ---------------------
uint8_t channel = 11; //network channel on 2.4 GHz
byte bssid[] = {0xD4, 0x20, 0xB0, 0x56, 0xB3, 0xE3}; //6 byte MAC address of AP you're targeting.

const char network[] = "RLE";
const char password[] = "";

const uint8_t LOOP_PERIOD = 10; //milliseconds
uint32_t primary_timer = 0;

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 10000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
uint16_t *subArr;

// ---------
// FOR AUDIO
// ---------
const int SONG_LENGTH = 447; // length of notes list being sent down (chnages from song to song)
float curr_sounds[SONG_LENGTH];
float song_note_period = 100;//150; //changes from song to song but 150 should be decent for most sounds (can easily look up for other songs)
bool new_song = true;

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

// -------------
// IMAGE & AUDIO
// -------------

uint8_t data[64] = {0};

const int HEIGHT = 160;
const int WIDTH = 128;
const int size = 2*WIDTH;
const int message_size = size;
uint8_t message[message_size] = { 0 };
int bit_count;
uint8_t PROGMEM image_chunk[size] = { 0 };
int buffer_start;
int buffer_end;
int chunk_count;

char image_file[] = "tft_test_image.png";
uint16_t curr_image[32];
uint32_t width_count = 0; // use these to wrap around in loop to make sure we do not exceed the length of the screen 
int height_count = 0; // use these dimension variables to make sure that we are filling the screen properly, essentially a state variable that helps us check that our image has been fully uploaded
bool new_image = true;
bool using_chunks = false;

// ---------------------
// ENCODER STATE MACHINE
// ---------------------
bool sending = false;
const byte LASER_ON[] = { 0b11111111 };
const byte AUDIO_SYNCH_SEQ[] = { 0b11111111, 0b00000000 };
const byte IMAGE_SYNCH_SEQ[] = { 0b00000000, 0b11111111 };
bool started;
const int BUTTON = 45; //pin connected to button 

enum alignment_state{ALIGN, WAIT, ENCODE};
enum alignment_state align_state;

enum transmission_state{GET_AUDIO, TRANSMIT_AUDIO, GET_IMAGE, TRANSMIT_IMAGE, DEAD_STATE};
enum transmission_state state;
uint16_t last_time;

// ----------------------------
// CODE TO CONNECT TO WIFI
// From lab assignments!
// ----------------------------
void connect_to_wifi() {
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
}


// ---------------------------------------------
// ---------------------------------------------
// ***************** SETUP *********************
// ---------------------------------------------
// ---------------------------------------------

void setup(void) {

  // Begin serial communication
  Serial.begin(38400, SERIAL_8N1);
  Serial1.begin(38400, SERIAL_8N1, RXD1, TXD1);
  while (!Serial); // wait for Serial to show up
  delay(50); //pause to make sure comms get set up
//
//  analogReadResolution(12);
//  pinMode(11, OUTPUT); 

  // Initialize TFT screen
  tft.init();
  tft.setSwapBytes(true);
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  
  // Init for image
  bit_count = 0;
  buffer_start = 0;
  buffer_end = 16;

  chunk_count = 0; 

  // ----------------------
  // ----------------------
  // *** SET TYPE HERE ****
  
  state = GET_IMAGE; // SET THIS TO GET_AUDIO OR GET_IMAGE
  
  // ** AUDIO OR IMAGE ****
  // ----------------------
  // ----------------------
  
  // connect to wifi
  connect_to_wifi();

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

  started = false;
  pinMode(BUTTON, INPUT_PULLUP);

  align_state = ALIGN;
}


// ---------------------------------------------
// ---------------------------------------------
// *************** MAIN LOOP *******************
// ---------------------------------------------
// ---------------------------------------------

void loop() {

  switch(align_state) {

    case ALIGN:

      if (digitalRead(BUTTON)==0){ align_state = WAIT; last_time = millis(); }
      else { Serial1.write(LASER_ON, 1); }
      
      break;

    case WAIT:

      if (digitalRead(BUTTON)==0 && millis() - last_time > 500){ align_state = ENCODE; }
      else { Serial1.flush(); }
      
      break;

    case ENCODE:

      switch(state) {
    
        // ~~~~~~~~~~~~~~~
        // ~~ GET_AUDIO ~~
        // ~~~~~~~~~~~~~~~
        
        case GET_AUDIO:
    
          if(new_song){
            
            // GET request for new song
            memset(curr_sounds, 0.0, SONG_LENGTH);
            memset(request_buffer, 0, IN_BUFFER_SIZE);
            sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/send_songs_pwm.py?song HTTP/1.1\r\n");
            strcat(request_buffer, "Host: 608dev-2.net\r\n");
            strcat(request_buffer,"\r\n"); //new line from header to body
            do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
            
            Serial.println(response_buffer);
    
            // Process GET request result
            new_song = false;
            char* trimmed_res = response_buffer;
            while (*trimmed_res != '['){ trimmed_res++; }
            char* end_of_res = response_buffer + strlen(response_buffer);
            while(*end_of_res != ']'){
              *end_of_res =0;
              end_of_res--;
            }
            float_extractor(trimmed_res, curr_sounds, ',');
            new_song = false;
            
          }
          
          state = TRANSMIT_AUDIO;
          chunk_count = 0;
    
          break;
    
          
        // ~~~~~~~~~~~~~~~~~~~~
        // ~~ TRANSMIT_AUDIO ~~
        // ~~~~~~~~~~~~~~~~~~~~
    
        case TRANSMIT_AUDIO:
    
          if (!started) {
            // Send audio starting sequence
            Serial1.write(AUDIO_SYNCH_SEQ, 2);
            started = 1;
            state = TRANSMIT_AUDIO;
          } else {    
            // Fill buffer with song info
            uint8_t data[64];
            if (chunk_count*16 < SONG_LENGTH) {
              for (int i = 0; i < 16; i++){
                // for converting between floats & byte arrays
                union {
                  byte b[4];
                  float fval;
                } sample;
                if (chunk_count*16+i < SONG_LENGTH){
                  // float --> 4 bytes
                  sample.fval = curr_sounds[chunk_count*16+i];
                  for(int j = 0; j < 4; j++){ data[4*i+j] = sample.b[j]; } 
                }    
              }
              Serial1.write(data, 64);
              chunk_count += 1;            
            }
          }
    
          break;
    
        // ~~~~~~~~~~~~~~~
        // ~~ GET_IMAGE ~~
        // ~~~~~~~~~~~~~~~
    
        case GET_IMAGE:
    
          if (new_image || using_chunks) {
    
            // GET request for image
            memset(request_buffer, 0, IN_BUFFER_SIZE);
            if (new_image) {
              // GET request for new image
              sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/send_image.py?new_image=%s HTTP/1.1\r\n", image_file);
            } else {
              // GET request for image chunk
              sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/send_image.py?chunk=%d HTTP/1.1\r\n", height_count);
            }
            strcat(request_buffer, "Host: 608dev-2.net\r\n");
            strcat(request_buffer,"\r\n"); //new line from header to body
            do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
            //Serial.println("NEW IMAGE RES");
            Serial.println(response_buffer);
      
            // Parse GET request output
            memset(curr_image, 0, 32); 
            DynamicJsonDocument doc(6000);
            DeserializationError error = deserializeJson(doc, response_buffer);
            if (error) {
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.f_str());
            }
            // Convert image of uint16_t to bytes
            memset(data, 0, 64);
            for (int i =0; i<32; i++){
              // Serial.println(i);
              char test [10] = ""; 
              strcpy(test, doc["pixels"][i]);
              //Serial.println(test);
              curr_image[i]= strtol(test, NULL, 16);
              Serial.println(curr_image[i], HEX);
              data[2*i] = (uint8_t)(curr_image[i] >> 8);
              data[2*i+1] = (uint8_t)(curr_image[i] & 0x00FF);
              Serial.println(data[2*i], HEX);
              Serial.println(data[2*i+1], HEX);
              Serial.println();
              // Serial.println(curr_image[i]);
            }
    //        // for testing coversion is working
    //        memset(curr_image, 0, 32); 
    //        for (int i=0; i<32; i++) {
    //          curr_image[i] = ((uint16_t)data[2*i] << 8) + (uint16_t)data[2*i+1];
    //        }
            tft.pushImage((int32_t)(32*(height_count%4)), (int32_t)(height_count/4), (int32_t)32, (int32_t)1, curr_image);
      
            if (new_image) {
              height_count++;
              new_image = false;
              using_chunks = true;
            } else {
              height_count++;
              if(height_count >= 640){
                using_chunks = false;
                height_count = 0;
              }
            }
      
            state = TRANSMIT_IMAGE;
            bit_count = 0;
            
          }
    
          break;
    
        // ~~~~~~~~~~~~~~~~~~~~
        // ~~ TRANSMIT_IMAGE ~~
        // ~~~~~~~~~~~~~~~~~~~~
    
        case TRANSMIT_IMAGE:
    
          if (!started) {
            Serial1.write(IMAGE_SYNCH_SEQ, 4);
            started = true;
          }
          Serial1.write(data, 64);
          state = GET_IMAGE;
    
          break;
    
        case DEAD_STATE:
    
          break;
      }
    
      break;
  }
  
}
