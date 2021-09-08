rpilights 16x32
==========

![](misc/title.jpg)

Software for a 16x32 LED display such as the one shown above, and in [this video](https://www.youtube.com/watch?v=XgLVOO9Eu0).

### Hardware

Two of [these displays](https://www.amazon.com/gp/product/B01DC0IPVU),
chained together.

Connect GND, 5V and DIN on the display to Ground, 5V power and GPIO18, respectively, on the Raspberry Pi.

### Software

Install into the directory /home/pi/rpilights:

	sudo apt-get update
	sudo apt-get install git
	sudo apt-get install imagemagick
	cd /home/pi
	git clone --recursive git://github.com/yggdrasilradio/rpilights.git

The command "make" should build the "rpilights" command.

The "snow" animation vertically scrolls the graphic data in the file images/snow.gif.rgb.

The "valentines" animation horizontally scrolls the graphic data in the file images/valentines.gif.rgb.

The "stpatricks" animation horizontally scrolls the graphic data in the file images/stpatricksday.gif.rgb.

The .rgb files are generated from the script gif2rgb (requires installation
of ImageMagick) via the commands:

	gif2rgb snow.gif
	gif2rgb valentines.gif
	gif2rgb stpatricksday.gif

Make your own files with the dimensions of your display if it differs from 16x32.

### map.txt file

The "map.txt" file describes the physical layout of your LED lights as an ASCII picture, viewed from the back of the display.
The existing map.txt file is for the 16x32 display described above.
Please edit it to reflect the arrangement of your lights, if they differ:

	o		LED lights
	| and --	wires
	0 and 1		The first LED light on channels 0 (pin 18) and 1 (pin 19)
	.		(dot) Unused light position (gaps between panels)

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

The format of the map.txt file is very fussy; it can't have comments in it, blank lines at the beginning, the spacing has to be exactly like
the examples, etc.

The map.txt file that is included works for a 16x32 display.

### rpilights command

The "rpilights" command by itself with no further arguments should give a list of possible commands:


	Usage:	rpilights on		Turn lights on (show last animation)
		rpilights off		Turn lights off
		rpilights weather	Display scrolling time, date, weather
		rpilights red		Set all lights to red
		rpilights blue		Set all lights to blue
		rpilights green		Set all lights to green
		rpilights magenta	Set all lights to magenta
		rpilights yellow	Set all lights to yellow
		rpilights cyan		Set all lights to cyan
		rpilights rainbow	Display scrolling rainbow pattern
		rpilights twinkle	Display twinkle lights animation
		rpilights ip		Display scrolling IP address
		rpilights lines		Display random lines animation
		rpilights fireworks	Display fireworks animation
		rpilights squares	Display animated squares
		rpilights shapes	Display animated shapes
		rpilights snow		Display animated snow
		rpilights pacman	Display Pacman animation
		rpilights valentines	Display Valentine's Day animation
		rpilights stpatricks	Display St Patrick's Day animation

### rc.local

The file misc/rc.local has suggested commands to add to your existing /etc/rc.local file.

### .messages5x8 and .messages5x5

If these files exist, messages from .messages5x8 will scroll as well as the temperature and weather in the upper line
of the weather display, and messages from .messages5x5 will scroll along with the date and time in the lower line.  If
there are multiple lines in the file each line will display in turn.

The default color for the 5x8 messages will be cyan, and the default color for the 5x5 messages will be yellow. but you can embed
escape sequences in these files to change the color of the fonts:

	ESC R	Red
	ESC G	Green
	ESC B	Blue
	ESC C	Cyan
	ESC M	Magenta
	ESC Y	Yellow
	ESC W	White

### updatewx script

Edit the file scripts/updatewx to put your zipcode in place of mine (55118).
This script gets weather info (forecast and temperature) every five minutes.
You'll want to start this script from rc.local.  If you don't run this, the 'weather' command won't work.
