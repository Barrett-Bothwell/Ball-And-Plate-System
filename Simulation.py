import numpy as np
import control as ctrl
import matplotlib.pyplot as plt

# Define the sampling period (consistent with your C++ code)
STEP_TIME = 10000  # in microseconds
T = STEP_TIME / 1000000.0  # Convert to seconds

g = 9.81
K_ball = 5/7 * g

# State-space matrices
A = np.array([
    [0, 1, 0, 0],
    [0, 0, 0, 0],
    [0, 0, 0, 1],
    [0, 0, 0, 0]
])
B = np.array([
    [0, 0],
    [K_ball, 0],
    [0, 0],
    [0, K_ball]
])
C = np.eye(4)  # full state output
D = np.zeros((4, 2))

# Sampling period
T_s = 0.02  # 20 ms

# Discretize the system
sys_continuous = ctrl.StateSpace(A, B, C, D)
sys_discrete = ctrl.c2d(sys_continuous, T_s, method='zoh')  # Zero-order hold
A_d = sys_discrete.A
B_d = sys_discrete.B

# Choose desired eigenvalues for the observer
observer_eigenvalues = [0.9, 0.88, 0.9, 0.88]  # Discrete-time poles (inside unit circle)

# Compute the observer gain matrix K using pole placement
K = ctrl.place(A_d.T, C.T, observer_eigenvalues).T

# Choose desired eigenvalues for the closed-loop system
desired_eigenvalues = [0.92, 0.89, 0.92, 0.89]  # Discrete-time poles for control
G = ctrl.place(A_d, B_d, desired_eigenvalues)

# Initial state
x0 = np.array([2, 0, 1, 0.0])
x_hat0 = np.array([0, 0, 0, 0])  # Initial state estimate

# Time vector
T = np.linspace(0, 10, int(10 / T_s))
dt = T[1] - T[0]

# Initialize state and control histories
x_hist = np.zeros((4, len(T)))
x_hat_hist = np.zeros((4, len(T)))
u_hist = np.zeros((2, len(T)))
y_hist = np.zeros((4, len(T)))

x_hist[:, 0] = x0
x_hat_hist[:, 0] = x_hat0

# Add noise to the measurements
noise_mean = 0 # Mean of noise
noise_std = 0.05  # Standard deviation of noise

# Simulate with discrete-time observer
for i in range(1, len(T)):
    x = x_hist[:, i - 1]
    x_hat = x_hat_hist[:, i - 1]

    # Output (measured positions)
    y = C @ x
    noise = np.random.normal(noise_mean, noise_std, size=y.shape)
    y_noisy = y + noise  # Add noise to the output
    y_hist[:, i] = y_noisy

    # Control input using estimated state
    setpoint = np.array([0, 0, 0, 0])  # Desired state
    u = -G @ (x_hat - setpoint)  # Use estimated state for control
    u = np.clip(u, -np.radians(10), np.radians(10))  # Limit angles to ±25 degrees
    u_hist[:, i - 1] = u

    # Update true state
    x_next = A_d @ x + B_d @ u
    x_hist[:, i] = x_next

    # Update observer state
    x_hat_next = A_d @ x_hat + B_d @ u + K @ (y_noisy - C @ x_hat)
    x_hat_hist[:, i] = x_hat_next

# Final input
u_hist[:, -1] = -G @ (x_hat_hist[:, -1] - setpoint)

# Plot position over time with noisy measurements and observer estimates
plt.figure(figsize=(12, 8))

# Print the results
print("Discrete-time A_d matrix:")
print(A_d)
print("\nDiscrete-time B_d matrix:")
print(B_d)

# Plot x position
plt.subplot(2, 1, 1)
plt.plot(T, x_hist[0, :], label="True x position", linewidth=2)
plt.plot(T, y_hist[0, :], '.', label="Noisy x measurement", alpha=0.5)
plt.plot(T, x_hat_hist[0, :], '--', label="Estimated x position (observer)", linewidth=2)
plt.xlabel("Time (s)")
plt.ylabel("x Position (m)")
plt.title("Comparison of True State, Noisy Measurements, and Observer Estimates (x)")
plt.grid(True)
plt.legend()

# Plot y position
plt.subplot(2, 1, 2)
plt.plot(T, x_hist[2, :], label="True y position", linewidth=2)
plt.plot(T, y_hist[2, :], '.', label="Noisy y measurement", alpha=0.5)
plt.plot(T, x_hat_hist[2, :], '--', label="Estimated y position (observer)", linewidth=2)
plt.xlabel("Time (s)")
plt.ylabel("y Position (m)")
plt.title("Comparison of True State, Noisy Measurements, and Observer Estimates (y)")
plt.grid(True)
plt.legend()

plt.tight_layout()
plt.show()

# Plot velocities
plt.figure(figsize=(10, 5))
plt.plot(T, x_hist[1, :], label="x velocity")
plt.plot(T, x_hist[3, :], label="y velocity")
plt.xlabel("Time (s)")
plt.ylabel("Velocity (m/s)")
plt.title("Ball Velocity Over Time with Feedback")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()

# Plot control inputs (u[0] and u[1]) over time
plt.figure(figsize=(10, 5))
plt.plot(T, u_hist[0, :], label="Theta x (u[0])")
plt.plot(T, u_hist[1, :], label="Theta y (u[1])")
plt.xlabel("Time (s)")
plt.ylabel("Control Input (rad)")
plt.title("Control Inputs Over Time")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()

