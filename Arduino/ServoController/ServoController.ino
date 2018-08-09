#include <Servo.h>

/*Servo myservo;

int pos = 0;

void setup() {
  // put your setup code here, to run once:
  myservo.attach(9, 1000, 2000);
  myservo.writeMicroseconds(1500);
}*/

const int servo1Pin = 11;
Servo servo1;

void setup() {
servo1.attach(servo1Pin, 1000, 2000);
Serial.begin(9600);
}

void loop() {
  if(Serial.available()>0)
    {
        char pos=Serial.read();
        servo1.write(pos);
        //delay(15);
    }
}
/*void loop() {
  // put your main code here, to run repeatedly:
  for (pos=0; pos<=180; pos++){
      myservo.write(pos);
      delay(15);
  }
  for (pos=0; pos>=-180; pos--){
      myservo.write(pos);
      delay(15);
  }
}*/
