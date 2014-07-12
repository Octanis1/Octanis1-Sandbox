//sleep code from http://donalmorrissey.blogspot.ch/2011/11/sleeping-arduino-part-4-wake-up-via.html
//rtty code from http://ukhas.org.uk/guides:linkingarduinotontx2
//bmp180 from adafruit
//hx1 datasheet http://www.radiometrix.com/files/additional/hx1.pdf
//this is a mashup for a range test

#define RADIOPIN 5
 
#include <string.h>
#include <util/crc16.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

char datastring[80];
char initbc[80];
volatile int f_timer=0;
int bc=0;
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

/***************************************************
 *  Name:        ISR(TIMER1_OVF_vect)
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Timer1 Overflow interrupt.
 *
 ***************************************************/
ISR(TIMER1_OVF_vect)
{
  /* set the flag. */
   if(f_timer == 0)
   {
     f_timer = 1;
   }
}


/***************************************************
 *  Name:        enterSleep
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Enters the arduino into sleep mode.
 *
 ***************************************************/
void enterSleep(void)
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  
  sleep_enable();


  /* Disable all of the unused peripherals. This will reduce power
   * consumption further and, more importantly, some of these
   * peripherals may generate interrupts that will wake our Arduino from
   * sleep!
   */
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer2_disable();
  power_twi_disable();  

  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the timer timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
}
 
void rtty_txstring (char * string)
{
 
/* Simple function to sent a char at a time to
 ** rtty_txbyte function.
 ** NB Each char is one byte (8 Bits)
 */
 
char c;
 
c = *string++;
 
while ( c != '\0')
 {
 rtty_txbyte (c);
 c = *string++;
 }
}
void rtty_txbyte (char c)
{
 /* Simple function to sent each bit of a char to
 ** rtty_txbit function.
 ** NB The bits are sent Least Significant Bit first
 **
 ** All chars should be preceded with a 0 and
 ** proceed with a 1. 0 = Start bit; 1 = Stop bit
 **
 */
 
int i;
 
rtty_txbit (0); // Start bit
 
// Send bits for for char LSB first
 
for (i=0;i<7;i++) // Change this here 7 or 8 for ASCII-7 / ASCII-8
 {
 if (c & 1) rtty_txbit(1);
 
else rtty_txbit(0);
 
c = c >> 1;
 
}
rtty_txbit (1); // Stop bit
rtty_txbit (1); // Stop bit
}
 
void rtty_txbit (int bit)
{
 if (bit)
 {
 // high
 analogWrite(RADIOPIN,110);
 }
 else
 {
 // low
 analogWrite(RADIOPIN,100);
 
}
 
// delayMicroseconds(3370); // 300 baud
 delayMicroseconds(10000); // For 50 Baud uncomment this and the line below.
 delayMicroseconds(10150); // You can't do 20150 it just doesn't work as the
 // largest value that will produce an accurate delay is 16383
 // See : http://arduino.cc/en/Reference/DelayMicroseconds
 
}
 
uint16_t gps_CRC16_checksum (char *string)
{
 size_t i;
 uint16_t crc;
 uint8_t c;
 
crc = 0xFFFF;
 
// Calculate checksum ignoring the first two $s
 for (i = 2; i < strlen(string); i++)
 {
 c = string[i];
 crc = _crc_xmodem_update (crc, c);
 }
 
return crc;
}
 
void setPwmFrequency(int pin, int divisor) {
 byte mode;
 if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
 switch(divisor) {
 case 1:
 mode = 0x01;
 break;
 case 8:
 mode = 0x02;
 break;
 case 64:
 mode = 0x03;
 break;
 case 256:
 mode = 0x04;
 break;
 case 1024:
 mode = 0x05;
 break;
 default:
 return;
 }
 if(pin == 5 || pin == 6) {
 TCCR0B = TCCR0B & 0b11111000 | mode;
 }
 else {
 TCCR1B = TCCR1B & 0b11111000 | mode;
 }
 }
 else if(pin == 3 || pin == 11) {
 switch(divisor) {
 case 1:
 mode = 0x01;
 break;
 case 8:
 mode = 0x02;
 break;
 case 32:
 mode = 0x03;
 break;
 case 64:
 mode = 0x04;
 break;
 case 128:
 mode = 0x05;
 break;
 case 256:
 mode = 0x06;
 break;
 case 1024:
 mode = 0x7;
 break;
 default:
 return;
 }
 TCCR2B = TCCR2B & 0b11111000 | mode;
 }
}

