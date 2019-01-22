// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SleepTestRTC (AVR Power-save Mode)
// 2019-01-22 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/3.0/
// Clock Fuses: RC-Oszillator 8MHz, BOD off
// 32,768kHz Quarz an XTAL (RTC an Timer2)
//
// Der AVR wechselt mit diesem Sketch zwischen 4sec Aktiv Mode und 8sec power-save Mode mit RTC wakeup
// !! Falls mit der HW alles stimmt benötigt die Schaltung im power-save Mode nur ca. 0,75uA !!
//
// Auf die uA-Messung sollte man nur dann kurz umschalten wenn der AVR im power-save Mode ist (LED aus) und vor Ablauf der 8sec
// wieder zurück auf den mA-Messbereich. Andernfalls wird der AVR im aktivem Zustand eventuell nicht wieder anlaufen, da der Spannungsabfall
// dann über den uA-Messbereich des Multimeters zu hoch ist.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "LowPower.h"

const int         ledPin   = 4;
volatile uint32_t tick8sec = 0;

void setup()
{
    pinMode(ledPin, OUTPUT);

    Serial.begin(57600);

    ASSR |= (1 << AS2);       // timer2 asynchron mode
    TIMSK2 = 0;               // disable timer2 interrupts
    TCCR2A = (1 << WGM21);    // CTC mode

    TCNT2  = 0;
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);    // prescaler 1024
    OCR2A  = 0xFF;                                       // 8sec
    while (ASSR & ((1 << TCN2UB) | (1 << OCR2AUB) | (1 << OCR2BUB) | (1 << TCR2AUB) | (1 << TCR2BUB)))
        ;
    TIFR2 = (1 << OCF2A);       // clear pending timer2 compare interrupt
    TIMSK2 |= (1 << OCIE2A);    // enable timer2 compare interrupt

    //    DDRB |= (1 << 3); // PB3 / OC2A (MOSI) output
    //    TCCR2A |= (1 << COM2A0); // Toggle OC2A on Compare Match -> 1,6384kHz output mit Prescaler = 1, OCR2A = 9

    delay(1000);
}

void loop()
{
    // Serial.println(tick8sec);

    // stop timer2
    TCCR2B = 0;

    // 4sec LED on
    digitalWrite(ledPin, HIGH);
    delay(4000);
    digitalWrite(ledPin, LOW);

    // start timer2
    TCNT2  = 0;
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);    // prescaler 1024
    while (ASSR & ((1 << TCN2UB) | (1 << OCR2AUB) | (1 << OCR2BUB) | (1 << TCR2AUB) | (1 << TCR2BUB)))
        ;

    // 8sec power-save Modus mit RTC wakeup
    LowPower.powerSave(SLEEP_FOREVER, ADC_OFF, BOD_OFF, TIMER2_ON);    // Leave timer2 in its default state
}

ISR(TIMER2_COMPA_vect)
{
    TCCR2B = TCCR2B;    // dummy write to get a ASSR status update
    tick8sec++;
    while (ASSR & ((1 << TCN2UB) | (1 << OCR2AUB) | (1 << OCR2BUB) | (1 << TCR2AUB) | (1 << TCR2BUB)))
        ;
    /* OCF2A-Flag in ISR löschen: es wird genau genommen nicht gelöscht, sondern es wird ein internes Bit gesetzt, daß mit dem nächsten Takt
    in das TIFR2- Register geschrieben wird. Der nächste Takt ist aber der nächste Takt des 32,768kHz Timers. Erst nach diesem Takt wird das
    Flag tatsächlich gelöscht! Um jetzt kein Delay in die ISR einzufügen, kann man einen Schreibvorgang auf ein Timer-Register ausführen und
    im ASSR-Register abfragen, ob dieser Schreibvorgang erfolgt ist. Wenn dies der Fall ist, sind auch alle anderen Timer-Register
    geschrieben. Vor allen Dingen ist das OCF2A-Flag gelöscht. */
    // Quelle: https://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/Die_Timer_und_Z%C3%A4hler_des_AVR#Timer2_im_Asynchron_Mode
}
