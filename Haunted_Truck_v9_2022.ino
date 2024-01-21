// Changes for Version 8
// Updated 10/2019 to use the new serially controlled sound module
// Changes for Version 9
// Updated 10/29/22 to use contant on for headlights instead of fast random

//Effects I/O
//  Inside Cab black light - 120V  - constant on
//  Coffin Lights - 120V - slow random 
//  Headlights - 12 volt - - constant on  (10/29/22 changed from fast random)
//  Lightening - 120V  - fast random
//  Sound effects - momentary on  - self powered 
//  Fog - self powered - 120V - cycle 3 or 4 seconds on.
//  motion detector - input - starts effects

// define digital I/O pins
// outputs
const byte cab_lights = 11;    // 120V relay 8
const byte coffin_lights = 10;   // 120V relay 7
const byte lightening = 9;     // 120V relay 6
const byte fog_trigger = 8;  // N.O. Switch - self powered 120 V relay 5
const byte coffin_power = 7;     // 12 V relay 4
const byte coffin_direction = 6;    // 12 V relay 3
const byte headlights = 5;     // 12 V relay 2  
const byte unused_relay_1= 4;   //  relay 1 not currently used
// inputs
const byte fx_trigger_input = 12; // PIR input
const byte unused_digital_input = 3;     //  digital input with pullup - currently unused

// define field positions - number of bits to shift to access the fields in the word thst holds the channel info
const byte pin_bit = 0; // start position of the I/O pin
const byte mode_bit = 4; // start position of the mode field
const byte onoff_state_bit = 7; // pin on/off state field
const byte timer_state_bit = 9;  // pin timer - used to track how long the pin is in it's currenton/off state
// define bitmaps
const unsigned long w_pin = 0x0000000F;   // pin = bits 0-3
const unsigned long w_mode = 0x00000070;  // mode = bits 4-6
const unsigned long w_onoff_state = 0x00000180; // on/off state bit 7
const unsigned long w_timer_state = 0xFFFFFE00; // time in current state in ms bits 8-32
// define the modes of operation defined in the mode bits
const byte mode_const_on = 0; 
const byte mode_momentary_on = 1;
const byte mode_fast_random = 2;
const byte mode_slow_random = 3;
const byte mode_fixed_cycle = 4;
const byte mode_interupt = 5;
// define FX relay on/off states - these are reversed b/c the relay board is acitve low
#define CHANNEL_RELAY_OFF HIGH
#define CHANNEL_RELAY_ON LOW
// define the various channel states
const byte CHANNEL_OFF = 0;
const byte CHANNEL_ON = 1;
const byte CHANNEL_DISABLE =3;


// define coffin motor direction states - specific to up/down linear motion
#define COFFIN_UP LOW
#define COFFIN_DOWN HIGH
// define coffin open/close cycle states and assocated delays
const byte COFFIN_CYCLE_IDLE = 0;
const byte COFFIN_CYCLE_PRE_SOUND =1;
const byte COFFIN_CYCLE_PRE_UP =2;
const byte COFFIN_CYCLE_UP = 3;
const byte COFFIN_CYCLE_IDLE_UP = 4;
const byte COFFIN_CYCLE_DOWN = 5;
const byte COFFIN_CYCLE_POST_SOUND =6;
const word COFFIN_PRE_UP_DELAY = 10000;
const word COFFIN_STATE_CHANGE_DELAY = 2000;
const word COFFIN_UP_IDLE_DELAY = 5000;


// define global channel control array and
// initialize the static settings of output pin # and the fx pin mode
unsigned long FX_channels[] = {
  fog_trigger | ( mode_fixed_cycle << mode_bit),
  unused_relay_1 | ( mode_momentary_on << mode_bit),
  lightening | (mode_fast_random << mode_bit) ,
  cab_lights | (mode_const_on << mode_bit) ,
  coffin_lights | (mode_slow_random <<mode_bit) ,
  headlights  | (mode_const_on << mode_bit) ,   // (10/29/22 changed from fast random)
  coffin_direction | ( mode_interupt << mode_bit),
  coffin_power | ( mode_interupt << mode_bit)
  };
  // number of channels in the array
  int FX_channels_max=sizeof(FX_channels)/sizeof(unsigned long) - 1;

