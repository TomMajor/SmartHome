
#include <avr/wdt.h>

#define LEDPIN  13
#define DELAY   200

int  flag_8s = 0;
long count = 0, counter = 0, time0;

void setup() {
  pinMode(LEDPIN, OUTPUT);
  Serial.begin(57600); 
  Serial.println("Starting");
  MCUSR = 0;
  WDTCSR = 1<<WDCE | 1<<WDE;
  WDTCSR = 1<<WDIE | 1<<WDP3 | 1<<WDP0;
  wdt_reset();
  time0 = (long)millis();
  //Serial.print("time0: "); Serial.println(time0);
}

void loop() {

  if (flag_8s) {
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
    WDTCSR = 1<<WDIE;
    flag_8s = 0;
    //Serial.print("count: "); Serial.println(count);
    //Serial.print("counter: "); Serial.println(counter);
    if (counter) {
      float ratio = 1.0 - ((float)(count * 8000UL) / (float)(counter - time0));
      Serial.print("Abweichung: "); Serial.print(ratio,6); Serial.println(" %");
    }
  }
}

ISR(WDT_vect) {
  counter = millis();
  count++;
  flag_8s = 1;
}

