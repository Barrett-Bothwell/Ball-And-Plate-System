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
| 4-Wire Resistive Touchscreen | Senses ball position                  |
| Custom-built Plate     | Platform for the ball                      |
| 6302view               | PC-based visualization & tuning tool       |
| Power Supply           | Stable 5V source for servos and ESP32      |

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
- **Visualized Parameters**:
  - Current ball position (X, Y)
  - Servo commands
  - Target/reference position slider
  - Axis Offset sliders to account for unlevel surfaces
    