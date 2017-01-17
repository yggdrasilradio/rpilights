#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#include "rpi_ws281x/ws2811.h"

#define TRUE 1
#define FALSE 0

#define TARGET_FREQ WS2811_TARGET_FREQ
#define GPIO_PIN0 18
#define GPIO_PIN1 19
#define DMA 5
#define PIDFILE "/var/run/lights.pid"

#define HOLIDAYS "/home/pi/rpilights/.holidays"		// Location of holiday definitions file
#define TEMPERATURE "/home/pi/rpilights/.temperature"	// Location of temperature file
#define FORECAST "/home/pi/rpilights/.forecast"		// Location of weather forecast file
#define MAP "/home/pi/rpilights/map.txt"		// Location of LED map file

#define INT 50		// LED intensity (0 to 255) normally 50

#define RED RGB(255, 0, 0)
#define DARKRED RGB(128, 0, 0)
#define LIGHTRED RGB(255, 85, 85)
#define GREEN RGB(0, 255, 0)
#define DARKGREEN RGB(0, 128, 0)
#define LIGHTGREEN RGB(85, 255, 85)
#define BLUE RGB(0, 0, 255)
#define DARKBLUE RGB(0, 0, 128)
#define LIGHTBLUE RGB(85, 85, 255)
#define CYAN RGB(0, 255, 255)
#define MAGENTA RGB(255, 0, 255)
#define YELLOW RGB(255, 255, 0)
#define ORANGE RGB(255, 128, 0)
#define PURPLE RGB(128, 0, 128)
#define PINK RGB(255, 0, 128)
#define BLACK RGB(0, 0, 0)
#define WHITE RGB(255, 255, 255)
#define GRAY RGB(128, 128, 128)

int16_t width = 0;
int16_t height = 0;
int16_t *ledmap = NULL;

ws2811_t ledstring = {

    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel = {
        [0] = {
            .gpionum = 0,
            .count = 0,
            .invert = 0,
            .brightness = 0,
        },
        [1] = {
            .gpionum = 0,
            .count = 0,
            .invert = 0,
            .brightness = 0
        },
    },
};

// 5x5 font
static const uint8_t font5x5[] = {

	0x00,0x00,0x00,0x00,0x00, // (space)
	0x38,0x44,0x7c,0x44,0x44, // A
	0x78,0x44,0x78,0x44,0x78, // B
	0x3C,0x40,0x40,0x40,0x3C, // C
	0x78,0x44,0x44,0x44,0x78, // D
	0x7c,0x40,0x78,0x40,0x7c, // E
	0x7c,0x40,0x70,0x40,0x40, // F
	0x3c,0x40,0x4c,0x44,0x38, // G
	0x44,0x44,0x7c,0x44,0x44, // H
	0x40,0x40,0x40,0x40,0x40, // I
	0x04,0x04,0x04,0x44,0x38, // J
	0x44,0x48,0x70,0x48,0x44, // K
	0x40,0x40,0x40,0x40,0x7c, // L
	0x44,0x6c,0x54,0x44,0x44, // M
	0x44,0x64,0x54,0x4c,0x44, // N
	0x38,0x44,0x44,0x44,0x38, // O
	0x78,0x44,0x78,0x40,0x40, // P
	0x7c,0x44,0x44,0x7c,0x10, // Q
	0x78,0x44,0x78,0x44,0x44, // R
	0x3c,0x40,0x38,0x04,0x78, // S
	0x7c,0x10,0x10,0x10,0x10, // T
	0x44,0x44,0x44,0x44,0x38, // U
	0x44,0x44,0x28,0x28,0x10, // V
	0x44,0x44,0x54,0x54,0x28, // W
	0x44,0x28,0x10,0x28,0x44, // X
	0x44,0x44,0x28,0x10,0x10, // Y
	0x7c,0x08,0x10,0x20,0x7c, // Z
	0x38,0x44,0x44,0x44,0x38, // 0
	0x20,0x60,0x20,0x20,0x70, // 1
	0x78,0x04,0x38,0x40,0x7c, // 2
	0x78,0x04,0x38,0x04,0x78, // 3
	0x18,0x28,0x48,0x7c,0x08, // 4
	0x7c,0x40,0x78,0x04,0x78, // 5
	0x38,0x40,0x78,0x44,0x38, // 6
	0x7c,0x04,0x08,0x10,0x10, // 7
	0x38,0x44,0x38,0x44,0x38, // 8
	0x38,0x44,0x3C,0x04,0x38, // 9
	0x00,0x40,0x00,0x40,0x00, // :
	0x00,0x00,0x00,0x20,0x40, // ,
	0x00,0x00,0x00,0x00,0x40, // .
	0x40,0x40,0x40,0x00,0x40, // !
	0x20,0x50,0x20,0x00,0x40, // (degree)
	0x28,0x7c,0x28,0x7c,0x28  // #
};

