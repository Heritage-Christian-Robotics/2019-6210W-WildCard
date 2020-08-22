using namespace vex;

extern brain Brain;

// VEXcode devices
extern motor WheelLeftFront;
extern motor WheelLeftRear;
extern motor WheelRightFront;
extern motor WheelRightRear;
extern motor Arms;
extern controller Controller1;
extern motor IntakeLeft;
extern motor IntakeRight;
extern motor Tray;
extern inertial GyroSensor;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Text.
 * 
 * This should be called at the start of your int main function.
 */
void  vexcodeInit( void );