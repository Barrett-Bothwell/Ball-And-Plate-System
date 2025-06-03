#include <Arduino.h>
#include <Six302.h>
#include <BasicLinearAlgebra.h>
#include <stdint.h>
#include <TouchScreen.h>
#include <ESP32Servo.h>

//Touchscreen Pin Definitions
#define YP 2  // must be an analog pin, use "An" notation! CHANGE TO CORRECT PINS
#define XM 1  // must be an analog pin, use "An" notation!
#define YM 0   // can be a digital pin
#define XP 3   // can be a digital pin


// These values are in microseconds
#define STEP_TIME 10000 // (0.01s)
#define REPORT_TIME 100000 // (second)

CommManager cm(STEP_TIME, REPORT_TIME);

float T = STEP_TIME / 1000000.0; // Convert to seconds

//Initialization of variables
float prev_x = 0;
float prev_y = 0;
float x_meas = 0;
float y_meas = 0;
float x_dot = 0;
float y_dot = 0;
float raw_x = 0;
float phi_x = 0;
float phi_y = 0;
float x_offset = -2.8;
float y_offset = -2.8;
float theta_x = 0;
float theta_y = 0;
float x_setpoint = 100; 
float y_setpoint = -150;
bool circle = false; // Circle mode toggle
float radius = 25;

//TouchScreen Setup
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

unsigned long lastTouchTime = 0; // Timestamp of the last valid touchscreen press
const unsigned long TOUCH_TIMEOUT = 500000; // Timeout period in microseconds (0.5 seconds)

#define TS_MINX -3072  // Replace with your calibrated minimum X value
#define TS_MAXX 420  // Replace with your calibrated maximum X value
#define TS_MINY -2500  // Replace with your calibrated minimum Y value
#define TS_MAXY 350  // Replace with your calibrated maximum Y value

//Servo Setup
Servo servox;
Servo servoy;
int minUs = 500; // Minimum pulse width in microseconds
int maxUs = 2500; // Maximum pulse width in microseconds

int servoxPin = 5;
int servoyPin = 4;

//Define Necessary Matrices
BLA::Matrix<4, 4> A = {
  0, 1, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 1,
  0, 0, 0, 0
};

BLA::Matrix<4, 2> B = {
  0, 0,
  5.0/7.0*9.81, 0,
  0, 0,
  0, 5.0/7.0*9.81
};

BLA::Matrix<4, 4> C = {
  1, 0, 0, 0,
  0, 1, 0, 0,
  0,0,1,0,
  0,0,0,1
};

BLA::Matrix<4, 2> D = {
  0, 0,
  0, 0,
  0, 0,
  0, 0
};

BLA::Matrix<4, 4> K = {
  0.12, 0.02, 0, 0,
  0, 0.12, 0, 0,
  0, 0, 0.1, 0.02,
  0, 0, 0, 0.1
}; // Observer gain matrix (tune this based on your system)

BLA :: Matrix<2,4> G = {
  0.3568 / 500, 0.4959 / 850, 0, 0, 
  0, 0, 0.3568 / 500 * 2.63/3.75, 0.4959 / 550 * 2.63/3.75
}; // Control gain matrix (worked pretty good with 420 x vel was 640)

// State and observer variables
BLA::Matrix<4> x_hat = {0, 0, 0, 0}; // Initial estimated state
BLA::Matrix<4> y = {0, 0, 0, 0};           // Measured output
BLA::Matrix<2> u = {0, 0};           // Control input
BLA::Matrix<4> error = {0, 0, 0, 0}; // Error between estimated and actual state
BLA::Matrix<4> setpoint = {x_setpoint, 0, y_setpoint, 0}; // Example: stationary at origin

// Compute discrete-time matrices A_d and B_d
BLA::Matrix<4, 4> A_d = {
    1, 0.02, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0.02,
    0, 0, 0, 1
};

BLA::Matrix<4, 2> B_d = {
    0.00140143, 0,
    0.14014286, 0,
    0, 0.00140143,
    0, 0.14014286
};

