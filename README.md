# arduino-halloween-costume
Arduino "Robot" Halloween Costume with NeoPixel LED, buttons, and sound effects.  21 animations and a very special  self-destruct sequence designed by my son. NEW!!! Eagles Mode for the SUPERBOWL 2-12-203

[![Image](https://raw.githubusercontent.com/davetorok/arduino-halloween-costume/master/images/20161022-1720-49_800.jpg)](https://raw.githubusercontent.com/davetorok/arduino-halloween-costume/master/images/20161022-1720-49_800.jpg)

## Overview
The costume was designed primarily around a 24-NeoPixel ring and two 8-NeoPixel strips.  A greeting card speaker was added for sound effects, and a 5 pushbutton module was added for controls.

There are 15 animation patterns for the main pixel ring and 6 for the side strips.  The sound effects include random "robot" tones, and various pitch up/down effects, and explosion / random noise.  By default the animation patterns change every 20 seconds.

The button controls let you change and hold the animation sequnce, generate several sound effects, go into "flashlight mode" to assist with halloween candy inspection, and adjust overall brightness.

## Self Destruct Sequence

Center Ring LEDs will start to Power Up... All Green through 50%, Yellow through 75%, Red through 100%.  Side LED strips will alternately flash red slowly, getting faster as the sequence continues.  Sound will increase in pitch. At 100%, LED Ring will turn full white and slowly fade to back, side LEDs will go through a slow color hue change, and speaker will do "explosion" sound for several seconds.

## Button Functions
* Button 1 (BIG RED BUTTON) - Self-Destruct Sequence
 * Short Press - SELF DESTRUCT SEQUENCE
 * Long Press - HOLD to TOGGLE EAGLES MODE!!!!  ALL COLORS ARE GREEN and a very special "Fly Eagles Fly" for Button 4 short press
* Button 2
 * Short Press - Go to next animation sequence (ring and side strips)
 * Long Press - HOLD animation sequence (ring and side strips)
* Button 3
 * Short Press - Cycle through Brightness Levels (20%,40%,60%,80%,100%)
 * Long Press - FLASHLIGHT MODE (Full 100% bright).  Each Long Press Cycles through (Ring only, Side Strips Only, All On, Off)
* Button 4
 * Short Press - Play "La Cucaracha" on speaker ("Fly Eagles Fly" in Eagles Mode)
 * Long Press - Play one of random sound effects (beeps, alarm sound, sound up/down, noise)
* Button 5
 * Short Press - Go to next Side Strip animation sequence
 * Long Press - HOLD Side Strip Animation Sequence (Long Press TOGGLES HOLD ON/OFF)

## Cool Code Things
* Detect Button Library keeps track of button debounce, short press / long press, and release after long press.  Internally it uses arrays to store state changes, but the output button "event" is a single byte you can mask to get the event type and button number
* Animations are based on 'animation frame' which is currently set to trigger every 10ms.  Some animations are stateless and can key off of the global animation frame number.  Others save some LED state, or need to be slowed down by refreshing every 'n'th call.

## Materials
* Arduino Uno R3
* NeoPixel 24-LED ring
* NeoPixel 8-LED strip (x 2)
* Greeting Card Speaker, 16 Ohm
* 100 Ohm Resistor (for speaker)
* 470 Ohm Resistors (x 3) (for data line NeoPixel)
* 1000 uF Capacitors (x 2) (for NeoPixel power protection)
* 5 Pushbuttons (Normally Open)
* Cardboard Box, painted silver
* Power Supply - 5V USB power pack (2800 maH / 1000ma)
* Various USB cables, barrel connectors, etc.
* Human Child


[![Image](https://raw.githubusercontent.com/davetorok/arduino-halloween-costume/master/images/20161023-1402-32_800.jpg)](https://raw.githubusercontent.com/davetorok/arduino-halloween-costume/master/images/20161023-1402-32_800.jpg)
