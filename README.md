# Meshtastic Drone Swarm Telemetry & Control

This project provides a comprehensive framework for integrating Meshtastic mesh networking with DroneBridge32 for telemetry and control of a drone swarm. It encompasses features like swarm coordination, geofencing, emergency landing, data logging, visualization, dynamic channel switching, AI integration (conceptual), encryption, and fail-safes.

## Features

* **Decentralized Control:** Operate your drone swarm over a resilient Meshtastic mesh network.
* **Encrypted Communication:** Secure communication using AES encryption.
* **Telemetry:** Real-time data including position, altitude, and battery voltage.
* **Control:** Commands for individual drones or the entire swarm.
* **MAVLink Integration:** Seamlessly works with MAVLink for flight controller communication.
* **Geofencing:** Prevents drones from entering restricted areas.
* **Emergency Landing:** Robust emergency landing protocol.
* **Data Logging:** Logs telemetry data for post-flight analysis.
* **Map Visualization:** Interactive map display of drone locations.
* **Dynamic Channel Switching:** Adapts to network conditions.
* **AI Integration (Conceptual):** Provides a framework for AI-driven insights.
* **Fail-Safes:** Signal loss and low battery protections.
* **Swarm Coordination:** Example of sending commands to the entire swarm.

## Components

1.  **Meshtastic Plugin (Telemetry Reception & Display):**
    * Python-based plugin for the Meshtastic ground station.
    * Displays telemetry data, map visualization, and provides control options.
    * Includes geofencing and data logging.
2.  **Meshtastic Plugin (Control Command Transmission):**
    * Python-based plugin for sending control commands.
    * Supports individual and swarm commands, including emergency landing.
    * Dynamic Channel switching.
3.  **DroneBridge32 (or Companion Process) - Telemetry Transmission:**
    * Arduino code for transmitting telemetry data.
    * Parses MAVLink messages, encrypts data, and handles fail-safes.
    * Battery voltage detection.
4.  **DroneBridge32 (or Companion Process) - Control Command Reception:**
    * Arduino code for receiving and executing control commands.
    * Decrypts commands and sends corresponding MAVLink commands.

## Installation

1.  **DroneBridge32 Integration:**
    * Upload the provided Arduino code to your drone's microcontroller.
    * Ensure MAVLink communication is correctly configured.
2.  **Meshtastic Node:**
    * Connect a Meshtastic-compatible device to your drone.
    * Configure the network settings.
3.  **Meshtastic Plugin:**
    * Install Python and required libraries (`pip install meshtastic folium`).
    * Run the plugin scripts (`python meshtastic_telemetry.py` and `python meshtastic_control.py`).

## Usage

1.  **Launch Plugins:** Start the telemetry and control plugins.
2.  **Monitor Telemetry:** Observe real-time data on the telemetry plugin.
3.  **Send Commands:** Use the control plugin to send commands.
4.  **Map Visualization:** Open the map to view drone locations.

## Configuration

* **Drone IDs:** Set unique IDs for each drone in the Arduino code.
* **MAVLink Settings:** Configure MAVLink baud rate and message IDs.
* **Meshtastic Settings:** Configure channel, encryption keys, and network settings.
* **Geofence:** Modify the geofence coordinates in the telemetry plugin.
* **Encryption Key:** Modify the Encryption keys in the arduino code and ensure they match.

## Considerations

* **Latency:** Meshtastic's LoRa communication introduces latency.
* **Bandwidth:** Optimize data transmission for limited bandwidth.
* **Power Consumption:** Minimize power usage on drone nodes.
* **Reliability:** Implement error handling for LoRa communication.
* **Regulations:** Adhere to local drone operation and radio communication regulations.

## Disclaimer

This project is provided "as is," without warranty. Use at your own risk.
