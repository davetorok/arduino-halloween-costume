#include <FastLED.h>


/*
HALLOWEEN COSTUME
- 3 NeoPixel Strips (24 RING, 8 strip left, 8 strip right
- 16 Ohm greeting card speaker with 100 Ohm resistor
- 5 pushbuttons
 */
#include "pitches.h"

// NeoPixel PINS
#define PIN        9
#define PIN_L      10
#define PIN_R     11

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      24
#define NUMPIXELS_L     8
#define NUMPIXELS_R     8

#define TONEPIN 8

#define BUTTON1PIN  2
#define BUTTON2PIN  3
#define BUTTON3PIN  4
#define BUTTON4PIN  5
#define BUTTON5PIN  6


#define BUTTONON  0x100 //pressed
#define BUTTONOFF  0x200 //released
#define BUTTONLONG  0x400 //long press
#define BUTTONLONGOFF 0x800 // release after long press
#define BUTTONNUMBERMASK  0xFF


// set up FASTLED library
CRGB pixels[NUMPIXELS];
CRGB pixels_l[NUMPIXELS_L];
CRGB pixels_r[NUMPIXELS_R];



/* BUTTON CONFIG DELAY */
const int bounceDelay = 20; // delay to detect debounce
const int longpressDelay = 1200; // interval of button press to detect long press
int sensorVals[5]; //current reading of buttons
int previousButtonVals[5] = {HIGH,HIGH,HIGH,HIGH,HIGH}; //previous temporary reading
int buttonState[5] = {HIGH,HIGH,HIGH,HIGH,HIGH}; // debounced BUTTON STATE
int buttonLongState[5] = {0,0,0,0,0}; //is button in a long press state?
long lastDebounceTime = 0; // start time to measure debounce
long longpressTime = 0; //start time to measure long presses
unsigned int buttonEvent = 0; // GLOBAL when there is an event.  Button number will be lower bits.  event will be BUTTONLONG/ON/OFF


/* LED ANIMATION */
const long patternChangeDelay = 20000; //20 seconds between pattern change
const int delayval = 20; // delay for each animation frame
uint8_t brightness = 105; // overall brightness

int ringmode = 0; //which ring animation are we on
const int ringpatterns = 15; //how many patterns do we have
int stripmode = 0; //which side strip animation are we on
const int strippatterns = 6; // how many patterns do we have

long animationframe = 0; //global counter for animation frames
long lastFrameTime = 0; //time since last animation frame change
long lastPatternChangeTime = 0; //time since last pattern change

int flashlight_mode = 0; // flashlight mode = super bright. 0 = off 1 = ring only; 2 = strips only  3 = all
int pattern_hold = 0; // 0 = Auto Advance 1 = HOLD pattern
int strip_pattern_hold = 0; // 0 = Auto Advance 1 = HOLD pattern

const int randomdim_numleds = 16;
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int melody_lc[] = {
  NOTE_C4, NOTE_C4, NOTE_C4, NOTE_F4, NOTE_A4, 0, 
  NOTE_C4, NOTE_C4, NOTE_C4,NOTE_F4,NOTE_A4,0,
  NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4 };

int noteDurations_lc[] = {
8,8,8,2,4,8,
8,8,8,2,4,2,
4,8,8,8,8,8,2 };


// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};


