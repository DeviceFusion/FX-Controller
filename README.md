# FX-Controller
This is the FX-Controller Readme
The Special FX Controller is a hobby project that provides automated control of a spooky Halloween Installation. The goal of the installation is to provide Halloween night Trick-or-treaters a spooky treat. First developed in 2019 and run every Halloween since with occasional updates and improvements.

The initial installation was in the form of a haunted pickup truck parked in my front yard. The major components consisted of:
1. An old 1954 Ford F100 pickup.
1. A skeleton driver and passenger in the truck cab. The skeleton’s have mechanically automated heads that when activated randomly move to any point left or right, giving the effect that they are “looking around”.
1. An automated coffin in the truck bed. When activated the lid opens, revealing a ghoul that rises with the lid. After a delay the ghoul descends back into the coffin as the lid closes.
1. An old Trailer pulled by the truck. Standing in the trailer is:
1. An automated Zombie, whose head bobbles when activated.
1. A frightened/screaming trick-or-treater mannequin.
1. Various computer controlled  lighting, sound, and smoke effects timed to be activated at distinct points in the special effects cycle. 
   - Simulated lightning.
   - Fast random flashing lights.
    - Slow Random flashing lights.
    - Steady-on black lighting
    - Computer triggered smoke machine.
    - Spooky sound effects track played through a PA speaker.
1. An IR motion sensor that triggers the automated special effects cycle.
   -The installation sits dark and silent until a trick-or-treater triggers the special effects by walking by the motion sensor.
   -Once triggered the automation runs its cycle and then returns to dark.
   -The automation cycle is a few minutes long with the following sequence of events:
   -Spooky sound track starts.
   -After a delay the truck cab black-lights turn on, giving the skeleton truck driver and passenger a neon glow. The skeletons then begin to  “look around” in all directions.
   -After a bit more delay the other lighting effects turn on and the coffin lid starts to open.
   -As the lid opens the smoke machine sends smoke from inside and around the coffin.
   -As the lid rises the ghoul inside the coffin rises with it.
   -Once the lid is open it stops in the open position.
   -After some delay the coffin lid closes and the ghoul descends back into the coffin with it.
   -Once the coffin lid is closed all other effects turn off, except for the soundtrack which plays a bit more until its end.
   -The installation then sits dark and silent until triggered by the next trick-or-treater and the above cycle then starts again.

In subsequent years the truck was not used and instead the components used to transform my house into a haunted house. Using the same automation control, the various character components are set up inside the house in the street facing windows. The coffin and related components are set up outside directly in front of the house. In addition to the original automation components, projected video was added. The video consists of a parade of Zombies projected in one window and a giant red eyeball slowly “looking around” projected into another.

On a technical basis I designed and built the entire installation: all of the automation control components and programming,all of the electro-mechanical characters, the electro-mechanical coffin, the motion-sensor, lighting fixtures, and created the sound effect track. The control components consist of:
2.An arduino based master controller:
  -Controls the overall run cycle described above.
  -Monitors the motion detector to trigger the display cycle.
  -Directly controls all lighting effects
  -Directly controls the sound track
  -Directly controls the smoke effects
  -Directly controls the coffin open/close cycle.
  -Indirectly controls the skeleton heads via RS232 communication to a secondary arduino.
  -Indirectly controls the zombie via a light sensing trigger.
2.Secondary Arduino skeleton head controller.
  -Receives commands from the master controller via RS232.
  -Determines each movement of each skeleton head.
  -Heads are moved to the desired position via output to servo motors.
2.Arduino zombie controller
  -Zombie head bobble is triggered by a light sensor directed at the master control’s lighting effects.
  -Zombie head bobble is accomplished by the Arduino controlling an automotive electric door lock mechanism.