char * floatToString(char * outstr, float value, int places, int minwidth=0, bool rightjustify=false) {
    // this is used to write a float value to string, outstr.  oustr is also the return value.
    int digit;
    float tens = 0.1;
    int tenscount = 0;
    int i;
    float tempfloat = value;
    int c = 0;
    int charcount = 1;
    int extra = 0;
    // make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
    // if this rounding step isn't here, the value  54.321 prints as 54.3209

    // calculate rounding term d:   0.5/pow(10,places)  
    float d = 0.5;
    if (value < 0)
        d *= -1.0;
    // divide by ten for each decimal place
    for (i = 0; i < places; i++)
        d/= 10.0;    
    // this small addition, combined with truncation will round our values properly
    tempfloat +=  d;

    // first get value tens to be the large power of ten less than value    
    if (value < 0)
        tempfloat *= -1.0;
    while ((tens * 10.0) <= tempfloat) {
        tens *= 10.0;
        tenscount += 1;
    }

    if (tenscount > 0)
        charcount += tenscount;
    else
        charcount += 1;

    if (value < 0)
        charcount += 1;
    charcount += 1 + places;

    minwidth += 1; // both count the null final character
    if (minwidth > charcount){        
        extra = minwidth - charcount;
        charcount = minwidth;
    }

    if (extra > 0 and rightjustify) {
        for (int i = 0; i< extra; i++) {
            outstr[c++] = ' ';
        }
    }

    // write out the negative if needed
    if (value < 0)
        outstr[c++] = '-';

    if (tenscount == 0)
        outstr[c++] = '0';

    for (i=0; i< tenscount; i++) {
        digit = (int) (tempfloat/tens);
        itoa(digit, &outstr[c++], 10);
        tempfloat = tempfloat - ((float)digit * tens);
        tens /= 10.0;
    }

    // if no places after decimal, stop now and return

    // otherwise, write the point and continue on
    if (places > 0)
    outstr[c++] = '.';


    // now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
    for (i = 0; i < places; i++) {
        tempfloat *= 10.0;
        digit = (int) tempfloat;
        itoa(digit, &outstr[c++], 10);
        // once written, subtract off that digit
        tempfloat = tempfloat - (float) digit;
    }
    if (extra > 0 and not rightjustify) {
        for (int i = 0; i< extra; i++) {
            outstr[c++] = ' ';
        }
    }


    outstr[c++] = '\0';
    return outstr;
}


void setup() {
  //SENSOR:
  /* Initialise the sensor */
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
    
  //BROADCAST:
   pinMode(RADIOPIN,OUTPUT);
   pinMode(13,OUTPUT);
   setPwmFrequency(RADIOPIN, 1);
  /*** Configure the timer.***/
  
  /* Normal timer operation.*/
  TCCR1A = 0x00; 
  
  /* Clear the timer counter register.
   * You can pre-load this register with a value in order to 
   * reduce the timeout period, say if you wanted to wake up
   * ever 4.0 seconds exactly.
   */
  TCNT1=0x0000; 
  
  /* Configure the prescaler for 1:1024, giving us a 
   * timeout of 4.09 seconds.
   */
  TCCR1B = 0x05;
  
  /* Enable the timer overlow interrupt. */
  TIMSK1=0x01;
  
//  Serial.begin(9600);
}
 
void loop() {
  
//check if its broadcast time yet
if(bc>50){ //wait 5min
   /* Get a new sensor event */ 
    sensors_event_t event;
    bmp.getEvent(&event); 
  

    /* current temperature from the BMP085 */
    float temperature;
    bmp.getTemperature(&temperature); 
    //use this:
    char buffer1[25]; 
    char buffer2[25]; 
    String temp = floatToString(buffer1,temperature,1);
    String pres = floatToString(buffer2,event.pressure,1);
   
   //encode
   char buffer3[20];
   String weather = "_T" + temp + "P" + pres + "_";
   weather.toCharArray(buffer3,2);
   
   snprintf(datastring,80,buffer3); // Puts the text in the datastring
   unsigned int CHECKSUM = gps_CRC16_checksum(datastring); // Calculates the checksum for this datastring
   char checksum_str[6];
   sprintf(checksum_str, "*%04X\n", CHECKSUM);
   strcat(datastring,checksum_str);
   
   snprintf(initbc,80,"This is a test 3. Weather following 2. Weather following 1."); // Puts the text in the datastring
   unsigned int CHECKSUM2 = gps_CRC16_checksum(initbc); // Calculates the checksum for this datastring
   char checksum_str2[6];
   sprintf(checksum_str2, "*%04X\n", CHECKSUM2);
   strcat(initbc,checksum_str2);
   
   
   //radio on
   digitalWrite(13,HIGH);
   
   //broadcast
    rtty_txstring (initbc);
    rtty_txstring (datastring);
//   Serial.println(buffer3);
   
   //radio off 
   digitalWrite(13,LOW);
   
   //reset bc counter
   bc = 0;

 }else{
  ++bc;
 }
 //and power-sleep 4s
 enterSleep();
}


