rpidisplay
==========

![](misc/title.jpg)

Software for LED window display such as the one in [this video](https://www.youtube.com/watch?v=n9-j6oeJJw4).

### Hardware

Instructions for making the hardware:

NOT DONE YET

### Software

Put all the files into the directory /home/pi/rpilights

The command "make" should build the "rpilights" command, but it won't work properly until you configure the map.txt file
to reflect the physical layout of your LED display.

### map.txt file

The "map.txt" file describes the physical layout of your LED lights as an ASCII picture, viewed from the back of the display.
The existing map.txt file is for a complicated two-channel multi-panel display that I have in my four basement windows right
now.  Please edit it to reflect the arrangement of your lights:

	o			LED lights
	| and -		wires
	0 and 1		The first LED light on channels 0 (pin 18) and 1 (pin 19)

For example, a simple 10x10 LED panel might look like this:

	o--o  o--o  o--o  o--o  o--o
	|  |  |  |  |  |  |  |  |  |
	o  o  o  o  o  o  o  o  o  o
	|  |  |  |  |  |  |  |  |  |
	o  o  o  o  o  o  o  o  o  o
	|  |  |  |  |  |  |  |  |  |
	o  o  o  o  o  o  o  o  o  o
	|  |  |  |  |  |  |  |  |  |
	o  o  o  o  o  o  o  o  o  o
	|  |  |  |  |  |  |  |  |  |
	o  o  o  o  o  o  o  o  o  o
	|  |  |  |  |  |  |  |  |  |
	o  o  o  o  o  o  o  o  o  o
	|  |  |  |  |  |  |  |  |  |
	o  o  o  o  o  o  o  o  o  o
	|  |  |  |  |  |  |  |  |  |
	o  o  o  o  o  o  o  o  o  o
	|  |  |  |  |  |  |  |  |  |
	0  o--o  o--o  o--o  o--o  o

### rpilights command

The "rpilights" command by itself with no further arguments should give a list of possible commands:

	rpilights on		Display scrolling time, date, weather
	rpilights off		Turn lights off
	rpilights red		Set all lights to red
	rpilights blue		Set all lights to blue
	rpilights green		Set all lights to green
	rpilights magenta	Set all lights to magenta
	rpilights cyan		Set all lights to cyan
	rpilights rainbow	Display scrolling rainbow pattern
	rpilights ip		Display scrolling IP address
	rpilights pacman	Display Pacman animation

### rc.local

The file misc/rc.local has suggested commands to add to your existing /etc/rc.local file.

### updatewx script

Edit the file scripts/updatewx to put your zipcode in place of mine (55337).
This script gets weather info (forecast, temperature and humidity) every five minutes.
You'll want to start this script from rc.local.