// 5x8 font
static const uint8_t font5x8[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, // (space)
	0x00, 0x00, 0x5F, 0x00, 0x00, // !
	0x00, 0x07, 0x00, 0x07, 0x00, // "
	0x14, 0x7F, 0x14, 0x7F, 0x14, // #
	0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
	0x23, 0x13, 0x08, 0x64, 0x62, // %
	0x36, 0x49, 0x55, 0x22, 0x50, // &
	0x00, 0x05, 0x03, 0x00, 0x00, // '
	0x00, 0x1C, 0x22, 0x41, 0x00, // (
	0x00, 0x41, 0x22, 0x1C, 0x00, // )
	0x10, 0x54, 0x38, 0x54, 0x10, // *
	0x08, 0x08, 0x3E, 0x08, 0x08, // +
	0x00, 0xA0, 0x60, 0x00, 0x00, // ,
	0x08, 0x08, 0x08, 0x08, 0x08, // -
	0x00, 0x60, 0x60, 0x00, 0x00, // .
	0x20, 0x10, 0x08, 0x04, 0x02, // /
	0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
	0x00, 0x42, 0x7F, 0x40, 0x00, // 1
	0x42, 0x61, 0x51, 0x49, 0x46, // 2
	0x21, 0x41, 0x45, 0x4B, 0x31, // 3
	0x18, 0x14, 0x12, 0x7F, 0x10, // 4
	0x27, 0x45, 0x45, 0x45, 0x39, // 5
	0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
	0x01, 0x71, 0x09, 0x05, 0x03, // 7
	0x36, 0x49, 0x49, 0x49, 0x36, // 8
	0x06, 0x49, 0x49, 0x29, 0x1E, // 9
	0x00, 0x36, 0x36, 0x00, 0x00, // :
	0x00, 0x56, 0x36, 0x00, 0x00, // ;
	0x00, 0x08, 0x14, 0x22, 0x41, // <
	0x14, 0x14, 0x14, 0x14, 0x14, // =
	0x41, 0x22, 0x14, 0x08, 0x00, // >
	0x02, 0x01, 0x51, 0x09, 0x06, // ?
	0x32, 0x49, 0x79, 0x41, 0x3E, // @
	0x7E, 0x11, 0x11, 0x11, 0x7E, // A
	0x7F, 0x49, 0x49, 0x49, 0x36, // B
	0x3E, 0x41, 0x41, 0x41, 0x22, // C
	0x7F, 0x41, 0x41, 0x22, 0x1C, // D
	0x7F, 0x49, 0x49, 0x49, 0x41, // E
	0x7F, 0x09, 0x09, 0x01, 0x01, // F
	0x3E, 0x41, 0x41, 0x51, 0x32, // G
	0x7F, 0x08, 0x08, 0x08, 0x7F, // H
	0x00, 0x41, 0x7F, 0x41, 0x00, // I
	0x20, 0x40, 0x41, 0x3F, 0x01, // J
	0x7F, 0x08, 0x14, 0x22, 0x41, // K
	0x7F, 0x40, 0x40, 0x40, 0x40, // L
	0x7F, 0x02, 0x04, 0x02, 0x7F, // M
	0x7F, 0x04, 0x08, 0x10, 0x7F, // N
	0x3E, 0x41, 0x41, 0x41, 0x3E, // O
	0x7F, 0x09, 0x09, 0x09, 0x06, // P
	0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
	0x7F, 0x09, 0x19, 0x29, 0x46, // R
	0x46, 0x49, 0x49, 0x49, 0x31, // S
	0x01, 0x01, 0x7F, 0x01, 0x01, // T
	0x3F, 0x40, 0x40, 0x40, 0x3F, // U
	0x1F, 0x20, 0x40, 0x20, 0x1F, // V
	0x7F, 0x20, 0x18, 0x20, 0x7F, // W
	0x63, 0x14, 0x08, 0x14, 0x63, // X
	0x03, 0x04, 0x78, 0x04, 0x03, // Y
	0x61, 0x51, 0x49, 0x45, 0x43, // Z
	0x00, 0x00, 0x7F, 0x41, 0x41, // [
	0x02, 0x04, 0x08, 0x10, 0x20, // "\"
	0x41, 0x41, 0x7F, 0x00, 0x00, // ]
	0x04, 0x02, 0x01, 0x02, 0x04, // ^
	0x40, 0x40, 0x40, 0x40, 0x40, // _
	0x02, 0x05, 0x02, 0x00, 0x00, // ` (degree symbol)
	0x20, 0x54, 0x54, 0x54, 0x78, // a
	0x7F, 0x48, 0x44, 0x44, 0x38, // b
	0x38, 0x44, 0x44, 0x44, 0x28, // c
	0x38, 0x44, 0x44, 0x48, 0x7F, // d
	0x38, 0x54, 0x54, 0x54, 0x18, // e
	0x08, 0x7E, 0x09, 0x01, 0x02, // f
	0x18, 0xA4, 0xA4, 0xA4, 0x7C, // g
	0x7F, 0x08, 0x04, 0x04, 0x78, // h
	0x00, 0x44, 0x7D, 0x40, 0x00, // i
	0x40, 0x80, 0x84, 0x7D, 0x00, // j
	0x00, 0x7F, 0x10, 0x28, 0x44, // k
	0x00, 0x41, 0x7F, 0x40, 0x00, // l
	0x7C, 0x04, 0x18, 0x04, 0x78, // m
	0x7C, 0x08, 0x04, 0x04, 0x78, // n
	0x38, 0x44, 0x44, 0x44, 0x38, // o
	0xFC, 0x44, 0x44, 0x44, 0x38, // p
	0x38, 0x44, 0x44, 0x44, 0xFC, // q
	0x7C, 0x08, 0x04, 0x04, 0x08, // r
	0x48, 0x54, 0x54, 0x54, 0x20, // s
	0x04, 0x3F, 0x44, 0x40, 0x20, // t
	0x3C, 0x40, 0x40, 0x20, 0x7C, // u
	0x1C, 0x20, 0x40, 0x20, 0x1C, // v
	0x3C, 0x40, 0x30, 0x40, 0x3C, // w
	0x44, 0x28, 0x10, 0x28, 0x44, // x
	0x1C, 0xA0, 0xA0, 0xA0, 0x7C, // y
	0x44, 0x64, 0x54, 0x4C, 0x44, // z
	0x00, 0x08, 0x36, 0x41, 0x00, // {
	0x00, 0x00, 0x7F, 0x00, 0x00, // |
	0x00, 0x41, 0x36, 0x08, 0x00, // }
	0x10, 0x08, 0x10, 0x20, 0x10, // ~
	0x00, 0x00, 0x00, 0x00, 0x00  // DEL (unused)
};

