# Simple internet radio build

An internet radio build using the following software/hardware components:
* [Raspberry PI model B](https://www.seeedstudio.com/Raspberry-Pi-Model-B-p-1634.html) as the "brain"
* [DietPi](http://dietpi.com/) as the OS running on the pi
* [mpd](https://www.musicpd.org/) as the music player
* [push button](https://www.adafruit.com/product/1119) to interact with the player

The project will be accompanied by step-by-step build instructions, starting from the easy bare-bone internet radio to customizing both hardware and firmware.

Ultimate goal is to have it enclosed in a [Thomson TG784n router](https://images.duckduckgo.com/iu/?u=http%3A%2F%2Fwww.avanzada7.com%2Fen%2Fimagen-producto%2F1-tg784n.jpg&f=1) as a radio case, as unfortunately I couldn't find a way to flash OpenWRT or DD-WRT to that router, although it would've made the whole thing probably a lot easier.

#### Build 1 - basic software setup
1. prepare sd card with DietPi
1. install DietPi with
  * mpd for music playback
  * mpc for mpd control
  * alsamixer for volume control
  * wiringpi library for GPIO input/output
1. add an internet radio station using mpc add ....
1. play using mpc play

#### Build 2 - basic interaction using a pushbutton
1. wire toggle button to pin 6 (GND) and pin 12 (GPIO18) of the rPi
1. create controller python script to start/stop playback
1. create systemd service to autostart controller script
