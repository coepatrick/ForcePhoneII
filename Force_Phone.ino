//This Program is designed to use outputs on the ForcePhone II prototype as well as log user input for later review. 
#include <SPI.h>
#include <SD.h>
#include <Servo.h> 

File datalog;

Servo rope;

int pos; //this keeps the servo's current position

const int push_button = 2; //push button
const int big_push_button = 12; //big purple push button
//const int pressure_pin = A3; //pressure input
//const int stretch_pin = A4; //Strech sensor input
const int heat_pin = 11; //heat output
const int vibration_pin = 5; //vibration output
const int servo_pin = 10; //servo output
int temp;  //a global temporary variable for use when needed
long timer_start; //global variable for use in a timer if needed

void setup()
{
  randomSeed(analogRead(0));
  int output_count[] = {0, 0, 0, 0, 0, 0, 0}; //keeps track of how many times each output has been used
  bool continue_experiment = true; //initializes to true, as long as this is true, the experiment goes on.
  
  set_pins();  //setting pins numbers as inputs or outputs
  all_low();   //make sure outputs are off
  rope_reset(); //makes sure rope is in loose starting position
  InitSD();  //initialize sd card, opens or creates a log file 

  new_line();//specifies a new session - this will start the column with times for this user
  save_time();           //writes time from start to the log file - this will be used often to log data

  error_code(2);// lets us know that we will be asking for an input choice
  in_or_out(); //waits for input and records if we are inside or outside.
  error_code(2);
  
  datalog.flush(); //ensure data is saved to SD card (means information is not lost in case of accidental shutdown)
    
  while(continue_experiment) //this is where the experiment begins
  { 
    
    delay(random (1000, 10000)) ; //delay between 1 and 10 seconds 

    temp = random_output(); // places number refering to output in temp
    
    datalog.print(temp); //writes the corresponding output after using it
    datalog.print(",");  // for CSV file
    save_time(); //saves start time of output

    timer_start = millis(); //saves a start time for a timer
    while ((digitalRead(big_push_button) == LOW) && (digitalRead(push_button) == LOW) && ((millis() - timer_start) < 10000)){}//while waiting for input do nothing, unless 10 seconds have passed (timeout)

    save_time(); //saves end time of output

    if(temp == 0)   //if rope was tightened, loosen it.
      rope_loosen();

    all_low();
    error_code(1); // sends a vibration feedback on completion
    datalog.flush();
    
   output_count[temp] += 1; //keeps track of how many times current output has been used
   
   for (int i = 0; i < 7; i = i + 1) { //goes through the array, if any value is under 4, we need to continue the experiment.
    if (output_count[i] < 4){
      continue_experiment = true;
      break; //breaks out of loop to continue experiment
      }
    else
      continue_experiment = false;
    }
   
  }

  save_time();
  all_low();  //shuts off all outputs
  error_code(2);  // sends  feedback to signal the program end

  datalog.close(); //saves and closes the file
}


void loop() {
  // put your main code here, to run repeatedly: 

}

int InitSD() //Initializes SD card for use and creats or opens an existing log file.
{
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10) must be left as an output or the SD library functions will not work. 
  pinMode(10, OUTPUT);

  while (!SD.begin(4)) { //Initializes the SD card for use, enabled by the CS pin (4 in this case).
    error_code(1); //if SD card is not detected, there will be an error output until card is inserted
  }

  datalog = SD.open("datalog.csv", FILE_WRITE); //Opens datalog.csv. If file does not exist it will be created
  return 1;
}


//old code, pressure and stretch inputs have been removed in order to focus on outputs.

/*bool enough_pressure() //returns a True or False based on pressure
{
  if (analogRead(pressure_pin) < 50)
    return false;
  return true;
}*/

/*bool enough_stretch() //returns a True or False based on stretch
{
  if (analogRead(stretch_pin) < 100) //need to check this value
    return false;
  return true;
}*/

int rope_tighten()
{
  rope.attach(10);
  rope.write(180); //rotate clockwise
}

int rope_loosen()
{
  rope.attach(10);
  rope.write(5);  //rotate counter clockwise
  delay(200); //give time to loosen
  rope.detach(); //stop servo
}

