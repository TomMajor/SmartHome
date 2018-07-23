#include "LowPower.h"
#include <SPI.h>

const int ledPin = 13;

void cc1101_powerdown();

void setup() 
{
  // cc1101_powerdown nur aktivieren falls ein CC1101 angeschlossen ist!
  //cc1101_powerdown();
  pinMode(ledPin, OUTPUT);
}

void loop() 
{
  digitalWrite(ledPin, HIGH);
  delay(8000); 
  digitalWrite(ledPin, LOW);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
}

// -------------------------------------------------------------------------
// CC1101 power down
// -------------------------------------------------------------------------

#define CC1101_GDO0         2           // GDO0 input interrupt pin
#define CC1101_SRES         0x30        // Reset CC1101 chip
#define CC1101_SIDLE        0x36        // Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable
#define CC1101_SPWD         0x39        // Enter power down mode when CSn goes high

#define cc1101_Select()     digitalWrite(SS, LOW)
#define cc1101_Deselect()   digitalWrite(SS, HIGH)
#define wait_Miso()         while(digitalRead(MISO)>0)

void cc1101_powerdown()
{
  SPI.begin();                          // Initialize SPI interface
  pinMode(CC1101_GDO0, INPUT);          // Config GDO0 as input

  //reset();                            // Reset CC1101
  cc1101_Deselect();                    // Deselect CC1101
  delayMicroseconds(5);
  cc1101_Select();                      // Select CC1101
  delayMicroseconds(10);
  cc1101_Deselect();                    // Deselect CC1101
  delayMicroseconds(41);
  cc1101_Select();                      // Select CC1101

  wait_Miso();                          // Wait until MISO goes low
  SPI.transfer(CC1101_SRES);            // Send reset command strobe
  wait_Miso();                          // Wait until MISO goes low

  cc1101_Deselect();                    // Deselect CC1101
  // reset() end

  delay(100);
  
  //cmdStrobe(byte cmd) 
  cc1101_Select();                      // Select CC1101
  wait_Miso();                          // Wait until MISO goes low
  SPI.transfer(CC1101_SIDLE);           // Send strobe command
  cc1101_Deselect();                    // Deselect CC1101
  
  //cmdStrobe(byte cmd) 
  cc1101_Select();                      // Select CC1101
  wait_Miso();                          // Wait until MISO goes low
  SPI.transfer(CC1101_SPWD);            // Send strobe command
  cc1101_Deselect();                    // Deselect CC1101

  delay(100);
}