// time between channel state updates when running and other FX delays
const word FX_CHANNEL_UPDATE_TIME = 125;
const word FX_CHANNEL_MOMENTARY_ON_TIME = 500;
const word FX_CHANNEL_FAST_RANDOM_BASE_TIME = 250;
const word FX_CHANNEL_SLOW_RANDOM_BASE_TIME = 250;
const unsigned int FX_CHANNEL_FIXED_CYCLE_ON_TIME = 20000;
const unsigned int FX_CHANNEL_FIXED_CYCLE_OFF_TIME = 30000;
const long int FX_INTERCYCLE_DELAY = 5000; //60000;
const unsigned int MOTOR_RUN_TIME = 30000; // 2016 - replace limit switches with a timer


// ******AUDI Definitions for the audio player ********
#include <SoftwareSerial.h>

#define ARDUINO_RX 2// RX is digital pin 2 - connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 13// TX is digital pin 13 - connect to RX of the module
SoftwareSerial audioSerial(ARDUINO_RX, ARDUINO_TX); // create a serial port to control the audio player

static int8_t Send_buf[8] = {0} ;

#define CMD_PLAY_W_INDEX 0X03
#define CMD_SET_VOLUME 0X06
#define CMD_SEL_DEV 0X09
#define DEV_TF 0X02
#define CMD_PLAY 0X0D
#define CMD_PAUSE 0X0E
#define CMD_SINGLE_CYCLE 0X19
#define SINGLE_CYCLE_ON 0X00
#define SINGLE_CYCLE_OFF 0X01
#define CMD_PLAY_W_VOL 0X22
// ********* END AUDIO DEFINTIONS *********

void setup()  { 
  pinMode(cab_lights, OUTPUT); 
  pinMode(headlights, OUTPUT); 
  pinMode(coffin_lights, OUTPUT); 
  pinMode(lightening, OUTPUT); 
  pinMode(coffin_power, OUTPUT); 
  pinMode(coffin_direction, OUTPUT); 
  pinMode(unused_relay_1, OUTPUT); 
  pinMode(fog_trigger, OUTPUT); 
  pinMode(fx_trigger_input, INPUT);
  pinMode(unused_digital_input, INPUT);

  //pinMode(13, OUTPUT);

  // reset all channels to be inactive
  reset_fx_channels();

  // start the hardware serial port - used to send commands to the skeleton controller
  Serial.begin(9600);
  Serial.println("Setup completed");
  
  // start the software serial port - used to send commands to the audio player
  audioSerial.begin(9600);
  delay(500);//Wait chip initialization is complete
  // initialize the audio player to use the SD card as the audio file system
  sendAudioCommand(CMD_SEL_DEV, DEV_TF);//select the TF card  
  delay(200);//wait for 200ms

  // delay some time (15 seconds)  to let the PIR stabilze so the FX cycle does not start when booting
  delay(15000);
}


void sendAudioCommand(int8_t command, int16_t dat)
{
  delay(20);
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0xff; //version
  Send_buf[2] = 0x06; //the number of bytes of the command without starting byte and ending byte
  Send_buf[3] = command; //
  Send_buf[4] = 0x00;//0x00 = no feedback, 0x01 = feedback
  Send_buf[5] = (int8_t)(dat >> 8);//datah
  Send_buf[6] = (int8_t)(dat); //datal
  Send_buf[7] = 0xef; //ending byte

  for(uint8_t i=0; i<8; i++)//
  {
    audioSerial.write(Send_buf[i]) ;
  }
}


