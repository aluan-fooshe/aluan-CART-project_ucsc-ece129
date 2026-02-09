"""
GPIO pin 23 (Trig), GPIO pin 24 (Echo), GND and 5V pins from raspberry pi 5
State Machine for controlling a cart based on distance measurements:
- TooClose: Cart backs up (distance < 20cm)
- OptimalRange: Cart stays in place (20cm <= distance < 30cm)
- TooFar: Cart moves forward (distance >= 30cm)
"""

from gpiozero import DistanceSensor  # Import the DistanceSensor class from the gpiozero library
import tkinter as tk  # Import the tkinter library for creating the GUI
from tkinter import font  # Import the font module from tkinter for customizing the font
from time import sleep  # Import the sleep function from the time module for delay
from statemachine import StateMachine, State

# Initialize the ultrasonic sensor
sensor = DistanceSensor(echo=24, trigger=23, max_distance=5)
