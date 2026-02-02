from gpiozero import DistanceSensor  # Import the DistanceSensor class from the gpiozero library
import tkinter as tk  # Import the tkinter library for creating the GUI
from tkinter import font  # Import the font module from tkinter for customizing the font

from time import sleep  # Import the sleep function from the time module for delay

import statemachine # Import the python state machine in order to build a framework that responds to sensor inputs.
from statemachine import StateMachine, State

"""
GPIO pin 23 (Trig), GPIO pin 24 (Echo), GND and 5V pins from raspberry pi 5
example array. Can be used for sorting files by type, date. Also can be used to sort my downloads folder.
And even microcontoller logic too. """


# Initialize the ultrasonic sensor

sensor = DistanceSensor(echo=24, trigger=23, max_distance=5)

# Initialize the Tkinter window

window = tk.Tk()

window.title("Distance Measurement")
custom_font = font.Font(size=30)  # Create a custom font object with size 30
window.geometry("800x400")  # Set the dimensions of the window
distance_label = tk.Label(window, text="Distance: ", anchor='center', font=custom_font)

# Create a label to display the distance, centered text, and use the custom font
distance_label.pack()  # Add the label to the window


def measure_distance():
    return int(sensor.distance * 100)  # Measure the distance and convert it to an integer

def display_distance_on_window():

   distance = measure_distance()  # Measure the distance and convert it to an integer
   #distance_label.config(text="Distance: {} cm".format(distance))  # Update the distance label with the new distance

   if distance < 20:
       distance_label.config(fg="red", text="Distance: {} cm\nHi!".format(distance))
       # If the distance is less than 20, set the label text to display "Hi!" in red

   elif distance >= 20 and distance < 30:

       distance_label.config(fg="green", text="Distance: {} cm\n".format(distance))
       # If the distance is greater than 30, set the label text to display "Bye!" in blue

   else:
       distance_label.config(fg="blue", text="Distance: {} cm\nBye!".format(distance))

   window.after(1000, display_distance_on_window)  # Schedule the next measurement after 1 second


# Start measuring distance

display_distance_on_window()

# Run the Tkinter event loop

window.mainloop()

# -----------------------------------------------