// initialize cycle control variables befor starting the main evaluation loop
byte coffin_cycle = COFFIN_CYCLE_IDLE; // current coffin up/down state
bool cycle_on = false;  // effects cycle on/off flag
int PIR_State = LOW;    // PIR input state
int limit_switch_State = LOW; // up/down limit switch - this var is obsolete with 2016 change to use a linear actuator for the coffin lid, keep it so old logic still works
int cycle_off_delay_time = 0;     // cycle off delay timer - used to complete sound switch momentary on at end of cycle
long cycle_on_delay_time = 0;      // cycle on delay timer - used to delay the checking of the limit switches when the coffin_cycle chnages to up or down
// this is needed because the limit switches my be closed at the start of the cycle but will open once the motor starts
// in the up/down direction
long motor_timer = 0; // 2016 - replace limit switches with a timer
void loop()  { 

  if ( cycle_on_delay_time > 0) {
    //Serial.print("next cycle count down:  ");
    //Serial.println(cycle_on_delay_time);
    // cycle delay time is set (non-zero) so update it until it's 0 or less
    // this will delay the start of the next FX cycle
    cycle_on_delay_time = cycle_on_delay_time - FX_CHANNEL_UPDATE_TIME;
    PIR_State = LOW;
  }
  else {
    // the inter-FX cycle delay time has expired so start
    // reading the PIR Input and if it's active start effect cycle
    PIR_State = digitalRead(fx_trigger_input);
    //Serial.print("PIR:  ");
    //Serial.println(PIR_State);
    //digitalWrite(13, LOW);
    //delay(20);
    // Serial.print("A1 POT Value:  ");
    //Serial.println(analogRead(A1));
    // Serial.print("A0 POT Value:  ");
    // Serial.println(analogRead(A0));

  }


  // fx cycle is not currently active
  if( cycle_on == false){
    // check if the PIR has detected motion if it is, the input is HIGH:
    if (PIR_State == HIGH) {  
      // PIR is high and the time between cycles has expired so start up the channel effects cycle 
      cycle_on = true;  
      PIR_State == LOW;
      cycle_off_delay_time =0;
      limit_switch_State = HIGH; //2016  replaced coffin limit switches with linear actuator but keep ths var so old logic still works
      // turn sound on
      //set_fx_channel_timer(&FX_channels[1], FX_CHANNEL_MOMENTARY_ON_TIME);  
      coffin_cycle = COFFIN_CYCLE_PRE_SOUND;
      cycle_on_delay_time = 16000;
      // turn only sound on
      disable_fx_channels();
      //set_fx_channel_onoff(&FX_channels[1], CHANNEL_OFF);
      //set_fx_channel_timer(&FX_channels[1], FX_CHANNEL_MOMENTARY_ON_TIME);
      
     // Play the audio track with the selected volume VVTT VV=volume 0 to 30, TT= the slected track 1-256
      sendAudioCommand(CMD_PLAY_W_VOL, 0X0F01); //play at volume 30 (0x1E00) the first sound file (0x01) = 0x1E01
      Serial.println("Coffin cycle pre-sound");
    }
    else if (cycle_off_delay_time >= 0) {
      //Serial.print("cycle delay: ");
      //Serial.println(cycle_off_delay_time);

      // the cycle is off because the coffin up/down cycle completed
      // but still have to update the channels to expire the sound channel 
      // momentary on time , which has been set below while the cycle was on but entering the coffin idle state to turn the sound off
      // this evaluation will continue until the cycle off delay time expires
      cycle_off_delay_time = cycle_off_delay_time - FX_CHANNEL_UPDATE_TIME;
      //  fx cycle is on, so continually update the states of the fx channels and wait delay time
      update_fx_channel_outputs();
      //delay(FX_CHANNEL_UPDATE_TIME);
    }
    else if (cycle_off_delay_time < 0) {
      // cycle is fully complete so reset all channel effects
      //Serial.println("reset_channels");
      reset_fx_channels();
    } 
  }
  else if (cycle_on == true) {
    if ( motor_timer <= 0 ) { //2016  replaced limit switches with a motor run timer for coffin linear actuator operation
      limit_switch_State = LOW; //2016  replaced limit switches with timer so just set this var to low so old logic below will still work
    }
    else {
      motor_timer = motor_timer - FX_CHANNEL_UPDATE_TIME;
      limit_switch_State = HIGH; //2016  replace limit switches with timer so just set this var to high so old logic below will still work
    }
    if (cycle_on_delay_time <= 0 && coffin_cycle == COFFIN_CYCLE_IDLE){
      // the coffin has gone through its up/down cycle and is now closed in the idle state position
      // turn the fx cycle off 
      cycle_on = false;
      // turn sound off and set the fx channel evaluation to continue from now
      // until the momentary on time of the sound channel
      //set_fx_channel_timer(&FX_channels[1], FX_CHANNEL_MOMENTARY_ON_TIME); 
      cycle_off_delay_time = FX_CHANNEL_MOMENTARY_ON_TIME;
      // set the time until the next FX cycle can start again
      // this uses the POT attached to Analog pin 1 
      // to calcualte an adjustable FX cycle delay 
      int av = analogRead(A1);
      Serial.print(av);
      Serial.print("-");
      cycle_on_delay_time = av*60; //FX_INTERCYCLE_DELAY;
      Serial.println(cycle_on_delay_time);
      //digitalWrite(13, HIGH);
      //Serial.print("cycle on delay time");
      //Serial.println(cycle_on_delay_time);
    }
    else if(cycle_on_delay_time <= 0 && coffin_cycle == COFFIN_CYCLE_PRE_SOUND) {
      // put the coffin into its pre-up state which
      // starts all effects but delays the start of the coffin up for 
      // the specified time;
      // this gets all the effects going and people looking before the
      // main attraction of the coffin opening and closing
      coffin_cycle = COFFIN_CYCLE_PRE_UP;
      cycle_on_delay_time = COFFIN_PRE_UP_DELAY;
      reset_fx_channels(); // if any channel has been previously disabled, this will enable all channels to be evaluated going forward
      Serial.println("Coffin cycle pre-up");
    }
    else if(cycle_on_delay_time <= 0 && coffin_cycle == COFFIN_CYCLE_PRE_UP){
      // start the coffin up cycle -> update the coffin state, and turn the motor on in the up direction
      coffin_cycle = COFFIN_CYCLE_UP;
      motor_timer = MOTOR_RUN_TIME; // 2016 - replace limit switches with a timer
      set_fx_motor_direction(&FX_channels[6],COFFIN_UP);
      set_fx_motor_onoff(&FX_channels[7],CHANNEL_ON);
      cycle_on_delay_time = COFFIN_STATE_CHANGE_DELAY;
      Serial.println("Coffin cycle up");
    }
    else if(cycle_on_delay_time <= 0 && coffin_cycle == COFFIN_CYCLE_UP && limit_switch_State== LOW) {
      // when the up limit switch is closed then change the coffin state to fully up and motor idle
      Serial.println("Coffin cycle up idle");
      // update the coffin state to fully up and motor idle
      // also set the delay time until the coffin state can transition from up idle to down cycle state
      coffin_cycle = COFFIN_CYCLE_IDLE_UP;
      set_fx_motor_onoff(&FX_channels[7],CHANNEL_OFF); 
      cycle_on_delay_time = COFFIN_UP_IDLE_DELAY;
    }
    else if(cycle_on_delay_time <= 0 && coffin_cycle == COFFIN_CYCLE_IDLE_UP) { // && limit_switch_State== LOW) {
      // The coffin has been in the fully up/motor idle state for the cycle_on_delay time so
      // transition the coffin state to down 
      Serial.println("Coffin cycle down");
      // update the coffin state and  start the down cycle
      coffin_cycle = COFFIN_CYCLE_DOWN;
      set_fx_motor_direction(&FX_channels[6],COFFIN_DOWN);
      set_fx_motor_onoff(&FX_channels[7],CHANNEL_ON); 
      motor_timer = MOTOR_RUN_TIME; // 2016 - replace limit switches with a timer
      // set the delay time until the next state (coffin down) can start checking the limit switches
      // in this case it will give the up limit switch time to open before the limit switches are checked again
      // once the motor starts on the down cycle and the up limit switch opens then the next limit switch
      // closure will be for the down limit switch
      cycle_on_delay_time = COFFIN_STATE_CHANGE_DELAY;
    }
    else if(cycle_on_delay_time <= 0 && coffin_cycle == COFFIN_CYCLE_DOWN && limit_switch_State== LOW){
      // when the down limit switch closes then shut the coffin motor off 
      // and update the coffin cycle state to idle

      set_fx_motor_onoff(&FX_channels[7],CHANNEL_OFF); 
      coffin_cycle = COFFIN_CYCLE_POST_SOUND;
      // this uses the POT attached to Analog pin 0 
      // to calcualte an adjustable cycle delay for the post motor up down effects (0 to 30.5 seconds);
      cycle_on_delay_time=analogRead(A0)*30; 
      Serial.println(cycle_on_delay_time);
      //cycle_on_delay_time=10000;
      Serial.println("Coffin cycle post-sound");
    }
    else if(cycle_on_delay_time <= 0 && coffin_cycle == COFFIN_CYCLE_POST_SOUND){
      // this state continues the effects after the motor up/down cycle is finished 
      // and update the coffin cycle state to idle
      coffin_cycle = COFFIN_CYCLE_IDLE;
      Serial.println("Coffin cycle idle");
    }


    // continually update the states of the fx channels and wait delay time
    update_fx_channel_outputs();
    //delay(FX_CHANNEL_UPDATE_TIME);

  }
  delay(FX_CHANNEL_UPDATE_TIME);



}

