#include <Servo.h>
#include <math.h>
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
const int ELBOW_MAX = 577;
const int ELBOW_MIN = 258;
const int SHOULDER_MAX = 396;
const int SHOULDER_MIN = 93;
const int BASE_MAX = 1092;
const int BASE_MIN = 296; // old==300
const int BASE_SAFE_MIN = BASE_MIN + 40; 
const int BASE_SAFE_MAX = BASE_MAX - 40;
const int BASE_MAX_SPEED = 4000;

const int E_max = ELBOW_MIN;
const int E_min = ELBOW_MAX;
const int S_max = SHOULDER_MIN;
const int S_min = SHOULDER_MAX;
const int B_max = BASE_MIN;
const int B_min = BASE_MAX;


// E min 52.5deg, 813...  max 147deg, 600 
// S min 65deg, 198...  max 33deg, 370
// B 0 deg, 998... 180deg 285

// New calibration after adding the VCC switch
// B 180deg 279, 0deg  1023
// S 78deg  139, 30deg 357
// E 147deg 574, 65deg 777

// New calibration after arm redesign
// S: 97deg 110, 27.7deg, 410
// E: 142deg 275, 27deg 611 

// S: 97deg 102,  24deg   396
// E: 140deg 258, 23.5deg 577

const float thetaE_max = 140;
const float thetaE_min = 23.5;
const float thetaE_range = thetaE_max-thetaE_min;
const float thetaS_max = 103;
const float thetaS_min = 24;
const float thetaS_range = thetaS_max-thetaS_min;
const float thetaB_min = 0;
const float thetaB_max = 180;
const float thetaB_range = thetaB_max-thetaB_min;
const int E_range = E_max-E_min;
const int S_range = S_max-S_min;
const int B_range = B_max-B_min;

const float T = 0.05; // 50ms

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
int gripper_min = 30;
int gripper_range = gripper_max-gripper_min;
float buffer[4];
int posB;
float error_B, speed_B;

