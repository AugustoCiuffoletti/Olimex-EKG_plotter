# A Linux plotter fo the Olimex EKG-EMG shield
A plotter for the Olimex-EKG-EMG using Python and GnuPlot on Ubuntu

## Why this project

The Olimex-EKG shield is an open source board for the acquisition of the electrical signals produced by our body. It can be effectively used to obtain a 3-leads non-diagnostic EKG. Its price is affordable (around 30$), and can be useful for didactical activities and support to medical activity.

It is mounted as an Arduino shield, the latter used to convert the analog signals from the board into digital signals, and possibly to analyze the data before delivering it to other digital devices.

Olimex provides a software that is specific for the board, but only runs on Windows system. Instead I wanted to use the board on a Linux/Ubuntu system. I found a project fitting my need here (https://github.com/logston/olimex-ekg-emg), but I finally decided to make a similar one on my own. A different approach is found here https://github.com/rrainey/ecgkit.

## Operation

The hardware part is straightforward to assemble: just plug the Olimex-EKG-EG shield on top of the arduino (I used an Arduino Uno), and the cable with the electrodes (I only used the wrist one for my experiments). Read carefully the Olimex board documentation and check jumper positioning and other regulations.

Upload the sketch on the Arduino in the usual way.

On the Ubuntu system you should have installed Python (V 2.7.12) and the gnuplot utility. The python libraries I used are not very specific: subprocess, argparse, serial and time. Gnuplot has version 5.0.

Find the device where the arduino USB is attached and launch the ecg-plotter.py program with syntax:
```
python ecg-plotter.py [-h] [--device [DEVICE]] [--username [USERNAME]]

Olimex EKG-EMG visualization

optional arguments:
  -h, --help            show this help message and exit
  --device [DEVICE]     Serial device address (e.g. /dev/ttyUSB0)
  --username [USERNAME]
                        User name associated to this ECG (e.g. John)
```
You can also indicate a string identifier for the person.

The ECG is shown in a window, together with the Id indicated as username and the estimated heartbeat rate (bpm). The data is recorded in a file, whose name is made of the user Id and of the date timestamp.

A tool to re-play the ECG is in preparation: if this project has any success I'll possibily put some more time on it.
