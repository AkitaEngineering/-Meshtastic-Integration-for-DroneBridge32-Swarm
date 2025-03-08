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

void setup() {
  MESHTASTIC_SERIAL.begin(BAUD_RATE);
  MAVLINK_SERIAL.begin(57600); // Adjust to your MAVLink baud rate.
  Serial.begin(115200);
  Serial.println("DroneBridge32 Meshtastic Integration - Telemetry");
}

void loop() {
  sendTelemetry();
  delay(50); // Adjust delay as needed.
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
        telemetryData.droneID = DRONE_ID;

        //Retrieve battery voltage from SYS_STATUS message.
        float voltage = getBatteryVoltage();
        if (voltage >=0){
          telemetryData.batteryVoltage = voltage;
        }else{
          telemetryData.batteryVoltage = 0; //Default to 0 if battery voltage is not available.
        }

        uint8_t buffer[sizeof(DroneTelemetry)];
        memcpy(buffer, &telemetryData, sizeof(DroneTelemetry));
        MESHTASTIC_SERIAL.write(buffer, sizeof(DroneTelemetry));
      }
    }
  }
}

float getBatteryVoltage(){
    mavlink_message_t msg;
    mavlink_status_t status;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    int timeout = 100; //Timeout in milliseconds.
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
    return -1.0f; //Return negative value if battery voltage is not available.
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
  serialErrorHandler(&MAVLINK_SERIAL);
  serialErrorHandler(&MESHTASTIC_SERIAL);
  delay(50);
}
