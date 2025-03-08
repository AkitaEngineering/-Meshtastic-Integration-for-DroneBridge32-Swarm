#include <Arduino.h>
#include <HardwareSerial.h>
#include <mavlink.h>
#include <AES.h> // For Encryption

// Configuration
#define MESHTASTIC_SERIAL Serial1
#define BAUD_RATE 115200
#define DRONE_ID 1
#define MAVLINK_SERIAL Serial
#define SYSTEM_ID 255
#define COMPONENT_ID 1
#define SIGNAL_LOSS_TIMEOUT 5000 // Milliseconds

// Encryption
AES aes;
uint8_t encryptionKey[] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C}; // Example Key

// Fail Safes
unsigned long last_signal_time = 0;
bool signal_lost = false;
bool low_battery = false;

// Telemetry Data Structure
struct DroneTelemetry {
  float latitude;
  float longitude;
  float altitude;
  float batteryVoltage;
  uint8_t droneID;
};

void setup() {
  MESHTASTIC_SERIAL.begin(BAUD_RATE);
  MAVLINK_SERIAL.begin(57600);
  Serial.begin(115200);
  Serial.println("DroneBridge32 Meshtastic Integration - Telemetry");
  aes.setKey(encryptionKey, sizeof(encryptionKey));
}

void loop() {
  sendTelemetry();
  checkFailSafes();
  delay(50);
}

void sendTelemetry() {
  mavlink_message_t msg;
  mavlink_status_t status;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];

  while (MAVLINK_SERIAL.available() > 0) {
    uint8_t c = MAVLINK_SERIAL.read();
    if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
      if (msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT) {
        mavlink_global_position_int_t global_pos;
        mavlink_msg_global_position_int_decode(&msg, &global_pos);

        DroneTelemetry telemetryData;
        telemetryData.latitude = global_pos.lat / 10000000.0;
        telemetryData.longitude = global_pos.lon / 10000000.0;
        telemetryData.altitude = global_pos.alt / 1000.0;
        telemetryData.droneID = DRONE_ID;

        float voltage = getBatteryVoltage();
        if (voltage >= 0) {
          telemetryData.batteryVoltage = voltage;
          low_battery = (voltage < 11.0); // example low battery threshold
        } else {
          telemetryData.batteryVoltage = 0;
        }

        uint8_t buffer[sizeof(DroneTelemetry)];
        memcpy(buffer, &telemetryData, sizeof(DroneTelemetry));

        // Encrypt data
        uint8_t encrypted_buffer[sizeof(DroneTelemetry)];
        aes.encrypt(buffer, encrypted_buffer);

        MESHTASTIC_SERIAL.write(encrypted_buffer, sizeof(DroneTelemetry));
        last_signal_time = millis();
        signal_lost = false;

      }
    }
  }
}

float getBatteryVoltage() {
  mavlink_message_t msg;
  mavlink_status_t status;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];

  int timeout = 100;
  unsigned long startTime = millis();

  while (MAVLINK_SERIAL.available() > 0 && (millis() - startTime) < timeout) {
    uint8_t c = MAVLINK_SERIAL.read();
    if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
      if (msg.msgid == MAVLINK_MSG_ID_SYS_STATUS) {
        mavlink_sys_status_t sys_status;
        mavlink_msg_sys_status_decode(&msg, &sys_status);
        return sys_status.voltage_battery / 1000.0f;
      }
    }
  }
  return -1.0f;
}

void checkFailSafes() {
  if (millis() - last_signal_time > SIGNAL_LOSS_TIMEOUT && !signal_lost) {
    Serial.println("Signal loss detected. Initiating return to home.");
    sendMavlinkCommand(MAV_CMD_NAV_RETURN_TO_LAUNCH);
    signal_lost = true;
  }
  if (low_battery && !signal_lost) {
    Serial.println("Low battery detected. Initiating landing.");
    sendMavlinkCommand(MAV_CMD_NAV_LAND);
    low_battery = false;
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
  sendTelemetry();
  checkFailSafes();
  serialErrorHandler(&MAVLINK_SERIAL);
  serialErrorHandler(&MESHTASTIC_SERIAL);
  delay(50);
}
