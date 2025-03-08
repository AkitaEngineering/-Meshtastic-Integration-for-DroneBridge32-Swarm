#include <Arduino.h>
#include <HardwareSerial.h>
#include <mavlink.h> // Include MAVLink library

// Configuration
#define MESHTASTIC_SERIAL Serial1
#define BAUD_RATE 115200
#define DRONE_ID 1
#define MAVLINK_SERIAL Serial // Assuming MAVLink from another serial port

// MAVLink System and Component IDs
#define SYSTEM_ID 255
#define COMPONENT_ID 1

// Telemetry Data Structure
struct DroneTelemetry {
  float latitude;
  float longitude;
  float altitude;
  float batteryVoltage;
  uint8_t droneID;
};

// Control Command Structure
struct DroneControlCommand {
  uint8_t droneID;
  int command;
};

void setup() {
  MESHTASTIC_SERIAL.begin(BAUD_RATE);
  MAVLINK_SERIAL.begin(57600); // Adjust to your MAVLink baud rate.
  Serial.begin(115200);
  Serial.println("DroneBridge32 Meshtastic Integration");
}

void loop() {
  sendTelemetry();
  receiveControlCommand();
  delay(50);
}

void sendTelemetry() {
  mavlink_message_t msg;
  mavlink_status_t status;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];

  while (MAVLINK_SERIAL.available() > 0) {
    uint8_t c = MAVLINK_SERIAL.read();
    if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
      // MAVLink message received.
      if (msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT) {
        mavlink_global_position_int_t global_pos;
        mavlink_msg_global_position_int_decode(&msg, &global_pos);

        DroneTelemetry telemetryData;
        telemetryData.latitude = global_pos.lat / 10000000.0;
        telemetryData.longitude = global_pos.lon / 10000000.0;
        telemetryData.altitude = global_pos.alt / 1000.0;
        telemetryData.batteryVoltage = 12.0; // Replace with actual battery data.
        telemetryData.droneID = DRONE_ID;

        uint8_t buffer[sizeof(DroneTelemetry)];
        memcpy(buffer, &telemetryData, sizeof(DroneTelemetry));
        MESHTASTIC_SERIAL.write(buffer, sizeof(DroneTelemetry));
      }
      else if(msg.msgid == MAVLINK_MSG_ID_SYS_STATUS){
        mavlink_sys_status_t sys_status;
        mavlink_msg_sys_status_decode(&msg, &sys_status);
        //example of getting battery voltage.
        float voltage = sys_status.voltage_battery/1000.0f;
        //Do something with the voltage.
      }
    }
  }
}

void receiveControlCommand() {
  if (MESHTASTIC_SERIAL.available() >= sizeof(DroneControlCommand)) {
    DroneControlCommand command;
    MESHTASTIC_SERIAL.readBytes((uint8_t*)&command, sizeof(DroneControlCommand));

    if (command.droneID == DRONE_ID) {
      Serial.print("Received command: ");
      Serial.println(command.command);

      if (command.command == 1) {
        sendMavlinkCommand(MAV_CMD_NAV_RETURN_TO_LAUNCH);
      } else if (command.command == 2) {
        sendMavlinkCommand(MAV_CMD_NAV_LAND);
      }
    }
  }
}

void sendMavlinkCommand(uint16_t command) {
  mavlink_message_t msg;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];
  mavlink_command_long_t cmd = {0};

  cmd.target_system = SYSTEM_ID;
  cmd.target_component = COMPONENT_ID;
  cmd.command = command;
  cmd.confirmation = 0;
  cmd.param1 = 0;
  cmd.param2 = 0;
  cmd.param3 = 0;
  cmd.param4 = 0;
  cmd.param5 = 0;
  cmd.param6 = 0;
  cmd.param7 = 0;

  mavlink_msg_command_long_encode(SYSTEM_ID, COMPONENT_ID, &msg, &cmd);
  uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
  MAVLINK_SERIAL.write(buf, len);
}

//Error handling.
void serialErrorHandler(HardwareSerial* serialPort){
    if(serialPort->getWriteError()){
        Serial.println("Serial Write Error!");
        serialPort->clearWriteError();
    }
    if(serialPort->getOverrun()){
        Serial.println("Serial Overrun Error!");
        serialPort->clearOverrun();
    }
    if(serialPort->getFramingError()){
        Serial.println("Serial Framing Error!");
        serialPort->clearFramingError();
    }
    if(serialPort->getParityError()){
        Serial.println("Serial Parity Error!");
        serialPort->clearParityError();
    }
}

void loop(){
  sendTelemetry();
  receiveControlCommand();
  serialErrorHandler(&MAVLINK_SERIAL);
  serialErrorHandler(&MESHTASTIC_SERIAL);
  delay(50);
}
