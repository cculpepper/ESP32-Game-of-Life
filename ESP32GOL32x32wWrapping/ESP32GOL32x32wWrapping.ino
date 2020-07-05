/*******************************************************************
 *                                                                 *
    An ESP32 port of Conway's Game of Life sketch for Arduino by Aerodynamics
    Project here: https://www.hackster.io/aerodynamics/arduino-led-matrix-game-of-life-093f06
    
    Based on the great work of Brian Lough and Leonid Rusnac (@lrusnac github)
    https://www.youtube.com/channel/UCezJOfu7OtqGzd5xrP3q6WA
    
   
    Edited by Emily Velasco (twitter.com/MLE_Online)
    Edge wrapping added by Ted Yapo (twitter/com/tedyapo)
 *******************************************************************/

// ----------------------------
// Standard Libraries - Already Installed if you have ESP8266 set up
// ----------------------------

#include <Ticker.h>

#include <PxMatrix.h>
// The library for controlling the LED Matrix
// Needs to be manually downloaded and installed
// https://github.com/2dom/PxMatrix

Ticker display_ticker;

// Pins for LED MATRIX
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
//#define P_OE 2
#define P_OE 16
#define ROWS 64
#define COLS 64

bool WORLD[ROWS][COLS]; // Creation of the world
bool WORLD2[ROWS][COLS]; // Creation of the world
int step_GOL; //used to know the generation
//uint16_t world_frame[16][32];

// PxMATRIX display(ROWS,COLS,P_LAT, P_OE,P_A,P_B,P_C);
// PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);
PxMATRIX display(ROWS, COLS, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);



// ISR for display refresh
void display_updater() {
  display.display(30);
  vTaskDelay(1);
}
void initializeWorld(){
  display.clearDisplay();
  delay(500);
  randomSeed(analogRead(5));
 
  for (byte i = 0; i < ROWS; i++) {
    for (byte j = 0; j < COLS; j++) {
        if (random(0,100) > 40){
          WORLD[i][j] = 1;
        } else {
          WORLD[i][j] = 0;
        }
      
    }
  }
}
void setup() {
  display.begin(32);
  display.clearDisplay();
  initializeWorld();
  display_ticker.attach(0.002, display_updater);
  yield(); 
}
void gayDrawPixel(int x, int y){
	// 64 rows
	// Red
	// Orange
	// Yellow
	// Green
	// Blue
	// Purple
	// 64/6 is 10, so 12, 22, 32, 42, 52, 64
	if (y < 12){
		// Red
		display.drawPixel(x, y, display.color565(random(0,120), random(0, 18),random(0, 18)));
	} else if ( y < 22){
		// Orange
		display.drawPixel(x, y, display.color565(random(80,180), random(20, 60),random(0, 18)));
	} else if ( y < 32){
		// Yellow
		display.drawPixel(x, y, display.color565(random(50,135), random(30, 140),random(0, 18)));
	} else if ( y < 42){
		// Green
		display.drawPixel(x, y, display.color565(random(0, 30), random(60, 130),random(0, 20)));
	} else if ( y < 52){
		// Blue
		display.drawPixel(x, y, display.color565(random(0,40), random(0,50),random(50, 140)));
	} else {
		// Purple
		display.drawPixel(x, y, display.color565(random(50,130), random(0, 20),random(80,150)));
	}

}

void drawFrame() {
  display.clearDisplay();
  int imageHeight = ROWS;
  int imageWidth = COLS;
  for (int yy = 0; yy < imageHeight; yy++) {
    for (int xx = 0; xx < imageWidth; xx++) {
      if(WORLD[yy][xx] == 1) {
        gayDrawPixel(xx,yy);
        /* display.drawPixel(xx, yy, display.color565(random(0,200), random(0,128), random(0,128))); */
      } else {
        display.drawPixel(xx, yy, 0x0000);
      }
    }
  }

  vTaskDelay(10);
  delay(350);
}

// wrap val so that it lies inside [0, size-1],
//   allowing negative values down to -size
inline int8_t wrap(int8_t val, int8_t size)
{
  return (val + size) % size;
}

void loop() {
  if (step_GOL == 480) { // This if reboot the world after 60 generation to avoid static world
    step_GOL = 0;
    initializeWorld();
  }

  //This double "for" is used to update the world to the next generation
  //The buffer state is written on the EEPROM Memory
  for (byte i = 0; i < ROWS; i++) {
    for (byte j = 0; j < COLS; j++) {

      byte num_alive = (WORLD[wrap(i - 1, ROWS)][wrap(j - 1, COLS)] +
                        WORLD[wrap(i - 1, ROWS)][j] +
                        WORLD[wrap(i - 1, ROWS)][wrap(j + 1, COLS)] +
                        WORLD[i][wrap(j - 1, COLS)] +
                        WORLD[i][wrap(j + 1, COLS)] +
                        WORLD[wrap(i + 1, ROWS)][wrap(j - 1, COLS)] +
                        WORLD[wrap(i + 1, ROWS)][j] +
                        WORLD[wrap(i + 1, ROWS)][wrap(j + 1, COLS)]);
      bool state = WORLD[i][j];
      
      //RULE#1 if you are surrounded by 3 cells --> you live
      if (num_alive == 3) {
        WORLD2[i][j] = 1;
      }
      //RULE#2 if you are surrounded by 2 cells --> you stay in your state
      else if (num_alive == 2) {
        WORLD2[i][j] = state;
      }
      //RULE#3 otherwise you die from overpopulation or subpopulation
      else {
        WORLD2[i][j] = 0;
      }
    }
  }

  //Updating the World
  for (byte i = 0; i < ROWS; i++) {
    for (byte j = 0; j < COLS; j++) {
      WORLD[i][j] = WORLD2[i][j];
    }
  }

  //Displaying the world
  drawFrame();

  //Increasing the generation
  step_GOL++;
//  yield();
}