// reset all channels to be inactive
void reset_fx_channels()
{
  int channel;
  for(int curr_channel = 0 ; curr_channel <= FX_channels_max; curr_channel++) { 
    //first extract the pin's state
    channel = get_fx_channel(FX_channels[curr_channel]); // & w_pin) >> pin_bit;
    // then turn the pin off
    digitalWrite(channel, CHANNEL_RELAY_OFF); 
    // next reset every channels state in the control array
    // set channel timer to 0 and on/off state to off
    set_fx_channel_timer(&FX_channels[curr_channel], 0);
    set_fx_channel_onoff(&FX_channels[curr_channel], CHANNEL_OFF);

  }
}

// disable all channels from having thier effects being evaluated
void disable_fx_channels()
{
  int channel;
  for(int curr_channel = 0 ; curr_channel <= FX_channels_max; curr_channel++) { 
    //first extract the pin's state
    channel = get_fx_channel(FX_channels[curr_channel]); // & w_pin) >> pin_bit;
    // then turn the pin off
    digitalWrite(channel, CHANNEL_RELAY_OFF); 
    // next reset every channels state in the control array
    // set channel timer to 0 and on/off state to off
    set_fx_channel_timer(&FX_channels[curr_channel], 0);
    set_fx_channel_onoff(&FX_channels[curr_channel], CHANNEL_DISABLE);

  }
}