uint32_t RGB(uint32_t r, uint32_t g, uint32_t b) {

	return ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
}

uint32_t ExtractR(uint32_t color) {

	return color >> 16 & 0xFF;
}

uint32_t ExtractG(uint32_t color) {

	return color >> 8 & 0xFF;
}

uint32_t ExtractB(uint32_t color) {

	return color & 0xFF;
}

void clearScreen() {

	uint32_t i, j;

	for (i = 0; i < width; i++)
		for (j = 0; j < height; j++)
			setPixel(i, j, BLACK);
}

uint32_t fadePixel(uint32_t color) {

	uint32_t r, g, b;

	r = ExtractR(color) * .60;
	g = ExtractG(color) * .60;
	b = ExtractB(color) * .60;
	return RGB(r, g, b);
}

void setPixel(int32_t x, int32_t y, uint32_t color) {

	int16_t value, channel;

	if (x < 0)
		return;
	if (x >= width)
		return;
	value = ledmap[(y % height) * width + x];
	if (value < 0)
		return;
	channel = value & 1;
	value = value / 2;
	ledstring.channel[channel].leds[value] = color;
}

uint32_t getPixel(int32_t x, int32_t y) {

	int16_t value, channel;

	if (x < 0)
		return 0;
	if (x >= width)
		return 0;
	value = ledmap[(y % height) * width + x];
	if (value < 0)
		return 0;
	channel = value & 1;
	value = value / 2;
	return ledstring.channel[channel].leds[value];
}

void setPixelNoWrap(uint32_t x, uint32_t y, uint32_t color) {

	if (y >= height)
		return;
	setPixel(x, y, color);
}

void setScreen(uint32_t color) {

	uint32_t i, j;

	for (i = 0; i < width; i++)
		for (j = 0; j < height; j++)
			setPixel(i, j, color);
}

void fadeScreen() {

	uint32_t i, j, color;

	for (i = 0; i < width; i++)
		for (j = 0; j < height; j++) {
			color = getPixel(i, j);
			color = fadePixel(color);
			setPixel(i, j, color);
		}
}

