// 
// Kerbal Control
// Simple control panel and display for Kerbal Space Program.
// Built on a Microchip PIC32MX
//

/*
	Reads data from KSP plugin via serial port.
	Displays status on OLED and LEDs
	Sends input from a couple of buttons
	Uses a tonne of floats in the 3d routine which are probably
	over kill considering the resolution of the OLED that we end up rendering to. 	But the PIC had flash/RAM and CPU to spare so why not for now.
	Also uses printf to display floats which could probably be avoided

	Hardware is a ChipKit DP32
	DP32 : https://digilentinc.com/Products/Detail.cfm?NavPath=2,892,1174&Prod=CHIPKIT-DP32
	Display is a 128x64px SH1106 based OLED

	Copyright (C) 2014 Matt Casey : catmacey.com

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#define VERSION "1.0"

// Use this Microchip library to make access to the peripherals easier
#include <PLIB.h>  
#include <DSPI.h>
#include <Adafruit_GFX.h>
#include <Catmacey_SH1106.h>
#include "config.h"
#include "serialCOMS.h"


//#include "models/model_test.h"
//#include "models/model_complex.h"
#include "models/model_mega.h"

// Digilent hardware SPI library
DSPI0 spi;
// OLED Driver : Uses Adafruit GFX library
Catmacey_SH1106 display(OLED_DC_PIN, OLED_RESET_PIN, OLED_CS_PIN, &spi);

char buff[100];

// Arduino setup routine
void setup() {
	// set the digital pin as output:
	pinMode(PIN_BTN1, INPUT);
	pinMode(PIN_BTN2, INPUT);
	pinMode(PIN_LED1, OUTPUT);
	pinMode(PIN_LED2, OUTPUT);
	pinMode(PIN_LED3, OUTPUT);
	pinMode(PIN_LED4, OUTPUT);
	mPORTBClearBits(LED1_BIT);
	mPORTBClearBits(LED2_BIT);
	mPORTBClearBits(LED3_BIT);
	mPORTBClearBits(LED4_BIT);

	// Setup hardware SPI. Note we're not using MISO
	spi.begin(0, SPI_MOSI_PIN, OLED_CS_PIN);
	spi.setSpeed(10000000);
	// Initialize the OLED
	display.begin();
	// Load the spashscreen into the displaybuffer
	display.drawBitmap(0, 0, g_splashscreen, 128, 64, 1);
	display.display(); // show splashscreen
	// Setup serial
	Serial.begin(38400);
	// Wait. Cos I read that you must wait after enabling serial before you use it.

	// We use this delay to show the splashscreen for a bit
	delay(1000);
	
  initLEDS();
  InitTxPackets();
  controlsInit();

  LEDSAllOff();  
	// init done
}
	

void loop() {
	uint8_t idx = 0;
	uint8_t effectCount = 0;
	int axisidx, btn2db, timer, fps, addr = 0;
	uint16_t ctr;
	float tmp;
	float degToRad = PI / 180;
	float axis[3]; // x,y,z

	mPORTBToggleBits(LED1_BIT);
	display.setTextColor(1);
	display.setCursor(2,53);
	display.print("Version ");
	display.print(VERSION);
	display.display();

	delay(250);
	mPORTBToggleBits(LED2_BIT);
	delay(500);
	display.clearDisplay();   // clears the buffer
	
	display.display();
	mPORTBToggleBits(LED3_BIT);

	// Setup dummy values for output
	// Handy for serial debugging.
	// These values are such that the serial output
	// continuously counts from 0 to f.
	/*
	CPacket.MainControls = 0x01;
	CPacket.Mode = 0x23;
	CPacket.ControlGroup = 0x6745;
	CPacket.AdditionalControlByte1 = 0x89;
	CPacket.AdditionalControlByte2 = 0xab;
	CPacket.Pitch = 0xefcd;
	CPacket.Roll = 0x2301;
	CPacket.Yaw = 0x6745;
	CPacket.TX = 0xab89;
	CPacket.TY = 0xefcd;
	CPacket.TZ = 0x2301;
	CPacket.Throttle = 0x6745;
	*/

	// Loop here. Not using the Arduino loop().
	while(1){
		display.clearDisplay();

		// Grab data from the serial port..
  	KSPinput();


  	// Dumb simple test that the 3d transform is working correctly
  	// BTN2 switches axis
  	btn2db <<= 1;
  	if(mPORTBReadBits(BTN2_BIT)){
  		btn2db += 1;
  	}
  	if(btn2db == 0xff){

  		axisidx++;
  		if(axisidx > 2){
  			axisidx = 0;
  		}
			display.setCursor(0,53);
			switch(axisidx){
				case 0:{
					display.print("X");
					break;
				}
				case 1:{
					display.print("Y");
					break;
				}
				case 2:{
					display.print("Z");
					break;
				}
			}
  	}
  	// BTN1 rotates to 22.5deg
		if(mPORTBReadBits(BTN1_BIT)){
			axis[0] = 0;
			axis[1] = 0;
			axis[2] = 0;
			
			axis[axisidx] = 22.5;
			switch(axisidx){
				case 0:{
					sprintf(buff, "X%3.0f", axis[axisidx]);
					break;
				}
				case 1:{
					sprintf(buff, "Y%3.0f", axis[axisidx]);
					break;
				}
				case 2:{
					sprintf(buff, "Z%3.0f", axis[axisidx]);
					break;
				}
			}
			display.setCursor(0,53);
			display.print(buff);
		}else{
			// Use incoming P R H values.
			// Need to sort out the mapping between the 3 axis
			axis[0] = VData.Pitch;  // Pitch is already -180 to 180
			axis[1] = 180 - VData.Heading; // Heading is 0 to 360
			axis[2] = VData.Roll; // Roll is already -180 to 180
		}

		// Display it
		// This is where the 3d bit happens
		timer = ReadCoreTimer();
		transform(axis[0] * degToRad, axis[1] * degToRad, axis[2] * degToRad);
		timer = (ReadCoreTimer() - timer);

		// Now display numerics for a few handy values
		// Note use of printf is probably overkill but I
		// have flash to spare and it makes formatting the output
		// very simple.

		display.setCursor(121,54);
		if (Connected) {
			display.print("Y");
		}else{
			display.print("N");
		}
		
		// Pitch
		display.setCursor(0,0);
		sprintf(buff, "P%3.0f", VData.Pitch);
		display.print(buff);
		
		// Roll
		display.setCursor(0,12);
		sprintf(buff, "R%3.0f", VData.Roll);
		display.print(buff);

		// Heading
		display.setCursor(0,24);
		sprintf(buff, "H%3.0f", VData.Heading);
		display.print(buff);

		// Altitude
		display.setCursor(0,36);
		if(VData.RAlt > 10000){
			tmp = VData.RAlt / 10000;
			sprintf(buff, "A%4.0fkm", tmp);
		}else{
			sprintf(buff, "A%5.0fm", VData.RAlt);
		}
		display.print(buff);

		// Speed (Surface)
		display.setCursor(0,48);
		if(VData.Vsurf > 1000){
			tmp = VData.Vsurf / 1000;
			sprintf(buff, "V%4.0fkm/s", tmp);
		}else{
			sprintf(buff, "V%5.0fm/s", VData.Vsurf);
		}
		display.print(buff);

		// Overlay the direction indicator
		display.drawBitmap(88, 30, g_direction_mask, 17, 8, 0);
		display.drawBitmap(88, 30, g_direction, 17, 8, 1);
		// Overlay the 3d with a bezel image
		display.drawBitmap(32, 0, g_panel_overlay, 128, 64, 1);

		// Times
		fps = (SYS_FREQ/2) / timer;
		timer <<= 8;
		timer /= CORE_TICK_RATE;
		
		// Display of frame rate and time spent in 3d
		/*
		display.setCursor(0,0);
		display.print(fps);
		display.print("f");

		display.setCursor(0,53);
		display.print(timer >> 8);
		display.print(".");
		display.print((timer & 0xff)/10, DEC);
		display.print("m");
		*/

		// Now send the whole display buffer to the OLED via SPI
		display.display();

		// Heartbeat
		mPORTBToggleBits(LED4_BIT);
		
		delay(20);
  	KSPoutput();
	} //while
}

