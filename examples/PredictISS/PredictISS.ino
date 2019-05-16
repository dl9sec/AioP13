/* ====================================================================

   Copyright (c) 2019 Thorsten Godau (dl9sec). All rights reserved.


   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.

   3. Neither the name of the author(s) nor the names of any contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.

   ====================================================================*/

#include <ArduinoP13.h>

#define MAP_MAXX   1150
#define MAP_MAXY    610

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

int          aiSatFP[32][2];          // array for storing the foorprint map coordinates


void setup()
{
  // Arduino UNO doens't support Serial.printf, so comment out those lines and put the parts for UNO in
  /* For UNO
  char buf[80]; 
  */
  int i;
  
  Serial.begin(115200);
  Serial.setDebugOutput(false);   // Comment out for UNO
  delay(10);  

  P13Sun Sun;                                                       // Create object for the sun
  P13DateTime MyTime(iYear, iMonth, iDay, iHour, iMinute, iSecond); // Set start time for the prediction
  P13Observer MyQTH(pcMyName, dMyLAT, dMyLON, dMyALT);              // Set observer coordinates

  P13Satellite MySAT(tleName, tlel1, tlel2);                        // Create ISS data from TLE

  latlon2xy(ixQTH, iyQTH, dMyLAT, dMyLON, MAP_MAXX, MAP_MAXY);      // Get x/y for the pixel map 

  /*  For UNO instead of Serial.printf
  sprintf(buf, "\r\n\Prediction for %s at %s (MAP %dx%d: x = %d,y = %d):\r\n\r\n", MySAT.name, MyQTH.name, MAP_MAXX, MAP_MAXY, ixQTH, iyQTH);
  Serial.print(buf);
  */
  Serial.printf("\r\n\Prediction for %s at %s (MAP %dx%d: x = %d,y = %d):\r\n\r\n", MySAT.name, MyQTH.name, MAP_MAXX, MAP_MAXY, ixQTH, iyQTH);

  MyTime.ascii(acBuffer);             // Get time for prediction as ASCII string
  MySAT.predict(MyTime);              // Predict ISS for specific time
  MySAT.latlon(dSatLAT, dSatLON);     // Get the rectangular coordinates
  MySAT.elaz(MyQTH, dSatEL, dSatAZ);  // Get azimut and elevation for MyQTH

  latlon2xy(ixSAT, iySAT, dSatLAT, dSatLON, MAP_MAXX, MAP_MAXY);  // Get x/y for the pixel map
  /* For UNO instead of Serial.printf
  Serial.print(acBuffer);
  Serial.print(" -> Lat: ");
  Serial.print(dSatLAT,4);
  Serial.print(" Lon: ");
  Serial.print(dSatLON,4);
  Serial.print(" (MAP ");
  Serial.print(MAP_MAXX);
  Serial.print("x");
  Serial.print(MAP_MAXY);
  Serial.print(": x = ");
  Serial.print(ixSAT);
  Serial.print(", y = ");
  Serial.print(iySAT);
  Serial.print(") Az: ");
  Serial.print(dSatAZ,2);
  Serial.print(" El: ");
  Serial.println(dSatEL,2);
  */
  Serial.printf("%s -> Lat: %.4f Lon: %.4f (MAP %dx%d: x = %d,y = %d) Az: %.2f El: %.2f\r\n\r\n", acBuffer, dSatLAT, dSatLON, MAP_MAXX, MAP_MAXY, ixSAT, iySAT, dSatAZ, dSatEL);

  /* For UNO instead of Serial.printf
  Serial.print("RX: ");
  Serial.print(MySAT.doppler(dfreqRX, P13_FRX),6);
  Serial.print(", TX: ");
  Serial.println(MySAT.doppler(dfreqTX, P13_FTX),6); Serial.println("");
  */
  Serial.printf("RX: %.6f MHz, TX: %.6f MHz\r\n\r\n", MySAT.doppler(dfreqRX, P13_FRX), MySAT.doppler(dfreqTX, P13_FTX));
 
  // Calcualte footprint
  // Serial.println("Footprint map coordinates:");
  Serial.printf("Footprint map coordinates:\n\r");
  
  MySAT.footprint(aiSatFP, (sizeof(aiSatFP)/sizeof(int)/2), MAP_MAXX, MAP_MAXY, dSatLAT, dSatLON);
  
  for (i = 0; i < (sizeof(aiSatFP)/sizeof(int)/2); i++)
  {
    /* For UNO instead of Serial.printf
    Serial.print(i);
    Serial.print(": x = ");
    Serial.print(aiSatFP[i][0]);
    Serial.print(", y = ");
    Serial.println(aiSatFP[i][1]);
    */
    Serial.printf("%2d: x = %d, y = %d\r\n", i, aiSatFP[i][0], aiSatFP[i][1]);
  }

  // Predict sun
  Sun.predict(MyTime);                // Predict ISS for specific time
  Sun.latlon(dSunLAT, dSunLON);       // Get the rectangular coordinates
  Sun.elaz(MyQTH, dSunEL, dSunAZ);    // Get azimut and elevation for MyQTH

  latlon2xy(ixSUN, iySUN, dSunLAT, dSunLON, MAP_MAXX, MAP_MAXY);

  /* For UNO instead of Serial.printf
  Serial.println("");
  Serial.print("Sun -> Lat: ");
  Serial.print(dSunLAT,4);
  Serial.print(" Lon: ");
  Serial.print(dSunLON,4);
  Serial.print(" (MAP ");
  Serial.print(MAP_MAXX);
  Serial.print("x");
  Serial.print(MAP_MAXY);
  Serial.print(": x = ");
  Serial.print(ixSUN);
  Serial.print(", y = ");
  Serial.print(iySUN);
  Serial.print(") Az: ");
  Serial.print(dSunAZ,2);
  Serial.print(" El: ");
  Serial.println(dSunEL,2);
  */
  Serial.printf("\r\nSun -> Lat: %.4f Lon: %.4f (MAP %dx%d: x = %d,y = %d) Az: %.2f El: %.2f\r\n", dSunLAT, dSunLON, MAP_MAXX, MAP_MAXY, ixSUN, iySUN, dSunAZ, dSunEL);

  // Serial.println(""); Serial.println("Finished.");
  Serial.printf("\r\nFinished\n\r");
  
}


void loop()
{
  // put your main code here, to run repeatedly:

}
