from statemachine import StateMachine, State
from gpiozero import DistanceSensor  # Import the DistanceSensor class from the gpiozero library
from time import sleep  # Import the sleep function from the time module for delay
import tkinter as tk
from tkinter import font

# any class structure with () is class inheritance.
class UltrasonicHCSR04:
	def __init__(self, sensor, max_distance):
		self.max_distance = max_distance
		self.sensor = sensor
		super().__init__()

	def measure_distance(self):
		"""Measure distance from sensor"""
		return int(self.sensor.distance * 100)

class CartStateMachine_v2(StateMachine):
	"""State machine for cart distance control"""

	# Define states
	idle = State(initial=True)
	forward = State()
	left = State()
	right = State()

	# Define transitions
	transition_to_idle = (
			forward.to(idle) |
			left.to(idle) |
			right.to(idle)
	)
	transition_to_forward = (
			idle.to(forward) |
			left.to(forward) |
			right.to(forward)
	)
	transition_to_left = (
			forward.to(left) |
			idle.to(left) |
			right.to(left)
	)
	transition_to_right = (
			forward.to(right) |
			idle.to(right) |
			left.to(right)
	)

	def __init__(self, left_distance, right_distance):
		self.left_distance = left_distance
		self.right_distance = right_distance
		super().__init__()

	def on_enter_idle(self):
		"""Called when entering IDLE state"""
		print("State: IDLE - Cart staying in place")
		# self.update_display("green", "IDLE")

	def on_enter_forward(self):
		print("State: FORWARD - Cart moving at +1 speed")
		# self.update_display("red", "FORWARD")

	def on_enter_left(self):
		print("State: LEFT - Cart moving at +1 speed")
		# self.update_display("red", "LEFT")

	def on_enter_right(self):
		print("State: RIGHT - Cart moving at +1 speed")
		# self.update_display("red", "RIGHT")


	def process_left_right_distances(self):
		CART_LENGTH_MM = 52  # mm
		hysteresis_bound = CART_LENGTH_MM / 2

		""" determines if the person is left or right. 
				left -> negative		 right -> positive
				forward -> between -hysteresis_bound and +hysteresis_bound
		"""
		diff_value = self.right_distance - self.left_distance

		# determine which state we should be in based on distance in millimeters
		if diff_value > hysteresis_bound:
			if not self.current_state == self.right:
				self.transition_to_right()
		elif diff_value < -hysteresis_bound:
			if not self.current_state == self.left:
				self.transition_to_left()
		elif self.left_distance > hysteresis_bound and self.right_distance > hysteresis_bound:
			if not self.current_state == self.forward:
				self.transition_to_forward()
		else:
			if not self.current_state == self.idle:
				self.transition_to_idle()	

		# # Update display even if state didn't change (distance value updates)
		# self.on_enter_state(self.current_state)


class DistanceSensorGUI:
	"""GUI Application for distance sensor with state machine"""

	def __init__(self):
		# Initialize the Tkinter window
		self.window = tk.Tk()
		self.window.title("Distance Measurement - State Machine")
		self.window.geometry("800x400")

		# Initialize two ultrasonic sensors, left-to-right
		self.sensor1 = DistanceSensor(echo=27, trigger=17, max_distance=5)
		self.sensor2 = DistanceSensor(echo=24, trigger=23, max_distance=5)

		# Initialize the left and right sensors
		self.left_sensor = UltrasonicHCSR04(self.sensor1, 500)
		self.right_sensor = UltrasonicHCSR04(self.sensor2, 500)

		# Create custom font
		custom_font = font.Font(size=30)

		# Create state label
		self.state_label = tk.Label(
			self.window,
			text="State: Initializing...!!",
			anchor='center',
			font=font.Font(size=16)
		)
		self.state_label.pack()

		# Initialize state machine
		self.left_distance = 0
		self.right_distance = 0
		self.state_machine = CartStateMachine_v2(self.left_distance, self.right_distance)

		# Bind state machine events to update state label
		self.state_machine.add_listener(self.on_state_change)

		# Start the measurement loop
		self.update_loop()

	def on_state_change(self, event, state):
		"""Callback for state machine state changes"""
		state_name = state.id.replace('_', ' ').title()
		self.state_label.config(text=f"Current State: {state_name}")

	def update_loop(self):
		"""Main update loop - processes distance and updates state machine"""
		try:
			self.left_distance = self.left_sensor.measure_distance()
			self.right_distance = self.right_sensor.measure_distance()
			self.state_machine.left_distance = left_distance
			self.state_machine.right_distance = right_distance
			self.state_machine.process_left_right_distances()
			#update state on application?
			state_name = self.state_machine.current_state
			self.state_label.config(text=f"Current State: {state_name}")
		except KeyboardInterrupt:
			print("\nSensor reading stopped.")
		except Exception as e:
			print(f"Error in update loop: {e}")
		finally:
			self.sensor1.close()  # Clean up GPIO resources}")
			self.sensor2.close() 

		# Schedule next update after 1 second
		self.window.after(1000, self.update_loop)

	def run(self):
		"""Start the GUI event loop"""
		self.window.mainloop()


class CartPID:
	def __init__(self, target_distance, kp, ki, kd):
		self.target = target_distance
		self.kp = kp
		self.ki = ki
		self.kd = kd

		self.previous_error = 0
		self.integral = 0

	def compute(self, measured_distance):
		# Calculate error (positive = too far, negative = too close)
		error = measured_distance - self.target

		# Proportional term
		p = self.kp * error

		# Integral term
		i = self.ki * self.integral

		# Derivative term - rate of change of error
		derivative = error - self.previous_error
		d = self.kd * derivative
		self.previous_error = error

		# Output is a motor speed/direction value
		output = p + i + d
		return output

if __name__ == "__main__":
	# Create and run the application
	app = DistanceSensorGUI()
	app.run()

	# # Initialize two ultrasonic sensors, left-to-right
	# sensor1 = DistanceSensor(echo=27, trigger=17, max_distance=5)
	# sensor2 = DistanceSensor(echo=24, trigger=23, max_distance=5)

	# # kp=1.0, ki=0, kd=0
	# CartPID = CartPID(target_distance=100, kp=1.0, ki=0, kd=0)
	# print(f"Starting distance sensor readings... (Press Ctrl+C to stop)")

	# left_sensor = UltrasonicHCSR04(sensor1, 500)
	# right_sensor = UltrasonicHCSR04(sensor2, 500)

	# cart = CartStateMachine_v2(left_distance=0, right_distance=0)

	# try:
	# 	while True:
	# 		left_distance = left_sensor.measure_distance()
	# 		right_distance = right_sensor.measure_distance()
	# 		cart.left_distance = left_distance
	# 		cart.right_distance = right_distance
	# 		cart.process_left_right_distances()
	# 		print(f"Left Distance: {left_distance:<4} cm\t\tRight Distance: {right_distance:<4} cm\t\tleft PID: {CartPID.compute(left_distance):<4} cm\t\tright PID: {CartPID.compute(right_distance):<4} cm")
	# 		print(f"State: {cart.current_state.id}")
	# 		sleep(0.5)  # Wait 0.5 seconds between readings
	# except KeyboardInterrupt:
	# 	print("\nSensor reading stopped.")
	# finally:
	# 	sensor1.close()  # Clean up GPIO resources}")
	# 	sensor2.close() 