void setup() {
  FastLED.addLeds<NEOPIXEL, PIN>(pixels, NUMPIXELS);
  FastLED.addLeds<NEOPIXEL, PIN_L>(pixels_l, NUMPIXELS_L);
  FastLED.addLeds<NEOPIXEL, PIN_R>(pixels_r, NUMPIXELS_R);

  // limit my draw to 1A at 5v of power draw
  FastLED.setMaxPowerInVoltsAndMilliamps(5,1000); 
  FastLED.setBrightness(brightness);
  //start serial connection
  //Serial.begin(9600);
  //configure button pins as an input and enable the internal pull-up resistor
  pinMode(BUTTON1PIN, INPUT_PULLUP);
  pinMode(BUTTON2PIN, INPUT_PULLUP);
  pinMode(BUTTON3PIN, INPUT_PULLUP);
  pinMode(BUTTON4PIN, INPUT_PULLUP);
  pinMode(BUTTON5PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  buttonEvent = 0; // set button event


  lastFrameTime = millis();
  lastPatternChangeTime = millis();
}


void loop() {

 
  //check the button
  checkButton();

  //set board Led if we have a button event
  if (buttonEvent) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  //handle button return value
  if (buttonEvent)
  {
    ////Serial.print("   BUTTON EVENT IS ");
    /////  Serial.println(buttonEvent);
    //Button 1 = Self Destruct
    if (((buttonEvent & BUTTONNUMBERMASK) == 1) && (buttonEvent & BUTTONON))
    {
      selfDestructSequence();
    }
    //Button 2 = Change Pattern; RELEASE hold if enabled
    if (((buttonEvent & BUTTONNUMBERMASK) == 2) && (buttonEvent & BUTTONOFF))
    {
      // clear pattern hold
      if (pattern_hold) { pattern_hold = 0; }

      //change patterns
      change_patterns();
      
      //reset pattern change time
      lastPatternChangeTime = millis(); 
    }
    //Button 2 LONG = Hold Pattern
    if (((buttonEvent & BUTTONNUMBERMASK) == 2) && (buttonEvent & BUTTONLONG))
    {
     quick_tone();
     pattern_hold = 1;
     
    }
    //Button 3 = Brightness change / Flashlight Off
    if (((buttonEvent & BUTTONNUMBERMASK) == 3) && (buttonEvent & BUTTONOFF))
    {
      if (flashlight_mode) //cancel flashlight mode
      {
        flashlight_mode = 0;
      }
      else
      {  
        brightness = (brightness + 50) % 256;
      }
      FastLED.setBrightness(brightness);
    }
    //Button 3 LONG = Flashlight Mode
    if (((buttonEvent & BUTTONNUMBERMASK) == 3) && (buttonEvent & BUTTONLONG))
    {
      quick_tone();
      
      FastLED.setBrightness(255);
      flashlight_mode = (flashlight_mode + 1) % 4;
      flashlight();
    }
    //Button 4 = Sound Effect - La Cucaracha
    if (((buttonEvent & BUTTONNUMBERMASK) == 4) && (buttonEvent & BUTTONOFF))
    {
      la_cucaracha();
    }
    //Button 4 LONG = Random Sound Effect
    if (((buttonEvent & BUTTONNUMBERMASK) == 4) && (buttonEvent & BUTTONLONG))
    {
      switch (random8(4)) {
        case 0: soundup2(); break;
        case 1: soundupdown1(); break;
        case 2: randombeeps(); break;
        case 3: soundbomb1(); break;
      }
    }
    //Button 5 =  Change Strip Pattern; RELEASE hold if enabled
    if (((buttonEvent & BUTTONNUMBERMASK) == 5) && (buttonEvent & BUTTONOFF))
    {
      change_strip_pattern();
    }
    //Button 5 LONG = Hold Strip Pattern TOGGLE
    if (((buttonEvent & BUTTONNUMBERMASK) == 5) && (buttonEvent & BUTTONLONG))
    {
     strip_pattern_hold = strip_pattern_hold ? 0 : 1;
     if (strip_pattern_hold) { quick_tone(); }
     else { quick_tone_down(); };
     
    }

    // clear button event
    buttonEvent = 0;
  }

  //time to advance pattern?
  if (!pattern_hold && ((millis() - lastPatternChangeTime) > patternChangeDelay))
  {
      change_patterns();
      //reset pattern change time
      lastPatternChangeTime = millis(); 
      //reset animation frame too
      animationframe = 0;
  }
  //time to draw a new frame?
  if (((millis() - lastFrameTime) > delayval) && !flashlight_mode)
  {
    //reset the timer
    lastFrameTime = millis();
    //draw the frame
    drawAnimationFrame();
    //increase frame
    animationframe++;
  }
  
  

} //end loop

void change_patterns()
{
    //change patterns
    if (!strip_pattern_hold) {
      stripmode = (stripmode + 1) % strippatterns;
    }
      ringmode = (ringmode + 1) % ringpatterns;
}

void change_strip_pattern() {
  stripmode = (stripmode + 1) % strippatterns;
}


void drawAnimationFrame()
{
  
  switch (stripmode)
  {
    case 0 : strips_redgreen(); break;
    case 1 : strips_superbright(); break;
    case 2 : strips_randompos(); break;
    case 3 : strips_pulse(); break;
    case 4 : strips_bounce(); break;
    case 5 : strips_slowgradient(); break;
  };

  switch (ringmode)
  {
    case 0 : ring_randomdim(randomdim_numleds,15); break;
    case 1 : fire(35,180); break;
    case 2 : randombuzz(); break;
    case 3 : rect(); break;
    case 4 : square(); break;
    case 5 : smile(); break;
    case 6 : halves(); break;
    case 7 : quarters(); break;
    case 8 : bounce2(); break;
    case 9 : bounce(); break;
    case 10 : randompos(); break;
    case 11 : colorflash(); break;
    case 12 : superbright(); break;
    case 13 : circlechase(); break;
    case 14 : gradientwipe1(); break;
  };
 
}

//////////////////
//////////////Strips animations
//////////////////
void strips_redgreen() {
  if (animationframe % 3 != 0) return;
  int i = animationframe / 3;
  
    for (int j=0; j < NUMPIXELS_L;j++ ){
      
      if (j <= (i%NUMPIXELS_L) )
      {
        pixels_l[j] = CRGB::Red; 
        pixels_r[j] = CRGB::Green;
      }
      else {
         pixels_l[j] = CRGB::Black; 
         pixels_r[j] = CRGB::Black; 
      }
     }
     FastLED.show();
     //no delay here... make it a multiple of frames? delay(oncycle);
     
  
}

void strips_superbright() {
    // every 5th animation frame
    if (animationframe % 5 != 0) return;
  int i = (animationframe / 5) % NUMPIXELS_L;
   // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
      for (int j=0; j < NUMPIXELS_L;j++ ){
      
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      if (i%4 == j%4 ) {
      pixels_l[j] = CRGB::White; // superbright white
      pixels_r[j] = CRGB::White; // superbright white
      
     }
      else {
         pixels_l[j] = CRGB::Black; 
         pixels_r[j] = CRGB::Black; 
      }
      FastLED.show(); // This sends the updated pixel color to the hardware.
      
      }//J
}

// simple red bounce around the 
void strips_bounce() {
  if (animationframe % 3 != 0) return;
  int i = (animationframe / 3) % NUMPIXELS_L;
 
      for (int j=0; j < NUMPIXELS_L;j++ ){
      
      if (i==j || NUMPIXELS_L-j == i ) {
        pixels_l[j] = CRGB::Red;
        pixels_r[j] = CRGB::Red;
      }
      else {
        pixels_l[j] = CRGB::Black;
        pixels_r[j] = CRGB::Black;
      }
     
      
      }//J
     FastLED.show(); // This sends the updated pixel color to the hardware.
}

void strips_slowgradient() {
  
  fill_gradient(pixels_l,0,CHSV((animationframe/4+128) % 256, 255, 255), NUMPIXELS_L-1,   CHSV(((animationframe/4) + 192) % 256, 255, 255));
  fill_gradient(pixels_r,0,CHSV((animationframe/4) % 256, 255, 255), NUMPIXELS_L-1,   CHSV(((animationframe/4) + 64) % 256, 255, 255));
  FastLED.show();  
}

void strips_pulse() {
  //pulse a single color in brightness, brighter towards the center of the strip
  static int the_color = random8();
  if (animationframe  % 256 == 0) the_color = random8();
  //brightness from 0 to 192 and back
  int pixelbright = ((animationframe % 64) < 32) ? animationframe % 64 * 3 : (191 - (animationframe % 64 * 3 ));
  for (int j = 0; j < 4; j++) {
    CHSV pix = CHSV(the_color,255,pixelbright + ((j+1) * 16));
    pixels_l[j] = pix;
    pixels_l[NUMPIXELS_L-j-1] = pix;
    pixels_r[j] = pix;
    pixels_r[NUMPIXELS_R-j-1] = pix;
  }
  FastLED.show();
}

//Random color at random position
void strips_randompos() {
     int thisone = random8(NUMPIXELS_L);
     fill_solid(pixels_l, NUMPIXELS_L, CRGB::Black);
     pixels_l[thisone].setRGB(random8(),random8(),random8());
     thisone = random8(NUMPIXELS_R);
     fill_solid(pixels_r, NUMPIXELS_R, CRGB::Black);
     pixels_r[thisone].setRGB(random8(),random8(),random8());
     FastLED.show(); // This sends the updated pixel color to the hardware.  
}

/////ANIMATION PATTERNS

void ring_randomdim(int numleds, int interval) {
  static byte  cred[randomdim_numleds], cgreen[randomdim_numleds],cblue[randomdim_numleds];
  static int cpos[randomdim_numleds];
  int icount = 0;
  icount = animationframe % interval;
    
      if (icount == 0) { //new interval, advance the leds & set new 0
      for (int leds = numleds - 1; leds > 0;leds--) {
        cpos[leds] = cpos[leds-1];
        cred[leds] = cred[leds-1];
        cgreen[leds] = cgreen[leds-1];
        cblue[leds] = cblue[leds-1];
      }
     
      //new led at position 0 
      cpos[0] = random(NUMPIXELS);
      cred[0] = random(256); 
      cgreen[0] = random(256);
      cblue[0] = random(256);
      //end new interval
    }
    //clear old leds
     for (int j=0; j < NUMPIXELS;j++ ){
       pixels[j] = CRGB::Black; 
     }
     //set leds and dim
      for (int leds = 0; leds < numleds;leds++) {
        pixels[cpos[leds]].setRGB(cred[leds],cgreen[leds],cblue[leds]);
        if (cred[leds] > 0) cred[leds]-=2;
        if (cgreen[leds] > 0) cgreen[leds]-=2;
        if (cblue[leds] > 0) cblue[leds]-=2;
       
      }
      
      FastLED.show();
     
 
}



void fire(int Cooling, int Sparking) {
  static const int NUMHEAT = NUMPIXELS/2;
  static byte heat[NUMHEAT];
  int cooldown;
   // every 2th animation frame
    if (animationframe % 2 != 0) return;
  ///pixels.setBrightness(25);
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUMHEAT; i++) {
    cooldown = random(0, ((Cooling * 10) / NUMHEAT) + 2);
    
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
  
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUMHEAT - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
    
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUMHEAT; j++) {
    setPixelHeatColor(j, heat[j] );
    setPixelHeatColor(NUMPIXELS - j - 1, heat[j]);
  }

  FastLED.show();  
}


