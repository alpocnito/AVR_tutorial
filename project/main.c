#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define FOSC 8000000 //Clock Speed
#define BAUD 9600
const uint8_t MYUBRR = FOSC / 8 / BAUD - 1;

void delay_us(uint32_t nopes)
{
    uint32_t count = nopes / (FOSC/1000000);
    count /= 2;
    for (uint32_t i = 0; i < count; ++i)
        ;
}

void USART_Init (void)
{
    // start writing to the UBBRH
    //UCSRC &= ~(1<<URSEL);

    UBRRH = (uint8_t)(MYUBRR >> 8);
    UBRRL = (uint8_t)MYUBRR;

    // enable receive and transmit
    UCSRB = (1 << RXEN) | (1 << TXEN);

    // disable all interruption
    UCSRB &= ~(1 << RXCIE) & ~(1 << TXCIE) & ~(1 << UDRIE);

    // asynchronous mode
    UCSRC &= ~(1 << UMSEL) | (1 << URSEL);

    // no parite checker;
    UCSRC &= (~(1 << UPM0) & ~(1 << UPM1)) | (1 << URSEL);

    // one stop bit
    UCSRC &= ~(1 << USBS) | (1 << URSEL);
    
    // double speed mode
    UCSRA |= (1 << U2X);

    // 8 bit character size
    UCSRC = (1 << UCSZ0) | (1 << UCSZ1) | (1 << URSEL);
    UCSRB &= ~(1 << UCSZ2);
}

int serial_available ()
{
    // waiting for unread data in receive buffer
    return (UCSRA & (1 << RXC));
};
int serial_read ()
{
    // read buffer!
    return UDR;
};

void serial_write (uint8_t data)
{
    // waitinng for transmit buffer
    while (!(UCSRA & (1 << UDRE)))
        ;

    // send data!    
    UDR = data;
};

void serial_write_array(char* buffer, uint8_t size)
{
    while (size--) {
        serial_write(*buffer++);
    }
}

void WRITE_HEX_8(const uint8_t data)
{
    uint8_t out = data;
    if (out < 10)
        out += '0';
    else
        out += 'A' - 10;
        
    serial_write(out);
}

void receive (uint8_t* buf , int len)
{
    for (int read_bytes = 0; read_bytes < len; ++read_bytes)
    {
        uint8_t ret = 0;
        while ((ret = serial_read ()) < 0)
            ;

        buf[read_bytes] = ret;
    }
}

void Timer_Init()
{
  //triger pin as output
  DDRA = 0x01;
  
  // turn on pullup
  PORTD = 0xFF;

  // CTC mode
  TCCR1B |= (1 << WGM12);
  
  // enable interrupt on compare
  TIMSK = (1 << OCIE1A);
  OCR1AL = 250;
  OCR1AH = 100;
}

uint8_t TimerOverflow = 0;
ISR(TIMER1_COMPA_vect)
{
  TimerOverflow = 1;
  TIFR &= ~(1 << ICF1);
}

const uint8_t DISTANCE_STRING_LEN = 2;
uint8_t* DistanceString = "12";

uint8_t* MeasureDistance(uint8_t* size, uint8_t trigger_pin)
{
  // Give 10us trigger pulse on trig. pin to HC-SR04
  while (1)
  {
  volatile uint8_t a;
  PORTA |= (1 << trigger_pin);
  delay_us(10);
  PORTA &= (~(1 << trigger_pin));
  delay_us(10);
}
	// Clear Timer counter
	TCNT1 = 0;   
  
  // Start timer without prescaler
	TCCR1B |= (1 << CS10);
  // Capture on rising edge
  TCCR1B |= (1 << ICES1);		
  // Clear ICP flag (Input Capture flag)
  TIFR |= (1 << ICF1);		
  // Clear Timer Overflow flag
  TIFR |= (1 << OCF1A); 
	
  // Calculate width of Echo by Input Capture (ICP)	
	while ((TIFR & (1 << ICF1)) == 0);

  // Clear Timer counter
	TCNT1 = 0;

  // Capture on falling edge
	TCCR1B &= ~(1 << ICES1);
  // Clear ICP flag (Input Capture flag)
  TIFR |= (1 << ICF1);
  // Clear Timer Overflow flag
  TIFR |= (1 << OCF1A);		
  // Clear Timer overflow count
  TimerOverflow = 0;
  
  // Wait for falling edge
	while ((TIFR & (1 << ICF1)) == 0);

  if (TimerOverflow)
  {
    *size = 0;
    return 0;
  }
  
  // MicroSeconds = ICR1 / (FOSC / 1000000);
  uint16_t distnaceMm = (ICR1 / (FOSC / 1000000) ) / 6;

  *size = 2;
  DistanceString[0] = distnaceMm % 256;
  DistanceString[1] = distnaceMm / 256;

  return DistanceString;
}

int main ()
{
    cli ();
    USART_Init();
    Timer_Init();
    sei();
    
    _delay_us(10);
    
    while (1)
    {
        uint8_t size = 0;
        uint8_t* dist = MeasureDistance(&size, 0);
        
        if (dist == 0)
          serial_write_array("Error with distance measure\n", 28);
        else
        {
          serial_write_array("Distance: ", 10);
          for (uint8_t i = 0; i < size; ++i)
            WRITE_HEX_8(dist[size - 1 - i]);
          serial_write_array("\n", 1);
        }

        _delay_us(500000);
    }
}

