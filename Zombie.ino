
// digital pin 2 has a pushbutton attached to it. Give it a name:

// Set up one relay to apply power to the motor
// and two to control direction by switching the 2 motor power leads between power and 
// ground in respectively opposite polarities. 
int CCMotorPower = 2;
int CCDirRelay_1 = 3;
int CCDirRelay_2 = 4;

//Project phototransistor setup
int photoTran = A7;
int reading = 0;
int lastval=0;
int initval=0;



char cmdbuf[200];
char valbuff[5];
unsigned long new_cmd_val=0;

boolean logtoserial=true;


void log_message(String message)
{
    if(logtoserial == true){
      Serial.println(message);
    }
}

void HeadUp()
{
  
        log_message("Head UP");
        digitalWrite(LED_BUILTIN, HIGH);
        // deenergize relay to connect NC contacts
        digitalWrite(CCDirRelay_1, HIGH);
        digitalWrite(CCDirRelay_2, HIGH);
        // wait a little for the direction relays to settle
        delay(250);
        // just turn on motor for 250ms second
        digitalWrite(CCMotorPower, LOW);
        delay(250); 
        digitalWrite(CCMotorPower, HIGH);
        digitalWrite(CCDirRelay_1, HIGH);
        digitalWrite(CCDirRelay_2, HIGH);
   
}

void HeadDown()
{
        log_message("Head Down");
        digitalWrite(LED_BUILTIN, LOW);
        // deenergize relays to connect NO contacts
        digitalWrite(CCDirRelay_1, LOW); 
        digitalWrite(CCDirRelay_2, LOW);
        delay(250);
        // just turn on motor for 500 ms second
        digitalWrite(CCMotorPower, LOW);
        delay(250);
        digitalWrite(CCMotorPower, HIGH);
        digitalWrite(CCDirRelay_1, HIGH);
        digitalWrite(CCDirRelay_2, HIGH);
}

void HeadCycle(){
    // init head dir
      int idir=1;
      int hdown = false;
      int trigger = false;
      int offdelay = 0;
      int dimmedval = 1;
      
      while(true){
          // sample the light trigger
          // take a current light level reading
          // force a min value of 0 so we are not multiplying by 0 below.
          reading = analogRead(photoTran);
          if(reading == 0){
            reading=1;
          }
          //Serial.println(reading);
          // if the current reading is 5 times brighter than the last reading
          // then assume the trigger light is on. 
          if(reading > (lastval*5)){
              Serial.print("on:");Serial.println(reading);
              lastval=reading;
              // if transitioning from trigger off to on then inti head)
              if(trigger == false) {
                  HeadUp();
                  delay(500);
              }
              trigger=true;
          }
          // if the current reading is 5 times dimmer than the last reading
          // then assume the trigger light is off. 
          else if(reading*5 < lastval){
             lastval=reading;
             if(trigger == true) {
                  // looks like the trigger light shut off
                  // do a sample of light to make sure
                  dimmedval = lastval;
                  // get a new sample for lastval
                  SampleLight(20);
                  // the new sample might vary but not be too much brighter
                  // than the inital dim value to be considered still off
                  Serial.print("off sample results ");Serial.print(lastval);Serial.print(":");Serial.println(dimmedval+10);
                  if(lastval <= dimmedval+10){
                      HeadDown();
                  //break;
                      trigger = false;
                      Serial.print("off:");Serial.println(lastval);
                  }
                 //}
                 //else {
                 //  Serial.print("off delay:");Serial.println(offdelay);
                 //}
              }
             
          }
          
          if(trigger == true){
                idir = random(1,3);
                Serial.println(idir);
                if(idir == 1){
                  HeadUp();
                  hdown = false;
                }
                else {
                    if (hdown == false){
                      HeadDown();
                      hdown = true;
                    }
                    else {
                      HeadUp();
                    }
                }
                //delay(random(3, 6)*250);
                
                delay(random(2, 7)*250);
              }
           // trigger is false so delay 200ms to take next trigger sample
           else {
             delay(200);
           }
      }
}