void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    pixels[Pixel].setRGB(255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    pixels[Pixel].setRGB(255, heatramp, 0);
  } else {                               // coolest
    pixels[Pixel].setRGB(heatramp, 0, 0);
  }
}


void randombuzz() {
  static int currpos = 0;
  static int prevpos = 0;
  static int nextrand = random(NUMPIXELS);
  static int dir = 1;
  // every 2th animation frame
  if (animationframe % 2 != 0) return;

    if (currpos == nextrand)
    {
    prevpos = currpos;
    nextrand = random(NUMPIXELS);
    dir *= -1; 
    }
    
    for (int j=0; j < NUMPIXELS;j++ ){
      
      
      if (currpos == j) {
          pixels[j] = CRGB::White;
      }
      else if (j == nextrand) {
          pixels[j].setRGB(20,0,0); // dim red
      }
      else {
         pixels[j] = CRGB::Black;  // off
      }
     }
     FastLED.show();
     currpos = (currpos + dir) % NUMPIXELS; 
     if (currpos < 0) currpos = NUMPIXELS - 1;
   
    
    
     
 
}

void rect() {
   // every 2th animation frame
    if (animationframe % 2 != 0) return;
    int i = (animationframe/2) % NUMPIXELS;
    int times = (animationframe/2)/(NUMPIXELS);
      for (int j=0; j < NUMPIXELS;j++ ){
      
     if (i%12 == j%12 || (j+6 + ((i+NUMPIXELS*times)/7)) % 12 == i  % 12) {
      pixels[j].setHue(animationframe%256); // superbright white
      
     }
      else {
        pixels[j] = CRGB::Black; // off.
      }
      
      
      }//J
       
    FastLED.show(); // This sends the updated pixel color to the hardware.
    
}

