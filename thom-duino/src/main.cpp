#include <Arduino.h>

/********************
Robert Roth robert.roth.off@gmail.com

Based on Rui Azevedo's ArduinoMenu Adafruit GFX example
output: Nokia 5110 display (PCD8544 HW SPI)
input: Encoder with switch + Serial + switch

***/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <menu.h>
#include <menuIO/encoderIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/adafruitGfxOut.h>
#include <menuIO/serialIn.h>

using namespace Menu;

//PCD8544 aka nokia5110
#define GFX_DC  5
#define GFX_CS  4
#define GFX_RST 10

Adafruit_PCD8544 gfx(GFX_DC,GFX_CS,GFX_RST);

#define BACKLIGHTPIN 7
#define CONTRAST 60

// rotary encoder pins
#define encA    8
#define encB    9
#define encBtn  3

// the back button
#define escBtn  2

// the modes to be supported
#define FAVOURITE_RADIO 0
#define VIDSTREAM 1
#define MUSIC 2

// buttons software debounce delay
#define DEBOUNCE_DELAY 200
#define BOOLEAN_TOGGLE(name,variable,label) result name() {Serial.print(label); Serial.println(variable?"on":"off");return proceed;}
// time of last button press for debounce
long lastPress = 0;

// status variables
int volume=60;
int backlightCtrl=LOW;
int modeCtrl = FAVOURITE_RADIO; // current mode
bool playing = false; // paused or playing
bool shuffle = false; // shuffle music
BOOLEAN_TOGGLE(shuffleToggled, shuffle, "shuffle:");
bool repeat = true; // to repeat or not to repeat
BOOLEAN_TOGGLE(repeatToggled, repeat, "repeat:")
bool stealth = false; // to turn of all leds
BOOLEAN_TOGGLE(stealthToggled, stealth, "stealth:")
bool menuMode = false; // whether we're in the menu or not

// callback for the volume changes
result changeVolume() {
  Serial.print("volume:");
  Serial.println(volume);
  return proceed;
}

// play/pause toggle callback
result playToggled() {
  if (playing)
    Serial.println("play");
  else
    Serial.println("pause");
  return proceed;
}

result jumpNext() {
  Serial.println("next");
  return proceed;
}

result jumpPrevious() {
  Serial.println("previous");
  return proceed;
}

TOGGLE(backlightCtrl,setBacklight,"Light     : ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("x",LOW,doNothing,noEvent)
  ,VALUE("o",HIGH,doNothing,noEvent)
);

TOGGLE(modeCtrl,setMode,"Mode: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("Radio",FAVOURITE_RADIO,doNothing,noEvent)
  ,VALUE("Youtube",VIDSTREAM,doNothing,noEvent)
  ,VALUE("Music", MUSIC, doNothing, noEvent)
);

TOGGLE(playing,playToggle,"", playToggled, enterEvent, noStyle
  ,VALUE("Play", false, doNothing,noEvent)
  ,VALUE("Pause", true, doNothing, noEvent)

);

TOGGLE(shuffle,shuffleToggle,"Shuffle   : ", shuffleToggled, enterEvent, noStyle
  ,VALUE("x", true, doNothing,noEvent)
  ,VALUE("o", false, doNothing, noEvent)
);

TOGGLE(repeat,repeatToggle,"Repeat    : ", repeatToggled, enterEvent, noStyle
  ,VALUE("x", true, doNothing,noEvent)
  ,VALUE("o", false, doNothing, noEvent)
);

TOGGLE(stealth,stealthToggle,"Stealth   : ", stealthToggled, enterEvent, noStyle
  ,VALUE("o", false, doNothing, noEvent)
  ,VALUE("x", true, doNothing,noEvent)
);

MENU(musicMenu, "Music      +", doNothing, noEvent, wrapStyle
  ,SUBMENU(playToggle)
  ,OP("Previous", jumpPrevious, enterEvent)
  ,OP("Next", jumpNext, enterEvent)
  ,EXIT("<Back")
);

MENU(optionsMenu, "Options    +", doNothing, noEvent, wrapStyle
  ,FIELD(volume,"Volume","%",0,100,5,1,changeVolume,enterEvent,noStyle)
  ,SUBMENU(setBacklight)
  ,SUBMENU(shuffleToggle)
  ,SUBMENU(repeatToggle)
  ,SUBMENU(stealthToggle)
  ,EXIT("<Back")
);

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle

  ,SUBMENU(musicMenu)
  ,SUBMENU(optionsMenu)
  ,SUBMENU(setMode)
  ,EXIT("<Back")
);

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
#define MAX_DEPTH 3

