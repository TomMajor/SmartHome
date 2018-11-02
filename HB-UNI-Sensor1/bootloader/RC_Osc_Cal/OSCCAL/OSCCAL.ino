
#include <EEPROM.h>

void setup()
{
  delay(1000);
  Serial.begin(57600);
  Serial.println("");
  
  uint8_t value = EEPROM.read(0x3FC);
  Serial.print("EEPROM OSCCAL storage (Addr 0x3FC): 0x");
  Serial.println(value, HEX);
  
  Serial.print("Default OSCCAL: 0x");
  Serial.println(OSCCAL, HEX);
  
  Serial.println("starting with OSCCAL 0x80");
  Serial.println("Press Send on serial monitor to increment one step.");
  
  delay(500);
  OSCCAL = 0x80;
}

void loop()
{
  if (Serial.available()) {

    uint8_t key = Serial.read(); 
    Serial.read();                      // 1 char sent could be more chars on misalligned baud rate
    Serial.read();
    Serial.read();
    Serial.flush();

    if (key == 'S') {
      Serial.print("Storing EEPROM OSCCAL (Addr 0x3FC): 0x");
      Serial.println(OSCCAL, HEX);
      EEPROM.write(0x3FC, OSCCAL);
      delay(200);
    }

    OSCCAL++;
    if (OSCCAL > 0xA8) {
      OSCCAL = 0x80;
    }
    delay(200);

    Serial.print("OSCCAL: ");
    Serial.println(OSCCAL, HEX);
    delay(500);                         // wait for serial transmission to finish
  }
  delay(100);
}