// get the channel from the bit map
word get_fx_channel(unsigned long FX_channel){
  word channel = (FX_channel & w_pin) >> pin_bit;
  return(channel); 
}

// get the mode from the bit map
word get_fx_channel_mode(word FX_channel) {
  word mode = (FX_channel & w_mode)>> mode_bit;
  return(mode); 
}

// set the channel timer in the bit map; time is in ms
void set_fx_channel_timer(unsigned long *FX_channel, unsigned long timer_val){
  //Serial.print("set_fx_channel_timer: ");
  //Serial.println(timer_val,HEX);


  //Serial.print("set_fx_channel_timer - CURRENT CHANNEL VALUE: ");
  //Serial.println(*FX_channel,HEX);

  unsigned long clearmask = ~w_timer_state;
  *FX_channel = *FX_channel & clearmask;
  //Serial.print("set_fx_channel_timer - CURRENT CHANNEL VALUE - after clear timers bits: ");
  //Serial.println(*FX_channel,HEX);
  *FX_channel = *FX_channel | (unsigned long)(timer_val << timer_state_bit);

  //Serial.print("set_fx_channel_timer - UPDATED CHANNEL VALUE: ");
  //Serial.println(*FX_channel,HEX);

  //Serial.print("set_fx_channel_timer - VALUE WRITTEN: ");
  //Serial.println((*FX_channel & w_timer_state) >> timer_state_bit,HEX);

}

// get the channel timer from the bit map; time is in ms
unsigned long get_fx_channel_timer(unsigned long FX_channel){
  unsigned long timer_val= (FX_channel & w_timer_state) >> timer_state_bit;
  //Serial.print("get_fx_channel_timer:");
  //Serial.print(timer_val, HEX);
  //Serial.print("\n\r");

  return(timer_val );
}


// set the channel state to on, off or diabled in the bit map
void set_fx_channel_onoff(unsigned long *FX_channel, byte onoff_state)
{

  // first clear the on/off bit so it can be set
  unsigned long clearmask = ~w_onoff_state;
  *FX_channel = *FX_channel & clearmask;
  *FX_channel = *FX_channel | (onoff_state << onoff_state_bit);

}

// get the channel channel on/off state from the bit map
byte get_fx_channel_onoff(unsigned long FX_channel){
  byte onoff_state;

  onoff_state = (FX_channel & w_onoff_state) >> onoff_state_bit;


  return(onoff_state);
}

// update the current state of all channels based on thier currentstate
void update_fx_channel_outputs() {

  int ch_number;
  int ch_mode;
  int ch_onoff;
  int ch_timer;
  // update each pin

  for(int curr_channel = 0 ; curr_channel <= FX_channels_max; curr_channel++) { 
    // get the pin's mode and update accordingly
    ch_mode = get_fx_channel_mode(FX_channels[curr_channel]);
    // update channels based on the mode
    switch (ch_mode) {
    case mode_fast_random:
      update_fx_channel_mode_fast_random(&FX_channels[curr_channel]);
      break;
    case mode_slow_random:
      update_fx_channel_mode_slow_random(&FX_channels[curr_channel]);
      break;
    case mode_momentary_on:
      update_fx_channel_mode_momentary_on(&FX_channels[curr_channel]);
      break;
    case mode_fixed_cycle:
      update_fx_channel_fixed_cycle(&FX_channels[curr_channel]);
      break;
    case mode_const_on:
      update_fx_channel_const_on(&FX_channels[curr_channel]);
      break;
    default: 
      // if nothing else matches, do nothing
      break;
    }
  } 
}


