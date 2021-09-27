/* ====================================================================

   Copyright (c) 2019 Thorsten Godau (dl9sec). All rights reserved.


   This example is licensed under GPL-3.0-only (see LICENSE in the same
   folder as the example) due to the license of the FabGL library which
   is statically linked to the example code.

   ====================================================================*/

// Used to check the sun's prediction algorithm.
// See https://en.wikipedia.org/wiki/Analemma for a detailed description
// of what an analemma is.


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

int          iYear    = 2019;        // Set start year
int          iMonth   = 1;           // Set start month
int          iDay     = 1;           // Set start day
int          iHour    = 12;          // Set start hour
int          iMinute  = 0;           // Set start minute
int          iSecond  = 0;           // Set start second

double       dSunLAT  = 0;           // Sun latitude
double       dSunLON  = 0;           // Sun longitude

int          ixSUN    = 0;           // Map pixel coordinate x of sun
int          iySUN    = 0;           // Map pixel coordinate y of sun

                            // -, J , F , M , A , M , J , J , A , S , O , N , D
uint8_t      dayspermonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


void setup()
{
  int i;

  uint8_t u8month, u8day;
  
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  delay(10);  

  P13Sun Sun;                                                       // Create object for the sun
  P13DateTime MyTime(iYear, iMonth, iDay, iHour, iMinute, iSecond); // Set start time for the prediction
   
  VGAController.begin(VGA_RED1, VGA_RED0, VGA_GREEN1, VGA_GREEN0, VGA_BLUE1, VGA_BLUE0, VGA_HSYNC, VGA_VSYNC);
  VGAController.setResolution(VGA_512x384_60Hz);

  Canvas.selectFont(Canvas.getPresetFontInfo(40, 14));
  Canvas.drawBitmap(0, MAP_YOFFSET, &worldmap_2_512x256);

  Canvas.setPenColor(Color::Blue);
  
  // Draw Greenwich prime meridian
  Canvas.drawLine(MAP_MAXX/2, MAP_YOFFSET, MAP_MAXX/2, MAP_MAXY+MAP_YOFFSET);
  // Draw equator
  Canvas.drawLine(0, MAP_MAXY/2+MAP_YOFFSET, MAP_MAXX, MAP_MAXY/2+MAP_YOFFSET);

  Canvas.setPenColor(Color::Green);
  Canvas.drawTextFmt(157, 48, "Greenwich prime meridian");
  Canvas.drawTextFmt(2, 177, "Equator");  

  Serial.printf("\r\n\Analemma prediction\r\n\r\n");

  Canvas.setPenColor(Color::White);
  Canvas.drawTextFmt(5, 20, "Analemma prediction for year %d at %02d:%02d:%02d o'clock:", iYear, iHour, iMinute, iSecond);
  
  Canvas.setPenColor(Color::BrightRed);

  for ( u8month = 1; u8month < 13; u8month++ )
  {
  
    for ( u8day = 1; u8day <= dayspermonth[u8month]; u8day++ )
    {
      // Set time
      MyTime.settime(iYear, (int)u8month, (int)u8day, iHour, iMinute, iSecond);
      
      // Predict sun
      Sun.predict(MyTime);                // Predict sun for specific time
      Sun.latlon(dSunLAT, dSunLON);       // Get the rectangular coordinates
    
      latlon2xy(ixSUN, iySUN, dSunLAT, dSunLON, MAP_MAXX, MAP_MAXY);

      Serial.printf("%4d-%02d-%02d %02d:%02d:%02d -> Lat: %.4f Lon: %.4f (MAP %dx%d: x = %d,y = %d)\r\n",iYear, u8month, u8day, iHour, iMinute, iSecond, dSunLAT, dSunLON, MAP_MAXX, MAP_MAXY, ixSUN, iySUN);
   
      if ( u8month == 12 && u8day == 31 )
      {
        // Draw the sun icon for the last point...
        Canvas.drawBitmap(ixSUN-8, (iySUN-8)+MAP_YOFFSET, &Sun_15x15);
      }
      else
      {
        // ...and pixels for any other
        Canvas.setPixel(ixSUN, iySUN+MAP_YOFFSET);
      }

    }
  }

  Serial.printf("\r\nFinished\n\r");
  
}


void loop()
{
  // put your main code here, to run repeatedly:

}
