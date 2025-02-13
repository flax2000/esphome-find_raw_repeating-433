

//made by swedude

#pragma once


#if defined(ESP8266)
// interrupt handler and related code must be in RAM on ESP8266,
// according to issue #46.
#define RECEIVE_ATTR IRAM_ATTR
#define VAR_ISR_ATTR
#elif defined(ESP32)
#define RECEIVE_ATTR IRAM_ATTR
#define VAR_ISR_ATTR DRAM_ATTR
#else
#define RECEIVE_ATTR
#define VAR_ISR_ATTR
#endif
volatile VAR_ISR_ATTR int8_t capture433 = 0;
volatile VAR_ISR_ATTR int8_t capture433_repeating = 0;
int RX433_PIN = 0;
volatile VAR_ISR_ATTR int16_t nSeparationLimit = 2000;
volatile VAR_ISR_ATTR int16_t tolerance = 90;
volatile VAR_ISR_ATTR int16_t nSeparation_pulse_state;
#define RAW_MAX_CHANGES 150
//--------------------------------------------------------------raw repeating find-- rc switch part of code from https://github.com/sui77/rc-switch------------------------------------------------------------------------------------------------

unsigned int RECEIVE_ATTR diff_raw(int A, int B) {
  return abs(A - B);
}

int32_t timings_raw[RAW_MAX_CHANGES];
volatile VAR_ISR_ATTR int raw_lenght_done = 0;
void RECEIVE_ATTR raw_data(uint16_t duration) {

  static uint16_t changeCount = 0;
  static uint16_t repeatCount = 0;

  static uint16_t lenght = 0;
  static uint16_t counter = 1;
  if (raw_lenght_done == 0) {

    if (duration > nSeparationLimit) {
      nSeparation_pulse_state = !digitalRead(RX433_PIN);
      // A long stretch without signal level change occurred. This could
      // be the gap between two transmission.
      if ((repeatCount == 0) || (diff_raw(duration, timings_raw[0]) < tolerance)) {
        // This long signal is close in length to the long signal which
        // started the previously recorded timings; this suggests that
        // it may indeed by a a gap between two transmissions (we assume
        // here that a sender will send the signal multiple times,
        // with roughly the same gap between them).
        repeatCount++;
        if (repeatCount == 2) {

          if ((changeCount - 1) > 8)  //ignore short transmissions
          {
            raw_lenght_done = 1;
            lenght = changeCount - 1;
            repeatCount = 0;
            changeCount = 0;
            return;
          }
          repeatCount = 0;
        }
      }
      changeCount = 0;
    }

    // detect overflow
    if (changeCount >= RAW_MAX_CHANGES) {
      changeCount = 0;
      repeatCount = 0;
    }


    timings_raw[changeCount++] = duration;
  }


  if (raw_lenght_done == 1) {
    if ((diff_raw(duration, timings_raw[counter]) < tolerance)) {
      counter++;

      if (counter == lenght) {
        raw_lenght_done = lenght;
        counter = 1;
        return;
      }
    } else {
      raw_lenght_done = 0;
      counter = 1;
    }
  }
}


#define RAW_CAPTURE_MAX_CHANGES 1000
//--------------------------------------------------------------raw repeating find-- rc switch part of code from https://github.com/sui77/rc-switch------------------------------------------------------------------------------------------------

int32_t timings_raw_capture[RAW_CAPTURE_MAX_CHANGES];
boolean raw_capture_done=0;

void RECEIVE_ATTR raw_data_capture(uint16_t duration) {


  static uint16_t counter = 0;
  
  
  if(raw_capture_done==0&&capture433==1)
  {
	if(counter<RAW_CAPTURE_MAX_CHANGES)
	{		
	 timings_raw_capture[counter]=duration; 
	 counter++;
	}
else
{
counter=0;	
capture433=0;	
raw_capture_done=1;	
	
}	
	
  }
  
  
  
  
  

}




void RECEIVE_ATTR ext_int_1() {


  static unsigned long edgeTimeStamp = 0;  // Timestamp of edge
  uint16_t duration = micros() - edgeTimeStamp;
  edgeTimeStamp = micros();
  if(capture433_repeating)raw_data(duration);
  raw_data_capture(duration);
}





class MyCustomComponent  : public PollingComponent {
  public:


