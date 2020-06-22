/*
 * bilge_pump_switch.c
 *
 * Created: 10/06/2020 11.29.35
 * Author : Leevi Hokkanen
 */

#ifndef F_CPU
#define F_CPU 8000000UL	// 8 MHz frequency
#endif

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <stdbool.h>

#define PUMP_DRIVE_PIN PIND1

void wdt_init();
void enter_sleep();
void power_save();
void setup();


int main(void) {
    setup();
    uint8_t sleep = 0;

    while(1) {
        /* Loop until water level has stayed 7 seconds above sensor */
        for(sleep = 0; sleep < 7;) {
            wdt_reset();    // Sleep 1 second
            enter_sleep();  //

            /* If water level drops below sensor */
            if(PIND & (1 << PIND2)) {
                sleep = 0;
                continue;
            }

            sleep++;
        }
        PORTD |= (1 << PUMP_DRIVE_PIN); // Turn bilge pump on

        /* Loop until water level has stayed 7 seconds below sensor */
        for(sleep = 0; sleep < 7;) {
            wdt_reset();    // Sleep 1 second
            enter_sleep();  //

            /* If water level rises above sensor */
            if(~PIND & (1 << PIND3)) {
                sleep = 0;
                continue;
            }

            sleep++;
        }
        PORTD &= ~(1 << PUMP_DRIVE_PIN); // Turn bilge pump off
    }
}


void wdt_init() {
    cli();  // Disable all interrupts

    /* Reset and disable watchdog timer */
    wdt_reset();
    MCUSR &= ~(1 << WDRF);
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = 0;

    /* Set watchdog timeout to 1 s and enable interrupt mode */
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDIE) | (1 << WDP1) | (1 << WDP2);

    sei();  // Enable all interrupts
}

void enter_sleep() {
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
    WDTCSR |= (1 << WDIE);  // Enable watchdog interrupt mode
}

void power_save() {
    ADCSRA &= ~(1 << ADEN); // Turn off A/D converter
    ACSR &= ~(1 <<  ACIE);  // Turn off analog comparator
    ACSR |= (1 << ACD);		//

    DIDR0 = 0;  // Disable digital input register 0
    DIDR1 = 0;  // Disable Digital input register 1

    PRR = 0b11011111;   // Shutdown individual modules except Timer0
}

void setup() {
    wdt_init();
    power_save();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    DDRD |= (1 << PUMP_DRIVE_PIN);  // Set pump drive pin to output

    PORTB = 0b11111111; // Set pull-up resistors for all unused pins
    PORTC = 0b11111111; //
    PORTD = 0b11110001; //
}

EMPTY_INTERRUPT(WDT_vect);