// Routine to draw and calc 3d transform of the model
// Based on 3D Cube by Jason Wright (C)opyright Pyrofer 2006
// http://www.pyrofersprojects.com/blog/3dpic/
// Modified by Matt Casey.
// Added use of structs for storage of model data (verts, lines, tris)
// Added halftone filled triangles.
// Added routine to cull hidden lines/tris using crude z technique

void transform(float rotx, float roty, float rotz){
	uint8_t idx;                    	// temp variable for loops
	float xt,yt,zt,x,y,z,sinax,cosax,sinay,cosay,sinaz,cosaz;  // lots of work variables
	
	// Not interested in translation
	float xpos = 0;
	float ypos = 0;
	float zpos = 256;

	const Vertex_t *vert;
	const Line_t *line;
	const Face_t *face;

	sinax = sin(rotx);			// precalculate the sin and cos values
	cosax = cos(rotx);			// for the rotation as this saves a 

	sinay = sin(roty);			// little time when running as we
	cosay = cos(roty);			// call sin and cos less often

	sinaz = sin(rotz);			// they are slow routines
	cosaz = cos(rotz);			// and we dont want slow!

	// translate 3d vertex position to 2d screen position
	for(idx=0; idx<g_model.vertcount; idx++){
		vert = &g_model.verts[idx];
		x = vert->x;
		y = vert->y;
		z = vert->z;

		// rotate around the Y axis : Heading
		xt = x * cosay - z * sinay;	
		zt = x * sinay + z * cosay;	// using X and Z
		x = xt;
		z = zt;
		// rotate around the x axis : Pitch
		yt = y * cosax - z * sinax;	
		zt = y * sinax + z * cosax;	// using the Y and Z for the rotation
		y = yt;
		z = zt;
		// rotate around the Z axis : Roll
		xt = x * cosaz - y * sinaz;	
		yt = x * sinaz + y * cosaz;	// using X and Y
		x = xt;
		y = yt;

		x += xpos;			// add the object position offset
		y += ypos;			// for both x and y

		// I want to know if the point is in the front or back half of the model in order to cull hideen lines
		g_pointBuffer[idx].z = z;  

		z += (OFFSETZ-zpos); // as well as Z

		// Translate 3d to 2d coordinates for screen
		g_pointBuffer[idx].x = (x*64/z)+OFFSETX;
		g_pointBuffer[idx].y = (y*64/z)+OFFSETY;


		// Draw vertex indexes for debugging
		// Handy but very cluttered for complex models
		//display.setCursor(newx[idx],newy[idx]);
		//display.print(idx, DEC);
	}

	// Draw the faces (triangles) 
	// Only for triandles where all vertexes are 
	// towards the front of the model
	for(idx=0; idx<g_model.facecount; idx++){
		face = &g_model.faces[idx];
		if(
			g_pointBuffer[face->a-1].z > -5
			&&
			g_pointBuffer[face->b-1].z > -5
			&&
			g_pointBuffer[face->c-1].z > -5
		){
			// Modified fill : color 4 produces nice dots
			display.fillTriangle(
					g_pointBuffer[face->a-1].x
				, g_pointBuffer[face->a-1].y
				, g_pointBuffer[face->b-1].x
				, g_pointBuffer[face->b-1].y
				, g_pointBuffer[face->c-1].x
				, g_pointBuffer[face->c-1].y
				, 4
			);
		}
	}

	// draw the lines that make up the object
	// Only for lines where both vertexes are towards 
	// the front of the model in it's current transform
	for(idx=0; idx<g_model.linecount; idx++){
		line = &g_model.lines[idx];
		if(
			g_pointBuffer[line->startvert-1].z > 5
			&&
			g_pointBuffer[line->endvert-1].z > 5
		){
			display.drawLine(
					g_pointBuffer[line->startvert-1].x
				, g_pointBuffer[line->startvert-1].y
				, g_pointBuffer[line->endvert-1].x
				, g_pointBuffer[line->endvert-1].y
				, 1
			);
		}
	}
}


