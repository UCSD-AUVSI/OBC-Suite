#include <Servo.h>

Servo openPilotPWM;

byte incomingByte = 0;
void setup(){
 Serial.begin(57600);
 openPilotPWM.attach(9);
 openPilotPWM.writeMicroseconds(1100);
}

void loop(){
  
 if(Serial.available() > 0){
   incomingByte = Serial.read();
   
   switch(incomingByte){
     case '0': openPilotPWM.writeMicroseconds(1100); break;
     case '1': openPilotPWM.writeMicroseconds(1300); break;
     case '2': openPilotPWM.writeMicroseconds(1500); break;
     //case '3': openPilotPWM.writeMicroseconds(1700); break;
     case '4': openPilotPWM.writeMicroseconds(1900); break;
     default: break;
   }
   
   delay(10);
 } 
}
