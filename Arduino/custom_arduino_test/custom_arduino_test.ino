#include <Servo.h>
#define dirPin1  7 //dir1 elbow
#define dirPin2 8 //dir2 shoulder
#define speedPin1 5 //pwm1
#define speedPin2 6 //pwm2 10
#define clkPin 4
#define dirPinBase 12
#define servoPin 11
Servo gripper;

void setup() {
  // put your setup code here, to run once:
pinMode(dirPin2, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(speedPin1, OUTPUT);
  pinMode(speedPin2, OUTPUT);
  pinMode(dirPinBase, OUTPUT);
  Serial.begin(9600); 
  gripper.attach(11);
  gripper.write(100);
}

void loop() {
  // put your main code here, to run repeatedly:
//digitalWrite(10,HIGH);
//delay(250);
//digitalWrite(10,LOW);
//delay(250);
int a0 = analogRead(A0);
delay(10);
int a1 = analogRead(A1);
delay(10);
int a2 = analogRead(A2);
/*
delay(10);
int a3 = analogRead(A3);
delay(10);
int a4 = analogRead(A4);
delay(10);
int a5 = analogRead(A5);
delay(10);*/
Serial.print(a0);
Serial.print(" ");
Serial.print(a1);
Serial.print(" ");
Serial.println(a2);
/*
Serial.print(" ");
Serial.print(a3);
Serial.print(" ");
Serial.print(a4);
Serial.print(" ");
Serial.println(a5);
*/
}
