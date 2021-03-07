#include <avr/io.h>
#include <avr/interrupt.h>

#define FOSC 1000000 //Clock Speed
#define BAUD 9600
const uint8_t MYUBRR = FOSC/16/BAUD - 1;

void USART_Init(void)
{
    // start writing to the UBBRH
    UCSRC &= ~(1<<URSEL);
    
    UBRRH = (uint8_t)(MYUBRR >> 8);
    UBRRL = (uint8_t) MYUBRR;
    
    uint8_t UCSRB_buf = 0;
    uint8_t UCSRC_buf = 0;

    // enable receive and transmit
    UCSRB_buf |= (1<<RXEN)|(1<<TXEN);
    
    // disable all interruption
    UCSRB_buf &= ~(1<<RXCIE) & ~(1<<TXCIE) & ~(1<<UDRIE);

    // start writing to the UCSRC
    UCSRC |= (1<<URSEL);
    
    // asynchronous mode
    UCSRC_buf &= ~(1<<UMSEL);
    
    // no parite checker;
    UCSRC_buf &= ~(1<<UPM0) & ~(1<<UPM1);
    
    // one stop bit
    UCSRC_buf &= ~(1<<USBS);

    // 8 bit character size
    UCSRC_buf |= (1<<UCSZ0)|(1<<UCSZ1);
    UCSRB_buf &= ~(1<<UCSZ2);
  
    UCSRB = UCSRB_buf;
    UCSRC = UCSRC_buf;
}
/*
int serial_available()
{
    // waiting for unread data in receive buffer
    return (UCSRA & (1<<RXC));
};

int serial_read()
{
    // read buffer!
    return UDR;
};
*/
void serial_write(uint8_t data)
{
    // waitinng for transmit buffer
    while (!( UCSRA & (1<<UDRE)))
        ;
    
    // send data!    
    UDR = data; 
};

int main()
{
    cli();
    USART_Init();
    sei();

    while(1)
    {
        serial_write('A');
    }
}

/*
int main ()
{


  //Setup the clock
  cli ();                        //Disable global interrupts
  TCCR1B |= 1 << CS11 | 1 << CS10;  //Divide by 64
  OCR1A = 15624;                //Count 15624 cycles for 1 second interrupt
  TCCR1B |= 1 << WGM12;           //Put Timer/Counter1 in CTC mode
  TIMSK |= 1 << OCIE1A;          //enable timer compare interrupt
  sei ();                        //Enable global interrupts

                                //Setup the I/O for the LED
  DDRD |= (1 << 0);               //Set PortD Pin0 as an output
  PORTD |= (1 << 0);              //Set PortD Pin0 high to turn on LED

  while (1) {}                  //Loop forever, interrupts do the rest
}

ISR (TIMER1_COMPA_vect)          //Interrupt Service Routine
{
  PORTD ^= (1 << 0);              //Use xor to toggle the LED
} */