void square() {
   // every 2th animation frame
    if (animationframe % 2 != 0) return;
    int i = (NUMPIXELS -1) - ((animationframe/2) % NUMPIXELS);
      for (int j=0; j < NUMPIXELS;j++ ){
      
      if (i%6 == j%6 ) {
        pixels[j].setHue(animationframe%256);
    
     }
       else {
        pixels[j] = CRGB::Black; // off.
      }
    
      
      }//J
       
    FastLED.show(); // This sends the updated pixel color to the hardware.
  
}


void smile() {

    //every 20th frame
    if (animationframe % 20 != 0) return;
    int i = animationframe/20;
    

    for (int j=0; j < NUMPIXELS;j++ ){
      
      if (j <= 4 || j == 9 || j == 15 || j >=20 ) {
          pixels[(j+i)%NUMPIXELS].setHue((i*4)%256); // from colorwheel
      }
      else {
         pixels[(j+i)%NUMPIXELS] = CRGB::Black; //off
      }
     }
     FastLED.show();
}

void halves() {
     //every 3rd frame
    if (animationframe % 3 != 0) return;
    int i = animationframe/3;
    
    for (int j=0; j < NUMPIXELS;j++ ){
      
      if (j/12 == i % 2) {
          pixels[(j+i)%NUMPIXELS].setHue( ((i*4)+((i%2)*128))%256); // from colorwheel
      }
      else {
         pixels[(j+i)%NUMPIXELS] = CRGB::Black;   // off
      }
     }
     FastLED.show();
  
}

