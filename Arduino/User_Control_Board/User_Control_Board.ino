const int button_one = 7;    
const int button_two = 6;   
const int button_three = 5;     
const int button_four = 4;   

const int led_one =  2;    
const int led_two =  1;   
const int led_three =  0;   
const int led_four =  3;     

// variables will change:
bool buttonState_one = false;         // variable for reading the pushbutton status
bool buttonState_two = false;         // variable for reading the pushbutton status
bool buttonState_three = false;         // variable for reading the pushbutton status
bool buttonState_four = false;         // variable for reading the pushbutton status


void setup() {
  // initialize the LED pin as an output:
  pinMode(led_one, OUTPUT);
  pinMode(led_two, OUTPUT);
  pinMode(led_three, OUTPUT);
  pinMode(led_four, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(button_one, INPUT);
  pinMode(button_two, INPUT);
  pinMode(button_three, INPUT);
  pinMode(button_four, INPUT);
  digitalWrite(led_one, LOW);
  digitalWrite(led_two, LOW);
  digitalWrite(led_three, LOW);
  digitalWrite(led_four, LOW);
}

void loop() {
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (digitalRead(button_one) == LOW ) {
    buttonState_one=!buttonState_one;
    if(buttonState_one)
      digitalWrite(led_one, LOW);
    else
      digitalWrite(led_one, HIGH);
  } 
  if (digitalRead(button_two) == LOW) {
    buttonState_two=!buttonState_two;
    if(buttonState_two)
      digitalWrite(led_two, LOW);
    else
      digitalWrite(led_two, HIGH);
  } 
  if (digitalRead(button_three) == LOW) {
    buttonState_three=!buttonState_three;
    if(buttonState_three)
      digitalWrite(led_three, LOW);
    else
      digitalWrite(led_three, HIGH);
  }
  if (digitalRead(button_four) == LOW) {
    buttonState_four=!buttonState_four;
    if(buttonState_four)
      digitalWrite(led_four, LOW);
    else
      digitalWrite(led_four, HIGH);
  } 
}
