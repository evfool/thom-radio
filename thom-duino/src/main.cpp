#include <Arduino.h>

/********************
Robert Roth robert.roth.off@gmail.com

Based on Rui Azevedo's ArduinoMenu Adafruit GFX example
output: Nokia 5110 display (PCD8544 HW SPI)
input: Encoder with switch + Serial + switch

***/

#include <SPI.h>
#include <menuIO/encoderIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/adafruitGfxOut.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

#include "config.h"

using namespace Menu;

Adafruit_PCD8544 gfx(GFX_DC,GFX_CS,GFX_RST);


// time of last button press for debounce
long lastPress = 0;

// status variables
int volume = 60, oldVolume = volume;
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
char nowPlaying[120] = "not playing";


#define STEPS 4

ClickEncoder clickEnc(encA, encB, encBtn, STEPS);


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
  //,FIELD(volume,"Volume","%",0,100,5,1,changeVolume,enterEvent,noStyle)
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

encoderIn<encA,encB> encoder;//simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder,4);// simple quad encoder fake Stream

//a keyboard with only one key as the encoder button
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}, {-escBtn, defaultNavCodes[escCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
keyIn<2> encButton(encBtn_map);//2 is the number of keys

//serialIn serial(Serial);
MENU_INPUTS(in,&encStream,&encButton);//,&serial);

#define CHARS_X gfxWidth/fontX
#define LINES_Y gfxHeight/fontY
#define MAX_DEPTH 2
#define textScale 1
MENU_OUTPUTS(out,MAX_DEPTH
  ,ADAGFX_OUT(gfx,colors,fontX,fontY,{0,0,CHARS_X,LINES_Y})
  ,NONE//,SERIAL_OUT(Serial)
);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

void timerIsr() {
  clickEnc.service();
}

void setMenuMode(bool yes) {
  menuMode = yes;
  if (yes)
    Timer1.detachInterrupt();
  else
    Timer1.attachInterrupt(timerIsr);
}
//when menu is suspended
result idle(menuOut& o,idleEvent e) {

  if (e == idleEnd){
    setMenuMode(true);
    return proceed;
  }

  setMenuMode(false);
  o.setCursor(0,0);
  if (playing)
    o.print(F("Playing... "));
  else
    o.print(F("Paused...  "));
  o.print(volume);
  o.setCursor(0,1);
  //Serial.println(nowPlaying);
  o.print(nowPlaying);
  return proceed;
}

bool isTooShort() {
  long now = millis();
  long previous = lastPress;
  lastPress = now;
  return (now - previous <= DEBOUNCE_DELAY);
}

// play/pause btn (escBtn)
void primaryButtonHandler() {
  if (isTooShort()) return;
  if (!menuMode) {
    playing = !playing;
    //nav.idleOff();
    nav.idleOn(nav.idleTask);
    playToggled();
  }
}

// menu button (enterBtn) - switch of rotary encoder
void secondaryButtonHandler() {
  //if (isTooShort()) return;
  setMenuMode(true);
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

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);


  clickEnc.setAccelerationEnabled(true);

}

void handleSerialCommand(String buffer) {
  /*Serial.print("You entered >");
  Serial.print(buffer);
  Serial.println("<");*/
  if (buffer.startsWith("volume:")) {
    oldVolume = volume;
    volume = buffer.substring(buffer.indexOf(':')+1).toInt();
    nav.idleOn(nav.idleTask);
  } else if (buffer.startsWith("playing:")) {
    String current = buffer.substring(buffer.indexOf(':')+1);
    current.toCharArray(nowPlaying, min(current.length()+1,80));
    nav.idleOn(nav.idleTask);
  }
}

void loop() {
  if (Serial.available()) {
    handleSerialCommand(Serial.readStringUntil('\r'));
    if (Serial.peek() == '\n') Serial.read();
  }

  // only do input handling in case we're in the menu
  // in idle mode input is handled with interrupts
  if (!menuMode) {

    // update volume if encoder turned while idle
    volume += 5 * clickEnc.getValue();
    volume = constrain(volume, 0, 100);
    if (volume != oldVolume) {
      changeVolume();
      oldVolume = volume;

      //nav.idleOff();
      nav.idleOn(nav.idleTask);
    }
  } else {
    nav.doInput();
  }
  if (nav.changed(0)) {//only draw if changed

    nav.doOutput();
    gfx.display();
  }


  digitalWrite(BACKLIGHTPIN, (stealth  ? HIGH : backlightCtrl));
  delay(100);//simulate a delay when other tasks are done

}