void quarters() {
    // every 10th animation frame (240 msec)
    if (animationframe % 10 != 0) return;
    int i = (animationframe / 10);
    
    for (int j=0; j < NUMPIXELS;j++ ){
      
      if (j/6 == i % 4) {
          pixels[j].setHue((i*8)%256); // from colorwheel
      }
      else {
         pixels[j] = CRGB::Black;
         
      }
     }
     FastLED.show();
}

void bounce2() {
    //from 0 to 12; pause 500msec, 12 to 0; pause 500 msec
    //pattern is actually 144 'clock ticks' long
    //0 to 23 (pixels up from 0 to 11)
    //24 to 71 (pixel at 12)
    //72 to 95 (pixel down 11 to 0
    //96 to 143 (pixel at 0);
    // every 2nd clock tick 
    if (animationframe % 2 != 0) return;
    int i = animationframe%144;
    
    if (i >= 0 && i < 24) { i = i/2; }
    else if (i >=24 && i <72) { i = 12; }
    else if (i >= 72 && i < 96) { i = (96 - i) / 2; }
    else if (i >= 96) { i = 0; };
  
    for (int j=0; j < NUMPIXELS;j++ ){
      
      if ((i==j || NUMPIXELS-j == i) && i < 6) {
        pixels[j] = CRGB::Fuchsia;
      
     } else  if ((i==j || NUMPIXELS-j == i) && (i >= 6 && i < 12)) {
        pixels[j] = (j < 12 ? CRGB::Red : CRGB::Lime);  //red on the left, green on the right
      
     }else  if ((i==j || NUMPIXELS-j == i) && (i = 12)) {
        pixels[j] = CRGB::Blue;
        
     }
      else {
        pixels[j] = CRGB::Black;
      }
     
      
      }//J
     
 FastLED.show();
  
}

