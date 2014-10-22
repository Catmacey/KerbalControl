/*
	Main header file
*/

#ifndef __Config__
#define __Config__

#include <stdint.h> 

// PIC port/pins : Handy for fast PORT access
// PORT A
//#define OLEDRES_BIT BIT_0  // RA0
//#define CLIP_BIT    BIT_1  // RA1

// PORT B
// Outputs
#define	LED4_BIT	BIT_0
#define	LED3_BIT	BIT_1 
#define	LED2_BIT	BIT_2
#define	LED1_BIT	BIT_3
// Inputs
#define BTN1_BIT BIT_4 // RB4
#define BTN2_BIT BIT_7 // RB7

#define BTN_MASK BTN2_BIT | BTN3_BIT // Mask for buttons
#define INPUT_MASK BTN_MASK // Mask for all inputs

// Stupid Arduino pin numbers
#define OLED_RESET_PIN 9 // RA0 
#define SPI_CLK_PIN 7 // RB14
#define OLED_CS_PIN 3 // RB9
#define OLED_DC_PIN 6 // RB13
#define SPI_MOSI_PIN 2 // RB8
// End of stupid Arduino pin numbers

#define SYS_FREQ 40000000 // PIC OSC frequency
#define GetSystemClock() SYS_FREQ
#define GetPeripheralClock()    (GetSystemClock() / (1 << OSCCONbits.PBDIV))

#define OFFSETX 96              	// offset for screen wont change unless
#define OFFSETY 32              	// i use different screen! so its kinda fixed
#define OFFSETZ 30
#define PI 3.14159265

// Splash screen
#include "includes/g_splashscreen.inc"
// Overlay for the entire panel
#include "includes/g_panel_overlay.inc"
// Direction indicator
#include "includes/g_direction.inc"
#include "includes/g_direction_mask.inc"


// Now for some excessive use of floats...

// Set of vertexes in a model
typedef struct {
	int8_t x;
	int8_t y;
	int8_t z;
} Vertex_t;

// Triplet of Vertex indexes that form a face
typedef struct {
	uint8_t a;
	uint8_t b;
	uint8_t c;
} Face_t;

// Set of lines as vertix indexes (start, end)
typedef struct {
	uint8_t startvert;
	uint8_t endvert;
} Line_t;

// Model made up of vertexes and lines
typedef struct {
	const Vertex_t *verts;
	const Line_t *lines;
	const Face_t *faces;
	const uint8_t vertcount;
	const uint8_t linecount;
	const uint8_t facecount;
} Model_t;

// Set of 2d screen points transformed from the vertexes
typedef struct {
	int8_t x;
	int8_t y;
	int8_t z;
} Point_t;

// Buffer of transformed vertexes
extern Point_t g_pointBuffer[];

#endif