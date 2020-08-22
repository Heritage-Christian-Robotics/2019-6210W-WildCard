/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       Caleb Buening                                             */
/*    Created:      Thu Sep 26 2019                                           */
/*    Description:  Competition Template                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* As of 2020-02-17:

//Updates:
+Cleaner code
+Less useless comments
+Fixed a non-resetting tray adjustment variable

//Problems:
+The arms are operating via pure luck

*/

// To dos:
// Fix stuff above

// Autonomous stack and tower
// Pre-auto of > 5 for unprotected
// Pre-auto of somethign for protected

// Something for cube gathering in protected
// Something for cube gathering in unprotected

// 2 more autonomous towers

#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// WheelLeftFront       motor         1
// WheelLeftRear        motor         16
// WheelRightFront      motor         2
// WheelRightRear       motor         17
// Arms                 motor         7
// Controller1          controller
// IntakeLeft           motor         5
// IntakeRight          motor         6
// Tray                 motor         21
// GyroSensor           inertial      9
// ---- END VEXCODE CONFIGURED DEVICES ----

// All the vex specific commands
using namespace vex;

// The competition object, for all things involving the field controller
competition Competition;

// Variables/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################

double VerticalRightJoystick; // Used to store right joystick
double VerticalLeftJoystick;  // Used to store left joystick

int TrayState = 0;       // Tray position
double TrayTarget = 0;   // tray location in (deg)
double LeftWheelsSpeed;  // Right wheel speed (deg)
double RightWheelsSpeed; // left wheel speed (deg)

int ArmsState = 0;     // Arms position
double ArmsTarget = 0; // Arms location (deg)

int AutoTask = 0;           // What type of run should the robot be prepared for
double speedfactor = 1;     // Speed multiplier for wheels
bool LowSpeed = false;      // Changes wheels' speeds to 30%
int TrayAdjustment = 0;     // Adjusts the tray for more accurate stacks
bool PrintCoasting = false; // prints "coasting..." to the brain
bool APressed = false;      // Has the a button been pressed once yet?
bool Unfolded = false;      // Has the expanding sequence run yet?
bool ButtonExpand = false;  // Has the button expand code been run yet?
int UserTimesCalled =
    0; // Times UserControl(){} has been called and possibly killed
bool PreAutoDone = false;

// functions
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################

// Return 0 if Joystick is near 0, otherwise return Joystick
int joyDeadZone(int Joystick) {
  if (Joystick < 10 && Joystick > -10) {
    return 0;
  } else {
    return Joystick;
  }
}

// Make sure that the wheels are running at max speed occasionally
int joyMaxMin(int Joystick) {
  if (Joystick > 80) { // Upper extreme
    return 100;
  } else if (Joystick < -80) { // Lower extreme
    return -100;
  } else {
    return Joystick; // keep it the same
  }
}

// intake on in autonomous
void IntakeOn() {
  IntakeLeft.spin(forward);
  IntakeRight.spin(forward);
}

// intake reverse in autonomous
void IntakeReverse() {
  IntakeLeft.spin(reverse);
  IntakeRight.spin(reverse);
}

// intake off in autonomous
void IntakeOff() {
  IntakeLeft.stop();
  IntakeRight.stop();
}

// Shift the arm state up one
void armsStateUp() {
  ArmsState++;

  if (ArmsState == 0) { // TODO: major problem.How have we been reaching 3? A
                        // heck lot of luck.

    ArmsState += 2;
  } else {
    ArmsState += 1;
  }
  if (ArmsState > 3) {
    ArmsState = 3;
  }
}

// Brings arm state back to zero when L2 is pressed//
void armsadjustup() {
  TrayAdjustment += 100; // shift the arm state upwards one
}

void armsadjustdown() {
  TrayAdjustment -= 100; // shift the arm state upwards one
}

void armsState0() { // down to 0 or 1
  if (ArmsState == 2) {
    ArmsState = 1;
  } else {
    ArmsState = 0;
  }
  TrayState = 0;
}

// Toggle low speed variable
void buttonYTrigger() { LowSpeed = !LowSpeed; }

// Expand if we haven't yet
void buttonATrigger() {
  // if (APressed == false) {
  //   APressed = true;
  //   PrintCoasting = true;
  //   Arms.setStopping(brakeType::coast);
  //   Arms.stop();
  //   Tray.setStopping(brakeType::coast);
  //   Tray.stop();
  // } else
  if (!Unfolded) {
    ButtonExpand = true;
  }
}

