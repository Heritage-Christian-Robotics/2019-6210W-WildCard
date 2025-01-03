#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain  Brain;

// VEXcode device constructors
motor WheelLeftFront = motor(PORT1, ratio18_1, true);
motor WheelLeftRear = motor(PORT16, ratio18_1, false);
motor WheelRightFront = motor(PORT2, ratio18_1, false);
motor WheelRightRear = motor(PORT17, ratio18_1, true);
motor Arms = motor(PORT7, ratio18_1, true);
controller Controller1 = controller(primary);
motor IntakeLeft = motor(PORT5, ratio18_1, false);
motor IntakeRight = motor(PORT6, ratio18_1, true);
motor Tray = motor(PORT21, ratio36_1, false);
inertial GyroSensor = inertial(PORT9);

// VEXcode generated functions
// define variable for remote controller enable/disable
bool RemoteControlCodeEnabled = true;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Text.
 * 
 * This should be called at the start of your int main function.
 */
void vexcodeInit( void ) {
  // nothing to initialize
}