// simple red bounce around the circle
void bounce() {
 
    int i = animationframe % NUMPIXELS;
      for (int j=0; j < NUMPIXELS;j++ ){
      
      if (i==j || NUMPIXELS-j == i ) {
        pixels[j] = CRGB::Red;
      }
      else {
        pixels[j] = CRGB::Black;
      }
     
      
      }//J
     FastLED.show(); // This sends the updated pixel color to the hardware.
}

//Random color at random position
void randompos() {
  
     int thisone = random8(NUMPIXELS);

     fill_solid(pixels, NUMPIXELS, CRGB::Black);
     pixels[thisone].setRGB(random8(),random8(),random8());
     
     FastLED.show(); // This sends the updated pixel color to the hardware.  
}

//random color on for 40msec and off for 200msec (on at frame 0, off at frame 5 ,mod 24)
void colorflash() {

    if (animationframe % 24 == 0)
    {
      fill_solid(pixels, NUMPIXELS, CHSV(random8(),255,255));
      FastLED.show();
    }
    else if (animationframe % 24 == 5)
    {
      fill_solid(pixels, NUMPIXELS, CRGB::Black);
      FastLED.show();
    }
}

//triangle of bright white
void superbright() {
  if (animationframe % 2 != 0) return;
  int i = (animationframe / 2) % NUMPIXELS;

    for (int j=0; j < NUMPIXELS;j++ ){
      
    if (i%8 == j%8 ) {
        pixels[j] = CRGB::White;
    }
    else {
      pixels[j] = CRGB::Black; // off.
    }
      
    }
    FastLED.show();
   
  
}


void circlechase() {

  int i = animationframe % NUMPIXELS;
  int pixelbright = ((animationframe % 64) < 32) ? animationframe % 64 * 3 + 64 : (255 - (animationframe % 64 * 3 ));
  // "pixel brightness" goes from 64 to 255 and back to 0
 
    for (int j=0; j < NUMPIXELS;j++ ){
    
    if (((NUMPIXELS + (j - i) )% NUMPIXELS) < 6 ) {
    
    pixels[j] = CHSV(animationframe%256, 255, pixelbright); 
   }
    else {
      pixels[j] = CRGB::Black; // off.
    }
  
    }//J
    FastLED.show();
   
  
}

void gradientwipe1() {

if (animationframe % 5 != 0) return;
int i = (animationframe / 5) % (NUMPIXELS/2);

static int the_color = random8();
static int startpos = random8(NUMPIXELS);
if (i == 0) { the_color = random8(); startpos += NUMPIXELS/3 + random8(5); }

for (int j = 0; j < NUMPIXELS/2; j++) {
  // color gradient wipe, brightness is highest at the frame number
  if (i >= j) {
    pixels[(startpos + j) % NUMPIXELS] = CHSV(the_color, 255, 255 - ((i-j) * 20));
      //brightness goes from 255 at the 'i' down by 20 as it gets further
    pixels[(startpos + (NUMPIXELS-j-1)) % NUMPIXELS] = CHSV(the_color, 255, 255 - ((i-j) * 20));
  }
}
FastLED.show();
  
  
}

//////////////
//////////////FLASHLIGHT MODE
//////////////
void flashlight()
{
  //set to all white 
  fill_solid(pixels, NUMPIXELS, (flashlight_mode != 2 ? CRGB::White : CRGB::Black));
  fill_solid(pixels_l, NUMPIXELS_L, (flashlight_mode >= 2 ? CRGB::White : CRGB::Black));
  fill_solid(pixels_r, NUMPIXELS_R, (flashlight_mode >= 2 ? CRGB::White : CRGB::Black));
  FastLED.show();
}



