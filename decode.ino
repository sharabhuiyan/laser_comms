#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


float values[100]; //used for remembering an arbitrary number of old previous values
int indx = 0;

float lower = -20;
float upper = 20;

const int message_size = 1000;
uint8_t message[message_size] = { 0 };
uint32_t delays[message_size] = { 0 };
int m_indx = 0;
int d_indx = 0;
int last_indx = 0;

uint8_t reconstruction[message_size] = { 0 };
uint8_t decoded[message_size] = { 0 };
int r_indx = 0;

uint32_t stepped;
uint32_t clock_cycle;

int started;
int ended;
int first_step;
int first_end;
const int synch_seq[4] = { 1, 0, 1, 0 };//, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
                            //0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };
const int end_seq[32] = { 0, 1, 0, 1 };//, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
                          //1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
int mult = 1;

void setup() {
  
  // Set up TFT screen
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  // put your setup code here, to run once:
  analogReadResolution(12);       // initialize the analog resolution
  Serial.begin(115200);         // initialize the analog resolution
  // Serial.println("Base,Value,Top");
  started = 0;
  first_step = 0;
  ended = 0;
  first_end = 0;

  memset(reconstruction, 0, sizeof(reconstruction));
  memset(decoded, 0, sizeof(decoded));

}


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


void manchester_decode(uint8_t* reconstruction, int* final_index) {
  // remove first & last byte
  for (int i = 8; i < *final_index-8; i+=2) {
    int b1 = test_bit(reconstruction,i);
    int b2 = test_bit(reconstruction,i+1);
    int ind = (i - 8) / 2;
    if (b1 == 0 && b2 == 1) {
      clear_bit(decoded, ind);
    } else if (b1 == 1 && b2 == 0) {
      set_bit(decoded, ind);
    }
  }
}

void update_actual_message(uint8_t* message, uint32_t* delays, int last_m_index, int last_d_index, uint8_t* reconstruction, int* r_index) {
    uint8_t b = message[last_m_index];
    uint32_t dt = delays[last_d_index];
    Serial.print("   ");
    Serial.print(b);
    Serial.print(", ");
    Serial.print(dt);
    Serial.print(", ");
    Serial.print((float)dt/(float)clock_cycle);
    int num_bits = round((float)dt/(float)clock_cycle);
    for (int i = *r_index; i < num_bits+*r_index; i++) {
      if (b == 1) { 
        set_bit(reconstruction, i);
      } else {
        clear_bit(reconstruction, i);
      }
    }
    Serial.print(", ");
    Serial.println(num_bits);
    *r_index += num_bits;
}


int check_synchronizing_seq(uint8_t* message, int* index, uint32_t* delays, int* d_index) {
  // check last 4 values 
  //1, 0, 1, 0, 1, 0, 1, 0
  int counter = 0;
  int i = *index - 1;
  float seq[4] = {0};
  while (counter < 4) {
    if (i < 0) {
      i = sizeof(message) - 1;
    }
    seq[counter] = message[i];
    i -= 1;
    counter += 1;
  }
  int match = 1;
  for (int j = 0; j < 4; j++) {
    if (seq[j] != synch_seq[j]) {
      match = 0;
      break;
    }
  }
  if (match == 1) {
    // set clock_cycle
    int d_counter = 0;
    int k = *d_index - 1;
    uint32_t sum = 0;
    while (d_counter < 3) {
      if (k < 0) {
        k = sizeof(delays) - 1;
      }
      sum += delays[k];
//      Serial.println(delays[k]);
      k -= 1;
      d_counter += 1;
    }
    clock_cycle = sum / 24;
//    Serial.println("MATCH");
//    Serial.println(clock_cycle); // should be about 15 ms :D
  }
  
  return match; 
}

//int check_end_seq(uint8_t* message, int* index, uint32_t* delays, int* d_index) {
//  // check last 4 values 
//  int counter = 0;
//  int i = *index - 1;
//  float seq[4] = {0};
//  while (counter < 4) {
//    if (i < 0) {
//      i = sizeof(message) - 1;
//    }
//    seq[counter] = message[i];
//    i -= 1;
//    counter += 1;
//  }
//  int match = 1;
//  for (int j = 0; j < 4; j++) {
//    if (seq[j] != end_seq[j]) {
//      match = 0;
//      break;
//    }
//  }
//
//  int ended = 0;
//  if (match == 1) {
//    // check clock_cycle
//    int d_counter = 0;
//    int k = *d_index - 1;
//    uint32_t sum = 0;
//    while (d_counter < 3) {
//      if (k < 0) {
//        k = sizeof(delays) - 1;
//      }
//      sum += delays[k];
//      k -= 1;
//      d_counter += 1;
//    }
////    Serial.println("Is it ended actually?");
////    Serial.println(sum);
////    Serial.println(sum/24);
//    
//    // sum / 24 should approximately equal clock cycle
//    uint32_t cc = sum / 24;
//    if (cc < clock_cycle + 300 && cc > clock_cycle - 300) {
//      ended = 1;
//    }
//  }
//  
//  return ended; 
//}

int check_end_seq(uint8_t* message, int* index, uint32_t* delays, int* d_index) {
  // check last value
  uint8_t last_value = message[*index - 1];
  int flag = 0;
  if (last_value == 0) {
    // check clock_cycle
    uint32_t last_delay = delays[*d_index - 1];
    // delay / 8 should approximately equal clock cycle
    uint32_t cc = last_delay / 8;
    if (cc < clock_cycle + 300 && cc > clock_cycle - 300) {
      flag = 1;
    }
  }
  return flag; 
}



