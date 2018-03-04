#include <menu.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <menuIO/adafruitGfxOut.h>

//PCD8544 aka nokia5110
#define GFX_DC  5
#define GFX_CS  4
#define GFX_RST 10
#define BACKLIGHTPIN 7

#define CONTRAST 100

// rotary encoder pins
#define encA    8
#define encB    9
#define encBtn  3

// the back button
#define escBtn  2

#define DEBOUNCE_DELAY 300

// define menu colors --------------------------------------------------------
//  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
//monochromatic color table
const colorDef<uint16_t> colors[] MEMMODE={
  {{WHITE,BLACK},{WHITE,BLACK,BLACK}},//bgColor
  {{BLACK,WHITE},{BLACK,WHITE,WHITE}},//fgColor
  {{BLACK,WHITE},{BLACK,WHITE,WHITE}},//valColor
  {{BLACK,WHITE},{BLACK,WHITE,WHITE}},//unitColor
  {{BLACK,WHITE},{WHITE,WHITE,WHITE}},//cursorColor
  {{BLACK,WHITE},{WHITE,BLACK,BLACK}},//titleColor
};

#define gfxWidth 84
#define gfxHeight 48
#define fontX 6
//5
#define fontY 9

// the modes to be supported
#define FAVOURITE_RADIO 0
#define VIDSTREAM 1
#define MUSIC 2

#define BOOLEAN_TOGGLE(name,variable,label) result name() {Serial.print(label); Serial.println(variable?"on":"off");return proceed;}
