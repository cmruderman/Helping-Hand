#include <Servo.h>
#define dirPin1  7 //dir1 elbow
#define dirPin2 8 //dir2 shoulder
#define speedPin1 5 //pwm1
#define speedPin2 6 //pwm2 10
#define clkPin 4
#define dirPinBase 12
#define servoPin 11
/*
 * Elbow LA: 
 *    dirPin1:  HIGH = moving down
 *              LOW  = moving up
 *    max value: 872 --extended - 884
 *    min value: 677 -- retracted - 633
 * Shoulder LA: 
 *    dirPin2:  HIGH = moving up
 *              LOW = moving down
 *    min value:190 --retracted -127 (11/9)
 *    max value: 430 --extended - 354
 */
const int potPin1 = A0;
const int potPin2 = A1;
const int potPin3 = A2;
const int SPEED = 100;
const int ELBOW_MAX = 839; //old 884
const int ELBOW_MIN = 602; // old 633
const int SHOULDER_MAX = 358; // old 354
const int SHOULDER_MIN = 198; // old 190
const int BASE_MAX = 1023;
const int BASE_MIN = 284; // old==300
const int BASE_SAFE_MIN = 349;// old==365; 
const int BASE_SAFE_MAX = 1000;

const int E_max = ELBOW_MIN;
const int E_min = ELBOW_MAX;
const int S_max = SHOULDER_MIN;
const int S_min = SHOULDER_MAX;
const int B_max = BASE_MIN;
const int B_min = BASE_MAX;


const float thetaE_max = 151.7;
const float thetaE_min = 43.7;
const float thetaE_range = thetaE_max-thetaE_min;
const float thetaS_max = 64.8;
const float thetaS_min = 28.9;
const float thetaS_range = thetaS_max-thetaS_min;
const float thetaB_min = 0;
const float thetaB_max = 180;
const float thetaB_range = thetaB_max-thetaB_min;
const int E_range = E_max-E_min;
const int S_range = S_max-S_min;
const int B_range = B_max-B_min;

