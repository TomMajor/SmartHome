
const int outPin = 8;
const int ledPin = 13;

int active = 0;

void setup()
{
  delay(500);

  Serial.begin(57600);
  Serial.println("PulseSimulatorS0");

  pinMode(outPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  randomSeed(analogRead(0));
}

void loop() 
{
    static long cnt = 0;
    
    if (active) {
    
        digitalWrite(outPin, HIGH);
        digitalWrite(ledPin, HIGH);
        int randPulse = 20 + random(40);    // 20..60ms pulse 
        delay(randPulse);
        digitalWrite(outPin, LOW);
        digitalWrite(ledPin, LOW);
    
        int randWait = 50 + random(200);    // 50..250 delay
        delay(randWait);
    
//    Serial.print(randPulse);
//    Serial.print("/");
//    Serial.println(randWait);
    
        cnt++;
        if (!(cnt%25)) {
            randomSeed(analogRead(0));
            Serial.print(cnt);
            Serial.print(" / 0x");
            Serial.println(cnt, HEX);
        }
    }
    
    if (Serial.available()) {
        int data = Serial.read();
        if (data == 'S') {
            active = 1;
            Serial.println("Started");
        } else if (data == 'P') {
            active = 0;
            Serial.print("Stopped ");
            Serial.print(cnt);
            Serial.print(" / 0x");
            Serial.println(cnt, HEX);
        }
    }
}
