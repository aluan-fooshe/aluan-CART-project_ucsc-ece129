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

	def __init__(self):
		super().__init__()

if __name__ == "__main__":
	# Initialize two ultrasonic sensors, left-to-right
	sensor1 = DistanceSensor(echo=27, trigger=17, max_distance=5)
	sensor2 = DistanceSensor(echo=24, trigger=23, max_distance=5)
	print(f"Starting distance sensor readings... (Press Ctrl+C to stop)")

	left_sensor = UltrasonicHCSR04(sensor1, 500)
	right_sensor = UltrasonicHCSR04(sensor2, 500)

	try:
		while True:
			left_distance = left_sensor.measure_distance()
			right_distance = right_sensor.measure_distance()
			print(f"Left Distance: {left_distance:.3f} cm \t\tRight Distance: {right_distance:.3f} cm")
			sleep(0.5)  # Wait 0.5 seconds between readings
	except KeyboardInterrupt:
		print("\nSensor reading stopped.")
	finally:
		sensor1.close()  # Clean up GPIO resources}")
		sensor2.close() 