#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "dome_image.h"
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


int x = 0;
uint16_t timer;

const int IMAGE_SIZE = 128*128;
const int size = 16; //sizeof test_3_image / sizeof test_3_image[0];
const int SEQ_SIZE = 8;
const int message_size = 2 * size + SEQ_SIZE;
uint8_t message[message_size] = { 0 };
uint8_t PROGMEM image_chunk[size] = { 0 };
int bit_count;
int buffer_start;
int buffer_end;
int chunk_count;

enum transmission_state{GET_IMAGE, DELAY1, TRANSMIT, DELAY2};
enum transmission_state state;


void set_bit(uint8_t* A, int k) {
  // adapted from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
  int i = k / 8;   // i = array index 
  int pos = k % 8; // pos = bit pos in A[i]
  unsigned int flag = 1; // flag = 000.....00001
  flag = flag << pos; // shifted k positions
  A[i] = A[i] | flag; // set the bit and the kth position in A[i]
};

void clear_bit(uint8_t* A, int k) {
  // adapted from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
  int i = k / 8;   // i = array index 
  int pos = k % 8; // pos = bit pos in A[i]
  unsigned int flag = 1; // flag = 000.....00001
  flag = flag << pos; // shifted k positions
  flag = ~flag;
  A[i] = A[i] & flag; // rest the bit at kth position
};

int test_bit(const uint8_t* A, int k) {
  // adapted from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
  int i = k / 8;   // i = array index 
  int pos = k % 8; // pos = bit pos in A[i]
  unsigned int flag = 1; // flag = 000.....00001
  flag = flag << pos; // shifted k positions
  if (A[i] & flag) {
    return 1;
  } else {
    return 0;
  }
};

// overloaded
int test_bit(uint8_t* A, int k) {
  // adapted from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
  int i = k / 8;   // i = array index 
  int pos = k % 8; // pos = bit pos in A[i]
  unsigned int flag = 1; // flag = 000.....00001
  flag = flag << pos; // shifted k positions
  if (A[i] & flag) {
    return 1;
  } else {
    return 0;
  }
};

void next_chunk(uint8_t* image_chunk) {
  if (buffer_end <= IMAGE_SIZE) {
    int counter = 0;
    for (int i = buffer_start; i < buffer_end; i++) {
      image_chunk[counter] = mit_608_dome[i];
      counter++;
    }
    buffer_start += 16;
    buffer_end += 16;
  }
};


void manchester_encode(uint8_t* message, uint8_t* image_chunk) {

  // encode rest of bits
  int num_bits = 8*size;
  int offset = SEQ_SIZE*4;

  // add synchronizing pattern
  if (test_bit(image_chunk, 0) == 0) {
    message[0] = 0x00; message[1] = 0xFF; message[2] = 0x00; message[3] = 0xFF;
  } else {
    message[0] = 0xFF; message[1] = 0x00; message[2] = 0xFF; message[3] = 0x00;
  }

  // add ending pattern
  if (test_bit(image_chunk, (int)(num_bits-1)) == 0) {
    message[message_size-4] = 0x00; message[message_size-3] = 0xFF;
    message[message_size-2] = 0x00; message[message_size-1] = 0xFF;
  } else {
    message[message_size-4] = 0xFF; message[message_size-3] = 0x00;
    message[message_size-2] = 0xFF; message[message_size-1] = 0x00;
  } 

  // loop through every bit in image chunk
  for (int k = 0; k < num_bits; k++) {
    // check bit 
    int bit = test_bit(image_chunk, k);
    // encode bit in message
    if (bit == 1) {
      set_bit(message, 2*k + offset);
      clear_bit(message, 2*k + 1 + offset);
    } else {
      clear_bit(message, 2*k + offset);
      set_bit(message, 2*k + 1 + offset);
    }
  }
};


void setup() {
  // Set up TFT screen
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  
  // Initialize the analog resolution
  analogReadResolution(12);
  
  Serial.begin(115200);

  pinMode(11, OUTPUT); 

  // tft.drawXBitmap(0, 30, mit_608_dome, 128, 128, TFT_RED);
  // display_bitmap(0, mit_608_dome);


  bit_count = 0;
  timer = millis();

  buffer_start = 0;
  buffer_end = 16;
  chunk_count = 0;
  
  state = GET_IMAGE;

}


void one_signal(int upscale) {
  digitalWrite(11, HIGH);  // on
  delay(1*upscale); // waits 1*upscale ms
};

void zero_signal(int upscale) {
  digitalWrite(11, LOW);  // off
  delay(1*upscale); // waits 1*upscale ms

}

void send_synch_seq() {
  // SEND SYNCHRONIZING SEQUENCE
  int count = 0;
  while (count < 32) {
    if (count < 8) {
      Serial.println("first 16 bits");
      one_signal(15);
    } else if (count < 16) {
      Serial.println("second 16 bits");
      zero_signal(15);
    } else if (count < 24) {
      Serial.println("third 16 bits");
      one_signal(15);
    } else if (count < 32) {
      Serial.println("last 16 bits");
      zero_signal(15);
    }
    count++;
  }
  Serial.print(".");
  zero_signal(1);
}


void loop() {

  
  // one_signal(1);
  // zero_signal(1);


  switch(state) {
    case GET_IMAGE:

      // Encode image
      memset(message, 0, sizeof(message)); // for automatically-allocated arrays
      memset(image_chunk, 0, sizeof(size));
      
      next_chunk(image_chunk);
      tft.drawXBitmap(0, 30+chunk_count, image_chunk, 128, 1, TFT_RED);

      
      manchester_encode(message, image_chunk);

      // Print images 
      Serial.println("image chunk");
      for(int i = 0; i < size; i++) {
        printf("%d ", image_chunk[i]);
      }
      printf("\n");
      // for(int i = 0; i < 8*size; i++) {
      //   printf("%d ", test_bit(image_chunk,i));
      // }
      // printf("\n");

      Serial.println("encoded image");
      for(int i = 0; i < message_size; i++) {
        printf("%d ", message[i]);
      }
      printf("\n");
      // for(int i = 0; i < 8*message_size; i++) {
      //   printf("%d ", test_bit(message,i));
      // }
      // printf("\n");

      bit_count = 0;
      chunk_count++;

      state = DELAY1;
      break;
    case DELAY1:
      delay(300);
      state = TRANSMIT;
      break;
    case TRANSMIT:

      // emitter
      if (bit_count < 8*message_size) {
        int bit = test_bit(message, bit_count);
        if (bit == 1) { one_signal(5); }
        else { zero_signal(5); }
        bit_count++;
      }

      // SEND 0 AFTER MESSAGE FINISHED
      if (bit_count >= 8*message_size) {
        zero_signal(1);
        state = DELAY2;
      }

      break;
    case DELAY2:
      delay(2000);
      state = GET_IMAGE;
      break;
  }




}