// take a 5 second sample of the current light level and use as the intial value;
void  SampleLight(int sdel){
  lastval=0;
  initval=0;
  for(int i=1; i<=10; ++i){
    reading = analogRead(photoTran);
    Serial.print(i);Serial.print(":");Serial.println(reading);
    initval = initval+ reading;
    lastval = initval/i;
    Serial.println(lastval);
    delay(sdel);
  }
  // make sure lastval is at least 1
  if(lastval <=0){
    lastval=1;
  }
}


void setup() 
{ 

  // init motor control relays
  // control pins set to High remove power from the relay coils 
  // and puts then in thier NC state
  pinMode(CCMotorPower, OUTPUT);
  digitalWrite(CCMotorPower, HIGH);
  pinMode(CCDirRelay_1, OUTPUT);
  digitalWrite(CCDirRelay_1, HIGH);
  pinMode(CCDirRelay_2, OUTPUT);
  digitalWrite(CCDirRelay_2, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);

  // opens serial port, sets data rate to 9600 bps
  Serial.begin(9600);    
  //Serial.setTimeout(time_increment);
  Serial.println("Starting Zombie Head");
  
  // setup and intialize photo transistor trigger
  pinMode(photoTran,INPUT);
  // take an initial sample of the current light level and use as the intial value;
  SampleLight(500);
  // set intial levl to atleast one so we are not multiplying by 0 in loop below.
  if(lastval == 0){
    lastval=1;
  }
} 



void loop() 
{

  String parse_cmd; // string object to parse
  int parse_eql; // index of equal sign in command string
  boolean parse_error; // parse error flag
  String cmd;
  String cmd_val;
  int bytes_read=0;
  int last_cmd_val;




  //Serial.println("checking for cmd");
  // check for a command
  if (Serial.available() > 0) {
      // read the command
      cmdbuf[0] ='\0';
      bytes_read =0;
      bytes_read=Serial.readBytesUntil('\n', cmdbuf, 200);
      //Serial.println(bytes_read);
      cmdbuf[bytes_read] ='\0';
      log_message("cmd:");
      log_message(cmdbuf);

      //Parse the command
      parse_cmd = String(cmdbuf);
      //Serial.print("cmd:");
      //Serial.println(parse_cmd);
      // first do some cleanup
      //parse_cmd.toLowerCase();
      parse_cmd.trim();

      //flag invalid commands or syntax
      parse_error = false;

      // find the command , assume the first 3 characters
      cmd = parse_cmd; //parse_cmd.substring(0,3);

    //runflag=false;
    // valid commands
    // LogSerial
    //"HeadUp"
    //"HeadDown"
    //
    if(cmd.startsWith("LogSerial")) // this sets if logging should be output to the serial port
    {
        parse_eql = cmd.indexOf("=");
        if(parse_eql > -1)
        {
            cmd_val=cmd.substring(parse_eql+1);
            cmd_val.toCharArray(valbuff, 4);
            new_cmd_val = atoi(&valbuff[0]);
            //Serial.println(new_insert_freq);
            //if(new_insert_freq> 0)
            if(new_cmd_val==1){
              logtoserial = true;
              Serial.println("Log to serial = true ");
            }
            else{
              logtoserial = false;
              Serial.println("Log to serial = false ");
            }
        
        }


    }
    // execute a head up command
    else if(cmd.startsWith("HeadUp")) 
    {
        HeadUp();
    }
    // execute a head down command
    else if(cmd.startsWith("HeadDown"))
    {
        HeadDown();
    }
    else if(cmd.startsWith("HeadCycle"))
    {
        HeadCycle();
    }
    else // just ignore the command b/c it's not recongized
    {
        parse_error = true;
    }



  }
  // not taking command via the serial port so just cycle the head
  else
  {
      HeadCycle();
  }







}



