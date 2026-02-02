from gpiozero import DistanceSensor
import tkinter as tk
from tkinter import font
from time import sleep
from statemachine import StateMachine, State

"""
GPIO pin 23 (Trig), GPIO pin 24 (Echo), GND and 5V pins from raspberry pi 5
State Machine for controlling a cart based on distance measurements:
- TooClose: Cart backs up (distance < 20cm)
- OptimalRange: Cart stays in place (20cm <= distance < 30cm)
- TooFar: Cart moves forward (distance >= 30cm)
"""

# Initialize the ultrasonic sensor
sensor = DistanceSensor(echo=24, trigger=23, max_distance=5)


class CartStateMachine(StateMachine):
    """State machine for cart distance control"""

    # Define states
    too_close = State(initial=True)
    optimal_range = State()
    too_far = State()

    # Define transitions
    move_to_optimal = (
            too_close.to(optimal_range) |
            too_far.to(optimal_range)
    )
    move_to_close = (
            optimal_range.to(too_close) |
            too_far.to(too_close)
    )
    move_to_far = (
            optimal_range.to(too_far) |
            too_close.to(too_far)
    )

    def __init__(self, distance_label):
        self.distance_label = distance_label
        super().__init__()

    def on_enter_too_close(self):
        """Called when entering TooClose state"""
        print("State: TOO_CLOSE - Cart backing up")
        self.update_display("red", "Hi!")

    def on_enter_optimal_range(self):
        """Called when entering OptimalRange state"""
        print("State: OPTIMAL_RANGE - Cart staying in place")
        self.update_display("green", "")

    def on_enter_too_far(self):
        """Called when entering TooFar state"""
        print("State: TOO_FAR - Cart moving forward")
        self.update_display("blue", "Bye!")

    def update_display(self, color, message):
        """Update the GUI display"""
        distance = self.measure_distance()
        display_text = f"Distance: {distance} cm"
        if message:
            display_text += f"\n{message}"
        self.distance_label.config(fg=color, text=display_text)

    def measure_distance(self):
        """Measure distance from sensor"""
        return int(sensor.distance * 100)

    def process_distance(self):
        """Process current distance and transition to appropriate state"""
        distance = self.measure_distance()

        # Determine which state we should be in based on distance
        if distance < 20:
            if not self.current_state == self.too_close:
                self.move_to_close()
        elif 20 <= distance < 30:
            if not self.current_state == self.optimal_range:
                self.move_to_optimal()
        else:  # distance >= 30
            if not self.current_state == self.too_far:
                self.move_to_far()

        # Update display even if state didn't change (distance value updates)
        self.on_enter_state(self.current_state)

    def on_enter_state(self, state):
        """Generic handler to update display for current state"""
        if state == self.too_close:
            self.update_display("red", "Hi!")
        elif state == self.optimal_range:
            self.update_display("green", "")
        elif state == self.too_far:
            self.update_display("blue", "Bye!")


class DistanceSensorGUI:
    """GUI Application for distance sensor with state machine"""

    def __init__(self):
        # Initialize the Tkinter window
        self.window = tk.Tk()
        self.window.title("Distance Measurement - State Machine")
        self.window.geometry("800x400")

        # Create custom font
        custom_font = font.Font(size=30)

        # Create distance label
        self.distance_label = tk.Label(
            self.window,
            text="Distance: ",
            anchor='center',
            font=custom_font
        )
        self.distance_label.pack(expand=True)

        # Create state label
        self.state_label = tk.Label(
            self.window,
            text="State: Initializing...",
            anchor='center',
            font=font.Font(size=16)
        )
        self.state_label.pack()

        # Initialize state machine
        self.state_machine = CartStateMachine(self.distance_label)

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
            self.state_machine.process_distance()
        except Exception as e:
            print(f"Error in update loop: {e}")

        # Schedule next update after 1 second
        self.window.after(1000, self.update_loop)

    def run(self):
        """Start the GUI event loop"""
        self.window.mainloop()


if __name__ == "__main__":
    # Create and run the application
    app = DistanceSensorGUI()
    app.run()