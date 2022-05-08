#include <mpu6050_esp32.h>
#include<math.h>
#include<string.h>
#include <TFT_eSPI.h>
#include <WiFi.h> 
#include <ArduinoJson.h>
#include <SPI.h>
#include <driver/dac.h>


TFT_eSPI tft = TFT_eSPI();  

uint8_t channel = 11; //network channel on 2.4 GHz
byte bssid[] = {0xD4, 0x20, 0xB0, 0x56, 0xB3, 0xE3}; //6 byte MAC address of AP you're targeting.

const char network[] = "EECS_Labs";
const char password[] = "";

const uint8_t LOOP_PERIOD = 10; //milliseconds
uint32_t primary_timer = 0;
bool new_image = true;
bool using_chunks = false;
int micros_timer = 0;
int chunk_count = 1;

//int humble_song [] = [128, 127, 128, 127, 128, 127, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 128, 127, 128, 128, 128, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 128, 128, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 128, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 127, 128, 127, 128, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 127, 128, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 128, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 128, 128, 127, 128, 127, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 127, 128, 127, 128, 127, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 128, 128, 128, 128, 128, 127, 128, 128, 127, 128, 127, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 127, 128, 127, 128, 127, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 127, 128, 127, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 127, 128, 128, 128, 128, 127, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 128, 128, 128, 127, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 127, 128, 127, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 128, 128, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 128, 128, 128, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 128, 128, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 127, 128, 128, 127, 128, 127, 128]

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 10000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

// Minimalist C Structure for containing a musical "riff"
//can be used with any "type" of note. Depending on riff and pauses,
// you may need treat these as eighth or sixteenth notes or 32nd notes...sorta depends
struct Riff {
  double notes[64]; //the notes (array of doubles containing frequencies in Hz. I used https://pages.mtu.edu/~suits/notefreqs.html
  int length; //number of notes (essentially length of array.
  float note_period; //the timing of each note in milliseconds (take bpm, scale appropriately for note (sixteenth note would be 4 since four per quarter note) then
};

//Kendrick Lamar's HUMBLE
// needs 32nd notes to sound correct...song is 76 bpm, so that's 100.15 ms per 32nd note though the versions on youtube vary by a few ms so i fudged it
Riff humble = {{
    311.13, 311.13, 0 , 0, 311.13, 311.13, 0, 0,  //beat 1
    0, 0, 0, 0, 329.63, 329.63, 0, 0,             //beat 2
    311.13, 311.13, 0, 0, 207.65, 0, 207.65, 0,   //beat 3
    0, 0, 207.65, 207.65, 329.63, 329.63, 0, 0    //beat 4
  }, 32, 100.15 //32 32nd notes in measure, 100.15 ms per 32nd note
};

//Beyonce aka Sasha Fierce's song Formation off of Lemonade. Don't have the echo effect
// needs 16th notes and two measures to sound correct...song is 123 bpm, so that's around 120.95 ms per 16th note though the versions on youtube
// vary even within the same song quite a bit, so sorta I matched to her video for the song.
Riff formation = {{
    261.63, 0, 261.63 , 0,   0, 0, 0, 0, 261.63, 0, 0, 0, 0, 0, 0, 0, //measure 1 Y'all haters corny with that illuminati messssss
    311.13, 0, 311.13 , 0,   0, 0, 0, 0, 311.13, 0, 0, 0, 0, 0, 0, 0 //measure 2 Paparazzi catch my fly and my cocky freshhhhhhh
  }, 32, 120.95 //32 32nd notes in measure, 120.95 ms per 32nd note
};

//Justin Bieber's Sorry:
// only need the 16th notes to make sound correct. 100 beats (notes) per minute in song means 150 ms per 16th note
// riff starts right at the doo doo do do do do doo part rather than the 2-ish beats leading up to it. That way you
// can go right into the good part with the song. Sorry if that's confusing.
Riff sorry = {{ 1046.50, 1244.51 , 1567.98, 0.0, 1567.98, 0.0, 1396.91, 1244.51, 1046.50, 0, 0, 0, 0, 0, 0, 0}, 16, 150};


//create a song_to_play Riff that is one of the three ones above. 
Riff song_to_play = humble;  //select one of the riff songs


const uint32_t READING_PERIOD = 150; //milliseconds
double MULT = 1.059463094359; //12th root of 2 (precalculated) for note generation
double A_1 = 55; //A_1 55 Hz  for note generation
const uint8_t NOTE_COUNT = 97; //number of notes set at six octaves from

//buttons for today 
uint8_t BUTTON1 = 45;
uint8_t BUTTON2 = 39;

//pins for LCD and AUDIO CONTROL
uint8_t LCD_CONTROL = 21;
uint8_t AUDIO_TRANSDUCER = 17;

//PWM Channels. The LCD will still be controlled by channel 0, we'll use channel 1 for audio generation
uint8_t LCD_PWM = 0;
uint8_t AUDIO_PWM = 1;

//arrays you need to prepopulate for use in the run_instrument() function
double note_freqs[NOTE_COUNT];
float accel_thresholds[NOTE_COUNT + 1];

//global variables to help your code remember what the last note was to prevent double-playing a note which can cause audible clicking
float new_note = 0;
float old_note = 0;
float timer = millis();
float riff_timer = song_to_play.note_period;
int curr_note_num = 0;

// MPU6050 imu; //imu object called, appropriately, imu


void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for Serial to show up
  Wire.begin();
  delay(50); //pause to make sure comms get set up