uint32_t Draw5x5Char(uint8_t c, uint32_t x, uint32_t y, uint32_t color) {

	uint8_t *p1, *p2, *p;
	uint32_t n = 0;
	uint32_t i, j, mask;
	uint8_t s[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:,.!`#";

	// If it's a space, advance 2 pixels
	if (c == ' ')
		return(2);

	p = font5x5;
	for (i = 0; i < strlen(s) && s[i] != toupper(c); i++)
		p += 5;

	// Draw character
	for (i = 0; i < 5; i++) {
		if (p[i] != 0) {
			mask = 0x40;
			for (j = 0; j < 5; j++) {
				if ((p[i] & mask) != 0) {
					setPixel(x + j, y + i, color);
					if (j > n)
						n = j;
				}
				mask = mask / 2;
			}
		}
	}

	// Return the character width
	return(n + 1);
}

void Draw5x5String(uint8_t *p, uint32_t x, uint32_t y, uint32_t color) {

	while (*p != '\0') {
		if (*p == ',') // give comma a descender
			x += (Draw5x5Char(*p, x, y + 1, color) + 1);
		else
			x += (Draw5x5Char(*p, x, y, color) + 1);
		*p++;
	}
}

uint32_t Draw5x8Char(uint8_t c, uint32_t x, uint32_t y, uint32_t color) {

	uint8_t *p1, *p2, *p = &font5x8[(c - ' ') * 5];
	uint32_t n = 0;
	uint32_t i, j, mask;

	// If it's a space, advance 2 pixels
	if (c == ' ')
		return(2);

	// Figure out where the character begins and ends so we can kern it nice and tight
	p1 = NULL;
	for (i = 0; i < 5; i++) {
		if (p[i] != '\0' && p1 == NULL)
			p1 = &p[i];
		if (p[i] != '\0')
			p2 = &p[i];
	}

	// How wide is the character?
	n = p2 - p1 + 1;

	// Draw character
	for (i = 0; i < n; i++) {
		if (p1[i] != 0) {
			mask = 1;
			for (j = 0; j < 8; j++) {
				if ((p1[i] & mask) != 0)
					setPixel(x + i, y + j, color);
				mask <<= 1;
			}
		}
	}

	// Return the character width
	return(n);
}

void Draw5x8String(uint8_t *p, uint32_t x, uint32_t y, uint32_t color) {

	while (*p != '\0') {
		x += (Draw5x8Char(*p, x, y, color) + 1);
		*p++;
	}
}

void Delay() {
	usleep(70000);
}

// angle : 0 - 1536
uint32_t Colors(uint32_t angle) {

	uint32_t t = angle & 255;
	uint32_t r, g, b;

	switch ((angle >> 8) % 6) {
		case 0: r = 255; g = t; b = 0; break;
		case 1: r = 255 - t; g = 255; b = 0; break;
		case 2: r = 0; g = 255; b = t; break;
		case 3: r = 0; g = 255 - t; b = 255; break;
		case 4: r = t; g = 0; b = 255; break;
		case 5: r = 255; g = 0; b = 255 - t; break;
	}
	return RGB(r, g, b);
}

static uint32_t itime = 0;

void ScrollString(uint8_t *s, uint32_t color) {

	uint32_t i, j, k, bcolor, y;

	y = floor((height - 10) / 2);
	uint32_t n = strlen(s) - 1;
	k = width;
	for (i = 0; i < width + (n * 6) + 5; i++) {

		clearScreen();

		// Scrolling color borders
		for (j = 0; j < k; j++) {
			bcolor = Colors((j % k) * (1536 / k));
			setPixel((j + itime) % width, y, bcolor);
			setPixel((j + itime) % width, y + 9, bcolor);
		}

		Draw5x8String(s, width - i, y + 1, color);
		itime++;

		ws2811_render(&ledstring);
		Delay();
	}
}

uint8_t Do5x8String(uint8_t *s, uint32_t color, uint32_t *i, int32_t y) {

	uint32_t j;
	uint32_t bcolor;
	uint32_t k = width;
	int32_t n = strlen(s) - 1;

	if (n <= 0)
		return(TRUE);

	Draw5x8String(s, width - *i, y, color);
	itime++;
	(*i)++;
	if (*i < (width + (n * 6) + 5))
		return(FALSE);
	else
		return(TRUE);
}

uint8_t Do5x5String(uint8_t *s, uint32_t color, uint32_t *i, int32_t y) {

	uint32_t j;
	uint32_t bcolor;
	uint32_t k = width;
	int32_t n = strlen(s) - 1;

	if (n <= 0)
		return(TRUE);

	Draw5x5String(s, width - *i, y, color);
	itime++;
	(*i)++;
	if (*i < (width + (n * 6) + 5))
		return(FALSE);
	else
		return(TRUE);
}

void TimeDateWeather () {

	uint32_t i0 = 0, i1 = 0, count;
	int32_t idur, i, j, y5x8, y5x5;
	FILE *fp;
	uint8_t s0[2046], s1[2046], s2[2046];
	uint8_t sTemperature[2046], sForecast[2046];
	uint8_t sDate[2046], s[2046];
	struct {
		uint32_t fonttype;
		uint32_t pos;
		uint8_t s[300];
		uint32_t color;
	} arr[10];
	char temp[10];

	itime = 0;
	uint32_t bgcolor = BLACK;

	s[0] = '\0';
	s0[0] = '\0';
	s1[0] = '\0';
	s2[0] = '\0';
	sTemperature[0] = '\0';
	sForecast[0] = '\0';

	for (i = 0; i < (sizeof(arr) / sizeof(arr[0])); i++) {
		arr[i].fonttype = 0;
		arr[i].pos = 0;
		arr[i].s[0] = '\0';
		arr[i].color = 0;
	}

	y5x8 = floor((height - 13) / 2);
	y5x5 = y5x8 + 8;

	ws2811_init(&ledstring);

	while(TRUE) {

		for (idur = 0; idur < 390; idur++) {

			// Clear screen
			setScreen(bgcolor);

			// Render strings
			for (i = 0; i < (sizeof(arr) / sizeof(arr[0])); i++) {
				if (arr[i].fonttype == 1) 
					if (Do5x8String(arr[i].s, arr[i].color, &arr[i].pos, y5x8))
						arr[i].fonttype = 0;

				if (arr[i].fonttype == 2) 
					if (Do5x5String(arr[i].s, arr[i].color, &arr[i].pos, y5x5))
						arr[i].fonttype = 0;
			}

			// Need to generate new 5x8 string?
			count = 0;
			for (i = width / 2; i < width; i++)
				for (j = 1; j < 9; j++)
					if (getPixel(i, j) != bgcolor)
						count++;
			if (count == 0) {

				// Generate new 5x8 string
				for (i = 0; arr[i].fonttype != 0; i++);
				
				arr[i].s[0] = '\0';
				arr[i].fonttype = 1;
				arr[i].pos = 1;
				arr[i].color = RGB(255, 255, 255);

				// Temperature
				s[0]= '\0';
				fp = fopen(TEMPERATURE, "r");
				if (fp != NULL) {
					fgets(s, sizeof(s) - 1, fp);
					fclose(fp);
					s[strlen(s) - 1] = '\0';
					if (strstr(s, ".0") != NULL)
						s[strlen(s) - 2] = '\0';
					if (strlen(s) > 0)
						strcat(s, "`F");
					if (strlen(s) > 0)
						strcpy(sTemperature, s);
				}

				// Forecast
				s[0]= '\0';
				fp = fopen(FORECAST, "r");
				if (fp != NULL) {
					fgets(s, sizeof(s) - 1, fp);
					fclose(fp);
					s[strlen(s) - 1] = '\0';
					if (strlen(s) > 0)
						strcpy(sForecast, s);
				}

				strcat(arr[i].s, sTemperature);
				strcat(arr[i].s, "   ");
				strcat(arr[i].s, sForecast);
			}

			// Need to generate new 5x5 string?
			count = 0;
			for (i = width / 2; i < width; i++)
				for (j = 9; j < 15; j++)
					if (getPixel(i, j) != bgcolor)
						count++;
			if (count == 0) {

				// Generate new 5x5 string
				for (i = 0; arr[i].fonttype != 0; i++);
				
				arr[i].s[0] = '\0';
				arr[i].fonttype = 2;
				arr[i].pos = 1;
				arr[i].color = RGB(0, 255, 0);

				// date
				fp = popen("date +'%a, %b %-d, %Y'", "r"); 
				if (fp != NULL) {
					fgets(s0, sizeof(s0) - 1, fp);
					pclose(fp);
					s0[strlen(s0) - 1] = '\0';
					strcat(arr[i].s, s0);
					strcat(arr[i].s, "   ");
				}

				// time
				fp = popen("date +'%-I:%M %P'", "r"); 
				if (fp != NULL) {
					fgets(s0, sizeof(s0) - 1, fp);
					pclose(fp);
					s0[strlen(s0) - 1] = '\0';
					strcat(arr[i].s, s0);
				}
			}

			ws2811_render(&ledstring);
			Delay();
		}

		// holidays
		sDate[0] = '\0';
		fp = popen("date +'%a, %b %-d, %Y'", "r"); 
		if (fp != NULL) {
			if (fgets(sDate, sizeof(sDate) - 1, fp) == NULL)
				sDate[0] = '\0';
			else
				sDate[strlen(sDate) - 1] = '\0';
			pclose(fp);
		}
		if (strlen(sDate) > 0) {
			fp = fopen(HOLIDAYS, "r");
			if (fp != NULL) {
				while (fp != NULL) {
					if (fgets(s, sizeof(s) - 1, fp) == NULL)
						break;
					s[strlen(s) - 1] = '\0';
					sprintf(s1, " %s ", sDate);
					sprintf(s2, " %s ", s);
					if (strstr(s1, s2) != NULL) {
						if (fgets(s, sizeof(s) - 1, fp) == NULL)
							break;
						s[strlen(s) - 1] = '\0';
						DrawShapes(70);
						ScrollString(s, RGB(255, 255, 255));
						DrawShapes(70);
						break;
					}
				}
				fclose(fp);
			}
		}
	}
}

