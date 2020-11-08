#include <avr/io.h>
#include <util/delay.h>

/* ---------------------------------------------------------------------
== Notes about the hardware: ==
=== Select keypad rows: ===
keypad pin 1 - PORT_B5
keypad pin 2 - PORT_B3
keypad pin 12 - PORT_B2
keypad pin 13 - PORT_B4

=== Read keypad Columns: ===
keypad pin 3 - PORT_E0
keypad pin 4 - PORT_D2
keypad pin 5 - PORT_E1
keypad pin 6 - PORT_D7
keypad pin 7 - PORT_D6
keypad pin 8 - PORT_D5
keypad pin 9 - PORT_D4
keypad pin 10 - PORT_D3

=== Display digits: ===
digit_7 - PORT_E2
digit_6 - PORT_E3
digit_5 - PORT_C0
digit_4 - PORT_C1
digit_3 - PORT_C2
digit_2 - PORT_C3
digit_1 - PORT_C4
digit_0 - PORT_C5

=== Display segments: ===
In parentheses are the resistor reference numbers on
the PCB for each of these signals.

seg_7 (R11) - PORT_D7
seg_6 (R12) - PORT_D6
seg_5 (R17) - PORT_D5
seg_4 (R13) - PORT_D4
seg_3 (R14) - PORT_D3
seg_2 (R15) - PORT_D2
seg_1 (R18) - PORT_E1
seg_0 (R16) - PORT_E0

=== Piezo buzzer: ===
Port B, bit 1

--------------------------------------------------------------------- */

uint8_t display[8];

void init_display(){
    DDRC = 0x3f;
    DDRD = 0xfc;
    DDRE = 0x0f;
    for (uint8_t digit=0; digit<8; digit++)
        display[digit] = 0;
}

void update_display(){
    for (uint8_t digit=0; digit<8; digit++){
        uint8_t PORTC_value = 0x3f;
        uint8_t PORTD_value = 0xfc;
        uint8_t PORTE_value = 0x0f;

        /*
        === Display digits: ===
        digit_7 - PORT_E2
        digit_6 - PORT_E3
        digit_5 - PORT_C0
        digit_4 - PORT_C1
        digit_3 - PORT_C2
        digit_2 - PORT_C3
        digit_1 - PORT_C4
        digit_0 - PORT_C5
        */

        // select digit
        if (digit <= 5)
            PORTC_value &= ~(0x20 >> digit);
        else
            PORTE_value &= ~(1 << (2 + 7-digit));

        /*
        === Display segments: ===
        seg_7 - PORT_D7
        seg_6 - PORT_D6
        seg_5 - PORT_D5
        seg_4 - PORT_D4
        seg_3 - PORT_D3
        seg_2 - PORT_D2
        seg_1 - PORT_E1
        seg_0 - PORT_E0
        */

        // set digit segments:
        uint8_t value = display[digit];
        PORTD_value &= ~(value & 0xfc);
        PORTE_value &= ~(value & 0x03);

        PORTE = PORTE_value;
        PORTD = PORTD_value;
        PORTC = PORTC_value;

        //give it some time to shine:
        _delay_ms(1);
    }
}

#define k 15
uint16_t count=0;
int main(){
    init_display();

    while (1){
        update_display();

	count++;
	if (count > 8*k) count=0;

        for (uint8_t digit=0; digit<8; digit++){
            display[digit] = (1<< ((digit+count/k)%8));
        }
    }
    return 0;
}