//   if (imu.setupIMU(1)) {
//     Serial.println("IMU Connected!");
//   } else {
//     Serial.println("IMU Not Connected :/");
//     Serial.println("Restarting");
//     ESP.restart(); // restart the ESP (proper way)
//   }
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

  Serial.printf("Frequencies:\n"); //print out your frequencies as you make them to help debugging
  double note_freq = A_1;
  //fill in note_freq with appropriate frequencies from 55 Hz to 55*(MULT)^{NOTE_COUNT-1} Hz
  for (int i=0; i<NOTE_COUNT; i++){
    if(i==0){
      note_freqs[0] = A_1;
    }
    else{
      note_freqs[i] = note_freqs[i-1]*MULT;
    }
  }

  //print out your accelerometer boundaries as you make them to help debugging
//   Serial.printf("Accelerometer thresholds:\n");
//   //fill in accel_thresholds with appropriate accelerations from -1 to +1
//   //start new_note as at middle A or thereabouts.
//   new_note = note_freqs[NOTE_COUNT - NOTE_COUNT / 2]; //set starting note to be middle of range.
//   for(int i = 0; i <= NOTE_COUNT; i++){
//     accel_thresholds[i] = (-1)+((2.0/97.0)*i);
//   }

  //four pins needed: two inputs, two outputs. Set them up appropriately:
//   pinMode(BUTTON1, INPUT_PULLUP);
//   pinMode(BUTTON2, INPUT_PULLUP);
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
  // dacWrite(AUDIO_TRANSDUCER, 255);
  // dac_output_enable(DAC_CHANNEL_1);
  dacWrite(AUDIO_TRANSDUCER, 100);
  Serial.println("PLAYING START");
  // dac_output_voltage (DAC_CHANNEL_1,255); 
  
  delay(5000);

  //play A440 for testing.
  //comment OUT this line for the latter half of the lab.
  // ledcWriteTone(AUDIO_PWM, 440); //COMMENT THIS OUT AFTER VERIFYING WORKING
  // delay(2000);
  // ledcWriteTone(AUDIO_PWM, 0); //COMMENT THIS OUT AFTER VERIFYING WORKING
  // riff_timer = millis();

}




void play_riff() {
  //your code here
    // for (int i = 0; i < song_to_play.length; i++)
      if (curr_note_num < song_to_play.length){
        float curr_note = song_to_play.notes[curr_note_num];
        ledcWriteTone(AUDIO_PWM, curr_note);
        delay(song_to_play.note_period);
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
        // ledcWriteTone(AUDIO_PWM, curr_note);
        curr_note_num += 1; 
        // Serial.println(curr_note_num);
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
void int_extractor(char* data_array, int* output_values, char delimiter){
    //your code here
   char delim[2];
   delim[0] = delimiter;
   delim[1] = '\0';
   char* val = strtok(data_array, delim);
   int i =0;
   while(val != NULL){
     output_values[i] = atoi(val);
     Serial.println(atoi(val));
     val = strtok(NULL, delim);
     i++;
   }
}

void loop() {
  if(new_image){
    memset(request_buffer, 0, IN_BUFFER_SIZE);
    sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/send_songs.py?new_song HTTP/1.1\r\n");
    strcat(request_buffer, "Host: 608dev-2.net\r\n");
    strcat(request_buffer,"\r\n"); //new line from header to body
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
    Serial.println("NEW IMAGE RES");
    Serial.println(response_buffer);
    new_image = false;
    Serial.println("response above");


   int curr_sounds[200];
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
    int_extractor(trimmed_res, curr_sounds, ',');
      for (int j = 0; j<200; j++){
      if(j == 0){
        micros_timer = micros();
        dacWrite(AUDIO_TRANSDUCER, curr_sounds[j]);
        // dac_output_voltage(DAC_CHANNEL_1,curr_sounds[j]); 
      }
      else if (micros()-micros_timer >= 250){

          // Serial.println(curr_sounds[j]);
          dacWrite(AUDIO_TRANSDUCER, curr_sounds[j]);
          // dac_output_voltage(DAC_CHANNEL_1,curr_sounds[j]); 
          micros_timer = micros();
      }
    }
    Serial.println("playyyyyy");
    using_chunks = true;

  }
  else{
    if(using_chunks){
      memset(request_buffer, 0, IN_BUFFER_SIZE);
      memset(response_buffer, 0, OUT_BUFFER_SIZE);
      sprintf(request_buffer, "GET http://608dev-2.net/sandbox/sc/team43/laser_comms/send_songs.py?chunk=%d HTTP/1.1\r\n", chunk_count);
      strcat(request_buffer, "Host: 608dev-2.net\r\n");
      strcat(request_buffer,"\r\n"); //new line from header to body
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
      Serial.println("NEW Chunk");
      Serial.println(response_buffer);
      // memset(curr_sounds, 0, 200);  
      int curr_sounds[200];
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
    int_extractor(trimmed_res, curr_sounds, ',');
    // for (int k = 0; k <10; k++){
      // for(int k=0; k<200; k++){
      //   Serial.println(curr_sounds[k]);
      // }
      for (int j = 0; j<200; j++){
      if(j == 0){
        micros_timer = micros();
        dacWrite(AUDIO_TRANSDUCER, 250);
        //dac_output_voltage(DAC_CHANNEL_1,curr_sounds[j]); 
      }
      else if (micros()-micros_timer >= 250){

          // Serial.println(curr_sounds[j]);
          dacWrite(AUDIO_TRANSDUCER, 250);
         //dac_output_voltage(DAC_CHANNEL_1,curr_sounds[j]); 
          micros_timer = micros();
      }
    }
    Serial.println("playyyyyy");
    chunk_count +=1;


    }
  }
}

