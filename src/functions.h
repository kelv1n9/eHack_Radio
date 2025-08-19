#pragma once

// #define DEBUG_eHack

#ifdef DEBUG_eHack
#define DBG(...)                \
  do                            \
  {                             \
    Serial.printf(__VA_ARGS__); \
  } while (0)
#else
#define DBG(...) \
  do             \
  {              \
  } while (0)
#endif

#include <Wire.h>
#include <SPI.h>

#include <Adafruit_Si4713.h>
#include <DataTransmission.h>
#include "hardware/adc.h"
#include "RF24.h"

#define APP_NAME "eHack Radio"
#define APP_VERSION "v1.0.0"

#define DISABLE_DEVICE_PIN 16
#define DISABLE_DEVICE_DELAY 180000 // ms
static uint32_t offTimer;

// ================== FM RADIO ===========================/
#define FM_RESETPIN 3
#define RDS_STATION "RAINBOW FM"
#define RDS_BUFFER "PEPEEEE"

Adafruit_Si4713 radio_fm = Adafruit_Si4713(FM_RESETPIN);

uint16_t radioFrequency = 10000; // 100.00 MHz
uint8_t currentLevelPacket[32];
uint32_t asqTimer;

/* ================== Common ================== */
enum LedMode
{
  LED_OFF,
  LED_ON,
  LED_BLINK_SLOW,
  LED_BLINK_FAST
};

LedMode currentLedMode = LED_ON;

static uint32_t ledTimer = 0;
static bool ledState = false;

/* ================= Battery ================== */
#define BATTERY_COEFFICIENT 0.988
#define R1 200000 // 200k
#define R2 100000 // 100k
#define BATTERY_RESISTANCE_COEFFICIENT (1 + R1 / R2)
#define V_REF 3.3

#define BATTERY_CHECK_INTERVAL 20000
#define BATTERY_READ_ITERATIONS 10

float batVoltage;
uint32_t batteryTimer;

// ================= 2.4 GHZ ===========================/
#define SCK_PIN_NRF 6
#define MOSI_PIN_NRF 7
#define MISO_PIN_NRF 4
#define CE_PIN_NRF 21
#define CSN_PIN_NRF 20

RF24 radio_RF24(CE_PIN_NRF, CSN_PIN_NRF, 16000000);

// ================== Communication ===========================/
#define CONNECTION_DELAY 5000
#define NUMBER_OF_RETRIES 3

#define SEND_DURATION_MS 10
#define LISTEN_DURATION_MS 20

byte ping[4] = {'P', 'I', 'N', 'G'};
byte pong[4] = {'P', 'O', 'N', 'G'};
// byte inited[4] = {'I', 'N', 'I', 'T'};

DataTransmission communication(&radio_RF24);

bool successfullyConnected = false;

uint32_t pingSentTime;
uint32_t checkConnectionTimer;
bool awaitingPong = false;

uint8_t recievedData[32];
uint8_t recievedDataLen = 0;

uint8_t batteryVoltagePacket[32];

/*=================== FUNCTIONS ==========================*/
/*********************** COMMON ***************************/

float readBatteryVoltage()
{
  uint16_t total = 0;
  for (int i = 0; i < BATTERY_READ_ITERATIONS; i++)
  {
    total += analogRead(A3);
    delayMicroseconds(500);
  }

  return (float)BATTERY_COEFFICIENT * (total / (float)BATTERY_READ_ITERATIONS) * (float)BATTERY_RESISTANCE_COEFFICIENT * (float)V_REF / 4095.0;
}

uint16_t getFrequencyFromPacket(const uint8_t *payload, uint8_t payloadLen)
{
  uint16_t freq = (uint16_t)payload[2] | ((uint16_t)payload[3] << 8);
  return freq;
}