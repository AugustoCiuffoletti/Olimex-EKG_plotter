#!/usr/bin/python
import serial
import subprocess 

from subprocess import Popen,PIPE

smoothed=0;

def analyze(data):
  global smoothed 
  fields=data.split();
  if len(fields)!=0:
    print " "+fields[1]
    smoothed = (smoothed*15+int(fields[1].rstrip()))/16
  print smoothed;
  return smoothed;

prologue="""
set xtics
set xdata time
set timefmt '%H:%M:%S'
set ytics
set style data lines
set yrange [-1:1]
set grid
set datafile separator ' '
"""

# Serial communication parameters
serialIF="/dev/ttyUSB1"
baudrate=115200
# Plot parameters
size=500;        # number of samples per plot
refresh=25;      # number of samples before refresh

ser = serial.Serial(serialIF, baudrate)           # open serial input
data=ser.readline()                               # remove an incomplete line
gnuplot=subprocess.Popen(
  ["gnuplot","-geometry","700x300","-display",":0.0"],
  stdin=PIPE);                                    # open gnuplot process

gnuplot.stdin.write(prologue) # set 

buffer=["0:0:0 512"]*size;  # FIFO fixed-size buffer for a single plot image 
count=0;                    # refresh control
while True:
  count=count+1;
  data=ser.readline().rstrip()
  print data
  s=analyze(data)
#  s=10
#  gnuplot.stdin.write("reset\n")
  gnuplot.stdin.write("set label 1 sprintf(\"m = %s\",\""+str(s)+"\") at screen 0.1, screen 0.9\n")
  buffer.append(data+"\n")
  while len(buffer)>size: # keep buffer length constant
    buffer.pop(0)
  if count < refresh: continue # do not refresh image
  gnuplot.stdin.write("plot \"-\" using 1:(($2-512)/512) title 'ecg'\n");
  for i, val in enumerate(buffer):
    gnuplot.stdin.write(val)
  gnuplot.stdin.write("e\n")
  count=0