float prev_B_val = 0;
int input = 0;
int goal_posE = 700;
int goal_posS = 300;
int goal_posB = 100;
int dir1_val = 1; //0 is LOW
int dir2_val = 1;
int speedE_val = SPEED;
int speedS_val = SPEED;
Servo gripper;
//float thetaS_deg = 45;
//float thetaE_deg = 45;
int gripper_max = 134;
int gripper_min = 70;
int gripper_range = gripper_max-gripper_min;
float buffer[4];
void setup() {
  
  pinMode(dirPin2, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(speedPin1, OUTPUT);
  pinMode(speedPin2, OUTPUT);
  pinMode(dirPinBase, OUTPUT);
  Serial.begin(9600);
  gripper.attach(11);
}
/*
TODO: 
 take ten pot values average value is best approx
*/

void loop() {
  //Seria l.print("here");
 
if (!Serial.available() ) {
  // Serial.println(Serial.available());
   delay(1);
   return;
 }
 
 Serial.readBytes((char*)buffer, 16);
//Serial.println("here2");

     
// Serial.println(buffer[0]);
// Serial.println(buffer[1]);
// Serial.println(buffer[2]);
// Serial.println();
 
 float base_angle = buffer[0];
 float thetaE_deg = buffer[1];
 float thetaS_deg = buffer[2];
 float pinch      = buffer[3]; // % of hand closed, from 0.0 (fully open) to 1.0 (fully closed)


 if( thetaE_deg<thetaE_max && thetaE_deg>thetaE_min && thetaS_deg<thetaS_max && thetaS_deg>thetaS_min){
     goal_posE = (thetaE_deg-thetaE_min)*(E_range/thetaE_range)+E_min;
     goal_posS = (thetaS_deg-thetaS_min)*(S_range/thetaS_range)+S_min;
     goal_posB = (base_angle-thetaB_min)*(B_range/thetaB_range)+B_min;
 }

 if (pinch > 1.0) {
  pinch = 1.0;
 } else if (pinch < 0.0){
  pinch = 0.0;
 }
 //Serial.println(pinch);
 gripper.write(pinch*gripper_range+70);
 


 //////////////////////DO NOT EDIT BELOW HERE//////////////////////////
 int posE = analogRead(potPin1);
 int posS = analogRead(potPin2);
 
 float posB = 0;
 for(int i = 0; i<10;i++){
  posB +=analogRead(potPin3);
 }
 posB/=10;
 Serial.println(posB);
 //check to make sure values are in range
 if(goal_posB<BASE_SAFE_MIN){
   goal_posB = BASE_SAFE_MIN+10;
 } else if(goal_posB>BASE_SAFE_MAX){
   goal_posB = BASE_SAFE_MAX-10;
 }
if( goal_posE < ELBOW_MIN){//ELBOW_MIN
  goal_posE = 690;
} else if(goal_posE >ELBOW_MAX){//ELBOW_MAX
  goal_posE = 720;
}

if( goal_posS < SHOULDER_MIN){
  goal_posS = SHOULDER_MIN+10;
} else if(goal_posS > SHOULDER_MAX){
  goal_posS = SHOULDER_MAX-10;
}

//////////////base rotate code///////////////
int speed_B = 3000;
float diff_B = (goal_posB-posB);
float deriv_B = 0.5*abs(diff_B-prev_B_val);

float diffB = abs(diff_B)/500;
double cB = exp(diffB)/(exp(diffB)+1);

if(cB==0){cB = 0.1;}
if(abs(diff_B)<20){
  speed_B = 1500;
}
if(abs(diff_B)<2){
  noTone(clkPin);
  speed_B = 0;
} else{
 if(diff_B>=0){
  digitalWrite(dirPinBase,HIGH);
 } else{
   digitalWrite(dirPinBase,LOW);
 }
 
 if(posB>BASE_SAFE_MAX+5 || posB<BASE_SAFE_MIN-5){
   speed_B = 0;
   noTone(clkPin);
   return;
   //delay(4000);
 }
 prev_B_val = diff_B;

 tone(clkPin,(cB*speed_B)); //speed_B is frequency in Hz
}
//////////////////////////////////////////
int diffE = abs(posE-goal_posE)/20;
double cE = exp(diffE)/(exp(diffE)+1);
if(cE == 0){cE = 0.1;}
if(cE>=255){ cE = 255;}
if(abs(posE-goal_posE)<=1){
  speedE_val = 0;
} else if( posE>goal_posE){
  dir1_val = 0;
  speedE_val = (int)255*cE;
} else if(posE<goal_posE){
  dir1_val = 1;
  speedE_val = (int)255*cE;
}
int diffS = abs(posS-goal_posS)/20;
double cS = exp(diffS)/(exp(diffS)+1);
if(cS == 0){cS = 0.1;}
if(cS>=255){ cS = 1;}

 if(abs(posS-goal_posS)<=1){
  speedS_val = 0;
} else if(abs(posS-goal_posS)<=10){
  speedS_val = 50;
} else if( posS>goal_posS){
  dir2_val = 1;
  speedS_val = (int)200*cS;
} else if(posS<goal_posS){
  dir2_val = 0;
  speedS_val = (int)200*cS;
}



 posE = analogRead(potPin1);
 posS = analogRead(potPin2);
/////////////////////DO NOT ADD CODE HERE//////////////
if(posS >= SHOULDER_MAX && dir2_val == 0){
  speedS_val = 0;
}else if(posS<=SHOULDER_MIN && dir2_val == 1){
  speedS_val = 0;
}

digitalWrite(dirPin2,dir2_val);
analogWrite(speedPin2,speedS_val);
/////////////////////DO NOT ADD CODE HERE/////////////
if(posE >= ELBOW_MAX && dir1_val == 1){
  speedE_val = 0;
}else if(posE<=ELBOW_MIN && dir1_val == 0){
  speedE_val = 0;
}
digitalWrite(dirPin1,dir1_val);
analogWrite(speedPin1,speedE_val);
///////////////DO NOT ADD CODE BELOW THIS////////////
}

