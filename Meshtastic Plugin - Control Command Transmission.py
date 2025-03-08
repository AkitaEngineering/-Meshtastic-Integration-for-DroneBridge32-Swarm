import meshtastic
import meshtastic.serial_interface
import struct
import tkinter as tk
from tkinter import ttk

DRONE_CONTROL_COMMAND_DATA_TYPE = 101 # Custom data type ID
interface = None

def send_control_command(drone_id, command):
    payload = struct.pack("Bi", drone_id, command)
    interface.sendData(payload, DRONE_CONTROL_COMMAND_DATA_TYPE)

def send_return_home():
    try:
        drone_id = int(drone_id_entry.get())
        send_control_command(drone_id, 1)  # 1 is the return home command
        print(f"Sending return home command to drone {drone_id}")
    except ValueError:
        print("Invalid drone ID")

def send_land():
    try:
        drone_id = int(drone_id_entry.get())
        send_control_command(drone_id, 2)  # 2 is the land command
        print(f"Sending land command to drone {drone_id}")
    except ValueError:
        print("Invalid drone ID")

def send_custom_command():
    try:
        drone_id = int(drone_id_entry.get())
        command = int(custom_command_entry.get())
        send_control_command(drone_id, command)
        print(f"Sending custom command {command} to drone {drone_id}")
    except ValueError:
        print("Invalid drone ID or command")

# GUI Setup
root = tk.Tk()
root.title("Drone Control")

frame = ttk.Frame(root, padding="10")
frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

drone_id_label = ttk.Label(frame, text="Drone ID:")
drone_id_label.grid(row=0, column=0, sticky=tk.W)

drone_id_entry = ttk.Entry(frame)
drone_id_entry.grid(row=0, column=1, sticky=tk.W)

return_home_button = ttk.Button(frame, text="Return Home", command=send_return_home)
return_home_button.grid(row=1, column=0, pady=5, columnspan=2, sticky=tk.W+tk.E)

land_button = ttk.Button(frame, text="Land", command=send_land)
land_button.grid(row=2, column=0, pady=5, columnspan=2, sticky=tk.W+tk.E)

custom_command_label = ttk.Label(frame, text="Custom Command:")
custom_command_label.grid(row=3, column=0, sticky=tk.W)

custom_command_entry = ttk.Entry(frame)
custom_command_entry.grid(row=3, column=1, sticky=tk.W)

custom_command_button = ttk.Button(frame, text="Send Custom Command", command=send_custom_command)
custom_command_button.grid(row=4, column=0, pady=5, columnspan=2, sticky=tk.W+tk.E)

try:
    interface = meshtastic.serial_interface.SerialInterface()
    root.mainloop()

except meshtastic.serial_interface.InterfaceError as e:
    print(f"Error connecting to Meshtastic device: {e}")
except Exception as e:
    print(f"An unexpected error occurred: {e}")
finally:
    if interface:
        interface.close()