// this function turns the channel on and leaves it on
void update_fx_channel_const_on(unsigned long *FX_channel){
  // get the channel's state info
  byte ch_onoff = get_fx_channel_onoff(*FX_channel);
  byte ch_number = get_fx_channel(*FX_channel);   
  // if the channel is off then turn it on and leave it on ;
  if(ch_onoff == CHANNEL_OFF)
  {
    // set the channel state to on 
    set_fx_channel_onoff(FX_channel, CHANNEL_ON);
    // turn output pin on   
    digitalWrite(ch_number, CHANNEL_RELAY_ON);
  }
  else if (ch_onoff == CHANNEL_DISABLE)
  {
    // turn output pin on   
    digitalWrite(ch_number, CHANNEL_RELAY_OFF);
  }

}


// defines the fast_random on/off effect
// lights controlled by this type of channel should be more sporadic in nature
// flashing or lightning type effect
void update_fx_channel_mode_fast_random(unsigned long *FX_channel){

  // get the channel's state info
  byte ch_onoff = get_fx_channel_onoff(*FX_channel);
  unsigned long ch_timer = get_fx_channel_timer(*FX_channel);
  byte ch_number = get_fx_channel(*FX_channel); 
  int fast_random_time = 0;

  // if the channel is off then update the off state;
  if(ch_onoff == CHANNEL_OFF)
  {
    // the current state time expired so switch the state to on
    if(ch_timer == 0){
      // set the channel state to on 
      set_fx_channel_onoff(FX_channel, CHANNEL_ON);
      // turn output pin on   
      digitalWrite(ch_number, CHANNEL_RELAY_ON);
      // get a random time to be in the current state
      fast_random_time = random(1,4);
      // the logic below defines the fast random on effect
      // 2 out of 3 times the on time will be minimal length - very short on time
      if(fast_random_time < 3){
        ch_timer = FX_CHANNEL_FAST_RANDOM_BASE_TIME;//*random(1,4);
      }
      // 1 out of 3 times the on time will be relatively long at 4 times the minimal length 
      else {
        ch_timer = FX_CHANNEL_FAST_RANDOM_BASE_TIME*4;
      }
      // set the on time as calculated above
      set_fx_channel_timer(FX_channel, ch_timer);
    }  
    // still time left in the current state so just decrement the time by the channel update time base
    else {
      ch_timer = ch_timer - FX_CHANNEL_UPDATE_TIME;
      set_fx_channel_timer(FX_channel, ch_timer);
    }
  } 
  // if the channel is on then update the on state
  else if(ch_onoff == CHANNEL_ON){
    /// the current state time expired so switch the state to off
    if(ch_timer == 0){
      // set the channel state to off 
      set_fx_channel_onoff(FX_channel, CHANNEL_OFF);
      // turn output pin off   
      digitalWrite(ch_number, CHANNEL_RELAY_OFF);
      // the logic below defines the fast mode off effects
      // calc the random off time
      fast_random_time = random(1,3);
      // one out of two times the off time will be the minimum time
      if(fast_random_time < 2){
        ch_timer = FX_CHANNEL_FAST_RANDOM_BASE_TIME;
      }
      //  one out of two times the off time will be relatively long at 4X the min time
      else {
        ch_timer = FX_CHANNEL_FAST_RANDOM_BASE_TIME*4;
      }
      // se the channel off time
      set_fx_channel_timer(FX_channel, ch_timer);
    }  
    // still time left in this state so just decrement the time by the channel update time base
    else {
      ch_timer = ch_timer - FX_CHANNEL_UPDATE_TIME;
      set_fx_channel_timer(FX_channel, ch_timer);
    }
  }
  else if (ch_onoff == CHANNEL_DISABLE)
  {
    // turn output pin off   
    digitalWrite(ch_number, CHANNEL_RELAY_OFF);
  }
}

