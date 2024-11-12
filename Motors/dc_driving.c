/**
 * @file dc_driving.c
 * @author Ali Nasrolahi (a.nasrolahi01@gmail.com)
 * @brief Gist for drving a DC motor using a L293D driver.
 * @date 2024-11-12
 *
 * @note This code snippet uses my own AVR library available at
 * https://github.com/Ali-Nasrolahi/avrlib
 *
 * @note There are 3 inputs for different control aspects of the driver:
 *          1. Enable / disable the driver, which is is pull-downed button.
 *          2. Potentiometer to control output PWM cycles.
 *          3. Direction which is pull-downed button to control the direction.
 *      Also there are 4 outputs
 *          1 and 2. Two PWM outputs.
 *          3. Enable and disable driver pin.
 *          4. A LED to indicate enabled driver.
 *      I've used predefined bidirectional driving circuit in L2932D datasheet application notes.
 *
 *      Connected two PWMs to two inputs of the driver, and provided with external power supply for
 *      for the motor.
 */
#include <stdlib.h>

#include "avrlib/hal.h"
#include "avrlib/utility.h"

int main(void)
{
    bool enabled = false;
    bool direction = false;

    DDRD |= _BV(PIND2 /* Enable / Disable the driver OUTPUT */);
    DDRB |= _BV(PINB1 /* PWM 1 */) | _BV(PINB5 /* BUILTIN_LED */) | _BV(PINB3 /* PWM 2 */);
    DDRC &= ~(_BV(PINC0) /* Enable / disable the driver INPUT */ |
              _BV(PINC1) /* Pot Analog Inputs to control speed */ | _BV(PINC2) /* Direction */);

    UNSET_BIT(PORTD, PIND2);
    UNSET_BIT(PORTB, PINB1);
    UNSET_BIT(PORTB, PINB3);
    UNSET_BIT(PORTB, PINB5);

    adc_init(ADC_REF_AVCC, 1, ADC_PRESCALER_128);

    pwm_init(&TCCR1A, &TCCR1B);  // OCR1A, PINB1
    pwm_init(&TCCR2A, &TCCR2B);  // OCR2A, PINB3

    while (1) {
        if (bit_is_set(PINC, PINC0)) {
            _delay_ms(250);
            if (bit_is_set(PINC, PINC0)) {
                if (enabled) UNSET_BIT(PORTD, PIND2), UNSET_BIT(PORTB, PINB5), enabled = false;
                else SET_BIT(PORTD, PIND2), SET_BIT(PORTB, PINB5), enabled = true;
            }
        }

        if (bit_is_set(PINC, PINC2)) {
            _delay_ms(250);
            if (bit_is_set(PINC, PINC2)) {
                if (direction) {
                    pwm_deinit(&TCCR1A);
                    pwm_init(&TCCR2A, &TCCR2B);
                    direction = false;
                } else {
                    pwm_deinit(&TCCR2A);
                    pwm_init(&TCCR1A, &TCCR1B);
                    direction = true;
                }
            }
        }
        if (direction) OCR1A = adc_read();
        else OCR2A = (adc_read() >> 2);
    }
}
