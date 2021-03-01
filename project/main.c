#include <avr/io.h>
#include <avr/interrupt.h>

#define COMMAND_GETFIRMWAREVERSION  (0x02)
#define COMMAND_SAMCONFIGURATION    (0x14)
#define COMMAND_INLISTPASSIVETARGET (0x4A)
uint8_t  packet_buffer[64];

int8_t writeCommand(const uint8_t* header, uint8_t hlen, const uint8_t* body = 0, uint8_t blen = 0); // Mark

// what will happen, if PN532 return 32 bytes, but I only read 16?
int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout = 1000); // Mark


///////////////////////////////////////////////////////////////
// Returns the chip's firmware version and ID
///////////////////////////////////////////////////////////////
uint32_t getFirmwareVersion()
{
  uint32_t response;

  packet_buffer[0] = COMMAND_GETFIRMWAREVERSION;
  if (writeCommand(&packet_buffer, 1))
    return 0;
  
  if (readResponse(packet_buffer, sizeof(packet_buffer)) < 0)
    return 0;
  
  response |= packet_buffer[0];
  response << 8;
  response |= packet_buffer[1];
  response << 8;
  response |= packet_buffer[2];
  response << 8;
  response |= packet_buffer[3];

  return response;
}

///////////////////////////////////////////////////////////////
// Configure board to read RFID tags
///////////////////////////////////////////////////////////////
bool SAMConfig()
{
  packet_buffer[0] = COMMAND_SAMCONFIGURATION;
  packet_buffer[1] = 0x01; // normal mode
  packet_buffer[2] = 0x14; // timeout: 50ms * 20 = 1 second
  packet_buffer[3] = 0x01; // use IRQ pin

  if (writeCommand(packet_buffer, 4))
    return false;

  return (readResponse(packet_buffer, sizeof(packet_buffer) > 0);
}

///////////////////////////////////////////////////////////////
// Read target UID
//
// uid       - for uid. Up to 7 bytes
// uidLength - for uid length
// timeout   - number of tries before timeout
//
// Return 1, if all is OK; 0 otherwise
///////////////////////////////////////////////////////////////
bool readPassiveTargetID(uint8_t cardBaudRate, uint8_t* uid, uint8_t* uidLength, uint16_t timeout = 1000);
{
  packet_buffer[0] = COMMAND_INLISTPASSIVETARGET;
  packet_buffer[1] = 1; // max 1 card at once
  packet_buffer[2] = cardBaudRate;

  if (writeCommand(packet_buffer, 3))
    return 0;

  if (readResponse(packet_buffer, sizeof(packet_buffer), timeout) < 0)
    return 0;

  /* Response format:
    
    byte            Description
    --------------  -------------
    b0              Tags found
    b1              Tag Number
    b2..3           SENS_REF
    b4              SEL_RES
    b5              NCFID Length
    b6..NFCIDLen    NFCID
  */

  if (packet_buffer[0] != 1)
    return 0;
  
  uint16_t sens_res = packet_buffer[2];
  sens_res <<= 8;
  sens_res |= packet_buffer[3]

  *uidLength = packet_buffer[5];

  for (uint8_t i = 0; i < packet_buffer[5]; ++i)
    uid[i] = packet_buffer[6 + i];

  return 1;
}



int main(void)
{
 
  //Setup the clock
  cli();                        //Disable global interrupts
  TCCR1B |= 1<<CS11 | 1<<CS10;  //Divide by 64
  OCR1A = 15624;                //Count 15624 cycles for 1 second interrupt
  TCCR1B |= 1<<WGM12;           //Put Timer/Counter1 in CTC mode
  TIMSK  |= 1<<OCIE1A;          //enable timer compare interrupt
  sei();                        //Enable global interrupts
 
                                //Setup the I/O for the LED
  DDRD |= (1<<0);               //Set PortD Pin0 as an output
  PORTD |= (1<<0);              //Set PortD Pin0 high to turn on LED
 
  while(1) { }                  //Loop forever, interrupts do the rest
}
 
ISR(TIMER1_COMPA_vect)          //Interrupt Service Routine
{
  PORTD ^= (1<<0);              //Use xor to toggle the LED
}
