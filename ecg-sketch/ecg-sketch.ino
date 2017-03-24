/*
* @Copyright (C) 2017 Augusto Ciuffoletti - All rights reserved
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>
*
*
* ================================ 
* Sketch for sending data from the OLIMEX ECG shield across 
* the Arduino serial line.
* ***
* DOES NOT ADHERE TO THE FORMAT SUGGESTED IN ShieldEkgEmgDemo.ino
* ***
* This format associates a timestamp with each new reading.
* The interrupt hanldler does not contain Serial calls: a
* 2-positions buffer is used to exchange data with the main loop.
* The main loop formats the line (without use of the String
* library) and sends to the serial.
* The sketch can be used in association with the 
* ecg-plotter.py utility
* =================================
*/
#include <compat/deprecated.h>
//http://www.arduino.cc/playground/Main/FlexiTimer2
#include <FlexiTimer2.h>

#define SAMPFREQ 200                // Samplig frequency (in Hz)
#define TIMER2VAL (1000/(SAMPFREQ)) // Sampling period (in msecs)
#define BAUDRATE 115200             // Baudrate on the serial line
#define BUFLEN 80                   // Record length

// --------- Global Variables
volatile unsigned long int Basetime;

volatile unsigned int Data[6][2];        // Data buffer (two positions)
volatile boolean full[]={false,false};   // Data buffer state
volatile unsigned char b=0;              // Ready buffer index

/*
 * Sampling function
 * Check data buffer cell is free
 * Fill buffer with data from Olimex-EKG board
 * Rotate the buffer
 */
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

/*
 * Initializes the serial line (see constant for the Baudrate)
 * Sets the basetime for the timestamps
 * Initializes the interrupt that controls sampling
 */
void setup() {
  Serial.begin(BAUDRATE);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Basetime=millis();
  noInterrupts();  
  FlexiTimer2::set(TIMER2VAL, Timer2_Overflow_ISR);
  FlexiTimer2::start();
  interrupts();
}


/*
 * Check if data available 
 * If there there is no data available in the data buffer, 
 * proceed with another loop 
 * When data is available:
 * - compute the timestamp and format as hh:mm:ss.mmm in the
 *   record string
 * - add the 6 raw data from Olimex-ECG board to the on the record 
 *   string
 * - send the record on the serial line
 * - mark the current data buffer as empty
 * - proceed with another loop
 */
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

