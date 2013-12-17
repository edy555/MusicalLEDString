#define FORCE_SOFTWARE_SPI
#define FORCE_SOFTWARE_PINS
#include "FastSPI_LED2.h"

//#define LOG_OUT 1
#define OCTAVE 1
//#define LIN_OUT8 1
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

// How many leds are in the strip?
#define NUM_LEDS 240

// Data pin that led data will be written out over
#define DATA_PIN 6

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

byte gamma[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11,
11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28,
29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40,
40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54,
55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 88, 89,
90, 91, 93, 94, 95, 96, 98, 99,100,102,103,104,106,107,109,110,
111,113,114,116,117,119,120,121,123,124,126,128,129,131,132,134,
135,137,138,140,142,143,145,146,148,150,151,153,155,157,158,160,
162,163,165,167,169,170,172,174,176,178,179,181,183,185,187,189,
191,193,194,196,198,200,202,204,206,208,210,212,214,216,218,220,
222,224,227,229,231,233,235,237,239,241,244,246,248,250,252,255};

void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0

  Serial.print("Rock FFT!");
}

void do_fft()
{
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    //Serial.print(fft_input[0]);
    //Serial.print('\n');
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    //fft_mag_log(); // take the output of the fft
    fft_mag_octave();
    //fft_mag_lin8();
    sei();
}

#define DEC(x, n) if(x>(n)) x-=(n); else x=0

void loop() {
   do_fft();  
   int i;
#if 0
   for (i = 0; i < 8; i++) {
     //Serial.print(fft_log_out[i]);
	 Serial.print(fft_oct_out[i]);
     Serial.print(' ');
   }
   Serial.print('\n');
#endif

   int r = fft_oct_out[1] + fft_oct_out[2]/2;
   int g = fft_oct_out[3]/2 + fft_oct_out[4] + fft_oct_out[5]/2;
   int b = fft_oct_out[6]/2 + fft_oct_out[7];
   DEC(r, 220);
   DEC(g, 100);
   DEC(b, 60);
   //r /= 2;
   g /= 2;
   //b /= 2;
   leds[0] = CRGB(gamma[g], gamma[r], gamma[b]);
   FastLED.show();
   for(i = NUM_LEDS-1; i > 0; i--) {
      leds[i] = leds[i-1];
   }
}

