# ArduinoP13
Arduino Plan13 C++ library originally ported by Mark VandeWettering K6HX (https://github.com/brainwagon/angst/tree/master/P13) from the BASIC implementation "PLAN13" of J.R. Miller G3RUH (http://www.amsat.org/amsat/articles/g3ruh/111.html). Compact and modular port to smaller processors including the Atmel AVR chips.

Suggestions for implementing P13Sun::elaz() are welcome.

Tested with ESP32 (AZ-Delivery ESP32DevKitC) and Arduino UNO.

# Examples

![alt Screenshot of PredictVGAISSsimple](https://github.com/dl9sec/ArduinoP13/raw/master/examples/PredictISSVGAsimple/docs/PredictISSVGAsimple_small.png)

<dl>
  <dt>PredictISS</dt>
  <dd>A prediction example for the ISS with output to the console (ESP32 and Arduino Uno).</dd>
  <dt>PredictISSVGAsimple</dt>
  <dd>A simple prediction example (nothing moving or dynamic, just a snapshot for a specific date/time) based on "PredictISS" for the ISS on a ESP32 with output to VGA using FabGL by fdivitto (https://github.com/fdivitto/fabgl). See the docs folder of the example for further information about connecting to a VGA female connector.</dd>
