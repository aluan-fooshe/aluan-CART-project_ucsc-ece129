from statemachine import StateMachine, State
from gpiozero import DistanceSensor  # Import the DistanceSensor class from the gpiozero library
from time import sleep  # Import the sleep function from the time module for delay

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

	def on_enter_right(self):
		print("State: RIGHT - Cart moving at +1 speed")


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

	def print_curr_state(self):
		print(self.current_state)

		# --------ADD GUI TO THIS!!!!!---------

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
	# Initialize two ultrasonic sensors, left-to-right
	sensor1 = DistanceSensor(echo=27, trigger=17, max_distance=5)
	sensor2 = DistanceSensor(echo=24, trigger=23, max_distance=5)

	# kp=1.0, ki=0, kd=0
	CartPID = CartPID(target_distance=100, kp=1.0, ki=0, kd=0)
	print(f"Starting distance sensor readings... (Press Ctrl+C to stop)")

	left_sensor = UltrasonicHCSR04(sensor1, 500)
	right_sensor = UltrasonicHCSR04(sensor2, 500)

	cart = CartStateMachine_v2(left_distance=0, right_distance=0)

	try:
		while True:
			left_distance = left_sensor.measure_distance()
			right_distance = right_sensor.measure_distance()
			cart.left_distance = left_distance
			cart.right_distance = right_distance
			cart.process_left_right_distances()
			print(f"Left Distance: {left_distance:<4} cm\t\tRight Distance: {right_distance:<4} cm\t\tleft PID: {CartPID.compute(left_distance):<4} cm\t\tright PID: {CartPID.compute(right_distance):<4} cm")
			print(f"State: {cart.current_state.id}")
			sleep(0.5)  # Wait 0.5 seconds between readings
	except KeyboardInterrupt:
		print("\nSensor reading stopped.")
	finally:
		sensor1.close()  # Clean up GPIO resources}")
		sensor2.close() 