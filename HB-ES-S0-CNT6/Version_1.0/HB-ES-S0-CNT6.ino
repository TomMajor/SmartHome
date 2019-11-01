
//---------------------------------------------------------
// HB-ES-S0-CNT6
// 2019-02-28 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
//---------------------------------------------------------
// Warning
// The resetAVR() function in this sketch relies on a bootloader which clears the watchdog.
// Otherwise the board will keep resetting on you until you power cycle the board.
// For details, see http://nongnu.org/avr-libc/user-manual/FAQ.html#faq_softreset
// For example, the standard bootloader ATmegaBOOT_168.c compiled with option '-DWATCHDOG_MODS' will do.
// https://github.com/TomMajor/SmartHome/blob/master/Info/Bootloader/mega328/Quarz_Osc/ATmegaBOOT_168_atmega328_pro_8MHz_wdt.hex
//---------------------------------------------------------
// S0-Zähler:
// Gesamtzähler:    Saia-Burgess, ALE3D5F10KB3A00, 1000 Imp./kWh, Impulsbreite 30 ms
// einzelne Kreise: Eltako, WSZ12DE-32A, 2000Imp./kWh, Impulsbreite 50ms
//---------------------------------------------------------

void loop() __attribute__((__optimize__("O2")));

#include <Ethernet.h>
#include <avr/wdt.h>
#include "src/Counter.h"
#include "src/CRC32.h"

#define PIN_LED A0
#define PIN_RESET_W5500 A1
//#define PIN_DEBUG 9
#define RXBUFFER_LEN 32

//---------------------------------------------------------
byte mac[]     = { 0x90, 0xA2, 0xDA, 0x0D, 0x85, 0xD9 };    // physical mac address
byte ip[]      = { 192, 168, 1, 227 };                      // ip in lan
byte subnet[]  = { 255, 255, 255, 0 };                      // subnet mask
byte gateway[] = { 192, 168, 1, 1 };                        // default gateway

// Initialize the Ethernet server library with the IP address and port you want to use (port 80 is default for HTTP):
EthernetServer         server(80);
COUNTER<PIN_LED>       counter;
CRC32                  crc32;
uint16_t               timerEthernetLinkOff;
uint8_t                timerEnableResetCmd;
EthernetHardwareStatus ethHWStatus = EthernetNoHardware;
char                   outBuf[40];
char                   requestBuf[64];
int                    requestPtr;

//---------------------------------------------------------
void setup()
{
    delay(3500);    // try to get a stabilized Vcc before Ethernet init and EEPROM read

    timerEthernetLinkOff = 0;
    timerEnableResetCmd  = 0;

    Serial.begin(57600);
    Serial.println(F("Starting HB-ES-CNT6-S0"));

    pinMode(PIN_RESET_W5500, OUTPUT);
    resetW5500();    // reset WIZnet W5500 Ethernet chip

#ifdef PIN_DEBUG
    pinMode(PIN_DEBUG, OUTPUT);
#endif

    if (startEthernet() != 1) {
        pinMode(PIN_LED, OUTPUT);
        digitalWrite(PIN_LED, LOW);
        blinkLED(100, 150);    // flash LED 100ms for 30 sec
        resetAVR();
    }

    counter.init(&Serial);

    startTimer();
}

//---------------------------------------------------------
void loop()
{
    // listen for incoming clients
    EthernetClient client = server.available();
    if (client) {
        // Serial.println("new client");
        // an http request ends with a blank line
        bool currentLineIsBlank = true;
        memset(requestBuf, 0, sizeof(requestBuf));
        requestPtr = 0;
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                if (requestPtr < (sizeof(requestBuf) - 1)) {
                    requestBuf[requestPtr++] = c;
                }
                // Serial.write(c);
                // if you've gotten to the end of the line (received a newline character) and the line is blank,
                // the http request has ended, so you can send a reply
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");    // the connection will be closed after completion of the response
                    //                    client.println("Refresh: 5");           // refresh the page automatically every 5 sec
                    client.println();
                    client.println("<!DOCTYPE HTML>");
                    client.println("<html>");
                    if (strstr(requestBuf, "GET /counter ") != 0) {
                        pageCounter(client);
                    } else if (strstr(requestBuf, "GET /debug ") != 0) {
                        counter.debugOutput(client);
                    } else {
                        client.println("Usage: <IP Addr>/counter, <IP Addr>/debug");
                    }
                    client.println("</html>");
                    break;
                }
                if (c == '\n') {
                    // you're starting a new line
                    currentLineIsBlank = true;
                } else if (c != '\r') {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }
        delay(100);       // give the web browser time to receive the data
        client.stop();    // close the connection
        // Serial.println("client disconnected");
    }

    serialCmdLineInterpreter();

    static unsigned long      timeStamp    = 0;
    static unsigned long      countSeconds = 0;
    static EthernetLinkStatus lastLink     = Unknown;
    if ((millis() - timeStamp) >= 1000ul) {    // 1sec
        timeStamp = millis();
        countSeconds++;
        if (timerEnableResetCmd > 0) {
            timerEnableResetCmd--;
            if (timerEnableResetCmd == 0) {
                Serial.println(F("Timeout"));
            }
        }
        if (!(countSeconds % 5)) {
            // counter maintenance (backup S0 counter values to EEPROM every hour)
            counter.loop();
            // ETH link status, W5100 does not have this
            if (ethHWStatus == EthernetW5500) {
                EthernetLinkStatus currentLink = Ethernet.linkStatus();
                if (currentLink != lastLink) {
                    lastLink = currentLink;
                    Serial.print(F("Ethernet link change: "));
                    printLinkStatus(currentLink);
                }
                if (currentLink == LinkOFF) {
                    timerEthernetLinkOff += 5;
                } else {
                    timerEthernetLinkOff = 0;
                }
                if (timerEthernetLinkOff > 5400) {
                    stopTimer();
                    Serial.println(F("*** No Ethernet link for 1.5 hours, reseting device ***"));
                    delay(1000);
                    resetAVR();
                }
            }
        }
    }
}

