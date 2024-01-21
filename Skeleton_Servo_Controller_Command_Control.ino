
#include <Servo.h> 

Servo driver_servo;  // create servo object to control a servo 
Servo passenger_servo; 
// a maximum of eight servo objects can be created 

int pos = 0;    // variable to store the servo position 

// digital pin 2 has a pushbutton attached to it. Give it a name:
int limitSwitch = 2;
int buttonState = LOW;

void setup() 
{ 
  driver_servo.attach(9);//9);  // attaches the servo on pin 9 to the servo object 
  driver_servo.write(90);
  passenger_servo.attach(10);
  passenger_servo.write(90);
  delay(5000);
  // make the pushbutton's pin an input:
  pinMode(limitSwitch, INPUT_PULLUP);
  //digitalWrite(limitSwitch, HIGH);

  // run mode active status light
  pinMode(13, OUTPUT);

  Serial.begin(9600);     // opens serial port, sets data rate to 9600 bps
  Serial.println("Starting slave");
} 


int i = 0;
boolean runflag = false;
char cmdbuf[200];
int passengerhomepos=HIGH;
void loop() 
{ 

  String parse_cmd; // string object to parse
  int parse_eql; // index of equal sign in command string
  boolean parse_error; // parse error flag
  String cmd;
  String cmd_val;
  int bytes_read=0;
  int last_cmd_val;
  int headspeed;
  int headposition;

  //Serial.println("checking for cmd");
  // check for a command
  if (Serial.available() > 0) {
    // read the command
    cmdbuf[0] ='\0';
    bytes_read =0;
    bytes_read=Serial.readBytesUntil('\n', cmdbuf, 200);
    Serial.println(bytes_read);
    cmdbuf[bytes_read] ='\0';
    Serial.print("cmd:");
    Serial.println(cmdbuf);

    //Parse the command
    parse_cmd = String(cmdbuf);
    Serial.print("cmd:");
    Serial.println(parse_cmd);
    // first do some cleanup
    //parse_cmd.toLowerCase();
    parse_cmd.trim();

    //flag invalid commands or syntax
    parse_error = false;

    // find the command , assume the first 3 characters
    cmd = parse_cmd; //parse_cmd.substring(0,3);

    //runflag=false;
    // valid commands
    //"Setup completed"
    //"Coffin cycle pre-up"
    //"Coffin cycle up"
    // "Coffin cycle up idle"
    //"Coffin cycle down"
    //"Coffin cycle idle"
    if(cmd.startsWith("Coffin cycle pre-up")) // the command means the master has started its fx cycle
    {
      runflag=true;
      randomSeed(analogRead(0));
    }
    else if(cmd.startsWith("Coffin cycle idle")) // the command means the master has finsihed its fx cycle and is an idle condition
    {
      runflag = false;
      passenger_servo.write(90); 
      delay(450); 
      driver_servo.write(90);
    }
    else // just ignore the command b/c the master is in an intermediate state
    {
      parse_error = true;
    }
    Serial.print("run flag");
    Serial.println(runflag);



  }  
  // actually run the motor if command was to do so
  if(runflag==true)
  {
    digitalWrite(13, HIGH);
    headspeed=random(93,95); //98);
    headposition=random(4,8)*100;
    passenger_servo.write(headspeed); 
    delay(headposition); 
    passenger_servo.write(90);//93); 
    delay(250);
    pos=random(30,131);
    driver_servo.write(pos);  
    delay(250);  
    headspeed=random(86,89);
    passenger_servo.write(headspeed); 
  
    passengerhomepos=HIGH;
     while(passengerhomepos==HIGH){
          passengerhomepos=digitalRead(limitSwitch);
    }
    //Serial.println("hit limit");
    passenger_servo.write(90);//93); 
    delay(250);
    pos=random(30,131);
    driver_servo.write(pos);   
    pos=random(1,4)*250;
    delay(pos);   
  }
  else
  {
    digitalWrite(13, LOW);
  }



  // this is the active control of the servos
  // if(i<= 1)
  // {
  // if( buttonState == HIGH) 
  //    for (i=0; i<= 6; i++)
  //    {


  //    }
  //  }
  //  else 
  //  {
  i=0;
  //delay(3000);
  //  }

  //limitSwitch = digitalRead();
  // if( buttonState == HIGH) 



} 