void setup() {
  
  pinMode(dirPin2, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(speedPin1, OUTPUT);
  pinMode(speedPin2, OUTPUT);
  pinMode(dirPinBase, OUTPUT);
  Serial.begin(9600); 
  gripper.attach(11);
  
  // initialize previous value for derivative
  error_B = 0;
  speed_B = 0;
}
/*
TODO: 
 take ten pot values average value is best approx
*/

void loop() {
  
 
 if (!Serial.available() ) {
   return;
 }
 Serial.readBytes((char*)buffer, 16);

     
// Serial.println(buffer[0]);
// Serial.println(buffer[1]);
// Serial.println(buffer[2]);
// Serial.println();
 
 float base_angle = buffer[0];
 float thetaE_deg = buffer[1];
 float thetaS_deg = buffer[2];
 float pinch      = buffer[3]; // % of hand closed, from 0.0 (fully open) to 1.0 (fully closed)

 if (pinch > 1.0) {
  pinch = 1.0;
 } else if (pinch < 0.0){
  pinch = 0.0;
 }

 gripper.write(pinch*gripper_range+gripper_min);


 // Emergency Stop
 if (isnan(base_angle) || isnan(thetaE_deg) || isnan(thetaS_deg)) {
   noTone(clkPin);
   analogWrite(speedPin1,0);
   analogWrite(speedPin2,0);
   return;
 }

 if( thetaE_deg<thetaE_max && thetaE_deg>thetaE_min && thetaS_deg<thetaS_max && thetaS_deg>thetaS_min){
     goal_posE = (thetaE_deg-thetaE_min)*(E_range/thetaE_range)+E_min;
     goal_posS = (thetaS_deg-thetaS_min)*(S_range/thetaS_range)+S_min;
     goal_posB = (base_angle-thetaB_min)*(B_range/thetaB_range)+B_min;
 }

 //////////////////////DO NOT EDIT BELOW HERE//////////////////////////
 int posE = analogRead(potPin1);
 int posS = analogRead(potPin2);
 
// float posB = 0;
// for(int i = 0; i<10;i++){
//  posB +=analogRead(potPin3);
// }
// posB/=10;
// 
// float posE = 0;
// for(int i = 0; i<10;i++){
//  posE +=analogRead(potPin1);
// }
// posE/=10;
//  
// float posS = 0;
// for(int i = 0; i<10;i++){
//  posS +=analogRead(potPin2);
// }
// posS/=10;
// Serial.println(posB);

 float posB_prev = posB;
 posB = analogRead(potPin3);
 

 //check to make sure values are in range
 if(goal_posB<BASE_SAFE_MIN){
   goal_posB = BASE_SAFE_MIN+10;
 } else if(goal_posB>BASE_SAFE_MAX){
   goal_posB = BASE_SAFE_MAX-10;
 }
if( goal_posE < ELBOW_MIN){//ELBOW_MIN
  goal_posE = ELBOW_MIN+10;
} else if(goal_posE >ELBOW_MAX){//ELBOW_MAX
  goal_posE = ELBOW_MAX-10;
}

if( goal_posS < SHOULDER_MIN){
  goal_posS = SHOULDER_MIN+10;
} else if(goal_posS > SHOULDER_MAX){
  goal_posS = SHOULDER_MAX-10;
}

//////////////base rotate code///////////////
int Kp = 30;
int Kd = 0;
float Ki = 0;//500;

int K = 65;
float a = 1.0/525.0;
float b = 1.0/125.0;

float error_B_prev = error_B;
error_B = (goal_posB-posB);


// In our case, we are measuring position but outputing velocity,
// therefore, our system has an implicit integrator.
// We are implementing a PI controller, so the I component is given 
// directly from the error and the P component is given by the derivative.


//float derivative_B = (error_B - error_B_prev) / T;

//speed_B = Kd*derivative_B + Kp*error_B; + Ki*(speed_B + error_B*T); // PID

speed_B = speed_B + T*( K*( (error_B-error_B_prev)/T + a*error_B) - b*speed_B );


int speed_B_abs = abs(speed_B);

if (speed_B_abs > BASE_MAX_SPEED) {
  speed_B_abs = BASE_MAX_SPEED;
}


///////////////////////
/*static unsigned int tick = 0;
tick++;
if (tick < 50)
  speed_B_abs = 2000;
else if (tick < 100)
  speed_B_abs = 4000;
else 
  speed_B_abs = 0;


Serial.println(posB);*/
///////////////////////


if(speed_B_abs<2){
  noTone(clkPin);
  speed_B_abs = 0;
} else{
 if(speed_B>=0){
  digitalWrite(dirPinBase,HIGH);
 } else{
   digitalWrite(dirPinBase,LOW);
 }
 if(posB>BASE_SAFE_MAX+5 || posB<BASE_SAFE_MIN-5){
   speed_B_abs = 0;
   noTone(clkPin);
   //return;
   delay(4000);
 }

 tone(clkPin,speed_B_abs); //speed_B is frequency in Hz
}
//////////////////////////////////////////
int diffE = abs(posE-goal_posE)/20;
double cE = exp(diffE)/(exp(diffE)+1);
//int Kp_e = 3;
//double cE = Kp_e * diffE;

if(cE == 0){cE = 0.1;}
if(cE>=255){ cE = 255;}
if(abs(posE-goal_posE)<=1){
  speedE_val = 0;
} else if(abs(posE-goal_posE)<=10) {
  speedE_val = 5;
} else if( posE>goal_posE){
  dir1_val = 0;
  speedE_val = (int)255*cE;
} else if(posE<goal_posE){
  dir1_val = 1;
  speedE_val = (int)255*cE;
}

int diffS = abs(posS-goal_posS)/20;
double cS = exp(diffS)/(exp(diffS)+1);
//int Kp_s = 3;
//double cS = Kp_s * diffS;

if(cS == 0){cS = 0.1;}
if(cS>=255){ cS = 1;}

 if(abs(posS-goal_posS)<=1){
  speedS_val = 0;
} else if(abs(posS-goal_posS)<=10){
  speedS_val = 5;
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