encoderIn<encA,encB> encoder;//simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder,4);// simple quad encoder fake Stream

//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}, {-escBtn, defaultNavCodes[escCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
keyIn<2> encButton(encBtn_map);//2 is the number of keys

//serialIn serial(Serial);
MENU_INPUTS(in,&encStream,&encButton);//,&serial);

#define MAX_DEPTH 2
#define textScale 1
MENU_OUTPUTS(out,MAX_DEPTH
  ,ADAGFX_OUT(gfx,colors,fontX,fontY,{0,0,gfxWidth/fontX,gfxHeight/fontY})
  ,NONE//,SERIAL_OUT(Serial)
);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

//initializing output and menu nav without macros
/*const panel default_serial_panels[] MEMMODE={{0,0,40,10}};
navNode* default_serial_nodes[sizeof(default_serial_panels)/sizeof(panel)];
panelsList default_serial_panel_list(
  default_serial_panels,
  default_serial_nodes,
  sizeof(default_serial_panels)/sizeof(panel)
);

//define output device
idx_t serialTops[MAX_DEPTH]={0};
serialOut outSerial(*(Print*)&Serial,serialTops);

//define outputs controller
idx_t gfx_tops[MAX_DEPTH];
PANELS(gfxPanels,{0,0,gfxWidth/fontX,gfxHeight/fontY});
adaGfxOut adaOut(gfx,colors,gfx_tops,gfxPanels);

menuOut* const outputs[] MEMMODE={&outSerial,&adaOut};//list of output devices
outputsList out(outputs,2);//outputs list controller

//define input device
serialIn serial(Serial);

//define navigation root and aux objects
navNode nav_cursors[MAX_DEPTH];//aux objects to control each level of navigation
navRoot nav(mainMenu, nav_cursors, MAX_DEPTH, serial, out);*/

//when menu is suspended
result idle(menuOut& o,idleEvent e) {
  menuMode = false;
  o.setCursor(0,0);
  if (playing)
    o.print(F("Playing..."));
  else
    o.print(F("Paused..."));
  o.setCursor(0,1);
  o.print(F("press [select]"));
  o.setCursor(0,2);
  o.print(F("to continue"));
  return proceed;
}

void primaryButtonHandler() {
  long now = millis();
  if (now - lastPress <= DEBOUNCE_DELAY) {
    lastPress = now;
    return;
  }
  lastPress = now;
  if (!menuMode) {
    playing = !playing;
    nav.idleOff();
    nav.idleOn(nav.idleTask);
    playToggled();
  }
}

void secondaryButtonHandler() {
  long now = millis();
  if (now - lastPress <= DEBOUNCE_DELAY) {
    lastPress = now;
    return;
  }
  lastPress = now;
  menuMode = true;
  nav.idleOff();
}

void setup() {
  pinMode(BACKLIGHTPIN,OUTPUT);
  pinMode(escBtn,INPUT_PULLUP);
  pinMode(encBtn,INPUT_PULLUP);
  Serial.begin(115200);
  while(!Serial);
  Serial.println(F("menu"));
  Serial.flush();
  nav.idleTask=idle;//point a function to be used when menu is suspended

  encButton.begin();
  encoder.begin();

  SPI.begin();
  gfx.begin();
  gfx.clearDisplay();
  gfx.println(F("Thom-Radio"));
  gfx.setContrast(CONTRAST);
  gfx.display(); // show splashscreen
  delay(2000);
  gfx.clearDisplay();
  gfx.display(); // show splashscreen
  // gfx.drawRect(0, 0, 84, 48, 1);
  nav.idleOn(nav.idleTask);
  // additional interrupt-based handling for buttons on idle screen
  attachInterrupt(digitalPinToInterrupt(escBtn), primaryButtonHandler, FALLING);
  attachInterrupt(digitalPinToInterrupt(encBtn), secondaryButtonHandler, FALLING);
}



void loop() {
  if (menuMode) {
    // only do input handling in case we're in the menu
    nav.doInput();
    // in idle mode input is handled with interrupts
  }
  if (nav.changed(0)) {//only draw if changed
    nav.doOutput();
    gfx.display();
  }

  menuMode = (nav.sleepTask == NULL);
  digitalWrite(BACKLIGHTPIN, (stealth  ? HIGH : backlightCtrl));
  delay(100);//simulate a delay when other tasks are done
}