/* Check buttons for state change and long presses, set global event if so */
void checkButton()
{
  //read the pushbutton value into a variable
  for (int i = 0; i < 5; i++)
  { 
    sensorVals[i] = digitalRead(2+i);
    if (sensorVals[i] != previousButtonVals[i])
    {
      lastDebounceTime = millis();
      /////Serial.println("debounce!");
    }

     //check debounce
    if ((millis() - lastDebounceTime) > bounceDelay)
    {
      if (sensorVals[i] != buttonState[i])
      {
        // it actually changed
        buttonState[i] = sensorVals[i];
        /////Serial.print("  BUTTON " );
        /////Serial.print(i);
        //////Serial.print(" STATE CHANGE ");
        /////Serial.println(buttonState[i]);

        //set the global event
        buttonEvent = (i+1) | (buttonState[i] == LOW ? BUTTONON : BUTTONOFF);
        
        //reset the long press delay if state is LOW (pressed)
        if (buttonState[i] == LOW)
        {
          longpressTime = millis();
         ////// Serial.println(" Resetting Long Press");
        }
        else if (buttonState[i] == HIGH && buttonLongState[i]) // HIGH + Release after long press
        {
          buttonEvent = (i+1) | BUTTONLONGOFF;
          buttonLongState[i] = 0;
        }


      }
    }

    // check for long press
    if ((buttonState[i] == LOW) && !buttonLongState[i] && ((millis() - longpressTime) > longpressDelay))
    {
      Serial.print("   LONG PRESS ON BUTTON ");
      Serial.println(i);
      buttonLongState[i] = 1;

      //set global
      buttonEvent = (i+1) | BUTTONLONG;
      //reset long press delay
      longpressTime = millis();
    }
    
    previousButtonVals[i] = sensorVals[i];
  }
  
}


/* ACTUAL SELF DESTRUCT SOUND */
void selfDestructSequence()
{
  soundup1();
  soundbomb6();
}


/* INCREASING SOUND - USED FOR SELF DESTRUCT */
void soundup1()
{
  int flash_side = 0;
  int swapdelay = 500;
  //clear leds
  fill_solid(pixels, NUMPIXELS,CRGB::Black);
  fill_solid(pixels_l, NUMPIXELS_L, CRGB::Red);
  fill_solid(pixels_r,NUMPIXELS_R, CRGB::Black);
  FastLED.show();
  for (int i = 50; i <= 5000; i+= 10)
  {
    noise(i,0,10);
    //Fill ring
    if (i%50 == 0)
    {
      for (int l = 0; l < NUMPIXELS; l++)
      {
        int stage = i/209;
        if (l <= stage) {
          if (stage < 12) { pixels[l] = CRGB::Green; }
          else if (stage >= 12 && stage < 18) { pixels[l] = CRGB::Yellow; } 
          else if (stage >= 18 && stage < 24) { pixels[l] = CRGB::Red;}
        } else { pixels[l] = CRGB::Black; }
        
      }
      FastLED.show();
    }
    if (i > swapdelay) {
      if (flash_side) { 
        fill_solid(pixels_l, NUMPIXELS_L, CRGB::Red);
        fill_solid(pixels_r,NUMPIXELS_R, CRGB::Black);
      }
      else {
        fill_solid(pixels_l, NUMPIXELS_L, CRGB::Black);
        fill_solid(pixels_r,NUMPIXELS_R, CRGB::Red);
    }
    flash_side = (flash_side + 1) % 2;
    swapdelay += ((5000 - i) / 10);
    FastLED.show();
  }
}
}

