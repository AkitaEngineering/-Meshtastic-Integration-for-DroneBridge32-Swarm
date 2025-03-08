import meshtastic
import meshtastic.serial_interface
import struct
import tkinter as tk
from tkinter import ttk, scrolledtext

DRONE_TELEMETRY_DATA_TYPE = 100  # Custom data type ID
DRONE_CONTROL_COMMAND_DATA_TYPE = 101 #custom data type ID
interface = None
drone_data = {}  # Dictionary to store drone telemetry data

def on_receive(packet, interface):
    if packet['decoded']['data']['portnum'] == DRONE_TELEMETRY_DATA_TYPE:
        try:
            telemetry_data = struct.unpack("fffiB", packet['decoded']['data']['payload'])
            latitude, longitude, altitude, battery, drone_id = telemetry_data
            drone_data[drone_id] = {
                "latitude": latitude,
                "longitude": longitude,
                "altitude": altitude,
                "battery": battery,
            }
            update_display()
        except Exception as e:
            print(f"Error decoding telemetry: {e}")
            print(packet['decoded']['data']['payload'])
    elif packet['decoded']['data']['portnum'] == DRONE_CONTROL_COMMAND_DATA_TYPE:
        try:
            drone_id, command = struct.unpack("Bi", packet['decoded']['data']['payload'])
            print(f"Control command {command} recieved for drone: {drone_id}")
        except Exception as e:
            print(f"Error decoding control command: {e}")

def update_display():
    telemetry_text.delete(1.0, tk.END)  # Clear previous text
    for drone_id, data in drone_data.items():
        telemetry_text.insert(tk.END, f"Drone {drone_id}:\n")
        telemetry_text.insert(tk.END, f"  Latitude: {data['latitude']}\n")
        telemetry_text.insert(tk.END, f"  Longitude: {data['longitude']}\n")
        telemetry_text.insert(tk.END, f"  Altitude: {data['altitude']}\n")
        telemetry_text.insert(tk.END, f"  Battery: {data['battery']}\n")
        telemetry_text.insert(tk.END, "\n")

def send_control_command(drone_id, command):
    payload = struct.pack("Bi", drone_id, command)
    interface.sendData(payload, DRONE_CONTROL_COMMAND_DATA_TYPE)

def send_return_home():
    try:
        drone_id = int(drone_id_entry.get())
        send_control_command(drone_id, 1)  # 1 is the return home command
    except ValueError:
        print("Invalid drone ID")

def send_land():
    try:
        drone_id = int(drone_id_entry.get())
        send_control_command(drone_id, 2)  # 2 is the land command
    except ValueError:
        print("Invalid drone ID")

# GUI Setup
root = tk.Tk()
root.title("Drone Swarm Telemetry")

frame = ttk.Frame(root, padding="10")
frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

telemetry_text = scrolledtext.ScrolledText(frame, wrap=tk.WORD, width=40, height=15)
telemetry_text.grid(row=0, column=0, columnspan=2, pady=10)

drone_id_label = ttk.Label(frame, text="Drone ID:")
drone_id_label.grid(row=1, column=0, sticky=tk.W)

drone_id_entry = ttk.Entry(frame)
drone_id_entry.grid(row=1, column=1, sticky=tk.W)

return_home_button = ttk.Button(frame, text="Return Home", command=send_return_home)
return_home_button.grid(row=2, column=0, pady=5)

land_button = ttk.Button(frame, text="Land", command=send_land)
land_button.grid(row=2, column=1, pady=5)

try:
    interface = meshtastic.serial_interface.SerialInterface()
    interface.add_receive_callback(on_receive)
    root.mainloop()

except meshtastic.serial_interface.InterfaceError as e:
    print(f"Error connecting to Meshtastic device: {e}")
except Exception as e:
    print(f"An unexpected error occurred: {e}")
finally:
    if interface:
        interface.close()