// Pre autonomous code//
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################

void pre_auton(void) {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();

  // Tray hard stop
  Tray.setVelocity(-100, percent);
  Tray.setMaxTorque(20, percent);
  Tray.spin(forward);

  // Arms hard stop
  Arms.setVelocity(-100, percent);
  Arms.setMaxTorque(20, percent);
  Arms.spin(forward);
  wait(500, msec);

  waitUntil(Arms.velocity(percent) < 1);
  waitUntil(Tray.velocity(percent) < 1);

  // Arms reset
  Arms.setRotation(0, rev);
  Arms.stop();
  Arms.setVelocity(0, percent);
  Arms.setMaxTorque(100, percent);
  Arms.setStopping(brakeType::coast);

  // Tray reset
  Tray.setRotation(0, rev); // this location is now zero
  Tray.stop();
  Tray.setVelocity(0, percent);
  Tray.setMaxTorque(100, percent);
  Tray.setStopping(brakeType::hold);

  if (!Competition.isFieldControl() && !Competition.isCompetitionSwitch()) {
    // Print autotask options
    Controller1.Screen.clearScreen();
    Controller1.Screen.setCursor(1, 1);
    Controller1.Screen.print("SA:/\\|SD:\\/");
    Controller1.Screen.newLine();
    Controller1.Screen.print("RL:<|RR:>|BL:Y|BR:A");
    Controller1.Screen.newLine();
    Controller1.Screen.print("1Cube: B None: X");

    // wait until an option is selected
    bool notDone = true;
    while (notDone) {
      if (Controller1.ButtonUp.pressing()) { // Skills Auto
        AutoTask = 1;
        notDone = false;
        Controller1.Screen.clearScreen();
      }
      if (Controller1.ButtonDown.pressing()) { // Skills Driver
        AutoTask = 2;
        notDone = false;
        Controller1.Screen.clearScreen();
      }
      if (Controller1.ButtonY.pressing()) { // Blue Left
        AutoTask = 3;
        notDone = false;
        Controller1.Screen.clearScreen();
      }
      if (Controller1.ButtonA.pressing()) { // Blue Right
        AutoTask = 4;
        notDone = false;
        Controller1.Screen.clearScreen();
      }
      if (Controller1.ButtonLeft.pressing()) { // Red Left
        AutoTask = 5;
        notDone = false;
        Controller1.Screen.clearScreen();
      }
      if (Controller1.ButtonRight.pressing()) { // Red Right
        AutoTask = 6;
        notDone = false;
        Controller1.Screen.clearScreen();
      }
      if (Controller1.ButtonX.pressing()) { // None
        AutoTask = 7;
        notDone = false;
        Controller1.Screen.clearScreen();
      }
      if (Controller1.ButtonB.pressing()) { // 1 Cube
        AutoTask = 8;
        notDone = false;
        Controller1.Screen.clearScreen();
      }
      wait(1, msec);
    }
  } else {
    AutoTask = 7;
    Unfolded = true;
  }
  Controller1.Screen.clearScreen();
  PreAutoDone = true;
}

// Autonomous//
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################

void stp(){
  LeftWheelsSpeed = 0;
  RightWheelsSpeed = 0;
}

void go(double LS, double RS){
  LeftWheelsSpeed = LS;
  RightWheelsSpeed = RS;
}

