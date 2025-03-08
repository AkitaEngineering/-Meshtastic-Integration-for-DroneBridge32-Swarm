#include <Arduino.h>
#include <HardwareSerial.h>
#include <mavlink.h>
#include <AES.h> // For Decryption

// Configuration
#define MESHTASTIC_SERIAL Serial1
#define BAUD_RATE 115200
#define DRONE_ID 1
#define MAVLINK_SERIAL Serial
#define SYSTEM_ID 255
#define COMPONENT_ID 1

// Encryption
AES aes;
uint8_t decryptionKey[] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C}; // Example Key

// Control Command Structure
struct DroneControlCommand {
  uint8_t droneID;
  int command;
};

void setup() {
  MESHTASTIC_SERIAL.begin(BAUD_RATE);
  MAVLINK_SERIAL.begin(57600);
  Serial.begin(115200);
  Serial.println("DroneBridge32 Meshtastic Integration - Control");
  aes.setKey(decryptionKey, sizeof(decryptionKey));
}

void loop() {
  receiveControlCommand();
  delay(50);
}

void receiveControlCommand() {
  if (MESHTASTIC_SERIAL.available() >= sizeof(DroneControlCommand)) {
    uint8_t encrypted_buffer[sizeof(DroneControlCommand)];
    MESHTASTIC_SERIAL.readBytes(encrypted_buffer, sizeof(DroneControlCommand));

    uint8_t decrypted_buffer[sizeof(DroneControlCommand)];
    aes.decrypt(encrypted_buffer, decrypted_buffer);

    DroneControlCommand command;
    memcpy(&command, decrypted_buffer, sizeof(DroneControlCommand));

    if (command.droneID == DRONE_ID) {
      Serial.print("Received command: ");
      Serial.println(command.command);

      if (command.command == 1) {
        sendMavlinkCommand(MAV_CMD_NAV_RETURN_TO_LAUNCH);
        Serial.println("Sending Return to Launch MAVLink command.");
      } else if (command.command == 2) {
        sendMavlinkCommand(MAV_CMD_NAV_LAND);
        Serial.println("Sending Land MAVLink command.");
      } else if (command.command == 3) {
        sendMavlinkCommand(MAV_CMD_NAV_LAND); // Emergency landing is just a land command for this example.
        Serial.println("Sending Emergency Land MAVLink command.");
      } else {
        Serial.println("Unknown command.");
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

void serialErrorHandler(HardwareSerial* serialPort) {
  if (serialPort->getWriteError()) {
    Serial.println("Serial Write Error!");
    serialPort->clearWriteError();
  }
  if (serialPort->getOverrun()) {
    Serial.println("Serial Overrun Error!");
    serialPort->clearOverrun();
  }
  if (serialPort->getFramingError()) {
    Serial.println("Serial Framing Error!");
    serialPort->clearFramingError();
  }
  if (serialPort->getParityError()) {
    Serial.println("Serial Parity Error!");
    serialPort->clearParityError();
  }
}

void loop() {
  receiveControlCommand();
  serialErrorHandler(&MAVLINK_SERIAL);
  serialErrorHandler(&MESHTASTIC_SERIAL);
  delay(50);
}
