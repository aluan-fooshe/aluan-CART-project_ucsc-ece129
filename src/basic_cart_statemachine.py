from gpiozero import DistanceSensor  # Import the DistanceSensor class from the gpiozero library

import tkinter as tk  # Import the tkinter library for creating the GUI

from tkinter import font  # Import the font module from tkinter for customizing the font

from time import sleep  # Import the sleep function from the time module for delay

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

array1 = [1, 2, 3, 2, 2, 1, 1, 1, 1, 3, 3, 3]

def switch_case(a):
    match a:
        case 1:
            return "state 1"
        case 2:
            return "state 2"
        case 3:
            return "state 3"
        case _:
            return "state unknown"

def cart_statemachine(a):
    match a:
        case "idle":
            return "idle state"
        case "follow":
            return "state 2"
        case "stuck":
            return "state 3"
        case _:
            return "state unknown"

# Sort the list in place
array1.sort()

# test if switch case actually works
for a in array1:
    print(switch_case(a))