/*
void computeDiscreteMatrices() {
    for (int i = 1; i <= 10; i++) {
        term = (term * (A * T)) / i; // Compute the next term: (A * T)^i / i!
        A_d += term; // Add the term to A_d
    }
    B_d = (A_d - identity) * (BLA::Invert(A) * B);
}*/

void setup() {
  //6302 ViewSetup
  cm.addPlot(&x_meas, "X Position", -100, 100);
  // optional to plot phi_x on 6302 view
  //cm.addPlot(&phi_x, "phi_x", 33, 57);
  cm.addPlot(&y_meas, "Y Position", -100, 100);
  //option to plot phi_y on 6302 view
  //cm.addPlot(&phi_y, "phi_y", 33, 57);
  //Optional to plot x_dot and y_dot on 6302 view 
  //cm.addPlot(&x_dot, "X Velocity", -150, 150);
  //cm.addPlot(&y_dot, "Y Velocity", -150, 150);
  

  // Add sliders for setpoint adjustment
  cm.addSlider(&x_setpoint, "X Setpoint", 0, 165, 50, 1);
  cm.addSlider(&y_setpoint, "Y Setpoint", -180, -100, -120, 1);
  cm.addSlider(&x_offset, "X Offset", -5, 5, -3.75, 1);
  cm.addSlider(&y_offset, "Y Offset", -5, 5, -2.5, 1);
  cm.addToggle(&circle, "Circle");

  //Servo Setup Code
    servox.attach(servoxPin, minUs, maxUs);
    servoy.attach(servoyPin, minUs, maxUs);

  // Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);

  cm.connect(&Serial, 115200);
}

void loop() {
  static unsigned long lastTime = 0; // Store the timestamp of the previous loop iteration
  unsigned long currentTime = micros(); // Get the current time in microseconds

  // Calculate the actual time step (T) in seconds
  float T = (currentTime - lastTime) / 1000000.0; // Convert microseconds to seconds
  lastTime = currentTime; // Update the last time for the next iteration

   // 1. Read position from resistive touchscreen
   TSPoint p = ts.getPoint();
 
   if (p.x == -3072 || p.y == 1023) {
     // Valid touchscreen press detected
     x_meas = prev_x;
     y_meas = prev_y;

   } else {
     // Use the last valid measurement
     x_meas = map(p.x, TS_MINX, TS_MAXX, -82.5, 82.5);
     y_meas = map(p.y, TS_MINY, TS_MAXY, -50, 50);
     lastTouchTime = currentTime; // Update the last touch timestamp
   }

  // 2. Estimate velocity
  x_dot = (x_meas - prev_x) / T;
  y_dot = (y_meas - prev_y) / T;
  prev_x = x_meas;
  prev_y = y_meas;

  // 3. Update state vector
  BLA::Matrix<4> y = {x_meas, x_dot, y_meas, y_dot};

  // 4. Define the setpoint (desired state)
  if (circle) {
    // Circle mode: Setpoint follows a circular path
    float time = currentTime / 1000000.0; // Convert to seconds
    setpoint = {radius*cos(time)+x_setpoint,-radius*sin(time) ,radius*sin(time)+y_setpoint ,radius*cos(time) };

  } else {
    setpoint = {x_setpoint, 0, y_setpoint, 0};
  }

  // Update Observer State
  x_hat = A_d * x_hat + B_d * u + K * (y - C * x_hat);

  BLA::Matrix<4> error = x_hat - setpoint; // Compute error
  
  u = -G * error;

  // 7. Convert control input to degrees
  theta_x = u(0) * 180 / PI; // Convert u(0) to degrees
  theta_y = u(1) * 180 / PI; // Convert u(1) to degrees

  phi_x = (theta_x + 45); // Convert to servo angle *3.75
  phi_y = (theta_y + 45); // Convert to servo angle *2.63

  phi_x = constrain(phi_x, 38, 57) - x_offset; //38 - 57
  phi_y = constrain(phi_y, 34, 51) - y_offset; // 34 - 51

  // Send phi_x and phi_y to servos
  servox.write(phi_x); // 45 degrees isapproximately zero for x
  servoy.write(phi_y); // 45 degrees is approximately zero for y

  cm.step();
}