    MyCustomComponent (): PollingComponent(111) {}
    float get_setup_priority() const override {
      return esphome::setup_priority::AFTER_WIFI;
    }
    void setup() override {

      pinMode(RX433_PIN, INPUT);
      attachInterrupt(RX433_PIN, ext_int_1, CHANGE);

    }
    void update() override {


      if (raw_lenght_done > 1)
      {
        //below modefied code from esphome raw_protocol.cpp
        static const char *const TAG = "diy raw";
        char buffer[256];
        uint32_t buffer_offset = 0;
        buffer_offset += sprintf(buffer, "size: %i idle pulse %i pulses: [", (raw_lenght_done - 1), timings_raw[0]);
        for (int32_t i = 0; i < raw_lenght_done - 1; i++)
        {
          const int32_t value = timings_raw[i + 1]; //-->> dont write the idle pulse here
          const uint32_t remaining_length = sizeof(buffer) - buffer_offset;
          int written;

          if (nSeparation_pulse_state)
          {
            if (i + 1 < raw_lenght_done - 1) {
              written = snprintf(buffer + buffer_offset, remaining_length, "-%d, ", value);
            } else {
              written = snprintf(buffer + buffer_offset, remaining_length, "-%d]", value);
            }

            if (written < 0 || written >= int(remaining_length)) {
              // write failed, flush...
              buffer[buffer_offset] = '\0';
              ESP_LOGD(TAG, "%s", buffer);
              buffer_offset = 0;
              written = sprintf(buffer, "  ");
              if (i + 1 < raw_lenght_done) {
                written += sprintf(buffer + written, "-%d, ", value);
              } else {
                written += sprintf(buffer + written, "-%d]", value);
              }
            }
          }
          else
          {

            if (i + 1 < raw_lenght_done - 1) {
              written = snprintf(buffer + buffer_offset, remaining_length, "%d, ", value);
            } else {
              written = snprintf(buffer + buffer_offset, remaining_length, "%d]", value);
            }

            if (written < 0 || written >= int(remaining_length)) {
              // write failed, flush...
              buffer[buffer_offset] = '\0';
              ESP_LOGD(TAG, "%s", buffer);
              buffer_offset = 0;
              written = sprintf(buffer, "  ");
              if (i + 1 < raw_lenght_done) {
                written += sprintf(buffer + written, "%d, ", value);
              } else {
                written += sprintf(buffer + written, "%d]", value);
              }
            }
          }

          nSeparation_pulse_state = !nSeparation_pulse_state;
          buffer_offset += written;
        }
        if (buffer_offset != 0) {
          ESP_LOGD(TAG, "%s", buffer);
        }

        ESP_LOGD(TAG, "signal done ");
        ESP_LOGD("", "");
        raw_lenght_done = 0;
      }






      if (raw_capture_done == 1)
      {
        //below modefied code from esphome raw_protocol.cpp
        static const char *const TAG = "diy raw capture";
        char buffer[256];
        uint32_t buffer_offset = 0;
        buffer_offset += sprintf(buffer, "size: %i  ", (RAW_CAPTURE_MAX_CHANGES - 1));
        for (int32_t i = 0; i < RAW_CAPTURE_MAX_CHANGES - 1; i++)
        {
          const int32_t value = timings_raw_capture[i + 1]; //-->> dont write the idle pulse here
          const uint32_t remaining_length = sizeof(buffer) - buffer_offset;
          int written;

            if (i + 1 < RAW_CAPTURE_MAX_CHANGES - 1) {
              written = snprintf(buffer + buffer_offset, remaining_length, "%d,", value);
            } else {
              written = snprintf(buffer + buffer_offset, remaining_length, "%d", value);
            }

            if (written < 0 || written >= int(remaining_length)) {
              // write failed, flush...
              buffer[buffer_offset] = '\0';
              ESP_LOGD(TAG, "%s", buffer);
              buffer_offset = 0;
              written = sprintf(buffer, "  ");
              if (i + 1 < RAW_CAPTURE_MAX_CHANGES) {
                written += sprintf(buffer + written, "%d,", value);
              } else {
                written += sprintf(buffer + written, "%d", value);
              }
            }
          


          buffer_offset += written;
        }
        if (buffer_offset != 0) {
          ESP_LOGD(TAG, "%s", buffer);
        }

        ESP_LOGD(TAG, "signal done ");
        ESP_LOGD("", "");
        raw_capture_done = 0;
      }











    }
};








