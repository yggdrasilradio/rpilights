rpidisplay
==========

![](misc/title.jpg)

### Hardware

Instructions for making the hardware:

NOT DONE YET

### Software

Put all the files into the directory /home/pi/rpilights

The command "make" should build the "rpilights" command.

NOT DONE YET

The "rpilights" command by itself with no further arguments should give a list of possible commands:

	rpilights on		Display scrolling time, date, weather
	rpilights off		Turn lights off
	rpilights red		Set all lights to red
	rpilights blue		Set all lights to blue
	rpilights green		Set all lights to green
	rpilights magenta	Set all lights to magenta
	rpilights cyan		Set all lights to cyan
	rpilights rainbow	Display rainbow pattern

### rc.local

The file misc/rc.local has suggested commands to put into /etc/rc.local

### updatewx script

Edit the file misc/updatewx to put your zipcode in place of mine (55337).
This script gets weather info (forecast, temperature and humidity) every five minutes.
