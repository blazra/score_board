#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#define SCLK 2
#define MOSI 0
#define OE 1   // Active low
#define SW_C 3
#define SW_A 4

uint8_t score;
uint8_t id;
uint8_t state;
uint32_t leds;
uint32_t idum;


void delay(uint8_t delay)
{
    uint8_t i;
    for (uint8_t j = 0; j < delay; ++j)
    {
        i = 0;
        while(--i)
        __asm__("");
    }
}

void WDT_off(void)
{
    cli();
    wdt_reset();
    /* Clear WDRF in MCUSR */
    MCUSR &= ~(1<<WDRF);
    /* Write logical one to WDCE and WDE */
    /* Keep old prescaler setting to prevent unintentional time-out */
    WDTCR |= (1<<WDCE) | (1<<WDE);
    /* Turn off WDT */
    WDTCR = 0x00;
    sei();
}

void EEPROM_write(uint8_t ucAddress, uint8_t ucData)
{
    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE))
    ;
    /* Set Programming mode */
    EECR = (0<<EEPM1)|(0>>EEPM0);
    /* Set up address and data registers */
    EEARL = ucAddress;
    EEDR = ucData;
    /* Write logical one to EEMPE */
    EECR |= (1<<EEMPE);
    /* Start eeprom write by setting EEPE */
    EECR |= (1<<EEPE);
}

unsigned char EEPROM_read(uint8_t ucAddress)
{
    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE))
    ;
    /* Set up address register */
    EEARL = ucAddress;
    /* Start eeprom read by writing EERE */
    EECR |= (1<<EERE);
    /* Return data from data register */
    return EEDR;
}

void spi_write_byte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        // Set SCLK low
        PORTB &= ~(1<<SCLK);
        // MSB first
        if(data & (0x80>>i))
            PORTB |= (1<<MOSI);     // Set MOSI high
        else
            PORTB &= ~(1<<MOSI);    // Set MOSI low
        // Set SCLK high
        PORTB |= (1<<SCLK);       
    }
}

void leds_update(uint32_t data)
{
    // Disable output
    PORTB |=  (1<<OE);

    spi_write_byte((uint8_t)(data >> 16));
    spi_write_byte((uint8_t)(data >> 8));
    spi_write_byte((uint8_t)(data));

    // Load data from shift regs to storage regs and enable output
    PORTB &= ~(1<<OE);
    PORTB |=  (1<<OE);
    PORTB &= ~(1<<OE);
}

void show_score(uint8_t score)
{
    uint8_t units = score % 10;
    uint8_t tens = score / 10;
    if((units == 0) && (tens != 0))
    {
        tens--;
        units = 10;
    }
    leds_update( (0xfffffc00 << (10-units)) | ((0xffff << (10-tens)) & 0x03ff) );
}

void leds_blink(uint32_t data, uint8_t duration, uint8_t blank, uint8_t num_of_blinks)
{
    for (uint8_t i = 0; i < num_of_blinks; i++)
    {
        leds_update(data);
        delay(duration);
        leds_update(0ul);
        delay(blank);
    }
}

uint32_t rand()
{
    idum = 1664525L*idum + 1013904223L;
    return idum;
}

int main(void)
{
    WDT_off();

    // Set pins to output mode
    DDRB = (1<<SCLK) | (1<<MOSI) | (1<<OE);
    // Enable pull-ups on inputs and set OE high, because it's active low
    PORTB = (1<<OE) | (1<<SW_A) | (1<<SW_C);


    leds_update(0x00000000);
    delay(255);

    leds_blink(0x000003ff, 100, 100, 1);
    leds_blink(0x00fffc00, 100, 100, 1);

    for (uint8_t i = 0; i < 10; ++i)
    {
        leds_update(rand());
        delay(100);
    }


    id = EEPROM_read(0);
    idum = id;
    score = EEPROM_read(1);

    if(score > 110)
        score = 0;
    EEPROM_write(1, score);
    
    

    
    leds = 1<<10;
    state = 0x00;

    while(1)
    {
        //leds_update(leds | (0xfffffffful>>(32-score)));
        // delay(100);
        // if(leds & (1<<10))
        //     state = 0;
        // if(leds & (1ul<<19))
        //     state = 1;
        // if(state)
        //     leds = leds >> 1;
        // else
        //     leds = leds << 1;


        //leds_update( (0xffff << (10-(score % 10)))  );
        show_score(score);

        if(~PINB & (1<<PINB3))
        {
            score++;
            show_score(score);
            EEPROM_write(1, score);
            delay(10);
            while(~PINB & (1<<SW_C))
                ;
        }
        else if(~PINB & (1<<SW_A))
        {
            score--;
            show_score(score);
            EEPROM_write(1, score);
            delay(10);
            while(~PINB & (1<<SW_A))
                ;
        }
        delay(100);
    }
}

