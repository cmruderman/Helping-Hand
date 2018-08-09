
// 

#define TX_DELAY_MS 5

int potPin = 1;
int pauseResumePin = 5;
int emergencyStopPin = 6;
int powerPin = 7;

int pot, pauseResume, emergencyStop, power;

void printState() {
  Serial.print("Gripper: ");
  Serial.print(pot/1024.0*100);
  Serial.print("%,  Pause/Resume: ");
  Serial.print(pauseResume);
  Serial.print(",  Emergency Stop: ");
  Serial.print(emergencyStop);
  Serial.print(",  Power: ");
  Serial.print(power);
  Serial.println();
}

void performEmergencyStop() {

  int needsReset = true;

  while (true) {
    Serial.write("!\n");

    power = digitalRead(powerPin);

    if (power == 0) {
      needsReset = false;
    } else if (power == 1 && needsReset == false) {
      return;
    }

    delay(10);
  }
  
}

void txState() {  
  Serial.write('@');
  Serial.write((byte)(pot/4));
  Serial.write(pauseResume);
  Serial.write(power);
  Serial.write('\n');
}

void setup() {

  Serial.begin(9600);
  Serial.println("Begin");

  pinMode(potPin,INPUT);
  pinMode(pauseResumePin,INPUT);
  pinMode(emergencyStopPin,INPUT_PULLUP);
  pinMode(powerPin,INPUT);

  delay(100);
}

int c=0;
void loop() {

  pot = analogRead(potPin);
  pauseResume = !digitalRead(pauseResumePin);
  emergencyStop = !digitalRead(emergencyStopPin);
  power = digitalRead(powerPin);

  if (emergencyStop) {
    performEmergencyStop();
  }


  if (c == TX_DELAY_MS) {
    c = 0;
    //printState();
    txState();
  }

  delay(1);
  c++;
}

