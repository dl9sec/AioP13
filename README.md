# ArduinoP13
Arduino Plan13 C++ library originally ported by Mark VandeWettering K6HX (https://github.com/brainwagon/angst/tree/master/P13) from the BASIC implementation "PLAN13" of J.R. Miller G3RUH (http://www.amsat.org/amsat/articles/g3ruh/111.html). Compact and modular port to smaller processors including the Atmel AVR chips.

Suggestions for implementing P13Sun::elaz() are welcome (which could be interesting, if someone wants to build e.g. a solar panel Sun tracker).

Tested with ESP32 (AZ-Delivery ESP32DevKitC) and Arduino UNO.

# Examples

<dl>
  <dt>PredictISS</dt>
  <dd>A prediction example for the ISS with output to the console (ESP32 and Arduino UNO).</dd>

  <dt>PredictISSVGAsimple</dt>
  <dd>A simple prediction example (nothing moving or dynamic, just a snapshot for a specific date/time) based on "PredictISS" for the ISS on an ESP32 with output to VGA (512x384@60Hz) using FabGL by fdivitto (https://github.com/fdivitto/fabgl). See the docs folder of the example for further information about connecting to a VGA female connector.</dd>
  
  ![alt Screenshot of PredictVGAISSsimple](https://github.com/dl9sec/ArduinoP13/raw/master/examples/PredictISSVGAsimple/docs/PredictISSVGAsimple_small.png)
    
   <dt>PredictAnalemmaVGA</dt>
  <dd>A simple analemma (https://en.wikipedia.org/wiki/Analemma) prediction example (nothing moving or dynamic) on an ESP32 with output to VGA (512x384@60Hz) using FabGL by fdivitto (https://github.com/fdivitto/fabgl) for checking the prediction algorithm for  the sun. See the docs folder of the example for further information about connecting to a VGA female connector.</dd>
  
  ![alt Screenshot of PredictAnalemmaVGA](https://github.com/dl9sec/ArduinoP13/raw/master/examples/PredictAnalemmaVGA/docs/PredictAnalemmaVGA_small.png) 
