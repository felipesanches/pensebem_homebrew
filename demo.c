#include <avr/io.h>
#include <avr/interrupt.h>
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


uint8_t font[] = {
    /* 0: */ 0b11011101, /* 1: */ 0b01010000, /* 2: */ 0b11101100, /* 3: */ 0b11111000,
    /* 4: */ 0b01110001, /* 5: */ 0b10111001, /* 6: */ 0b10111101, /* 7: */ 0b11010000,
    /* 8: */ 0b11111101, /* 9: */ 0b11111001,
    /* A: */ 0b11110101, /* B: */ 0b00111101, /* C: */ 0b10001101, /* D: */ 0b00000000,
    /* E: */ 0b10101101, /* F: */ 0b00000000, /* G: */ 0b10011101, /* H: */ 0b01110101,
    /* I: */ 0b00000100, /* J: */ 0b11011100, /* K: ?! */ 0b00000000, /* L: */ 0b01010000,
    /* M: */ 0b11010101, /* N: */ 0b00110100, /* O: */ 0b00000000, /* P: */ 0b11100101,
    /* Q: */ 0b00000000, /* R: */ 0b00100100, /* S: */ 0b10111001, /* T: */ 0b00000000,
    /* U: */ 0b01011101, /* V: */ 0b01011101, /* W: */ 0b00000000, /* X: */ 0b00000000,
    /* Y: */ 0b01111001, /* Z: */ 0b00000000,
    /* : */ 0b00000000, /* : */ 0b00000000, /* : */ 0b00000000, /* : */ 0b00000000,
};

void print_char(uint8_t position, const char c){
    if (position > 7) return;
    if (position > 2) position++; // skip the extra segments of the special 14seg digit
    position = 7 - position;
    if (c >= '0' && c <= '9') { display[position] = font[c - '0']; return; }
    if (c >= 'A' && c <= 'Z') { display[position] = font[c - 'A' + 10]; return; }
    if (c >= 'a' && c <= 'z') { display[position] = font[c - 'a' + 10]; return; }

    // otherwise, print a blank since we do not have a glyph for this character yet in the font.
    display[position] = 0x00; return;
}

void print_string(uint8_t position, const char* str){
    while (position < 7 && *str)
        print_char(position++, *str++);
}

uint16_t count=0;
uint16_t x=0;
void scroll_text(const char* str){
    count++;
    if (count > 50){
        count=0;
        x++;
    }
    print_string(0, &str[x]);
}

#define k 10
void wave_animation(){
    count++;
    if (count > 8*k) count=0;

    for (uint8_t digit=0; digit<8; digit++){
        display[digit] = (1<< ((digit+count/k)%8));
    }
}

void init_sound(){
    // Configuração do TIMER0 (base de tempo para playback de música)
    TIMSK0 = _BV(OCIE0A);  // Enable Interrupt TimerCounter0 Compare Match A (SIG_OUTPUT_COMPARE0A)
    TCCR0A = _BV(WGM01);  // Mode = CTC
    TCCR0B = _BV(CS02) | _BV(CS00);   // Clock/1024, 0.001024 seconds per tick
    OCR0A = 244;          // 0.001024*244 ~= .25 SIG_OUTPUT_COMPARE0A will be triggered 4 times per second.
    sei();         //enable interrupts

    // Configuração do TIMER1 (gerador de frequencias das notas musicais)
    TCCR1A = (1 << COM1A0);	// Toggle OC1A on compare match
    TCCR1B = (1 << WGM12) | (1 << CS10);	// CTC mode, no prescaler
    DDRB |= 0b00000010;		//Set OC1A as an Output.
}

void set_freq(int f){
    OCR1A = 16000000.0/f;	// carrega registrador de comparação: 16MHz/freq
}

uint32_t time=0;
uint16_t notes[] = {
    440, 880, 440, 550, 660, 770, 440, 300,
    440, 880, 440, 550, 660, 770, 440, 300,
    660, 920, 660, 550, 880, 770, 300, 380,
    880, 1760, 880, 1100, 1320, 1540, 880, 600,
};
ISR (TIMER0_COMPA_vect)
{
    set_freq(((time/(48*8) + 1)%2 + 0.5) * notes[(time/3)%32]);
    time++;
}

int main(){
    init_display();
    init_sound();

    while (1){
        update_display();
//        scroll_text("123 happy hacking 1234567890");
        wave_animation();
    }
    return 0;
}
