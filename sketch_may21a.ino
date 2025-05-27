#include "HX711.h"
 
#define SCK 5
#define DT1 16
#define DT2 17
#define DT3 18
#define DT4 33
 
HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scale4;
 
// calibration factors
// reading over known mass
#define SCALE_FACTOR_1 427.772
#define SCALE_FACTOR_2 422.633
#define SCALE_FACTOR_3 428.811
#define SCALE_FACTOR_4 445.3465
 
void setup() {
  Serial.begin(115200);
 
  scale1.begin(DT1, SCK);
  scale2.begin(DT2, SCK);
  scale3.begin(DT3, SCK);
  scale4.begin(DT4, SCK);
 
  while (!scale1.is_ready() || !scale2.is_ready() || !scale3.is_ready() || !scale4.is_ready()) {
    Serial.println("Waiting for HX711 modules...");
    delay(500);
  }
    
  scale1.tare();
  scale2.tare();
  scale3.tare();
  scale4.tare();
 
  scale1.set_scale(SCALE_FACTOR_1); // reading over known mass
  scale2.set_scale(SCALE_FACTOR_2);
  scale3.set_scale(SCALE_FACTOR_3);
  scale4.set_scale(SCALE_FACTOR_4);
 
 
  Serial.println("All scales tared and calibrated. Ready for measurements in grams.");
  
}
 
void loop() {
  float weight1 = scale1.get_units(3);
  float weight2 = scale2.get_units(3);
  float weight3 = scale3.get_units(3);
  float weight4 = scale4.get_units(3);
 
  float totalWeight = weight1 + weight2 + weight3 + weight4;
 
  Serial.print("Weight1: "); Serial.print(weight1, 2); Serial.print(" g\t");
  Serial.print("Weight2: "); Serial.print(weight2, 2); Serial.print(" g\t");
  Serial.print("Weight3: "); Serial.print(weight3, 2); Serial.print(" g\t");
  Serial.print("Weight4: "); Serial.print(weight4, 2); Serial.print(" g\t");
 
  Serial.print(" | Total Weight: ");
  Serial.print(totalWeight, 2);
  Serial.println(" g");
 
  delay(500);
}
