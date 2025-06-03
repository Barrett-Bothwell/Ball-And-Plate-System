# Ball and Plate Control System

A dynamic balancing system that uses an **ESP32-C3 Super Mini**, **MG995 servos**, and a **resistive touchscreen** to control the position of a ball on a plate. The system implements **modern control theory** for real-time position stabilization and uses **6302view** for visualization and debugging.

## 🛠️ Features

- Real-time ball position control using state-space or observer-based feedback
- Input via resistive touchscreen for ball position sensing
- Output actuation using MG995 servos
- Visualization with **6302view**
- Built on the ESP32-C3 Super Mini platform

## 📦 Components Used

| Component              | Description                                |
|------------------------|--------------------------------------------|
| ESP32-C3 Super Mini    | Microcontroller for control and I/O        |
| MG995 Servos (x2)      | Control pitch and roll of the plate        |
| M2x65mm Linkage Rod | Linkage Rod used to connect plate to servo motors |
| M2 Metal Clevis    | Clevis used to connect the rod to the servo and top plate |
| Universal Joint Coupler | 10mm inner diameter universal joint |
| 7/8" Stainless Steel Ball | Ideal ball size for system and plate |
| 4-Wire Resistive Touchscreen | Senses ball position (165mm by 100mm)  
| 6302view               | PC-based visualization & tuning tool       |
| 5v Power Supply           | Stable 5V source for servos and ESP32      |

## 📐 Control Approach

This project uses a **state-space feedback controller** designed using modern control theory. Key features:
- Real-time state estimation
- Robust control tuning using matrix-based gain design
- Manual gain tuning

## 🖥️ 6302view Integration

- **Purpose**: Enables real-time monitoring and set point placement of system over serial communication.
- **Setup**:
  - Connect ESP32-C3 to PC via USB
  - Configure serial output from your code (e.g., using `Serial.println()` or a binary protocol)
  - Run `6302view` on PC and link to the correct COM port
  - More details for running 6302 view can be seen at https://github.com/almonds0166/6302view?tab=readme-ov-file
- **Visualized Parameters**:
  - Current ball position (X, Y)
  - Currently commented out servo command plots
  - Target/reference position slider
  - Axis Offset sliders to account for uneven surfaces
  - Circle toggle to target a circular pattern for the ball

## Potential Improvements

- **Hardware Improvements**
  - The 3D printed structure had some minor warping on the top piece
    - Modifications to CAD model could be made to reduce this warping
    - Printing procedure could change to modify this as well
  - Cheap servos were used which although had enough power and range of motion the clarity is low
    - Higher quality, more accurate servos could be used to ensure desired angle is output
    - 3 or more servos could have been used in a different arrangement to produce more accurate plate angle outputs

- **Software Improvements**
  - Both G and K matrices can be fine tuned to ensure optimal convergence
  - Additional plots can be added using 6302 view
    - Currently code for angles is commented out
    - Observer state plots could be added


    