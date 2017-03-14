/***************************************************************/
/* A Sketch for sending Olimex EKG/EMG data to olimex-ekg-emg: */
/* https://github.com/logston/olimex-ekg-emg                   */
/*                                                             */
/* UPDATER: Paul Logston                                       */
/* License: GNU General Public License (GPLv3)                 */
/* Date: 2015-10-11                                            */
/*                                                             */
/* This sketch was derived from Penko Todorov Bozhkov's        */
/* ShieldEkgEmgDemo.ino sketch. Details of that sketch follow. */
/***************************************************************/

/**********************************************************/
/* Demo program for:                                      */
/*    Board: SHIELD-EKG/EMG + Olimexino328                */
/*  Manufacture: OLIMEX                                   */
/*  COPYRIGHT (C) 2012                                    */
/*  Designed by:  Penko Todorov Bozhkov                   */
/*   Module Name:   Sketch                                */
/*   File   Name:   ShieldEkgEmgDemo.ino                  */
/*   Revision:  Rev.A                                     */
/*    -> Added is suppport for all Arduino boards.        */
/*       This code could be recompiled for all of them!   */
/*   Date: 19.12.2012                                     */
/*   Built with Arduino C/C++ Compiler, version: 1.0.3    */
/**********************************************************/

/***********************************************************
For proper communication, packets sent from Arduino
must be formatted as described below.
///////////////////////////////////////////////
////////// Packet Format Version 2 ////////////
///////////////////////////////////////////////
// 17-byte packets are transmitted from Olimexino328 at 256Hz,
// using 1 start bit, 8 data bits, 1 stop bit, no parity.

// Minimial Transmission Speed
// A sample is taken every 8ms (ie. 125 samples per second)
// 125 samples/s * sizeof(Olimexino328_packet) = 2,125 bytes per second
// 2125 bytes per second = 17,000 bits per second.
// 2.125 kBps (I think we can manage that :)
// 7,650 kB per hour ~ 7.5M MB per hour

struct Olimexino328_packet
{
  uint8_t	sync0;		// = 0xa5
  uint8_t	sync1;		// = 0x5a
  uint8_t	version;	// = 2 (packet version)
  uint8_t	count;		// packet counter. Increases by 1 each packet.
  uint16_t	data[6];	// 10-bit sample (= 0 - 1023) in big endian (Motorola) format.
  uint8_t	switches;	// State of PD5 to PD2, in bits 3 to 0.
};
*/
/**********************************************************/

#include <compat/deprecated.h>
//http://www.arduino.cc/playground/Main/FlexiTimer2
#include <FlexiTimer2.h>

//~~~~~~~~~~
// Constants
//~~~~~~~~~~

// -------- Define Pins
#define LED1 6
#define SPEAKER 7

// -------- Packet Constants
#define NUMCHANNELS 6
#define HEADERLEN 4
#define PACKETLEN (NUMCHANNELS * 2 + HEADERLEN + 1)

// -------- Transmission
//#define SAMPFREQ 125                      // ADC sampling rate
#define SAMPFREQ 200
#define TIMER2VAL (1000/(SAMPFREQ))       // Set 125Hz sampling frequency
#define TXSPEED 115200// 57600 or 115200.

#define BUFLEN 80

// --------- Global Variables
volatile unsigned long int Basetime;
volatile unsigned char counter = 0;	  // Additional divider used to toggle LED
volatile unsigned int ADC_Value = 0;	  // ADC current value

volatile unsigned int Data[6][2];
volatile unsigned long int Data_counter;
volatile boolean full[2];
volatile unsigned char b=0;

//~~~~~~~~~~
// Functions
//~~~~~~~~~~

/****************************************************/
/*  Function name: Toggle_LED1                      */
/*  Parameters                                      */
/*    Input   :  No	                            */
/*    Output  :  No                                 */
/*    Action: Switches-over LED1.                   */
/****************************************************/
void Toggle_LED1(void){
 if (digitalRead(LED1) == HIGH) {
   digitalWrite(LED1, LOW);
 } else {
   digitalWrite(LED1, HIGH);
 }
}


