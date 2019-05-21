/* ====================================================================

   Copyright (c) 2019 Thorsten Godau (dl9sec). All rights reserved.


   This example is licensed under GPL-3.0-only (see LICENSE in the same
   folder as the example) due to the license of the FabGL library which
   is statically linked to the example code.

   ====================================================================*/

#include <ArduinoP13.h>
#include <fabgl.h>
#include "graphics.h"

#define MAP_MAXX    512
#define MAP_MAXY    256

#define MAP_YOFFSET 64

#define VGA_RED1    GPIO_NUM_13
#define VGA_RED0    GPIO_NUM_15
#define VGA_GREEN1  GPIO_NUM_16
#define VGA_GREEN0  GPIO_NUM_14
#define VGA_BLUE1   GPIO_NUM_21
#define VGA_BLUE0   GPIO_NUM_19
#define VGA_HSYNC   GPIO_NUM_5
#define VGA_VSYNC   GPIO_NUM_18

const char *tleName = "ISS (ZARYA)";
const char *tlel1   = "1 25544U 98067A   19132.94086806  .00001341  00000-0  28838-4 0  9999";
const char *tlel2   = "2 25544  51.6422 176.3402 0001360 345.7469  23.7758 15.52660993169782";

// For testing purpose (geostationary, no motion)
//const char *tleName = "ES'HAIL 2";
//const char *tlel1   = "1 43700U 18090A   19132.49026609  .00000132  00000-0  00000-0 0  9990";
//const char *tlel2   = "2 43700   0.0182 271.8232 0001691 166.1779 354.2495  1.00270105  1678";


const char  *pcMyName = "DL9SEC";    // Observer name
double       dMyLAT   =  48.661563;  // Latitude (Breitengrad): N -> +, S -> -
double       dMyLON   =   9.779416;  // Longitude (Längengrad): E -> +, W -> -
double       dMyALT   = 386.0;       // Altitude ASL (m)

double       dfreqRX  = 145.800;     // Nominal downlink frequency
double       dfreqTX  = 437.800;     // Nominal uplink frequency

int          iYear    = 2019;        // Set start year
int          iMonth   = 5;           // Set start month
int          iDay     = 16;          // Set start day
int          iHour    = 21;          // Set start hour
int          iMinute  = 1;           // Set start minute
int          iSecond  = 41;          // Set start second

double       dSatLAT  = 0;           // Satellite latitude
double       dSatLON  = 0;           // Satellite longitude
double       dSatAZ   = 0;           // Satellite azimuth
double       dSatEL   = 0;           // Satellite elevation

double       dSunLAT  = 0;           // Sun latitude
double       dSunLON  = 0;           // Sun longitude
double       dSunAZ   = 0;           // Sun azimuth
double       dSunEL   = 0;           // Sun elevation

int          ixQTH    = 0;           // Map pixel coordinate x of QTH
int          iyQTH    = 0;           // Map pixel coordinate y of QTH
int          ixSAT    = 0;           // Map pixel coordinate x of satellite
int          iySAT    = 0;           // Map pixel coordinate y of satellite
int          ixSUN    = 0;           // Map pixel coordinate x of sun
int          iySUN    = 0;           // Map pixel coordinate y of sun

char         acBuffer[20];            // Buffer for ASCII time

int          aiSatFP[90][2];          // Array for storing the satellite footprint map coordinates
int          aiSunFP[180][2];         // Array for storing the sunlight footprint map coordinates