// defines the slow_random on/off effect
// lights controlled by this type of channel should be more slow pulsing in nature
// so mostly and much short off times, lights should feel more glowing or twinkling in nature
void update_fx_channel_mode_slow_random(unsigned long *FX_channel){

  // get the channel's state info
  byte ch_onoff = get_fx_channel_onoff(*FX_channel);
  unsigned long ch_timer = get_fx_channel_timer(*FX_channel);
  byte ch_number = get_fx_channel(*FX_channel); 
  int slow_random_time = 0;

  // if the channel is off then update the off state;
  if(ch_onoff == CHANNEL_OFF)
  {
    // the current state time expired so switch the state to on
    if(ch_timer == 0){
      // set the channel state to on 
      set_fx_channel_onoff(FX_channel, CHANNEL_ON);
      // turn output pin on   
      digitalWrite(ch_number, CHANNEL_RELAY_ON);
      // get a random time to be in the current state
      slow_random_time = random(1,9);
      // the logic below defines the slow random on effect
      // 6 out of 8 times the on time will be longer in length
      if(slow_random_time >= 3){
        ch_timer = FX_CHANNEL_SLOW_RANDOM_BASE_TIME * random(2,5) * 2;
      }
      // 4 out of 8 times the on time will be short long in lenght 
      else {
        ch_timer = FX_CHANNEL_SLOW_RANDOM_BASE_TIME;
      }
      // set the on time as calculated above
      set_fx_channel_timer(FX_channel, ch_timer);
    }  
    // still time left in the current state so just decrement the time by the channel update time base
    else {
      ch_timer = ch_timer - FX_CHANNEL_UPDATE_TIME;
      set_fx_channel_timer(FX_channel, ch_timer);
    }
  } 
  // if the channel is on then update the on state
  else if(ch_onoff == CHANNEL_ON){
    /// the current state time expired so switch the state to off
    if(ch_timer == 0){
      // set the channel state to off 
      set_fx_channel_onoff(FX_channel, CHANNEL_OFF);
      // turn output pin off   
      digitalWrite(ch_number, CHANNEL_RELAY_OFF);
      // the logic below defines the fast mode off effects
      // calc the random off time
      slow_random_time = random(1,4);
      //  3 out of 4 times the off time will be relativley short time
      if(slow_random_time > 1){
        ch_timer = FX_CHANNEL_SLOW_RANDOM_BASE_TIME;
      }
      //  1 out of 4 times the off time will be relatively long at 3X the min time
      else {
        ch_timer = FX_CHANNEL_SLOW_RANDOM_BASE_TIME;//*3;
      }
      // set the channel off time
      set_fx_channel_timer(FX_channel, ch_timer);
    }  
    // still time left in this state so just decrement the time by the channel update time base
    else {
      ch_timer = ch_timer - FX_CHANNEL_UPDATE_TIME;
      set_fx_channel_timer(FX_channel, ch_timer);
    }
  }
  else if (ch_onoff == CHANNEL_DISABLE)
  {
    // turn output pin off   
    digitalWrite(ch_number, CHANNEL_RELAY_OFF);
  }

}


void update_fx_channel_mode_momentary_on(unsigned long *FX_channel){

  // get the channel's state info
  byte ch_onoff = get_fx_channel_onoff(*FX_channel);
  unsigned long ch_timer = get_fx_channel_timer(*FX_channel);
  byte ch_number = get_fx_channel(*FX_channel); 

  // if the channel is currenlty off and the time is non-zero then turn it on
  // a non-zero channel time is the trigger to turn the channel on and
  // needs to be set by the caller of this function , this function will
  // then turn the channel on for the amount of time specified by the time
  // shut it off once the time reaches zero and leave it off until
  // the caller sets the time non-zero again
  if(ch_onoff == CHANNEL_OFF  && ch_timer != 0)
  {
    //Serial.print("mom_on ->turning channel on: timer= ");
    //Serial.println(ch_timer, HEX);

    // set the channel state to on a
    set_fx_channel_onoff(FX_channel, CHANNEL_ON);
    // turn channel on   
    digitalWrite(ch_number, CHANNEL_RELAY_ON);  
  }
  else if(ch_onoff == CHANNEL_ON){
    // decrement the time the channel has been on
    ch_timer = ch_timer - FX_CHANNEL_UPDATE_TIME;

    // if timer is 0 then turn the channel off
    if(ch_timer <= 0){
      //Serial.print("mom_on ->turning channel off: timer=");
      //Serial.println(ch_timer, HEX);

      // turn channel off    
      digitalWrite(ch_number, CHANNEL_RELAY_OFF);  
      // set the channel state to off and the timer to 0
      set_fx_channel_onoff(FX_channel, CHANNEL_OFF);
      set_fx_channel_timer(FX_channel, 0);
    }
    // just update the time
    else
    {
      //Serial.print("mom_on ->updating channel time: timer = ");
      //Serial.print(ch_timer,HEX);
      set_fx_channel_timer(FX_channel, ch_timer);
    }

  }
  else if (ch_onoff == CHANNEL_DISABLE)
  {
    // turn output pin off   
    digitalWrite(ch_number, CHANNEL_RELAY_OFF);
  }

}


