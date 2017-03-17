/*
 * ================================ 
 * Sketch from reading data from the OLIMEX ECG shield
 * ***
 * DOES NOT ADHERE TO THE FORMAT SUGGESTED IN ShieldEkgEmgDemo.ino
 * ***
 * This format associates a timestamp on each new reading.
 * The interrupt hanldler does not contain Serial calls: a
 * 2-positions buffer is used to exchange data with the main loop.
 * The main loop formats the line (without use of the String
 * library) and sends to the serial.
 * The sketch should be used in association with the plot utility
 * Augusto Ciuffoletti - University of Pisa - 2017
 * =================================
*/
#include <compat/deprecated.h>
//http://www.arduino.cc/playground/Main/FlexiTimer2
#include <FlexiTimer2.h>


//~~~~~~~~~~
// Constants
//~~~~~~~~~~

// -------- Define Pins
#define LED1 6
#define SPEAKER 7

// -------- Transmission DA CORREGGERE
//#define SAMPFREQ 125                      // ADC sampling rate
#define SAMPFREQ 200
#define TIMER2VAL (1000/(SAMPFREQ))       // Set 125Hz sampling frequency
#define TXSPEED 115200// 57600 or 115200.

#define BUFLEN 80

// --------- Global Variables
volatile unsigned long int Basetime;
volatile unsigned char counter = 0;	  // Additional divider used to toggle LED
volatile unsigned int ADC_Value = 0;	  // ADC current value

volatile unsigned int Data[6][2];        // Data buffer (two positions)
volatile boolean full[]={false,false};   // Data buffer state
volatile unsigned long int Data_counter;
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
  b=b^1; //toggle 0,1 values
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
  int i=0;
  char Next=b^1;
  unsigned long int t;
  
  if ( full[Next] ) {
    // format data line
    char data_line[80];
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
      itoa(Data[Channel][Next],data_line+i,10);
      while (data_line[i] != '\0' && i < BUFLEN ) i++;
      data_line[i++]=' ';
      data_line[i]='\0';
    }
    Serial.println(data_line);
    
    full[Next]=false;
  }
}