void autonomous(void) {

  // Disable coasting
  PrintCoasting = false;
  Arms.setStopping(brakeType::hold);
  Arms.spin(forward);
  Tray.setStopping(brakeType::hold);
  Tray.spin(forward);

  // Expand robot for all autos besides none and 1 cube////////////////////////////////////////////////////////////
  if (AutoTask != 7 && AutoTask != 8) {

    // Spin wheels in reverse to release tray
    LeftWheelsSpeed = -60;
    RightWheelsSpeed = -60;
    wait(1, sec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;
    wait(1, sec);

    // Release arms
    ArmsState = -1;
    wait(500, msec);
    ArmsState = 0;
    wait(500, msec);
    Unfolded = true;
  }

  ArmsState = 0;
  TrayState = 0;
  GyroSensor.resetRotation(); // MARKER2 Reset Gyro

  //Skills Autonomous////////////////////////////////////////////////////////////////////////////////////////////////////
  if (AutoTask == 1) {//MARKERAUTO

    IntakeOn();

    // TODO: Before stacking, spin cubes in, the send them out a specific
    // amount, then stack.

    // Curve Right when collecting first couple cubes
    while (WheelLeftFront.rotation(rotationUnits::rev) < 1.75) {
      LeftWheelsSpeed = 31.5;
      RightWheelsSpeed = 30;
      wait(1, msec);
    }
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    wait(3, sec); // DEBUG

    // Curve left when picking up the rest of the cubes
    WheelLeftFront.setRotation(0, rev);
    while (WheelLeftFront.rotation(rotationUnits::rev) < 6.25) {
      LeftWheelsSpeed = 30;
      RightWheelsSpeed = 31.5;
      wait(1, msec);
    }
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    wait(3, sec); // DEBUG

    // Run into the wall and hopefully pick up the last cube
    LeftWheelsSpeed = 40;
    RightWheelsSpeed = 40;
    wait(1500, msec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    wait(3, sec); // DEBUG

    // Scoot back a bit
    LeftWheelsSpeed = -20;
    RightWheelsSpeed = -20;
    wait(500, msec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;
    IntakeOff();

    wait(3, sec); // DEBUG

    // Turn a bit
    while (GyroSensor.yaw(deg) > -35) { // MARKER2 Turn
      LeftWheelsSpeed = -30;
      wait(1, msec);
    }
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    wait(3, sec); // DEBUG

    LeftWheelsSpeed = 80; // Drive towards the goal
    RightWheelsSpeed = 80;

    wait(2, sec); // Wait four seconds lol
    wait(2, sec);

    LeftWheelsSpeed = 0; // stop
    RightWheelsSpeed = 0;

    wait(3, sec); // DEBUG

    // Slowly put bottom cube on the ground
    IntakeLeft.setVelocity(30, pct);
    IntakeRight.setVelocity(30, pct);
    // IntakeLeft.setMaxTorque(30, pct);
    // IntakeRight.setMaxTorque(30, pct);
    IntakeReverse();
    wait(2.5, sec);
    IntakeOff();
    IntakeLeft.setVelocity(100, pct);
    IntakeRight.setVelocity(100, pct);
    IntakeLeft.setMaxTorque(100, pct);
    IntakeRight.setMaxTorque(100, pct);

    wait(3, sec); // DEBUG

    TrayState = 2; // Lift the tray
    ArmsState = -2;
    wait(500, msec);

    // Make sure the tower is against the wall
    LeftWheelsSpeed = 10;
    RightWheelsSpeed = 10;

    wait(4, sec);

    // send the tray up a bit more
    TrayAdjustment = 150; // TODO: reset this variable
    wait(1, sec);

    // Take the tray down
    TrayState = 0;
    wait(1, sec);

    // Scoot back
    LeftWheelsSpeed = -30;
    RightWheelsSpeed = -30;
    IntakeReverse();

    // Drive back a foot
    WheelLeftRear.setRotation(0, rev);
    while (WheelLeftRear.rotation(rev) > -1) {
      wait(1, msec);
    }
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    IntakeOff();

    // Curve back to avoid blue unprotected
    LeftWheelsSpeed = -48;
    RightWheelsSpeed = -50;
    wait(1.5, sec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    // Run into the wall
    LeftWheelsSpeed = -50;
    RightWheelsSpeed = -50;
    wait(3, sec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    IntakeOn();

    // Arc towards tower
    WheelRightFront.setRotation(0, rev);
    while (WheelRightFront.rotation(rev) < 5.5608) {
      LeftWheelsSpeed = 37;
      RightWheelsSpeed = 70;
      wait(1, msec);
    }
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    wait(.5, sec);

    // Drive towards the tower
    LeftWheelsSpeed = 50;
    RightWheelsSpeed = 50;
    wait(.5, sec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;
    wait(.1, sec);

    // Drive reverse a bit
    WheelLeftRear.setRotation(0, rev);
    while (WheelLeftRear.rotation(rev) > -1) {
      LeftWheelsSpeed = -50;
      RightWheelsSpeed = -50;
      wait(1, msec);
    }
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    // Get a cube into the arms
    IntakeReverse();
    wait(.5, sec);
    IntakeOff();

    // Lift the cube
    ArmsState = 2;
    TrayState = 1;
    wait(2, sec);

    // Drive towards the goal
    LeftWheelsSpeed = 50;
    RightWheelsSpeed = 50;
    wait(1, sec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    // Put cube in goal
    ArmsState = 1;
    wait(.5, sec);

    // get cube out of armms
    LeftWheelsSpeed = -50;
    RightWheelsSpeed = -50;
    wait(1, sec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    // run into wall, scoot back, Spit out bottom cube, stop intake, tray up,
    // intake out robot back
  }

  // Blue Left////MARKERBL
  if (AutoTask == 3) {
    //Mirror of Red Right
    
  }

  //Blue Right////MARKERBR
  if(AutoTask == 4){
    //Mirror of Blue Right

  }

  //Red Left////MARKERRL
  if(AutoTask == 5){
    //Step 1: Turn towards first cube
    while(GyroSensor.orientation(yaw, degrees) > -45){
      go(-25, 25);
      wait(5, msec);
    }
    stp(); //Stops wheels

    //Step 2: Turn on intakes
    IntakeOn();

    //Step 3: Drive to first cube
    WheelLeftFront.setRotation(0, rev);
    while(WheelLeftFront.rotation(rev) < 1){
      go(50, 50);
      wait(5, msec);
    }
    stp(); //Stops wheels
    
    //Step 4: Turn towards next cube
    while(GyroSensor.orientation(yaw, degrees) > -90){
      go(-25, 25);
      wait(5, msec);
    }
    stp(); //Stops wheels

    //Step 5: Drive to second cube
    WheelLeftFront.setRotation(0, rev);
    while(WheelLeftFront.rotation(rev) < 2){
      go(50, 50);
      wait(5, msec);
    }
    stp(); //Stops wheels

    //Step 5.5: Turn off intake
    IntakeOff();
    
    //Step 6: Turn towards goal
    while(GyroSensor.orientation(yaw, degrees) > -135){
      go(-25, 25);
      wait(5, msec);
    }
    stp(); //Stops wheels

    //Step 7: Ram into goal
    go(80, 80);
    wait(1000, msec);
    stp();

    //Step 8: Stack (Copy this from the other auto program. Make sure to make changes to both versions though)

  }

  // Red Right////MARKERRR
  if (AutoTask == 6) {
    IntakeOn();
    

    // Drive forward and pick up 4 cubes//
    WheelLeftFront.setRotation(0, rev);
    while (WheelLeftFront.rotation(rev) < 3) {
      LeftWheelsSpeed = 40;
      RightWheelsSpeed = 40;
      wait(5, msec);
    }
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    

    LeftWheelsSpeed = -80;
    RightWheelsSpeed = -80;
    wait(500, msec);

    IntakeOff();
    IntakeRight.spinFor(forward, 2, rev, false);
    IntakeLeft.spinFor(forward, 2, rev, false);
    wait(1000, msec);

    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    

    LeftWheelsSpeed = 30;
    RightWheelsSpeed = 30;
    wait(750, msec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    TrayState = 1;

    // Turn Towards goal
    while (GyroSensor.orientation(yaw, degrees) < 90) {
      LeftWheelsSpeed = 50;
      RightWheelsSpeed = -66;
      wait(5, msec);
    }
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    // Drive towards goal
    LeftWheelsSpeed = 80;
    RightWheelsSpeed = 80;
    wait(2000, msec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    //MARKERSTACK \/

    // Slowly put bottom cube on the ground
    IntakeLeft.setVelocity(50, pct);
    IntakeRight.setVelocity(50, pct);
    // IntakeLeft.setMaxTorque(30, pct);
    // IntakeRight.setMaxTorque(30, pct);
    IntakeReverse();
    wait(1, sec);
    IntakeOff();
    IntakeLeft.setVelocity(100, pct);
    IntakeRight.setVelocity(100, pct);
    IntakeLeft.setMaxTorque(100, pct);
    IntakeRight.setMaxTorque(100, pct);

    TrayState = 2; // Lift the tray
    ArmsState = -2;
    wait(500, msec);

    // Make sure the tower is against the wall
    LeftWheelsSpeed = 10;
    RightWheelsSpeed = 10;

    wait(.5, sec);

    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    // send the tray up a bit more
    TrayAdjustment = 150; // TODO: reset this variable
    wait(1, sec);

    // Take the tray down
    TrayState = 0;
    wait(1, sec);

    // Scoot back
    LeftWheelsSpeed = -40;
    RightWheelsSpeed = -40;
    IntakeReverse();
    wait(1000, msec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;
    IntakeOff();
  }

  // Push 1//MARKERPUSH
  if (AutoTask == 8) { // Push into corner auto

    // push cube into goal
    LeftWheelsSpeed = -50;
    RightWheelsSpeed = -50;

    wait(2, sec);

    // Get out of the goal
    LeftWheelsSpeed = 25;
    RightWheelsSpeed = 25;
    wait(3, sec);
    LeftWheelsSpeed = 0;
    RightWheelsSpeed = 0;

    // Release arms
    //>>>>>>>>>>>>Assumes the tray unlatched when we moved
    ArmsState = -1;
    wait(500, msec);
    ArmsState = 0;
    wait(500, msec);
    Unfolded = true;
  }
}

// User Control..
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################
//#######################################################################################################################################

void usercontrol(void) {
  // The user control loop has been called
  UserTimesCalled += 1;

  if(UserTimesCalled == 1){
    pre_auton();
  }

  // Coast if not in match or connected to a field controller
  if ((UserTimesCalled == 1 && AutoTask != 7) && 
      (!Competition.isFieldControl() &&
       !Competition.isCompetitionSwitch())  ) { // CONFIRM: When rebooted on
                                              // field, the robot doesn't coast
    // coast
    APressed = true;
    PrintCoasting = true;
    Arms.setStopping(brakeType::coast);
    Arms.stop();
    Tray.setStopping(brakeType::coast);
    Tray.stop();
  } else {
    // hold
    PrintCoasting = false;
    Arms.setStopping(brakeType::hold);
    Arms.spin(forward);
    Tray.setStopping(brakeType::hold);
    Tray.spin(forward);
  }

  // Make sure the Intakes are running at max
  IntakeLeft.setVelocity(100, percent);
  // make sure the intake will spin at it's fastest speed
  IntakeLeft.setMaxTorque(100, pct);
  IntakeRight.setVelocity(100, percent);
  IntakeRight.setMaxTorque(100, pct);
  while (true) {

    // Store Wheel Speeds from joysticks
    VerticalRightJoystick = Controller1.Axis2.value();
    VerticalLeftJoystick = Controller1.Axis3.value();

    // Round to zero
    VerticalRightJoystick = joyDeadZone(VerticalRightJoystick); // deadzone them
    VerticalLeftJoystick = joyDeadZone(VerticalLeftJoystick);

    // Round up or down if the joysticks are close to the maximum or minimum
    VerticalRightJoystick = joyMaxMin(VerticalRightJoystick);
    VerticalLeftJoystick = joyMaxMin(VerticalLeftJoystick);

    // Assign wheel speed
    LeftWheelsSpeed = VerticalLeftJoystick;
    RightWheelsSpeed = VerticalRightJoystick;

    // Unfold if Button expand triggered, and we haven't unfolded yet
    if (Unfolded == false && ButtonExpand == true) {
      // Arms
      ArmsState = -1;
      wait(500, msec);
      ArmsState = 0;
      wait(500, msec);

      Unfolded = true;
    }

    if(ArmsState == 1){
      IntakeLeft.setVelocity(40, percent);
      IntakeRight.setVelocity(40, percent);
    }else{
      IntakeLeft.setVelocity(100, percent);
      IntakeRight.setVelocity(100, percent);
    }

    // Intake
    

    if (Controller1.ButtonR1
            .pressing()) { // If R1 is being pressed, spin inwards
      IntakeLeft.spin(forward);
      IntakeRight.spin(forward);
    } else if (Controller1.ButtonR2
                   .pressing()) { // If R2 is being pressed, spin outwards
      IntakeLeft.spin(reverse);
      IntakeRight.spin(reverse);
    } else {
      IntakeLeft.stop(); // Otherwise don't move
      IntakeRight.stop();
    }

    wait(5, msec);
  }
}

int WheelsTask(void) {
  WheelLeftFront.spin(forward); // make sure the wheels are spinning forwards
  WheelLeftRear.spin(forward);
  WheelRightFront.spin(forward);
  WheelRightRear.spin(forward);
  while (true) {
    // Set velocities to assigned values, factoring in speedfactor
    WheelLeftFront.setVelocity(LeftWheelsSpeed * speedfactor, percent);
    WheelLeftRear.setVelocity(LeftWheelsSpeed * speedfactor, percent);
    WheelRightFront.setVelocity(RightWheelsSpeed * speedfactor, percent);
    WheelRightRear.setVelocity(RightWheelsSpeed * speedfactor, percent);
    wait(5, msec);
  }
}

int TrayTask(void) {
  Tray.spin(forward); // make sure the tray is spinning
  while (true) {

    // Controller -> States
    // Up arrow > Vertical state
    if (Controller1.ButtonUp.pressing()) {
      TrayState = 2;
      ArmsState = -2;
    }
    // Left and right arrows > Middle state
    if (Controller1.ButtonRight.pressing() ||
        Controller1.ButtonLeft.pressing()) {
      TrayState = 1;
    }
    // Down arrow > Bottom state
    if (Controller1.ButtonDown.pressing()) {
      TrayState = 0;
    }
    // Make sure the tray is up if the arms are
    if (ArmsState > 0) {
      TrayState = 1;
    }

    // State -> Target
    switch (TrayState) {
    // Low state
    case 0:
      TrayTarget = 100;
      speedfactor = 1;
      break;
    // Mid state
    case 1:
      TrayTarget = 600;
      speedfactor = 1;
      break;
    // Vertical state
    case 2:
      TrayTarget = 1650 + TrayAdjustment;
      speedfactor = .1;
      break;
    }

    // Run slow if the tray is high, unless we have low speed turned on
    // TODO: clarify these speeds with Will
    if (LowSpeed == true) {
      speedfactor = .6;
    } else if (TrayState == 0 || TrayState == 1) {
      speedfactor = 1;
    } else if (TrayState == 2) {
      speedfactor = .1;
    }

    // Target -> Speed
    Tray.setVelocity(0.25 * (TrayTarget - Tray.rotation(deg)), percent);

    wait(5, msec);
  }
}

int ArmsTask(void) {
  Arms.spin(forward);
  ArmsState = 0;
  while (true) {

    // Bring arms back up if we are intaking while arms are too low
    if ((ArmsState == -2 && TrayState == 0) &&
        Controller1.ButtonR1.pressing()) {
      ArmsState = 0;
    }
    // State -> Target
    switch (ArmsState) {
    case -2: // Stacking state
      ArmsTarget = 20;
      break;
    case -1: // Expanding State
      ArmsTarget = 300;
      break;
    case 0: // Picking up and stack
      ArmsTarget = 120;
      break;
    case 1: // Case 2 drops to here
      ArmsTarget = 750;
      break;
    case 2: // Half way mode
      ArmsTarget = 1000;
      break;
    case 3: // Stacking mode
      ArmsTarget = 1600;
      break;
    }

    // Target -> Speed
    Arms.setVelocity(.75 * (ArmsTarget - Arms.rotation(deg)), percent);
    wait(5, msec);
  }
}

int ControllerScreenTask() {
  while (true) {
    Controller1.Screen.clearScreen();
    Controller1.Screen.setCursor(1, 1);
    switch (AutoTask) {
    case 1:
      Controller1.Screen.print("SA");
      break;
    case 2:
      Controller1.Screen.print("SD");
      break;
    case 3:
      Controller1.Screen.print("BL");
      break;
    case 4:
      Controller1.Screen.print("BR");
      break;
    case 5:
      Controller1.Screen.print("RL");
      break;
    case 6:
      Controller1.Screen.print("RR");
      break;
    case 7:
      Controller1.Screen.print("None");
      break;
    case 8:
      Controller1.Screen.print("1 Cube");
      break;
    case 9:
      Controller1.Screen.print("AutoTask ERROR");
      break;
    }

    Controller1.Screen.setCursor(2, 1);
    if (PrintCoasting) {
      Controller1.Screen.print("Coasting...");
    }

    Controller1.Screen.setCursor(3, 1);
    if (LowSpeed) {
      Controller1.Screen.print("Low Speed");
    }

    wait(200, msec);
  }
}

int main() {

  Controller1.ButtonA.pressed(buttonATrigger);

  wait(500, msec);

  // Tasks:
  task taskTray(TrayTask);
  task taskArms(ArmsTask);
  task taskWheels(WheelsTask);

  Controller1.ButtonL1.pressed(armsStateUp);
  Controller1.ButtonL2.pressed(armsState0);
  Controller1.ButtonX.pressed(armsadjustup);
  Controller1.ButtonB.pressed(armsadjustdown);
  Controller1.ButtonY.pressed(buttonYTrigger);

  waitUntil(PreAutoDone == true);
  task taskControllerScreen(ControllerScreenTask);
}