/* FINAL EXPLOSION NOISE */
void soundbomb6() // FINAL EXPLOSION NOISE
{
  //set to all white 
  fill_solid(pixels, NUMPIXELS, CRGB::White);
  fill_solid(pixels_l, NUMPIXELS_L, CRGB::Black);
  fill_solid(pixels_r, NUMPIXELS_R, CRGB::Black);
  FastLED.show();
for (int freq = 300; freq <= 1000; freq += 1)
  {
  noise(freq, 200, 5);
  //fade slowly
  if (freq % 6 == 0) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels[i].fadeToBlackBy( 1 );
    }
  fill_gradient(pixels_l,0,CHSV((freq/6+128) % 256, 255, 255), NUMPIXELS_L-1,   CHSV(((freq/6) + 148) % 256, 255, 255));
  fill_gradient(pixels_r,0,CHSV((freq/6) % 256, 255, 255), NUMPIXELS_L-1,   CHSV(((freq/6) + 20) % 256, 255, 255));
  FastLED.show();  
  }
  }
for (int freq = 1000; freq >= 100; freq -= 1)
  {
  noise(freq, 100, 5);
  if (freq % 6 == 0) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels[i].fadeToBlackBy( 1 );
    }
  fill_gradient(pixels_l,0,CHSV((freq/6+128) % 256, 255, 255), NUMPIXELS_L-1,   CHSV(((freq/6) + 148) % 256, 255, 255));
  fill_gradient(pixels_r,0,CHSV((freq/6) % 256, 255, 255), NUMPIXELS_L-1,   CHSV(((freq/6) + 20) % 256, 255, 255));
  FastLED.show();  
  }
  }
  
   
}

void quick_tone()
{
  tone(TONEPIN, NOTE_C3, 100);
  delay(100);
  tone(TONEPIN, NOTE_G3, 100);
  delay(100);
  noTone(TONEPIN);
 }
void quick_tone_down()
{
  tone(TONEPIN, NOTE_C3, 30);
  delay(30);
  noTone(TONEPIN);
 }
void la_cucaracha() {
 for (int thisNote = 0; thisNote < 19; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations_lc[thisNote];
    tone(TONEPIN, melody_lc[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(TONEPIN);
  }
}
void soundup2()
{
  for (int j = 0; j < 3; j++) {
  for (int i = 50; i < 1000; i+= 10)
  {
    noise(i,0,10);
  }
  }
}

void soundupdown1()
{
  for (int j = 0; j < 3; j++) {
  for (int i = 50; i <= 1000; i+= 10)
  {
    noise(i,0,10);
  }
  for (int i = 1000; i >= 50; i-= 10)
  {
    noise(i,0,10);
  }
  }
}

void soundmelody1()
{
  for (int i = 0; i < 8; i++)
  {
    noise(melody[i],0, 500);
  }
}

void soundbomb1()
{
  for (int loop=1; loop < 5; loop++)
  {
  noise(loop*1000,loop*900,1000);
  }
}

void soundbomb2()
{
  noise(300, 100, 200);
  delay(100);
  noise(300, 100, 1000);
}

void soundbomb4()
{
  noise(300, 100, 1000);
  delay(500);
  noise(300, 100, 3000);
}  
void soundbomb3()
{

  for (int freq = 3000; freq >= 500; freq -= 100)
  {
  noise(freq, 300, 50);
  }
  
   noise (350,100, 2000);
}

void soundbomb5()
{
noise (500,200, 1000);
delay(500);
for (int freq = 700; freq >= 300; freq -= 1)
  {
  noise(freq, 200, 5);
  }
  
   
}

void randombeeps()
{
  for (int i = 0; i < 20; i++)
    {
      tone(TONEPIN, random(75, 1000),200);
      delay(250);
    }
    
}

/* RANDOM NOISE in a range for a duration */
void noise(int freq, int spread, int duration)  {
  int low = freq - spread;
  int high = freq + spread;
  unsigned long time = millis();
  while(millis() - time <= duration)  {
    tone(TONEPIN, random(low, high));
  }
  noTone(TONEPIN);
}