//---------------------------------------------------------
void pageCounter(EthernetClient client)
{
    crc32.reset();
    for (uint8_t ch = 0; ch < cCOUNTER_SIZE; ch++) {
        uint32_t value = counter.getCounter(ch);
        memset(outBuf, 0, sizeof(outBuf));
        sprintf(outBuf, "Counter_%d,%.10lu,", ch + 1, value);
        client.println(outBuf);
        client.println("<br>");
        crc32.update(value);
    }
    uint32_t crc = crc32.finalize();
    memset(outBuf, 0, sizeof(outBuf));
    sprintf(outBuf, "CRC,%.10lu,", crc);
    client.println(outBuf);
}

//---------------------------------------------------------
void serialCmdLineInterpreter()
{
    static char buffer[RXBUFFER_LEN];
    static int  length = 0;

    while (Serial.available()) {
        int data = Serial.read();
        if (data == '\b' || data == '\177') {    // BS and DEL
            if (length) {
                length--;
            }
        } else if (data == '\r') {
            buffer[length] = '\0';
            if (length) {
                if (strcmp_P(buffer, PSTR("FACTORY_RESET")) == 0) {
                    Serial.println(F("Factory reset, are you sure? (Type 'YES' within 10sec)"));
                    timerEnableResetCmd = 10;
                } else if ((strcmp_P(buffer, PSTR("YES")) == 0) && (timerEnableResetCmd > 0)) {
                    stopTimer();
                    counter.clearAll();
                    Serial.println(F("*** FACTORY RESET: clearing all counters and eeprom storage ***"));
                    delay(1000);
                    resetAVR();
                } else {
                    Serial.print(F("Unknown command: "));
                    Serial.println(buffer);
                }
            }
            length = 0;
        } else if (length < (RXBUFFER_LEN - 1)) {
            if (data != '\n') {    // ignore \n
                buffer[length++] = data;
            }
        }
    }
}

//---------------------------------------------------------
uint8_t startEthernet()
{
    uint8_t ret = 0;

    // You can use Ethernet.init(pin) to configure the CS pin
    // Ethernet.init(10);  // Most Arduino shields
    // start the Ethernet connection and the server
    Ethernet.begin(mac, ip, gateway, subnet);

    // Check for Ethernet hardware present
    ethHWStatus = Ethernet.hardwareStatus();
    switch (ethHWStatus) {
    case EthernetW5100:
        Serial.println(F("WIZnet W5100 found"));
        break;
    case EthernetW5200:
        Serial.println(F("WIZnet W5200 found"));
        break;
    case EthernetW5500:
        Serial.println(F("WIZnet W5500 found"));
        ret = 1;    // only a WIZnet W5500 is expected with this project, change this for other hardware
        break;
    case EthernetNoHardware:
    default:
        Serial.println(F("Error: No Ethernet HW found"));
        break;
    }

    if (ethHWStatus != EthernetNoHardware) {
        // link status
        Serial.print(F("Ethernet link: "));
        printLinkStatus(Ethernet.linkStatus());
        // start the server
        server.begin();    // start to listen for clients
        Serial.print(F("IP Address: "));
        Serial.println(Ethernet.localIP());
    }

    return ret;
}

//---------------------------------------------------------
void printLinkStatus(EthernetLinkStatus linkStatus)
{
    switch (linkStatus) {
    case LinkON:
        Serial.println(F("on"));
        break;
    case LinkOFF:
        Serial.println(F("off"));
        break;
    case Unknown:
    default:
        Serial.println(F("unknown"));
        break;
    }
}

//---------------------------------------------------------
void blinkLED(uint16_t timeLED, uint16_t loops)
{
    for (uint16_t i = 0; i < loops; i++) {
        digitalWrite(PIN_LED, HIGH);
        delay(timeLED);
        digitalWrite(PIN_LED, LOW);
        delay(timeLED);
    }
}

//---------------------------------------------------------
void resetAVR()
{
    wdt_enable(WDTO_500MS);
    while (1) {
    }
}

//---------------------------------------------------------
void resetW5500()
{
    digitalWrite(PIN_RESET_W5500, LOW);
    delay(50);
    digitalWrite(PIN_RESET_W5500, HIGH);
    delay(500);
}

//---------------------------------------------------------
void startTimer()
{
    TCCR2A = (1 << WGM21);                               // Timer CTC mode
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);    // clk/1024 = 7812,5 Hz @8MHz
    OCR2A  = 15;                                         // compare match OCR2A interrupt @2ms (7812,5/16 = 488,28Hz)
    TIFR2  = (1 << OCF2A);                               // clear pending OCR2A interrupt
    TIMSK2 = (1 << OCIE2A);                              // enable OCR2A interrupt
}

//---------------------------------------------------------
void stopTimer()
{
    TCCR2B = 0;               // stop timer
    TIMSK2 = 0;               // disable all timer2 interrupts
    TIFR2  = (1 << OCF2A);    // clear pending OCR2A interrupt
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ISR(TIMER2_COMPA_vect)    // @2ms
{
    counter.timer2ms();
#ifdef PIN_DEBUG
    digitalWrite(PIN_DEBUG, digitalRead(PIN_DEBUG) ^ 1);
#endif
}
