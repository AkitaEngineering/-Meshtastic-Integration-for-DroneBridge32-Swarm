import meshtastic
import meshtastic.serial_interface
import struct
import tkinter as tk
from tkinter import ttk, scrolledtext
import time
import json
import threading
import folium  # For map visualization
import webbrowser
import os

DRONE_TELEMETRY_DATA_TYPE = 100
DRONE_CONTROL_COMMAND_DATA_TYPE = 101
interface = None
drone_data = {}
geofence = [  # Example geofence (latitude, longitude)
    (40.7128, -74.0060),
    (40.7228, -74.0060),
    (40.7228, -74.0160),
    (40.7128, -74.0160),
]
log_file = "drone_telemetry.jsonl"
map_file = "drone_map.html"
map_obj = folium.Map(location=[40.7128, -74.0060], zoom_start=14)  # Initialize map

def on_receive(packet, interface):
    if packet['decoded']['data']['portnum'] == DRONE_TELEMETRY_DATA_TYPE:
        try:
            latitude, longitude, altitude, battery, drone_id = struct.unpack("fffiB", packet['decoded']['data']['payload'])
            drone_data[drone_id] = {"latitude": latitude, "longitude": longitude, "altitude": altitude, "battery": battery}
            check_geofence(drone_id, latitude, longitude)
            log_telemetry(drone_id, latitude, longitude, altitude, battery)
            update_display()
            update_map(drone_id, latitude, longitude)
            analyze_telemetry(drone_id) # Example AI analysis per drone.
        except Exception as e:
            print(f"Error decoding telemetry: {e}")
    elif packet['decoded']['data']['portnum'] == DRONE_CONTROL_COMMAND_DATA_TYPE:
        try:
            drone_id, command = struct.unpack("Bi", packet['decoded']['data']['payload'])
            print(f"Control command {command} received for drone: {drone_id}")
        except Exception as e:
            print(f"Error decoding control command: {e}")

def check_geofence(drone_id, latitude, longitude):
    inside = False
    n = len(geofence)
    p1x, p1y = geofence[0]
    for i in range(n + 1):
        p2x, p2y = geofence[i % n]
        if longitude > min(p1y, p2y):
            if longitude <= max(p1y, p2y):
                if latitude <= max(p1x, p2x):
                    if p1y != p2y:
                        xinters = (longitude - p1y) * (p2x - p1x) / (p2y - p1y) + p1x
                    if p1x == p2x or latitude <= xinters:
                        inside = not inside
        p1x, p1y = p2x, p2y
    if not inside:
        print(f"Drone {drone_id} outside geofence! Initiating emergency landing.")
        send_control_command(drone_id, 3)  # Emergency landing command

def log_telemetry(drone_id, latitude, longitude, altitude, battery):
    data = {
        "timestamp": time.time(),
        "drone_id": drone_id,
        "latitude": latitude,
        "longitude": longitude,
        "altitude": altitude,
        "battery": battery,
    }
    with open(log_file, "a") as f:
        f.write(json.dumps(data) + "\n")

def update_display():
    telemetry_text.delete(1.0, tk.END)
    for drone_id, data in drone_data.items():
        telemetry_text.insert(tk.END, f"Drone {drone_id}:\n")
        telemetry_text.insert(tk.END, f"  Latitude: {data['latitude']}\n")
        telemetry_text.insert(tk.END, f"  Longitude: {data['longitude']}\n")
        telemetry_text.insert(tk.END, f"  Altitude: {data['altitude']}\n")
        telemetry_text.insert(tk.END, f"  Battery: {data['battery']}\n\n")

def send_control_command(drone_id, command):
    payload = struct.pack("Bi", drone_id, command)
    interface.sendData(payload, DRONE_CONTROL_COMMAND_DATA_TYPE)

def send_emergency_landing():
    try:
        drone_id = int(drone_id_entry.get())
        send_control_command(drone_id, 3)  # 3 is the emergency landing command
    except ValueError:
        print("Invalid drone ID")

def update_map(drone_id, latitude, longitude):
    folium.Marker([latitude, longitude], popup=f"Drone {drone_id}").add_to(map_obj)
    map_obj.save(map_file)

def open_map():
    if os.path.exists(map_file):
        webbrowser.open("file://" + os.path.realpath(map_file))
    else:
        print("Map file not found.")

def switch_channel(channel_name):
    try:
        interface.set_channel(channel_name)
        print(f"Switched to channel: {channel_name}")
    except Exception as e:
        print(f"Error switching channel: {e}")

def analyze_telemetry(drone_id):
    # Placeholder for AI-based telemetry analysis
    if drone_id in drone_data:
        data = drone_data[drone_id]
        if data["battery"] < 20: #Example AI analysis.
            print(f"Warning: Drone {drone_id} battery low!")

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

emergency_landing_button = ttk.Button(frame, text="Emergency Landing", command=send_emergency_landing)
emergency_landing_button.grid(row=2, column=0, pady=5)

open_map_button = ttk.Button(frame, text="Open Map", command=open_map)
open_map_button.grid(row=2, column=1, pady=5)

switch_channel_button = ttk.Button(frame, text="Switch Channel", command=lambda: switch_channel("MyNewChannel"))
switch_channel_button.grid(row=3, column=0, pady=5)

try:
    interface = meshtastic.serial_interface.SerialInterface()
    threading.Thread(target=root.mainloop, daemon=True).start() # GUI in its own thread.
    while True:
        time.sleep(1)
except meshtastic.serial_interface.InterfaceError as e:
    print(f"Error connecting to Meshtastic device: {e}")
except Exception as e:
    print(f"An unexpected error occurred: {e}")
finally:
    if interface:
        interface.close()