void save_values(float input, float* stored_values, int* index) {
    stored_values[*index] = input;
    *index += 1;
    // wrap around if necessary 
    if (*index > sizeof(stored_values) - 1) {
      *index = 0;
    }
}



void update_message(uint8_t value, uint8_t* message, int* index) {
    message[*index] = value;
    *index += 1;
    // wrap around if necessary 
//    Serial.print("size of message: ");
//    Serial.println(sizeof(message) - 1);
    if (*index > sizeof(message) - 1) {
      *index = 0;
    }
}

void update_delays(uint32_t value, uint32_t* delays, int* index) {
    delays[*index] = value;
    *index += 1;
    // wrap around if necessary 
    if (*index > sizeof(delays) - 1) {
      *index = 0;
    }
}

int is_step(float* stored_values, int* index,
              uint8_t* message, int* m_index,
              uint32_t* delays, int* d_index,
              uint8_t* reconstruction, int* r_index) {
                
    int counter = 0;
    int i = *index - 1;
    float last_values[2] = {0};
    while (counter < 2) {
      if (i < 0) {
        i = sizeof(stored_values) - 1;
      }
      last_values[counter] = stored_values[i];
      i -= 1;
      counter += 1;
    }

    int flag = 0;

    if (last_values[0] < 0 && last_values[1] > 0) {
      // 1 to 0
      int last_m_index = *m_index;
      update_message(0, message, m_index);
      uint32_t dt = micros() - stepped;
      if (!first_step) { first_step = 1; dt = 0; }
      int last_d_index = *d_index;
      update_delays(dt, delays, d_index);
      if (started) { update_actual_message(message, delays, last_m_index, last_d_index, reconstruction, r_index); }
      flag = 1;
      stepped = micros();
    } else if (last_values[0] > 0 && last_values[1] < 0) {
      // 0 to 1
      int last_m_index = *m_index;
      update_message(1, message, m_index);
      uint32_t dt = micros() - stepped;
      if (!first_step) { first_step = 1; dt = 0; }
      int last_d_index = *d_index;
      update_delays(dt, delays, d_index);
      if (started) { update_actual_message(message, delays, last_m_index, last_d_index, reconstruction, r_index); } 
      flag = 1;
      stepped = micros();
    }
//    } else if (started && micros() - stepped >= clock_cycle) {
//      // add if clock cycle
//      uint8_t temp = 1;
//      if (last_values[0] < 0) { temp = 1; }
//      //uint8_t temp = message[*m_index];
//      update_message(temp, message, m_index);
//      uint32_t dt = micros() - stepped;
//      update_delays(dt, delays, d_index);
//      stepped = micros();
//    }

  return flag;
}


// set clock cycle based on synchronizing sequence 

void loop() {

  if (!ended) {
  
    // read input fom diode 
    float v_photo = analogRead(7);
    float actual_v = v_photo * 3.3 / 4096;
    
    // scale & offset 
    float v_offset = actual_v * 10 - 18;
    
    // update stored array
    save_values(v_offset, values, &indx);
    int took_step = is_step(values, &indx, message, &m_indx, delays, &d_indx, reconstruction, &r_indx);
   
    // identify synchronizing sequence
    if (!started) {
      int match = check_synchronizing_seq(message, &indx, delays, &d_indx);
      if (match) { started = 1; }
    }

//    Serial.println(actual_v);

    int ind = (int)((float)r_indx / (float)8);
    Serial.print(lower);
    Serial.print('\t');
    Serial.print(v_offset);
    Serial.print('\t');
    Serial.print(message[m_indx]);
    Serial.print('\t');
    Serial.print(reconstruction[ind]);
    Serial.print('\t');
    Serial.println(upper);

//    if (started && took_step) {
//      Serial.print(m_indx);
//      Serial.print(": ");
//      Serial.print(message[m_indx]);
//      Serial.print("  ,  ");
//      Serial.print(d_indx);
//      Serial.print(": ");
//      Serial.println(delays[d_indx]);
//      //update_actual_message(message, delays, &m_indx, &d_indx, reconstruction, &r_indx);
//      last_indx = m_indx;
//    }
  
    
    // identify ending sequence
    if (started) {
      int end_match = check_end_seq(message, &m_indx, delays, &d_indx);
      if (end_match) { Serial.println("ENDED"); ended = 1; first_end = 0; } 
    }

  } else if (ended && !first_end) {
    // parse signal and display
    first_end = 1;
    Serial.println("MESSAGE:");
    for (int i = 0; i < m_indx; i++) {
      Serial.print(message[i]);
      Serial.print(' ');
    }
    Serial.println();
    Serial.println("RECONSTRUCTION:");
    for (int j = 0; j < r_indx; j++) {
      Serial.print(test_bit(reconstruction,j));
      Serial.print(' ');
    }
    Serial.println();
    Serial.println("ORIGINAL MESSAGE");
    manchester_decode(reconstruction, &r_indx);
    for (int k = 0; k < sizeof(decoded); k++) {
      Serial.print(decoded[k]);
      Serial.print(' ');
    }
    Serial.println();
    tft.drawXBitmap(0, 30, decoded, 16, 14, TFT_RED);
  }

//  Serial.println(actual_v);
//  Serial.print(lower);
//  Serial.print('\t');
//  Serial.print(v_offset);
//  Serial.print('\t');
//  Serial.print(message[m_indx]);
//  Serial.print('\t');
//  Serial.println(upper);
//  delay(0.5);

  // keep track of past stored values using exercise code 
//  float brightness = brightnessExtractor(actual_v);


}
