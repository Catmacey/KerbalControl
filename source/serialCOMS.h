/*
	Main header file
	Modified to include packing to ensure 
	the correct number of byte in the structs
*/

#ifndef __SerialCOMS__
#define __SerialCOMS__

#include <stdint.h> 

//pins for LEDs
#define GLED PIN_LED1
#define YLED PIN_LED2
#define RLED PIN_LED3

//pins for input
#define SASPIN 8
#define RCSPIN 9
#define CG1PIN 10
#define THROTTLEPIN 0

#define THROTTLEDB 4 //Throttle axis deadband

#define SAS 7
#define RCS 6
#define LIGHTS 5
#define GEAR 4
#define BRAKES 3
#define PRECISION 2
#define ABORT 1
#define STAGE 0

//macro 
#define details(name) (uint8_t*)&name,sizeof(name)

//if no message received from KSP for more than 2s, go idle
#define IDLETIMER 2000
#define CONTROLREFRESH 25

//warnings
#define GWARN 9                  //9G Warning
#define GCAUTION 5               //5G Caution
#define FUELCAUTION 10.0         //10% Fuel Caution
#define FUELWARN 5.0             //5% Fuel warning

uint32_t deadtime, deadtimeOld, controlTime, controlTimeOld, now;

boolean Connected = false;

uint8_t caution = 0, warning = 0, id;

#pragma pack(push, 1) // exact fit - no padding
typedef struct {
	uint8_t id;          		//1   packet id
	float AP;            		//2   apoapsis (m to sea level)
	float PE;            		//3   periapsis (m to sea level)
	float SemiMajorAxis; 		//4   orbit semi major axis (m)
	float SemiMinorAxis; 		//5   orbit semi minor axis (m)
	float VVI;           		//6   vertical velocity (m/s)
	float e;             		//7   orbital eccentricity (unitless, 0 = circular, > 1 = escape)
	float inc;           		//8   orbital inclination (degrees)
	float G;             		//9   acceleration (Gees)
	int32_t TAp;         		//10  time to AP (seconds)
	int32_t TPe;         		//11  time to Pe (seconds)
	float TrueAnomaly;   		//12  orbital true anomaly (degrees)
	float Density;       		//13  air density (presumably kg/m^3, 1.225 at sea level)
	int32_t period;      		//14  orbital period (seconds)
	float RAlt;          		//15  radar altitude (m)
	float Alt;           		//16  altitude above sea level (m)
	float Vsurf;         		//17  surface velocity (m/s)
	float Lat;           		//18  Latitude (degree)
	float Lon;           		//19  Longitude (degree)
	float LiquidFuelTot; 		//20  Liquid Fuel Total
	float LiquidFuel;    		//21  Liquid Fuel remaining
	float OxidizerTot;   		//22  Oxidizer Total
	float Oxidizer;      		//23  Oxidizer remaining
	float EChargeTot;    		//24  Electric Charge Total
	float ECharge;       		//25  Electric Charge remaining
	float MonoPropTot;   		//26  Mono Propellant Total
	float MonoProp;      		//27  Mono Propellant remaining
	float IntakeAirTot;  		//28  Intake Air Total
	float IntakeAir;     		//29  Intake Air remaining
	float SolidFuelTot;  		//30  Solid Fuel Total
	float SolidFuel;     		//31  Solid Fuel remaining
	float XenonGasTot;   		//32  Xenon Gas Total
	float XenonGas;      		//33  Xenon Gas remaining
	float LiquidFuelTotS;		//34  Liquid Fuel Total (stage)
	float LiquidFuelS;   		//35  Liquid Fuel remaining (stage)
	float OxidizerTotS;  		//36  Oxidizer Total (stage)
	float OxidizerS;     		//37  Oxidizer remaining (stage)
	uint32_t MissionTime;		//38  Time since launch (s)
	float deltaTime;     		//39  Time since last packet (s)
	float VOrbit;        		//40  Orbital speed (m/s)
	uint32_t MNTime;     		//41  Time to next node (s) [0 when no node]
	float MNDeltaV;      		//42  Delta V for next node (m/s) [0 when no node]
	float Pitch;         		//43  Vessel Pitch relative to surface (degrees)
	float Roll;          		//44  Vessel Roll relative to surface (degrees)
	float Heading;       		//45  Vessel Heading relative to surface (degrees)
} VesselData_t;

typedef struct {
  uint8_t id;
  uint8_t M1;
  uint8_t M2;
  uint8_t M3;
} HandShakePacket_t;

typedef struct {
  uint8_t id;
  uint8_t MainControls;                  //SAS RCS Lights Gear Brakes Precision Abort Stage 
  uint8_t Mode;                          //0 = stage, 1 = docking, 2 = map
  uint16_t ControlGroup;          //control groups 1-10 in 2 uint8_ts
  uint8_t AdditionalControlByte1;        //other stuff
  uint8_t AdditionalControlByte2;
  int16_t Pitch;                          //-1000 -> 1000
  int16_t Roll;                           //-1000 -> 1000
  int16_t Yaw;                            //-1000 -> 1000
  int16_t TX;                             //-1000 -> 1000
  int16_t TY;                             //-1000 -> 1000
  int16_t TZ;                             //-1000 -> 1000
  int16_t Throttle;                       //    0 -> 1000
} ControlPacket_t;

HandShakePacket_t HPacket;
VesselData_t VData;
ControlPacket_t CPacket;

#pragma pack(pop) //back to whatever the previous packing mode was
#endif