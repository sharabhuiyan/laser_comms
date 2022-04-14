//#include "support.ino"
//#include <string.h>
#include <mpu6050_esp32.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <stdio.h>
#include <WiFi.h>
#include <ArduinoJson.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.

const char network[] = "EECS_Labs";
const char password[] = "";

const uint8_t LOOP_PERIOD = 10; //milliseconds
uint32_t primary_timer = 0;

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 10000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

char image_file[] = "tft_test_image.png";
uint16_t image_bitmap[7700];
int width_count = 0; // use these to wrap around in loop to make sure we do not exceed the length of the screen 
int height_count = 0; // use these dimension variables to make sure that we are filling the screen properly, essentially a state variable that helps us check that our image has been fully uploaded

void setup(void) {

  Serial.begin(115200);

  tft.init();
  tft.setSwapBytes(true);
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

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

  memset(request_buffer, 0, IN_BUFFER_SIZE);
  sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/image_get.py?image=%s HTTP/1.1\r\n", image_file);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer,"\r\n"); //new line from header to body
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
  Serial.println(response_buffer);
  // char* start = strchr(response_buffer, '{');
  // char* end = strrchr(response_buffer, '}');


  // sprintf(end+1, "\0");

  // Serial.println(start);     
  // Serial.println(end); 

  DynamicJsonDocument doc(6000);
  // StaticJsonDocument<OUT_BUFFER_SIZE> doc;
  DeserializationError error = deserializeJson(doc, response_buffer);

// // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  // Fetch values.
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do doc["time"].as<long>();

  char test [200] = ""; 
  strcpy(test, doc["pixels"][0]);
  Serial.println(test);

  // JsonObject&  parsed= doc.parseObject(JSONMessage);

  // char res_bitmap[77000] = doc["pixels"]; // memcpy
  for (int i =0; i<100; i++){
    Serial.println(i);
    // Serial.println(doc["pixels"][i]);
    char test [10] = ""; 
    strcpy(test, doc["pixels"][i]);
    // Serial.println(atoi(test));
    image_bitmap[i]= strtol(test, NULL, 16);
  }
  for(int j =0; j<100; j++){
    Serial.println(image_bitmap[j]);
  }
  
//   for(i = 0; i < strlen(res_bitmap); i++)
// {
//   do_something(fooarr[i].data);
// }

}
void int_extractor(char* data_array, int* output_values, char delimiter){
    //your code here
   char delim[2];
   delim[0] = delimiter;
   delim[1] = '\0';
   char* val = strtok(data_array, delim);
   int i =0;
   while(val != NULL){
     output_values[i] += atoi(val);
     val = strtok(NULL, delim);
     i++;
   }
}

void loop() {

  tft.pushImage(0, 0, 240, 320, image_bitmap);
}


/*--------------------
 * draw_stuff Function:
 * 
 * Arguments:
 *    None
 * Return Values:
 *    None
 * Five-state State machine that cycles through various drawings on TFT screen.
 * Uses global variables draw_state (to represent state), and timer, to keep track of transition times
 * Only draws image on change of state!
 * State 0: Simple message!
 * State 1: Draw a one-bit monochrome image of MIT Dome and a title in MIT Colors
 * State 2: Draw Black background with a filled and empty circle of various sizes
 * State 3: Draw yellow background with two rectangles
 * State 4: Draw a bunch of lines green lines on a white backdrop using a for loop
 */