uint32_t GetColor(uint8_t c) {

	switch (c) {
		case 'r':
			return RGB(255, 0, 0); // red
		case 'R':
			if (rand() % 100 > 75)
				return RGB(255, 0, 0); // red
			else
				return RGB(255 / 2, 0, 0); // dark red
		case 'y':
			return RGB(255, 255, 0); // yellow
		case 'Y':
			if (rand() % 100 > 75)
				return RGB(255, 255, 0); // yellow
			else
				return RGB(255 / 2, 255 / 2, 0); // dark yellow
		case 'g':
			return RGB(0, 255, 0); // green
		case 'c':
			return RGB(0, 255, 255); // cyan (blue-green)
		case 'C':
			if (rand() % 100 > 75)
				return RGB(0, 255, 255); // cyan
			else
				return RGB(0, 255 / 2, 255 / 2); // dark cyan
		case 'b':
			return RGB(0, 0, 255); // blue
		case 'B':
			if (rand() % 100 > 75)
				return RGB(0, 0, 255); // blue
			else
				return RGB(0, 0, 255 / 2); // dark blue
		case 'm':
			return RGB(255, 0, 255); // magenta
		case 'w':
			return RGB(255, 255, 255); // white
		case 'I':
			return RGB(255 / 3, 255 / 3, 255); // light blue
		case 'O':
			return RGB(255, 255 / 2, 0); // orange
		case 'p':
			return RGB(255 / 2, 0, 255 / 2); // purple
		case 'P':
			return RGB(255, 0, 255 / 2); // pink
		case 'W':
			if (rand() % 100 > 75)
				return RGB(255 / 2, 255 / 2, 255 / 2); // grey
			else
				return RGB(255, 255, 255); // white
		case '.':
			return RGB(0, 0, 0); // black
	}
	return RGB(0, 0, 0);
}

void RainbowLights() {

	uint32_t i, j, initcolor, color;

	ws2811_init(&ledstring);
	initcolor = 0;
	clearScreen();
	while (TRUE) {
		initcolor = (initcolor + 70) % 1536;
		for (i = 0; i < width; i++) {
			color = Colors((initcolor + (i * (1536 / width) )) % 1536);
			for (j = 0; j < height; j++) 
				setPixel(i, j, color);
		}
		Delay();
		ws2811_render(&ledstring);
	}
}