/****************************************************/
/*  Function name: Timer2_Overflow_ISR              */
/*  Parameters                                      */
/*    Input   :  No	                            */
/*    Output  :  No                                 */
/*    Action: Determines ADC sampling frequency.    */
/****************************************************/
void Timer2_Overflow_ISR() {  
  int i;
  unsigned long int t;
  
  if ( full[b] ) {
      Serial.println("fail");
      return;
  }  
  for (int Channel = 0; Channel < 6; Channel++) {
    Data[Channel][b] = analogRead(Channel);
  }
  full[b]=true;
  b=(b+1)%2;

  digitalWrite(LED1, HIGH);

// format data line
  char data_line[80];
  i=0;
  t=millis()-Basetime;
  // hours
  itoa(((t/60)/60)/1000%60,data_line+i,10);
  while (data_line[i] != '\0' && i < BUFLEN ) i++;
  data_line[i++]=':';
  data_line[i]='\0';
  // min
  itoa((t/60)/1000%60,data_line+i,10);
  while (data_line[i] != '\0' && i < BUFLEN ) i++;
  data_line[i++]=':';
  data_line[i]='\0';
  // secs
  int msecs=t/1000%60;
  itoa(t/1000%60,data_line+i,10);
  while (data_line[i] != '\0' && i < BUFLEN ) i++;
  data_line[i++]='.';
  int frac=t%1000;
  if (frac<100) data_line[i++]='0';
  if (frac<10) data_line[i++]='0';
  data_line[i]='\0';
  // msecs
  itoa(frac,data_line+i,10);
  while (data_line[i] != '\0' && i < BUFLEN ) i++;
  data_line[i++]=' ';
  data_line[i]='\0';
  for (int Channel = 0; Channel < 6; Channel++) {
    itoa(Data[Channel][b],data_line+i,10);
    while (data_line[i] != '\0' && i < BUFLEN ) i++;
    data_line[i++]=' ';
    data_line[i]='\0';
  }
  Serial.println(data_line);
  
  full[(b+1)%2]=false;
  
  counter++;
  if (counter / 128 == 0) {
    Toggle_LED1();
  }
}

/****************************************************/
/*  Function name: setup                            */
/*  Parameters                                      */
/*    Input   :  No	                            */
/*    Output  :  No                                 */
/*    Action: Initializes all peripherals           */
/****************************************************/
void setup() {
  // Setup LED1
  pinMode(LED1, OUTPUT);  // Setup LED1 direction
  digitalWrite(LED1, LOW); // Setup LED1 state

  // Setup Speaker
  pinMode(SPEAKER, OUTPUT);
  digitalWrite(SPEAKER, LOW);

  // Start serial port
  Serial.begin(TXSPEED);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  noInterrupts();  // Disable all interrupts before initialization

  Basetime=millis();
  full[0]=false;
  full[1]=false;
  // Timer2
  // Timer2 is used to setup the analag channels sampling frequency and packet update.
  // Whenever interrupt occures, the current read packet is sent to the PC
  // In addition the CAL_SIG is generated as well, so Timer1 is not required in this case!
  FlexiTimer2::set(TIMER2VAL, Timer2_Overflow_ISR);
  FlexiTimer2::start();

  // MCU sleep mode = idle.
  //outb(MCUCR,(inp(MCUCR) | (1<<SE)) & (~(1<<SM0) | ~(1<<SM1) | ~(1<<SM2)));

  interrupts();  // Enable all interrupts after initialization has been completed
}


/****************************************************/
/*  Function name: loop                             */
/*  Parameters                                      */
/*    Input   :  No	                            */
/*    Output  :  No                                 */
/*    Action: Puts MCU into sleep mode.             */
/****************************************************/
void loop() {
  __asm__ __volatile__ ("sleep");
}