// this function manages channels that are on a fixed on/off cycle
void update_fx_channel_fixed_cycle(unsigned long *FX_channel){

  // get the channel's state info
  byte ch_onoff = get_fx_channel_onoff(*FX_channel);
  unsigned long ch_timer = get_fx_channel_timer(*FX_channel);
  //Serial.print(FX_CHANNEL_FIXED_CYCLE_OFF_TIME); Serial.print("-");
  //Serial.println(ch_timer);
  byte ch_number = get_fx_channel(*FX_channel); 

  // if the channel is off then update the off state;
  if(ch_onoff == CHANNEL_OFF)
  {
    // the current state time expired so switch the state to on
    if(ch_timer == 0){
      // set the channel state to on 
      set_fx_channel_onoff(FX_channel, CHANNEL_ON);
      // turn output pin on   
      digitalWrite(ch_number, CHANNEL_RELAY_ON);
      // set the on time to the fixed on cycle time
      ch_timer = FX_CHANNEL_FIXED_CYCLE_ON_TIME;
      set_fx_channel_timer(FX_channel, ch_timer);
    }
    // still time left in the current state so just decrement the time by the channel update time base
    else {
      ch_timer = ch_timer - FX_CHANNEL_UPDATE_TIME;
      set_fx_channel_timer(FX_channel, ch_timer);
    }
  } 
  // if the channel is on then update the on state
  else if(ch_onoff == CHANNEL_ON){
    /// the current state time expired so switch the state to off
    if(ch_timer == 0){
      // set the channel state to off 
      set_fx_channel_onoff(FX_channel, CHANNEL_OFF);
      // turn output pin off   
      digitalWrite(ch_number, CHANNEL_RELAY_OFF);
      // set the channel off time to the fixed off cycle time
      ch_timer=FX_CHANNEL_FIXED_CYCLE_OFF_TIME;
      set_fx_channel_timer(FX_channel, ch_timer);
    }  
    // still time left in this state so just decrement the time by the channel update time base
    else {
      ch_timer = ch_timer - FX_CHANNEL_UPDATE_TIME;
      set_fx_channel_timer(FX_channel, ch_timer);
    }
  }
  else if (ch_onoff == CHANNEL_DISABLE)
  {
    // turn output pin off   
    digitalWrite(ch_number, CHANNEL_RELAY_OFF);
  }

}


// this function sets the motor direction according to the motor_dir param
void set_fx_motor_direction(unsigned long *FX_channel, byte motor_dir){  
  // get the channel pin number
  byte ch_number = get_fx_channel(*FX_channel); 
  // set the channel state to direction; this overrides the on/off state bit and uses it for dir
  set_fx_channel_onoff(FX_channel, motor_dir);
  // turn output pin on   
  //digitalWrite(ch_number, motor_dir);
  if(motor_dir == CHANNEL_ON)
  {
    digitalWrite(ch_number, CHANNEL_RELAY_ON);
  }
  else if (motor_dir == CHANNEL_OFF)
  {
    digitalWrite(ch_number, CHANNEL_RELAY_OFF);
  }
  else if (motor_dir == CHANNEL_DISABLE) 
  {
    digitalWrite(ch_number, CHANNEL_RELAY_OFF);
  }
}

// this function turns the motor on/off accoding ot the on/off param
void set_fx_motor_onoff(unsigned long *FX_channel, byte motor_onoff){ 
  // get the channel pin number
  byte ch_number = get_fx_channel(*FX_channel);  
  // set the channel state to direction; this over rides the on/off state bit and uses it for dir
  set_fx_channel_onoff(FX_channel, motor_onoff);
  // turn output pin on   
  if(motor_onoff == CHANNEL_ON)
  {
    digitalWrite(ch_number, CHANNEL_RELAY_ON);
  }
  else if (motor_onoff == CHANNEL_OFF)
  {
    digitalWrite(ch_number, CHANNEL_RELAY_OFF);
  }
  else if (motor_onoff == CHANNEL_DISABLE) 
  {
    digitalWrite(ch_number, CHANNEL_RELAY_OFF);
  }
}




























