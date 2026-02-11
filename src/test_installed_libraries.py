# Testing every newly installed library here
from statemachine import StateMachine, State

class LightMachine(StateMachine):
	on = State(initial=True)
	off = State()

	cycle = (on.to(off)
		| off.to(on) 
	)

	def before_cycle(self, event: str, source: State, target: State, message: str = ""):
			message = ". " + message if message else ""
			return f"Running {event} from {source.id} to {target.id}{message}"
	def enter_on(self):
		print("LIGHTS ON")
	def enter_off(self):
		print("LIGHTS OFF")

# Create instance
light = LightMachine()

while True:
	# Toggle the light
	light.cycle()  # Prints: LIGHTS OFF
	light.cycle()  # Prints: LIGHTS ON
	light.cycle()  # Prints: LIGHTS OFF

	# Check current state
	print(light.current_state)  # Shows current state

