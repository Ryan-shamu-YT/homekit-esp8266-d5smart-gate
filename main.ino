#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);
static uint32_t next_heap_millis = 0;
const int statusPin = 12;  // The status pin connected to the gate
const int relayPin = 14; // The relay pin that actually controls the gate

unsigned long lastChangeTime = 0;    // Time of the last pin state change
unsigned long highDuration = 0;      // Duration the pin was HIGH
unsigned long lowDuration = 0;       // Duration the pin was LOW
unsigned long stableThreshold = 1000; // Time to confirm the gate is fully open/closed

  // Flashing intervals
const unsigned long flashIntervalClosing = 150; // Interval for "Closing" (HIGH/LOW)
const unsigned long flashIntervalOpening = 300; // Interval for "Opening" (HIGH/LOW)
const unsigned long hysteresis = 50;            // Tolerance for timing variations

int lastPinState = LOW;    // Last known pin state
int currentPinState = LOW; // Current pin state
int gateState = -1;        // Current gate state



extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_current_state;
extern "C" homekit_characteristic_t cha_target_state;
extern "C" homekit_characteristic_t cha_obstruction_detected;

bool isWithinRange(unsigned long value, unsigned long target, unsigned long tolerance)
{
  return (value >= (target - tolerance)) && (value <= (target + tolerance));
}


void setup() {
	Serial.begin(115200);
	wifi_connect(); // in wifi_info.h
  pinMode(relayPin, OUTPUT);
  pinMode(statusPin, INPUT_PULLUP);
	//homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
	my_homekit_setup();
}

void my_homekit_setup() {
  
    cha_target_state.setter = update;
	arduino_homekit_setup(&config);

	//homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value); 
}

void loop() {
  arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_heap_millis) {
		next_heap_millis = t + 5 * 1000;
		LOG_D("Free heap: %d, HomeKit clients: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

	}

  int newPinState = digitalRead(statusPin); // Read the current pin state
    unsigned long currentTime = millis();     // Get the current time

    // Detect pin state changes
    if (newPinState != lastPinState)
    {
      unsigned long duration = currentTime - lastChangeTime; // Duration of the previous state
      lastChangeTime = currentTime;                          // Update the time of the change

      // Record HIGH/LOW durations
      if (newPinState == HIGH)
      {
        lowDuration = duration;
      }
      else
      {
        highDuration = duration;
      }

      lastPinState = newPinState; // Update the last pin state

      // Determine flashing patterns
      if (isWithinRange(highDuration, flashIntervalClosing, hysteresis) &&
          isWithinRange(lowDuration, flashIntervalClosing, hysteresis))
      {
        // Closing pattern detected
        if (gateState != 3)
        {
          gateState = 3;
          Serial.println("Gate is Closing");
          if (cha_current_state.value.int_value != 3) {
            cha_current_state.value.int_value = 3;
            homekit_characteristic_notify(&cha_current_state, cha_current_state.value); 
          }
          if (cha_target_state.value.int_value != 1) {
            cha_target_state.value.int_value = 1;
            homekit_characteristic_notify(&cha_target_state, cha_target_state.value); 
          }
        }
      }
      else if (isWithinRange(highDuration, flashIntervalOpening, hysteresis) &&
               isWithinRange(lowDuration, flashIntervalOpening, hysteresis))
      {
        // Opening pattern detected
        if (gateState != 2)
        {
          gateState = 2;
          Serial.println("Gate is Opening");
          if (cha_current_state.value.int_value != 2) {
            cha_current_state.value.int_value = 2;
            homekit_characteristic_notify(&cha_current_state, cha_current_state.value); 
          }
          if (cha_target_state.value.int_value != 0) {
            cha_target_state.value.int_value = 0;
            homekit_characteristic_notify(&cha_target_state, cha_target_state.value); 
          }
        }
      }
    }

    // Determine stable states (Open/Closed)
    if ((currentTime - lastChangeTime) > stableThreshold)
    {
      if (lastPinState == LOW && gateState != 1)
      {
        // Signal is stable LOW, indicating the gate is fully closed
        gateState = 1;
        Serial.println("Gate is Closed");
        if (cha_current_state.value.int_value != 1) {
          cha_current_state.value.int_value = 1;
          homekit_characteristic_notify(&cha_current_state, cha_current_state.value); 
        }
        if (cha_target_state.value.int_value != 1) {
          cha_target_state.value.int_value = 1;
          homekit_characteristic_notify(&cha_target_state, cha_target_state.value); 
        }
      }
      else if (lastPinState == HIGH && gateState != 0)
      {
        // Signal is stable HIGH, indicating the gate is fully open
        gateState = 0;
        Serial.println("Gate is Open");
        if (cha_current_state.value.int_value != 0) {
          cha_current_state.value.int_value = 0;
          homekit_characteristic_notify(&cha_current_state, cha_current_state.value); 
        }
        if (cha_target_state.value.int_value = 0) {
          cha_target_state.value.int_value = 0;
          homekit_characteristic_notify(&cha_target_state, cha_target_state.value); 
        }
      }
    }

    delay(50);  
}

void update(const homekit_value_t value) {
  
  uint8_t state = value.int_value;
  cha_current_state.value.int_value = state;
  Serial.println("Target state requested: " + String(state)); // Debug print to trace execution
  
  if (state == 0)
  {
    if (gateState == 2)
      {
        // Gate is already opening, do nothing
        Serial.println("Gate is already opening");
      }
      else if (gateState == 3)
      {
        Serial.println("Gate was closing; now opening");
        // Gate is closing, pulse the relay twice to reverse direction
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
        delay(500);
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
      }
      else
      {
        Serial.println("Opening the gate");
        // Open the gate
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
      }
      if (cha_current_state.value.int_value != 0) {
        cha_current_state.value.int_value = 0;
        homekit_characteristic_notify(&cha_current_state, cha_current_state.value); 
      }
      if (cha_target_state.value.int_value != 0) {
        cha_target_state.value.int_value = 0;
        homekit_characteristic_notify(&cha_target_state, cha_target_state.value); 
      }
      Serial.println("Gate state updated to OPEN");
    }
    else if (state == 1)
    {
      if (gateState == 3)
      {
        // Gate is already closing, do nothing
        Serial.println("Gate is already closing");
      }
      else if (gateState == 2)
      {
        Serial.println("Gate was opening; now closing");
        // Gate is opening, pulse the relay twice to reverse direction
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
        delay(500);
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
      }
      else
      {
        Serial.println("Closing the gate");
        // Close the gate
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
      }
      if (cha_current_state.value.int_value != 1) {
        cha_current_state.value.int_value = 1;
        homekit_characteristic_notify(&cha_current_state, cha_current_state.value); 
      }
      if (cha_target_state.value.int_value != 1) {
        cha_target_state.value.int_value = 1;
        homekit_characteristic_notify(&cha_target_state, cha_target_state.value); 
      }
      Serial.println("Gate state updated to CLOSED");
    }
  }
 





void open_lock(){
  Serial.println("unsecure"); 
}

void close_lock(){
  Serial.println("secure");  
}