void DrawShapes(uint32_t duration) {

	#define NSHAPES 7

	uint32_t i, j;
	struct {
		uint8_t type;
		int32_t xcenter;
		int32_t ycenter;
		uint32_t size;
		uint32_t color;
	} arr[NSHAPES];

	// start with no shapes
	for (i = 0; i < NSHAPES; i++)
		arr[i].size = 0;

	clearScreen();
	for (i = 0; i < duration; i++) {

		fadeScreen();

		// create a shape if necessary
		if (i % 4 == 0) {
			for (j = 0; j < NSHAPES; j++) {
				if (arr[j].size == 0) {
					arr[j].type = rand() & 1;
					arr[j].xcenter = rand() % width;
					arr[j].ycenter = rand() % height;
					arr[j].size = 1;
					arr[j].color = Colors(rand() % 1536);
					break;
				}
			}
		}

		// render shapes
		for (j = 0; j < NSHAPES; j++) {
			if (arr[j].size > 0) {
				if (arr[j].type == 0)
					DrawCircle(arr[j].xcenter, arr[j].ycenter, arr[j].size, arr[j].color);
				else
					DrawBox(arr[j].xcenter, arr[j].ycenter, arr[j].size, arr[j].color);

				// delete shape if it's expanded enough
				if (arr[j].size++ == 9)
					arr[j].size = 0;
			}
		}

		ws2811_render(&ledstring);
		Delay();
	}
	clearScreen();
	ws2811_render(&ledstring);
}

void DrawPicture(const uint8_t **picture, uint32_t *x, uint32_t *y, uint32_t xdelta, uint32_t ydelta) {

	uint32_t i, j;

	clearScreen();
	for (i = 0; i < height; i++) {
		uint8_t *p = picture[i];
		for (j = 0; p[j] != '\0'; j++)
			setPixel(*x + j, *y + i, GetColor(p[j]));
	}
	*x += xdelta;
	*y += ydelta;
}

void DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
 
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
	int err = (dx > dy ? dx : -dy) / 2, e2;
 
	for(;;){
		setPixelNoWrap(x0, y0, color);
		if (x0 == x1 && y0 == y1)
			break;
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

void DrawBox(uint32_t x0, uint32_t y0, uint32_t size, uint32_t color) {

	int32_t i, x1, y1, width;
	
	width = size + 1;
	x1 = x0 - (width / 2);
	y1 = y0 - (width / 2);
	for (i = x1; i <= x1 + width; i++) {
		setPixelNoWrap(i, y1, color);
		setPixelNoWrap(i, y1 + width, color);
	}
	for (i = y1; i <= y1 + width; i++) {
		setPixelNoWrap(x1, i, color);
		setPixelNoWrap(x1 + width, i, color);
	}
}

void DrawCircle(uint32_t x0, uint32_t y0, uint32_t size, uint32_t color) {

	int32_t x = size;
	int32_t y = 0;
	int32_t radiusError = 1 - x;

	while (x >= y) {
		setPixelNoWrap(x + x0, y + y0, color);
		setPixelNoWrap(y + x0, x + y0, color);
		setPixelNoWrap(-x + x0, y + y0, color);
		setPixelNoWrap(-y + x0, x + y0, color);
		setPixelNoWrap(-x + x0, -y + y0, color);
		setPixelNoWrap(-y + x0, -x + y0, color);
		setPixelNoWrap(x + x0, -y + y0, color);
		setPixelNoWrap(y + x0, -x + y0, color);
		y++;
		if (radiusError < 0) {
			radiusError += 2 * y + 1;
		} else {
			x--;
			radiusError += 2 * (y - x) + 1;
		}
	}
}

static void kill_handler(uint32_t signum) {

	ws2811_init(&ledstring);
	clearScreen();
	ws2811_render(&ledstring);
	ws2811_fini(&ledstring);
}

static void setup_handlers(void) {

	struct sigaction sa = {
		.sa_handler = kill_handler,
	};

	sigaction(SIGUSR1, &sa, NULL);
}

int16_t CharCount(const uint8_t *str, uint8_t character) {

    const uint8_t *p = str;
    int16_t count = 0;

    do {
        if (*p == character)
            count++;
    } while (*(p++));

    return count;
}

// Read LED placement map
void ReadMap() {

	int16_t lines_allocated = 128;
	int16_t max_line_len = 400;
	uint8_t **words = (uint8_t **) malloc(sizeof(uint8_t*) *lines_allocated);
	int16_t i, j, k, maxlength = 0, xchannel0 = -1, xchannel1 = -1, ychannel0 = -1, ychannel1 = -1;
	int16_t xcurr, ycurr, xpos, ypos;
	uint8_t ccurr;

	if (words == NULL) {
		fprintf(stderr,"Out of memory.\n");
		exit(1);
        }

	FILE *fp = fopen(MAP, "r");
	if (fp == NULL) {
		fprintf(stderr, "Error opening map file.\n");
		exit(1);
	}

	// Read map.txt file
	for (i = 0; TRUE; i++) {
		int16_t j;
		if (i >= lines_allocated) {
			int16_t new_size;
			new_size = lines_allocated * 2;
			words = (uint8_t **) realloc(words, sizeof(uint8_t*) *new_size);
			if (words == NULL) {
				fprintf(stderr,"Out of memory.\n");
				exit(1);
			}
			lines_allocated = new_size;
		}
		words[i] = malloc(max_line_len);
		if (words[i] == NULL) {
			fprintf(stderr,"Out of memory.\n");
			exit(1);
		}

		// Quit on EOF
		if (fgets(words[i], max_line_len - 1, fp) == NULL)
			break;

		// Strip line terminators
		for (j = strlen(words[i]) - 1; j >= 0 && (words[i][j] == '\n' || words[i][j] == '\r'); j--);
		words[i][j + 1] = '\0';


		// Get LED display width, height, max line size
		k = CharCount(words[i], 'o') + CharCount(words[i], '.');
		if (k > width)
			width = k;
		if (k > 0)
			height++;
		if (j > maxlength)
			maxlength = j;

		// Beginning of channel 0?
		if (CharCount(words[i], '0') > 0) {
			xchannel0 = (uint8_t*) strchr(words[i], '0') - &words[i][0];
			ychannel0 = i;
		}

		// Beginning of channel 1?
		if (CharCount(words[i], '1') > 0) {
			xchannel1 = (uint8_t*) strchr(words[i], '1') - &words[i][0];
			ychannel1 = i;
		}
	}
	fclose(fp);

	maxlength++;
	
	for (j = 0; j < i; j++)
		while (strlen(words[j]) <= maxlength)
			strcat(words[j], " ");

	ledmap = (int16_t*) malloc(width * height * sizeof(int16_t));
	for (j = 0; j < width * height; j++)
		ledmap[j] = -1;

	// Trace out channel 0
	xcurr = xchannel0;
	ycurr = ychannel0;
	k = 0;
	while (TRUE) {
		ccurr = words[ycurr][xcurr];
		words[ycurr][xcurr] = ' ';
		if (ccurr == 'o' || ccurr == '0' || ccurr == '1') {
			xpos = width - (floor(xcurr / 3) + 1);
			ypos = floor(ycurr / 2);
			ledmap[ypos * width + xpos] = k * 2;
			k++;
		}
		if (xcurr > 0 && (words[ycurr][xcurr - 1] != ' '))
			xcurr--;
		else if (xcurr < maxlength && words[ycurr][xcurr + 1] != ' ')
			xcurr++;
		else if (ycurr > 0 && words[ycurr - 1][xcurr] != ' ')
			ycurr--;
		else if (ycurr < i && words[ycurr + 1][xcurr] != ' ')
			ycurr++;
		else
			break;
	}

	// Init channel 0
	if (k > 0) {
		ledstring.channel[0].gpionum = GPIO_PIN0;
		ledstring.channel[0].brightness = INT;
		ledstring.channel[0].count = k;
	}

	// Trace out channel 1
	xcurr = xchannel1;
	ycurr = ychannel1;
	k = 0;
	while (TRUE) {
		ccurr = words[ycurr][xcurr];
		words[ycurr][xcurr] = ' ';
		if (ccurr == 'o' || ccurr == '0' || ccurr == '1') {
			xpos = width - (floor(xcurr / 3) + 1);
			ypos = floor(ycurr / 2);
			ledmap[ypos * width + xpos] = k * 2 + 1;
			k++;
		}
		if (xcurr > 0 && (words[ycurr][xcurr - 1] != ' '))
			xcurr--;
		else if (xcurr < maxlength && words[ycurr][xcurr + 1] != ' ')
			xcurr++;
		else if (ycurr > 0 && words[ycurr - 1][xcurr] != ' ')
			ycurr--;
		else if (ycurr < i && words[ycurr + 1][xcurr] != ' ')
			ycurr++;
		else
			break;
	}

	// Init channel 1
	if (k > 0) {
		ledstring.channel[1].gpionum = GPIO_PIN1;
		ledstring.channel[1].brightness = INT;
		ledstring.channel[1].count = k;
	}
}

void RenderSprite(uint8_t *filename, int32_t xpos, int32_t ypos, int32_t width) {

	uint8_t mbyte[3], byte[3];
	uint32_t color;
	int32_t x, y;
	FILE *fp = fopen(filename, "rb");

	if (!fp) {
		printf("Unable to open file %s\n", filename);
		exit(1);
	}
	x = 0;
	y = 0;
	while(fread(byte, sizeof(byte[0]), 3, fp) != NULL) {
		color = RGB(byte[0], byte[1], byte[2]);
		setPixel(xpos + x, ypos + y, color);
		x++;
		if (x >= width) {
			x = 0;
			y++;
		}
	}
	fclose(fp);
}

void RenderFile(uint8_t *filename) {

	uint8_t mbyte[3], byte[3];
	uint32_t x, y, mcolor, color;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Unable to open file %s\n", filename);
		exit(1);
	}
	clearScreen();
	x = 0;
	y = 0;
	while(fread(byte, sizeof(byte[0]), 3, fp) != NULL) {
		color = RGB(byte[0], byte[1], byte[2]);
		setPixel(x++, y, color);
		if (x >= width) {
			x = 0;
			y++;
			if (y >= height)
				y = 0;
		}
	}
	fclose(fp);
}

int FileExists(uint8_t *filename) {

	if (access(filename, F_OK) == -1)
		return FALSE;
	else
		return TRUE;
}

void PlayFrames() {
	
	while (TRUE) {
		uint8_t bExists;
		uint8_t filename[512];
		int i = 0;
		do {
			i++;
			sprintf(filename, "/home/pi/rpilights/frames/frame%03d.gif.rgb", i);
			bExists = FileExists(filename);
			if (bExists) {
				RenderFile(filename);
				ws2811_render(&ledstring);
				Delay();
			}
		} while (bExists);
	}
}

void StopService() {

	pid_t mypid;

	FILE *fp = fopen(PIDFILE, "r");
	if (fp != NULL) {
		fscanf(fp, "%d\n", &mypid);
		printf("Stopped service, PID %d\n", mypid);
		fclose(fp);
		unlink(PIDFILE);
		kill(mypid, SIGUSR1);
		Delay();
	}
}