void setup()
{
  int i;
  
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  delay(10);  

  P13Sun Sun;                                                       // Create object for the sun
  P13DateTime MyTime(iYear, iMonth, iDay, iHour, iMinute, iSecond); // Set start time for the prediction
  P13Observer MyQTH(pcMyName, dMyLAT, dMyLON, dMyALT);              // Set observer coordinates
  P13Satellite MySAT(tleName, tlel1, tlel2);                        // Create ISS data from TLE
 
  VGAController.begin(VGA_RED1, VGA_RED0, VGA_GREEN1, VGA_GREEN0, VGA_BLUE1, VGA_BLUE0, VGA_HSYNC, VGA_VSYNC);
  VGAController.setResolution(VGA_512x384_60Hz);
  
  Canvas.drawBitmap(0, MAP_YOFFSET, &worldmap_2_512x256);

  latlon2xy(ixQTH, iyQTH, dMyLAT, dMyLON, MAP_MAXX, MAP_MAXY);      // Get x/y for the pixel map 

  Serial.printf("\r\n\Prediction for %s at %s (MAP %dx%d: x = %d,y = %d):\r\n\r\n", MySAT.name, MyQTH.name, MAP_MAXX, MAP_MAXY, ixQTH, iyQTH);

  Canvas.selectFont(Canvas.getPresetFontInfo(40, 14));
  Canvas.setPenColor(Color::White);
  Canvas.drawTextFmt(5, 5, "Prediction for %s at", MySAT.name);
  Canvas.drawTextFmt(5, 20, "%s (Lat = %.4f%c, Lon = %.4f%c, Alt = %.1fm ASL):", MyQTH.name, dMyLAT, (char)248, dMyLON, (char)248, dMyALT);
  Canvas.drawBitmap(ixQTH-5, (iyQTH-5)+MAP_YOFFSET, &Groundstation_9x9);

  MyTime.ascii(acBuffer);             // Get time for prediction as ASCII string
  MySAT.predict(MyTime);              // Predict ISS for specific time
  MySAT.latlon(dSatLAT, dSatLON);     // Get the rectangular coordinates
  MySAT.elaz(MyQTH, dSatEL, dSatAZ);  // Get azimut and elevation for MyQTH

  latlon2xy(ixSAT, iySAT, dSatLAT, dSatLON, MAP_MAXX, MAP_MAXY);  // Get x/y for the pixel map

  Serial.printf("%s -> Lat: %.4f Lon: %.4f (MAP %dx%d: x = %d,y = %d) Az: %.2f El: %.2f\r\n\r\n", acBuffer, dSatLAT, dSatLON, MAP_MAXX, MAP_MAXY, ixSAT, iySAT, dSatAZ, dSatEL);
    
  Canvas.drawBitmap(ixSAT-8, (iySAT-8)+MAP_YOFFSET, &Satellite_15x15);

  Canvas.drawTextFmt(5, 323, "%s UTC", acBuffer);
  Canvas.drawTextFmt(5, 338, "Lat: %.2f%c Lon: %.2f%c Az: %.2f%c El: %.2f%c", dSatLAT, (char)248, dSatLON, (char)248, dSatAZ, (char)248, dSatEL, (char)248);
  
  Serial.printf("RX: %.6f MHz, TX: %.6f MHz\r\n\r\n", MySAT.doppler(dfreqRX, P13_FRX), MySAT.doppler(dfreqTX, P13_FTX));
  Canvas.drawTextFmt(5, 353, "RX: %.5f MHz", MySAT.doppler(dfreqRX, P13_FRX));
  Canvas.drawTextFmt(5, 368, "TX: %.5f MHz", MySAT.doppler(dfreqTX, P13_FTX));
  
  // Calcualte ISS footprint
  Serial.printf("Satellite footprint map coordinates:\n\r");
  
  MySAT.footprint(aiSatFP, (sizeof(aiSatFP)/sizeof(int)/2), MAP_MAXX, MAP_MAXY, dSatLAT, dSatLON);

  Canvas.setPenColor(Color::BrightRed);

  // Print ISS footprint
  for (i = 0; i < (sizeof(aiSatFP)/sizeof(int)/2); i++)
  {
    Serial.printf("%2d: x = %d, y = %d\r\n", i, aiSatFP[i][0], aiSatFP[i][1]);
    Canvas.setPixel(aiSatFP[i][0], MAP_YOFFSET+aiSatFP[i][1]);
  }

  // Predict sun
  Sun.predict(MyTime);                // Predict ISS for specific time
  Sun.latlon(dSunLAT, dSunLON);       // Get the rectangular coordinates
  Sun.elaz(MyQTH, dSunEL, dSunAZ);    // Get azimut and elevation for MyQTH

  latlon2xy(ixSUN, iySUN, dSunLAT, dSunLON, MAP_MAXX, MAP_MAXY);

  Serial.printf("\r\nSun -> Lat: %.4f Lon: %.4f (MAP %dx%d: x = %d,y = %d) Az: %.2f El: %.2f\r\n\r\n", dSunLAT, dSunLON, MAP_MAXX, MAP_MAXY, ixSUN, iySUN, dSunAZ, dSunEL);
  Canvas.drawBitmap(ixSUN-8, (iySUN-8)+MAP_YOFFSET, &Sun_15x15);

  // Calcualte sunlight footprint
  Serial.printf("Sunlight footprint map coordinates:\n\r");
  
  Sun.footprint(aiSunFP, (sizeof(aiSunFP)/sizeof(int)/2), MAP_MAXX, MAP_MAXY, dSunLAT, dSunLON);

  Canvas.setPenColor(Color::BrightYellow);

  // Print sunlight footprint
  for (i = 0; i < (sizeof(aiSunFP)/sizeof(int)/2); i++)
  {
    Serial.printf("%2d: x = %d, y = %d\r\n", i, aiSunFP[i][0], aiSunFP[i][1]);
    Canvas.setPixel(aiSunFP[i][0], MAP_YOFFSET+aiSunFP[i][1]);
  }

  Serial.printf("\r\nFinished\n\r");
  
}


void loop()
{
  // put your main code here, to run repeatedly:

}
