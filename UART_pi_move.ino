//#include <math.h>
//#include <Wire.h>

#define MOTOR1 1
#define MOTOR2 2
#define MOTOR3 3

#define CW 5
#define CCW 6

#define NOT_VALID -1

//PINS
#define MOTOR1_PIN 3
#define MOTOR2_PIN 9
#define MOTOR3_PIN 10

#define MOTOR1_DIR_PIN 11
#define MOTOR2_DIR_PIN 12
#define MOTOR3_DIR_PIN 13

//i2c states
#define STATE_WHEEL 90      //Wheel No.
#define STATE_DIR 91        //Wheel Dir
#define STATE_DC 92        //Wheel Duty Cycle

//Wheel states
#define STATE_WH1 1
#define STATE_WH2 2
#define STATE_WH3 3

/*****************************I2C_COMM***********************************
Pi -> Mini Pro
          1) 1 or 2 or 3 [Wheel No.]
          2) 5 or 6 [Wheel Dir.]
          3) 0 - 255 [Duty cycle sent has to be converted from percent]
          4) Repeat for number of wheels (i.e 3)
          5) sending 1 and then 8 at anytime triggers ASM RESET
          
            (front)
             
     (WH2)0--------0(WH1)
           |------|
            |----|
              0(WH3)
            
            (back)
          
Small delay might be needed between bytes.
*************************************************************************/

//Global Variables
int state_i2c = STATE_WHEEL;
int state_wh = STATE_WH1;

int wh1_dir = NOT_VALID;
int wh2_dir = NOT_VALID;
int wh3_dir = NOT_VALID;

int wh1_dc = NOT_VALID;
int wh2_dc = NOT_VALID;
int wh3_dc = NOT_VALID;

int prev_byte = 0;

void setup() {
  //3 Pins for PWM
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR3_PIN, OUTPUT);
  
  //3 pins for direction
  pinMode(MOTOR1_DIR_PIN, OUTPUT);
  pinMode(MOTOR2_DIR_PIN, OUTPUT);
  pinMode(MOTOR3_DIR_PIN, OUTPUT);
  
  TCCR1B = TCCR1B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz ------ for   9 and 10
  TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz ------ for 3
  
  
  //Serial for debugging
  Serial.begin(9600);           // start serial for output

}

void loop() {
  
 if (Serial.available() > 0) {
 
    // read the incoming byte:
    int x = Serial.read();
    
    if ( (prev_byte=1) && (x==8)){      //reset condition met
      //reset variables
      set_dcycle(MOTOR1, CW, 0);
      set_dcycle(MOTOR2, CW, 0);
      set_dcycle(MOTOR3, CW, 0);
      
      wh1_dir = NOT_VALID;
      wh2_dir = NOT_VALID;
      wh3_dir = NOT_VALID;
      wh1_dc = NOT_VALID;
      wh2_dc = NOT_VALID;
      wh3_dc = NOT_VALID;
      
      state_i2c = STATE_WHEEL;
      state_wh = STATE_WH1;
    }  
    else if (state_i2c==STATE_WHEEL){                                          //set wheel
      if(x>=1 && x<=3){                          
        state_wh = x;
        state_i2c++;      //go to DIR state      
      }
      else{
        //Serial.println("INVALID WHEEL NUMBER RECIEVED");
      }
    }
    else if(state_i2c==STATE_DIR){                                        //set DIR
      if(x==CW  ||  x==CCW){
         if(state_wh==STATE_WH1){
           wh1_dir = x;
         }
         else if(state_wh==STATE_WH2){
           wh2_dir = x;
         }
         else if(state_wh==STATE_WH3){
           wh3_dir = x;
         }
         state_i2c++;      //go to Duty Cycle state
      }
      else{
        //Serial.println("INVALID WHEEL DIR RECIEVED");
      }      
    }
    else if(state_i2c==STATE_DC){                                        //set duty cycle
      if(x>=0  && x<=255){
        if(state_wh==STATE_WH1){
          wh1_dc = x;
          state_wh++;          //go to STATE_WH2
        }
        else if(state_wh==STATE_WH2){
          wh2_dc = x;
          state_wh++;          //go to STATE_WH3
        }
        else if(state_wh==STATE_WH3){
          wh3_dc = x;
          state_wh = STATE_WH1;          //go to STATE_WH1
          
          //end of comm., so set wheel PWMs and reset state vars.
          set_dcycle(MOTOR1, wh1_dir, wh1_dc);
          set_dcycle(MOTOR2, wh2_dir, wh2_dc);
          set_dcycle(MOTOR3, wh3_dir, wh3_dc);
          
          wh1_dir = NOT_VALID;
          wh2_dir = NOT_VALID;
          wh3_dir = NOT_VALID;
          wh1_dc = NOT_VALID;
          wh2_dc = NOT_VALID;
          wh3_dc = NOT_VALID;
          
          Serial.println("GOT DATA");
          
        }
         state_i2c = STATE_WHEEL;
      }
      else{
        //Serial.println("INVALID WHEEL DUTY CYCLE RECIEVED");
      }  
    }
    
    prev_byte = x;
 }

}


/*motor - [MOTOR1, MOTOR2, MOTOR3]
duty_cyc - [0, 255]
dir - [CCW, CW]
*/
void set_dcycle(int motor, int dir, int duty_cyc){  

  if(motor == MOTOR1){                        //Motor 1
    if (dir == CW){
      digitalWrite(MOTOR1_DIR_PIN, LOW);
    }
    else{
      digitalWrite(MOTOR1_DIR_PIN, HIGH);
    }
    analogWrite(MOTOR1_PIN, duty_cyc);
  }
  else if(motor == MOTOR2){                  //Motor 2
    if (dir == CW){
      digitalWrite(MOTOR2_DIR_PIN, HIGH);
    }
    else{
      digitalWrite(MOTOR2_DIR_PIN, LOW);
    }
    analogWrite(MOTOR2_PIN, duty_cyc);
  }
  else if(motor == MOTOR3){                  //Motor 3
    if (dir == CW){
      digitalWrite(MOTOR3_DIR_PIN, HIGH);
    }
    else{
      digitalWrite(MOTOR3_DIR_PIN, LOW);
    }
    analogWrite(MOTOR3_PIN, duty_cyc);
  }
  else{
    //error = 1;
  }
}















