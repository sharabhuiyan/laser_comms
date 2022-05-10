#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


#define RXD1 9
#define TXD1 10

byte byte_read;
// byte byte2_read;

int i;
int chunk_count;
const int IMAGE_SIZE = 16 * 128;
const int num_chunks = 16;
const int BUFFER_MAX = 16 * num_chunks;
uint8_t PROGMEM image[BUFFER_MAX] = { 0 };


byte starting_sequence[] = { 0b11111111, 0b00000000 };
int started;



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

void setup() {
  // put your setup code here, to run once:
  // Set up TFT screen
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  Serial.begin(19200, SERIAL_8N1); // initialize UART with baud rate of 9600 bps
  Serial1.begin(19200, SERIAL_8N1, RXD1, TXD1);
  Serial1.flush();

  i = 0;
  chunk_count = 0;
  started = 0;

}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial1.available() > 2){ 
    if (started) {
      byte_read = Serial1.read();
      // byte2_read = Serial1.read();
      // Serial.println(byte_read, HEX);
      // Serial.print("0b");
      // for (int i = 0; i < 8; i++) {
      //   Serial.print(test_bit(byte_read,i));
      // }
      // Serial.println();

      // Serial.println(byte_read);
      image[i] = byte_read;
      // image[i+1] = byte2_read;
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
      }
    }
  }
  
  if (i >= BUFFER_MAX) {//} || Serial.available() == 0) {
    tft.drawXBitmap(0, 30+chunk_count, image, 128, num_chunks, TFT_RED);
    chunk_count += num_chunks;
    i = 0;
    memset(image, 0, sizeof image);

  }

}
