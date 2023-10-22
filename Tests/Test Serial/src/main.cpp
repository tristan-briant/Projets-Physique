#include "M5stack.h"

// HardwareSerial

void setup()
{
  M5.begin();

  Serial2.begin(100,SERIAL_8N1,16,17,true);
  Serial.begin(115200);
}

void loop()
{
  M5.update();
  char c = 0;

  if (Serial.available())
  {
    c = Serial.read();
    Serial2.print(c);
  }

  if (Serial2.available())
  {
    c = Serial2.read();
    M5.Lcd.print(c);
  }

 
  if (M5.BtnA.isPressed())
    Serial2.print("A");
  if (M5.BtnA.isPressed())
    Serial2.print("B");
  if (M5.BtnA.isPressed())
    Serial2.print("C");

  delay(10);
}
