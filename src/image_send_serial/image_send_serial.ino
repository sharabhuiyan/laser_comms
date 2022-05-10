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

char image_file[] = "tft_test_image.png";
//const int bitmap_array_size = 7700;
uint16_t curr_image[32];
// uint16_t image_bitmap[bitmap_array_size];
uint32_t width_count = 0; // use these to wrap around in loop to make sure we do not exceed the length of the screen 
int height_count = 0; // use these dimension variables to make sure that we are filling the screen properly, essentially a state variable that helps us check that our image has been fully uploaded
bool new_image = true;
bool using_chunks = false;
bool sending = false;

byte buf[] = { 0b00000000 };
byte starting_sequence[] = { 0b11110000, 0b11110000 };
byte final_padding[] = { 0b00000000, 0b00000000, 0b00000000, 0b00000000 };


int started;
int ended;

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

enum transmission_state{GET_IMAGE, DELAY1, TRANSMIT, DELAY2};
enum transmission_state state;

void setup(void) {

  Serial.begin(2400, SERIAL_8N1);
  Serial1.begin(2400, SERIAL_8N1, RXD1, TXD1);

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
  
  state = GET_IMAGE;

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


void loop() {

  // tft.pushImage(0, 0, 160, 128, image_bitmap);
  // tft.pushImage(20, 30, 160, 50, test_map);
  // tft.pushImage(0, 0, 160, 50, test_map);
  if(state == GET_IMAGE){

//    if (new_image || using_chunks) {
//      memset(request_buffer, 0, IN_BUFFER_SIZE);
//      
//      if (new_image) {
//        sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/send_image.py?new_image=%s HTTP/1.1\r\n", image_file);
//      } else {
//        sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/send_image.py?chunk=%d HTTP/1.1\r\n", height_count);
//      }
//
//      strcat(request_buffer, "Host: 608dev-2.net\r\n");
//      strcat(request_buffer,"\r\n"); //new line from header to body
//      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
//      //Serial.println("NEW IMAGE RES");
//      Serial.println(response_buffer);
//
//    
//      memset(curr_image, 0, 32); 
//      DynamicJsonDocument doc(6000);
//      DeserializationError error = deserializeJson(doc, response_buffer);
//      if (error) {
//        Serial.print(F("deserializeJson() failed: "));
//        Serial.println(error.f_str());
//      }
//      memset(data, 0, 64);
//      for (int i =0; i<32; i++){
//        // Serial.println(i);
//        char test [10] = ""; 
//        strcpy(test, doc["pixels"][i]);
//        //Serial.println(test);
//        curr_image[i]= strtol(test, NULL, 16);
//        Serial.println(curr_image[i], HEX);
//        data[2*i] = (uint8_t)(curr_image[i] >> 8);
//        data[2*i+1] = (uint8_t)(curr_image[i] & 0x00FF);
//        Serial.println(data[2*i], HEX);
//        Serial.println(data[2*i+1], HEX);
//        Serial.println();
//        // Serial.println(curr_image[i]);
//      }
//      memset(curr_image, 0, 32); 
//      for (int i=0; i<32; i++) {
//        curr_image[i] = ((uint16_t)data[2*i] << 8) + (uint16_t)data[2*i+1];
//      }
//      tft.pushImage((int32_t)(32*(height_count%4)), (int32_t)(height_count/4), (int32_t)32, (int32_t)1, curr_image);
//
//      if (new_image) {
//        height_count++;
//        new_image = false;
//        using_chunks = true;
//      } else {
//        height_count++;
//        if(height_count >= 640){
//          using_chunks = false;
//          height_count = 0;
//        }
//      }
//
//      state = DELAY1;
//      bit_count = 0;
//      
//    }
//    

    if(new_image){
      memset(request_buffer, 0, IN_BUFFER_SIZE);
      sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/send_image.py?new_image=%s HTTP/1.1\r\n", image_file);
      strcat(request_buffer, "Host: 608dev-2.net\r\n");
      strcat(request_buffer,"\r\n"); //new line from header to body
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
      //Serial.println("NEW IMAGE RES");
      Serial.println(response_buffer);

    
      memset(curr_image, 0, 32); 
      DynamicJsonDocument doc(6000);
      DeserializationError error = deserializeJson(doc, response_buffer);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
      }
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
      memset(curr_image, 0, 32); 
      for (int i=0; i<32; i++) {
        curr_image[i] = ((uint16_t)data[2*i] << 8) + (uint16_t)data[2*i+1];
      }
      tft.pushImage((int32_t)(32*(height_count%4)), (int32_t)(height_count/4), (int32_t)32, (int32_t)1, curr_image);

      // memset(message, 0, message_size);
      // message = data;
      //manchester_encode(message, data, size);

      height_count++;
      new_image = false;
      using_chunks = true;
    }
    //IMP
    else{
      if(using_chunks){
        memset(request_buffer, 0, IN_BUFFER_SIZE);
        sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/send_image.py?chunk=%d HTTP/1.1\r\n", height_count);
        strcat(request_buffer, "Host: 608dev-2.net\r\n");
        strcat(request_buffer,"\r\n"); //new line from header to body
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
        Serial.println(response_buffer);
        memset(curr_image, 0, 32);    
        DynamicJsonDocument doc(6000);
        DeserializationError error = deserializeJson(doc, response_buffer);
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
          }
          memset(data, 0, 64);
          for (int i =0; i<32; i++){
            // Serial.println(i);
            char test [10] = ""; 
            strcpy(test, doc["pixels"][i]);
            // Serial.println(test);
            curr_image[i]= strtol(test, NULL, 16);
            Serial.println(curr_image[i], HEX);
            data[2*i] = (uint8_t)(curr_image[i] >> 8);
            data[2*i+1] = (uint8_t)(curr_image[i] & 0x00FF);
            Serial.println(data[2*i], HEX);
            Serial.println(data[2*i+1], HEX);
            Serial.println();
            // Serial.println(curr_image[i]);
          }
          //Serial.println(height_count);
        memset(curr_image, 0, 32); 
        for (int i=0; i<32; i++) {
          curr_image[i] = ((uint16_t)data[2*i] << 8) + (uint16_t)data[2*i+1];
        }
        tft.pushImage((int32_t)(32*(height_count%4)), (int32_t)(height_count/4), (int32_t)32, (int32_t)1, curr_image);

        // memset(message, 0, message_size);
        // message = data;
        //manchester_encode(message, data, size);
        
        // for(int j =0; j<160; j++){
        //   Serial.print(subArr[j]);
        //   Serial.print(" ");
        // }
        height_count++;
        if(height_count >= 640){
          using_chunks = false;
          height_count = 0;
        }
      }
      state = DELAY1;
      bit_count = 0;
    }
  }
  else if (state == DELAY1) {
//    delay(300);
    state = TRANSMIT;
    ended = 0;
  }
  else if (state == TRANSMIT){
    // if (bit_count < 8*message_size) {
    //   int bit = test_bit(message, bit_count);
    //   if (bit == 1) {
    //     one_signal(5);
    //   } else {
    //     zero_signal(5);
    //   }
    //   bit_count++;
    // }

    // if (bit_count >= 8*message_size) {
    //   zero_signal(1);
    //   state = DELAY2;
    // 
    if (!started) {
        Serial1.write(starting_sequence, 2);
        started = 1;
      }
    Serial1.write(data, 64);
    state = DELAY2;

  }
  else if (state == DELAY2){
//    delay(300);
    if (!ended) {
      Serial1.write(final_padding, 4);
      ended = 1;
    }
    state = GET_IMAGE;
    started = 0;
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
