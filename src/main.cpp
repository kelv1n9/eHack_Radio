#include "functions.h"

void setup()
{
#ifdef DEBUG_eHack
  Serial.begin(9600);
  delay(2000);
#endif

  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();

  pinMode(FM_RESETPIN, OUTPUT);
  digitalWrite(FM_RESETPIN, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  pinMode(DISABLE_DEVICE_PIN, OUTPUT);
  digitalWrite(DISABLE_DEVICE_PIN, LOW);

  while (!radio_fm.begin())
  {
    DBG("Couldn't find radio?\n");
    delay(2000);
  }

  delay(1000);
  radio_fm.setTXpower(115);
  radio_fm.tuneFM(radioFrequency);

  radio_fm.readTuneStatus();
  DBG("FM: Current frequency: %.2f MHz\n", radio_fm.currFreq / 100.0);
  DBG("FM: Current dBuV: %d\n", radio_fm.currdBuV);
  DBG("FM: Current Antenna Cap: %d\n", radio_fm.currAntCap);

  radio_fm.beginRDS();
  radio_fm.setRDSstation(RDS_STATION);
  radio_fm.setRDSbuffer(RDS_BUFFER);

  DBG("FM: RDS is enabled.\n");

  SPI.setSCK(SCK_PIN_NRF);
  SPI.setTX(MOSI_PIN_NRF);
  SPI.setRX(MISO_PIN_NRF);
  SPI.begin();

  radio_RF24.begin();

  analogReadResolution(12);
  batVoltage = readBatteryVoltage();

  communication.setSlaveMode();
  communication.init();

  currentLedMode = LED_ON;
}

void setup1()
{
}

void loop()
{
  if (!successfullyConnected)
  {
    currentLedMode = LED_ON;

    if (millis() - offTimer > DISABLE_DEVICE_DELAY)
    {
        DBG("Going to sleep...: %d\n");
        digitalWrite(DISABLE_DEVICE_PIN, HIGH);
        delay(1000);
    }

    if (communication.receivePacket(recievedData, &recievedDataLen))
    {
      if (recievedData[0] == 'P' && recievedData[1] == 'I' && recievedData[2] == 'N' && recievedData[3] == 'G')
      {
        DBG("Slave: PING received. Sending PONG...\n");
        if (communication.sendPacket(pong, sizeof(pong)))
        {
          DBG("Slave: PONG sent successfully.\n");
          DBG("Connection established\n");
          checkConnectionTimer = millis();
          successfullyConnected = true;
          currentLedMode = LED_BLINK_SLOW;
          batteryTimer = millis() - BATTERY_CHECK_INTERVAL;
        }
        else
        {
          DBG("Slave: FAILED to send PONG\n");
        }
      }
    }
  }
  else if (successfullyConnected)
  {
    if (millis() - checkConnectionTimer > CONNECTION_DELAY)
    {
      DBG("Slave: Connection LOST (Master timeout)!\n");
      successfullyConnected = false;
      offTimer = millis();
      return;
    }

    if (communication.receivePacket(recievedData, &recievedDataLen))
    {
      if (recievedData[0] == PROTOCOL_HEADER)
      {
        checkConnectionTimer = millis();
        radioFrequency = getFrequencyFromPacket(recievedData, recievedDataLen);
        radio_fm.tuneFM(radioFrequency);
        DBG("Received frequency: %lu MHz\n", radioFrequency);
      }
      else if (recievedData[0] == 'P' && recievedData[1] == 'I' && recievedData[2] == 'N' && recievedData[3] == 'G')
      {
        DBG("Master: PING received!\n");
        checkConnectionTimer = millis();
        if (communication.sendPacket(pong, sizeof(pong)))
        {
          DBG("Master: PONG sent! Connection OK.\n");
        }
      }
      else if (recievedData[0] == 'O' && recievedData[1] == 'F' && recievedData[2] == 'F')
      {
        DBG("Master: OFF received!\n");
        DBG("Going to sleep...\n");
        digitalWrite(DISABLE_DEVICE_PIN, HIGH);
        delay(1000);
      }
    }

    if (millis() - batteryTimer > BATTERY_CHECK_INTERVAL)
    {
      batVoltage = readBatteryVoltage();
      DBG("Battery voltage: %.2fV\n", batVoltage);
      uint8_t batteryVoltagePacketLen = communication.buildPacket(COMMAND_BATTERY_VOLTAGE, (uint8_t *)&batVoltage, sizeof(batVoltage), batteryVoltagePacket);
      communication.sendPacket(batteryVoltagePacket, batteryVoltagePacketLen);
      batteryTimer = millis();
    }
    
    if (millis() - asqTimer > 200)
    {
      radio_fm.readASQ();
      int8_t inLevel = radio_fm.currInLevel;
      currentLedMode = (inLevel > -50) ? LED_BLINK_FAST : LED_BLINK_SLOW;
      DBG("FM: Current sound level: %d\n", InLevel);
      uint8_t pktLen = communication.buildPacket(0x19, (uint8_t *)&inLevel, sizeof(inLevel), currentLevelPacket);
      communication.sendPacket(currentLevelPacket, pktLen);
      asqTimer = millis();
    }
  }
}

void loop1()
{
  uint32_t onTime = 0;
  uint32_t offTime = 0;

  switch (currentLedMode)
  {
  case LED_ON:
    digitalWrite(LED_BUILTIN, HIGH);
    break;
  case LED_OFF:
    digitalWrite(LED_BUILTIN, LOW);
    break;
  case LED_BLINK_SLOW:
    onTime = 500;
    offTime = 500;
    break;
  case LED_BLINK_FAST:
    onTime = 50;
    offTime = 500;
    break;
  }

  if (onTime > 0 || offTime > 0)
  {
    if ((ledState && millis() - ledTimer >= onTime) || (!ledState && millis() - ledTimer >= offTime))
    {
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
      ledTimer = millis();
    }
  }
}