void RootCheck() {

	if (access("/dev/mem", R_OK) == -1) {
		printf("You need to be root to perform this command.\n");
		exit(1);
	}
}

void StartService(void (*procedure)(void)) {

	pid_t mypid;

	RootCheck();
	StopService();
	mypid = fork();
	if (mypid == 0) {
		procedure();
	} else {
		FILE *fp = fopen(PIDFILE, "w");
		fprintf(fp, "%d\n", mypid);
		fclose(fp);
		printf("Started service, PID %d\n", mypid);
	}
	exit(0);
}

void SetAllLights(uint32_t color) {

	RootCheck();
	StopService();
	ws2811_init(&ledstring);
	setScreen(color);
	ws2811_render(&ledstring);
	ws2811_fini(&ledstring);
	exit(0);
}

void ShowFrame() {

	RootCheck();
	StopService();
	ws2811_init(&ledstring);
	RenderFile("/home/pi/rpilights/lights.gif.rgb");
	ws2811_render(&ledstring);
	ws2811_fini(&ledstring);
	exit(0);
}

void ShowIP() {

	uint8_t s[500];
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	strcpy(s, inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));

	ws2811_init(&ledstring);
	while (TRUE)
		ScrollString(s, Colors(rand() % 1536));
}

void Pacman() {

	int32_t i, y, k = width;

	ws2811_init(&ledstring);
	y = floor((height - 10) / 2);

	while (TRUE) {

		// dots
		setScreen(BLACK);
		for (i = 2; i < width; i += 4) {
			setPixel(i, y + 4, WHITE);
			setPixel(i, y + 5, WHITE);
			setPixel(i + 1, y + 4, WHITE);
			setPixel(i + 1, y + 5, WHITE);
		}
		ws2811_render(&ledstring);

		for (i = k; i > -k; i--) {
			if (i & 1)
				RenderSprite("/home/pi/rpilights/pacman/pacman1.gif.rgb", i, y, 47);
			else
				RenderSprite("/home/pi/rpilights/pacman/pacman2.gif.rgb", i, y, 47);
			ws2811_render(&ledstring);
			Delay();
		}

		// dots
		setScreen(BLACK);
		for (i = 2; i < width; i += 4) {
			setPixel(i, y + 4, WHITE);
			setPixel(i, y + 5, WHITE);
			setPixel(i + 1, y + 4, WHITE);
			setPixel(i + 1, y + 5, WHITE);
		}
		ws2811_render(&ledstring);

		for (i = -k; i < k; i++) {
			if (i & 1)
				RenderSprite("/home/pi/rpilights/pacman/pacman3.gif.rgb", i, y, 47);
			else
				RenderSprite("/home/pi/rpilights/pacman/pacman4.gif.rgb", i, y, 47);
			ws2811_render(&ledstring);
			Delay();
		}
	}
}

int main(int argc, char *argv[]) {

	srand(time(NULL));
	setup_handlers();

	// Read LED mapping from map.txt file
	ReadMap();

	if (argc == 2) {

		// LIGHTS ON
		if (strcmp(argv[1], "on") == 0)
			StartService(TimeDateWeather);

		// LIGHTS OFF
		if (strcmp(argv[1], "off") == 0)
			SetAllLights(BLACK);

		// LIGHTS STATUS
		if (strcmp(argv[1], "status") == 0) {
			if (FileExists(PIDFILE))
				printf("Lights are on.\n");
			else
				printf("Lights are off.\n");
			exit(0);
		}

		// LIGHTS RED
		if (strcmp(argv[1], "red") == 0)
			SetAllLights(RED);

		// LIGHTS GREEN
		if (strcmp(argv[1], "green") == 0)
			SetAllLights(GREEN);

		// LIGHTS BLUE
		if (strcmp(argv[1], "blue") == 0)
			SetAllLights(BLUE);

		// LIGHTS MAGENTA
		if (strcmp(argv[1], "magenta") == 0)
			SetAllLights(MAGENTA);

		// LIGHTS CYAN
		if (strcmp(argv[1], "cyan") == 0)
			SetAllLights(CYAN);

		// LIGHTS RAINBOW
		if (strcmp(argv[1], "rainbow") == 0)
			StartService(RainbowLights);

		// LIGHTS IP
		if (strcmp(argv[1], "ip") == 0)
			StartService(ShowIP);

		// LIGHTS PACMAN
		if (strcmp(argv[1], "pacman") == 0)
			StartService(Pacman);
	}

	// USAGE
	printf("Usage:");
	printf("\trpilights on\t\t\tTurn lights on\n");
	printf("\trpilights timedateweather\tDisplay scrolling time, date, weather\n");
	printf("\trpilights off\t\t\tTurn lights off\n");
	printf("\trpilights red\t\t\tSet all lights to red\n");
	printf("\trpilights blue\t\t\tSet all lights to blue\n");
	printf("\trpilights green\t\t\tSet all lights to green\n");
	printf("\trpilights magenta\t\tSet all lights to magenta\n");
	printf("\trpilights cyan\t\t\tSet all lights to cyan\n");
	printf("\trpilights rainbow\t\tDisplay scrolling rainbow pattern\n");
	printf("\trpilights ip\t\t\tDisplay scrolling IP address\n");
	printf("\trpilights pacman\t\tDisplay Pacman animation\n");
	exit(0);
}
