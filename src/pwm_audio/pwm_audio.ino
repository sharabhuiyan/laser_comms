#include <mpu6050_esp32.h>
#include<math.h>
#include<string.h>
#include <TFT_eSPI.h>
#include <WiFi.h> 
#include <SPI.h>


TFT_eSPI tft = TFT_eSPI();  

uint8_t channel = 11; //network channel on 2.4 GHz
byte bssid[] = {0xD4, 0x20, 0xB0, 0x56, 0xB3, 0xE3}; //6 byte MAC address of AP you're targeting.
const int SONG_LENGTH = 840; // length of notes list being sent down (chnages from song to song)
float curr_sounds[SONG_LENGTH];
float song_note_period = 150; //changes from song to song but 150 should be decent for most sounds (can easily look up for other songs)


const char network[] = "EECS_Labs";
const char password[] = "";

bool new_song = true;

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 10000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

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


void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for Serial to show up
  Wire.begin();
  delay(50); //pause to make sure comms get set up
  tft.init(); //initialize the screen
  tft.setRotation(2); //set rotation for our layout
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

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



void play_song() {
 
  if (curr_note_num < (SONG_LENGTH-1)){
    float curr_note = curr_sounds[curr_note_num];
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

/*----------------------------------
 * char_append Function:
 * Arguments:
 *    char* buff: pointer to character array which we will append a
 *    char c: 
 *    uint16_t buff_size: size of buffer buff
 *    
 * Return value: 
 *    boolean: True if character appended, False if not appended (indicating buffer full)
 */
uint8_t char_append(char* buff, char c, uint16_t buff_size) {
        int len = strlen(buff);
        if (len>buff_size) return false;
        buff[len] = c;
        buff[len+1] = '\0';
        return true;
}

/*----------------------------------
 * do_http_request Function:
 * Arguments:
 *    const char* host: null-terminated char-array containing host to connect to
 *    char* request: null-terminated char-arry containing properly formatted HTTP request
 *    char* response: char-array used as output for function to contain response
 *    uint16_t response_size: size of response buffer (in bytes)
 *    uint16_t response_timeout: duration we'll wait (in ms) for a response from server
 *    uint8_t serial: used for printing debug information to terminal (true prints, false doesn't)
 * Return value:
 *    void (none)
 */
void do_http_request(const char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial){
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n',response,response_size);
      if (serial) Serial.println(response);
      if (strcmp(response,"\r")==0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis()-count>response_timeout) break;
    }
    memset(response, 0, response_size);  
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response,client.read(),OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");  
  }else{
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}  
void float_extractor(char* data_array, float* output_values, char delimiter){
    //your code here
   char delim[2];
   delim[0] = delimiter;
   delim[1] = '\0';
   char* val = strtok(data_array, delim);
   int i =0;
   while(val != NULL){
     output_values[i] = atof(val);
     Serial.println(atof(val));
     val = strtok(NULL, delim);
     i++;
   }
}

void loop() {
  if(new_song){
    memset(curr_sounds, 0.0, SONG_LENGTH);
    memset(request_buffer, 0, IN_BUFFER_SIZE);
    sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/send_songs_pwm.py?song HTTP/1.1\r\n");
    strcat(request_buffer, "Host: 608dev-2.net\r\n");
    strcat(request_buffer,"\r\n"); //new line from header to body
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
    Serial.println("NEW SONG RESULTS");
    Serial.println(response_buffer);
    new_song = false;
    Serial.println("response above");
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
      play_song();
    Serial.println("playyyyyy");

  }
  else{
    play_song();
  }
}