int rope_reset() //tries to reset rope position to a loose position, regardless of previous position
{
  rope_tighten();
  delay(1000);
  rope_loosen();
}

int all_low(){
  digitalWrite(heat_pin, LOW);
  analogWrite(vibration_pin, 0);
}

int set_pins(){
  pinMode(push_button, INPUT);
 // pinMode(pressure_pin, INPUT);
//  pinMode(stretch_pin, INPUT);
  pinMode(heat_pin, OUTPUT);
  pinMode(vibration_pin, OUTPUT);
}

int save_time(){
  datalog.print(millis());
  datalog.print(",");
}

int new_line(){
  datalog.print("\n"); 
  datalog.print("new,");
}

/* OLD - not in use, now using only one variable motor

int vibration_strength(int vibs){   //sets vibration strengh. This is controlled by 3 seperate motors which can be on or off.
  switch(vibs){
  case 0:
    digitalWrite(vibration_pin_1, LOW);
    digitalWrite(vibration_pin_2, LOW);
    digitalWrite(vibration_pin_3, LOW);
    return 0;
  case 1:
    digitalWrite(vibration_pin_1, HIGH);
    digitalWrite(vibration_pin_2, LOW);
    digitalWrite(vibration_pin_3, LOW);
    return 1;
  case 2:
    digitalWrite(vibration_pin_1, HIGH);
    digitalWrite(vibration_pin_2, HIGH);
    digitalWrite(vibration_pin_3, LOW);
    return 2;
  case 3:
    digitalWrite(vibration_pin_1, HIGH);
    digitalWrite(vibration_pin_2, HIGH);
    digitalWrite(vibration_pin_3, HIGH);
    return 3;
  }
  return 9;
}*/


int in_or_out()
{
  while(true){
    if(digitalRead(big_push_button) == HIGH){ //if the big button is pushed we are testing indoors
       datalog.print(0); //writes 0 to csv file, corresponds to indoor
       datalog.print(",");  // for CSV file
       return 0;
       }
    else if(digitalRead(push_button) == HIGH){ //if small button is pushed we are testing outdoors
       datalog.print(1); //writes 1 to csv file, corresponds to outdoors
       datalog.print(",");  // for CSV file
       return 1;
       }
    }
}
    

int random_output()
{
 // randomSeed(analogRead(0));
  switch(random(7)){
  case 0:
    rope_tighten();
    return 0;
  case 1: 
    analogWrite(vibration_pin, 85); //vibration low
    return 1;
  case 2:
    analogWrite(vibration_pin, 170); //vibration mid
    return 2;
  case 3:
    analogWrite(vibration_pin, 255); //vibration high
    return 3;
  case 4:
    analogWrite(heat_pin, 85); //heat low
    return 4;
  case 5:
    analogWrite(heat_pin, 170); //heat mid
    return 5;
  case 6:
    analogWrite(heat_pin, 255); //heat high
    return 6;
  }  
return 9;  
}

int error_code(int error_number){  //still being worked on... at the moment these codes are used arbitrarily
  switch(error_number){
    case 1: //no SD card . . .
     analogWrite(vibration_pin, 255);
     delay(500);
     analogWrite(vibration_pin, 0);
     delay(100);
     analogWrite(vibration_pin, 255);
     delay(500);
     analogWrite(vibration_pin, 0);
     delay(100);
     analogWrite(vibration_pin, 255);
     delay(500);
     analogWrite(vibration_pin, 0);
     delay(100);
     return 1;
    case 2: //End of program .-.-.
     analogWrite(vibration_pin, 255);
     delay(200);
     analogWrite(vibration_pin, 0);
     delay(100);
     analogWrite(vibration_pin, 255);
     delay(500);
     analogWrite(vibration_pin, 0);
     delay(100);
     analogWrite(vibration_pin, 255);
     delay(200);
     analogWrite(vibration_pin, 0);
     delay(100);
     analogWrite(vibration_pin, 255);
     delay(500);
     analogWrite(vibration_pin, 0);
     delay(100);
     analogWrite(vibration_pin, 255);
     delay(200);
     analogWrite(vibration_pin, 0);
     delay(100);
     return 2;
     }
      
     return 9;
  //case 0... 1... 2... error codes will be placed, here, output will be the vibration motor
}

