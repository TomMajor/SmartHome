
#include <AskSinPP.h>

// Bad Hack: absoluter Pfad zu den HB-UNI-Sensor1 TSL2561/MAX44009 class includes, da Arduino kein includes relativ zum aktuellen sketch erlaubt
// ich moechte die Sensorklassen aber im HB-UNI-Sensor1 Dir. lassen, keine Lib daraus erstellen sowie auch hier keine Dubletten erzeugen
// Pfad muss natürlich auf anderen Systemen entsprechend angepasst werden!
#include "e:/_user7/Documents/Arduino/20_HomeMatic/SmartHome/HB-UNI-Sensor1/Sensors/Sens_Tsl2561.h"
#include "e:/_user7/Documents/Arduino/20_HomeMatic/SmartHome/HB-UNI-Sensor1/Sensors/Sens_Max44009.h"

using namespace as;

Sens_Tsl2561  tsl2561;
Sens_Max44009 max44009;
char str[160];

const int pinGA1A12S202 = A1;

// --------------------------------------------------------
void setup(void)
{
  analogReference(INTERNAL);          // VRef 1,1V für GA1A12S202 analog Sensor

  Serial.begin(57600);

  tsl2561.init();

  max44009.init();
}

// --------------------------------------------------------
void loop(void)
{
  static int counter;
  int32_t luxGA1A12, luxTSL2561, luxMAX44009;
  int senTSL2561;

  // -----------------------------
  // Helligkeit TSL2561
  if (tsl2561.measure()) {
    luxTSL2561 = tsl2561.brightnessLux();
    senTSL2561 = tsl2561.sensitivity();
  }
  else {
    luxTSL2561 = -1;
    senTSL2561 = -1;
  }
  //Serial.println(luxTSL2561);

  // -----------------------------
  // Helligkeit MAX44009
  if (max44009.measure()) {
    luxMAX44009 = max44009.brightnessLux();
  }
  else {
    luxMAX44009 = -1;
  }
  //Serial.println(luxMAX44009);

  // -----------------------------
  // Helligkeit GA1A12S202
  int adc = 0;
  for (int i = 0; i < 4; i++) {
    adc += analogRead(pinGA1A12S202);
  }
  adc /= 4;
  float uIn = (float)adc * 1.0742;          // Uin = adc/1024 * 1100
  float iR = uIn / 22.0;                    // Strom durch den 22K = U/R in uA
  float lux = pow(10, (iR/10.0));           // Umrechnung I in LogRange 1..5, 5uA = 0.5; 45uA = 4.5, siehe Datenblatt Sensor, dann Lux Wert berechnen
  luxGA1A12 = lux;
  //Serial.print(adc);    Serial.print("  ");
  //Serial.print(uIn, 1); Serial.print("mV  ");
  //Serial.print(iR, 1);  Serial.print("uA  ");
  //Serial.print(lux, 1); Serial.println("Lux");

  // -----------------------------
  // Helligkeiten Log
  sprintf(str, "%u, %d, %ld, %ld, %ld", counter++, senTSL2561, luxGA1A12, luxTSL2561, luxMAX44009);
  Serial.println(str);

  delay(2500);
}

