
#include <avr/wdt.h>

#define LEDPIN  13

int  flag_8s = 0;
long count = 0, counter = 0, time0;

void setup() {
  pinMode(LEDPIN, OUTPUT);
  Serial.begin(57600); 
  Serial.println("Berechnung WDT drift...");
  Serial.println("Ein externer Quarz an XTAL ist hierfür notwendig.");
  MCUSR = 0;
  WDTCSR = 1<<WDCE | 1<<WDE;
  WDTCSR = 1<<WDIE | 1<<WDP3 | 1<<WDP0;
  wdt_reset();
  time0 = (long)millis();
}

void loop() {

  if (flag_8s) {
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
    WDTCSR = 1<<WDIE;
    flag_8s = 0;
    //Serial.print("count:   "); Serial.println(count); Serial.print("counter: "); Serial.println(counter);
    if (count) {
      float ratio = (float)(counter - time0) / (float)(count * 8000UL);
      float percnt = (1.0 - ratio) * 100.0;
      Serial.print("Count:  "); Serial.print(count); Serial.print("  ");
      Serial.print("Abweichung [%]:  "); Serial.print(percnt, 4); Serial.print("  ");
      Serial.print("Korrekturfaktor: "); Serial.println(1.0 / ratio, 4);
    }
  }
}

ISR(WDT_vect) {
  counter = millis();
  count++;
  flag_8s = 1;
}

