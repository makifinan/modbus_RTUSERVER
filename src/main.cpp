
// https://github.com/eModbus/eModbus/discussions/96
// https://ipc2u.com/articles/knowledge-base/modbus-rtu-made-simple-with-detailed-descriptions-and-examples/

// =================================================================================================
// eModbus: Copyright 2020 by Michael Harwerth, Bert Melis and the contributors to ModbusClient
//               MIT license - see license.md for details
// =================================================================================================

// Example code to show the usage of the eModbus library.
// Please refer to root/Readme.md for a full description.

// Includes: <Arduino.h> for Serial etc.
#include <Arduino.h>

// Include the header for the ModbusClient RTU style
#include "ModbusClientRTU.h"

// Create a ModbusRTU client instance
// In my case the RS485 module had auto halfduplex, so no second parameter with the DE/RE pin is required!
ModbusClientRTU MB(Serial2);

// Define an onData handler function to receive the regular responses
// Arguments are Modbus server ID, the function code requested, the message data and length of it,
// plus a user-supplied token to identify the causing request
void handleData(ModbusMessage response, uint32_t token)
{
  Serial.printf("Response: serverID=%d, FC=%d, Token=%08X, length=%d:\n", response.getServerID(), response.getFunctionCode(), token, response.size());

  // for (auto &byte : response)
  // {
  //   Serial.printf("%02X ", byte);
  // }
  uint16_t myVar = 0;
  myVar = response[3];         // 1 Dec
  myVar <<= 8;                 // Left shit 8 bits, so from 1 Dec it's not 256 Dec. From 0x01 to 0x100;
  myVar = myVar | response[4]; // OR operation, basically 0x100 merged with 0x71, which will result in 0x171
  Serial.println(myVar, DEC);
  Serial.println("");
}

// Define an onError handler function to receive error responses
// Arguments are the error code returned and a user-supplied token to identify the causing request
void handleError(Error error, uint32_t token)
{
  // ModbusError wraps the error code and provides a readable error message for it
  ModbusError me(error);
  Serial.printf("Error response: %02X - %s\n", (int)me, (const char *)me);
}
uint32_t Token = 1111;
// Setup() - initialization happens here
void setup()
{
  // Init Serial monitor
  Serial.begin(115200);
  while (!Serial)
  {
  }
  Serial.println("__ OK __");

  // Set up Serial2 connected to Modbus RTU
  // (Fill in your data here!)
  Serial2.begin(9600, SERIAL_8N1, GPIO_NUM_16, GPIO_NUM_17); //RX-TX

  // Set up ModbusRTU client.
  // - provide onData handler function
  MB.onDataHandler(&handleData);
  // - provide onError handler function
  MB.onErrorHandler(&handleError);
  // Set message timeout to 2000ms
  MB.setTimeout(2000);
  // Start ModbusRTU background task
  MB.begin();
}

// loop() - nothing done here today!
void loop()
{

  Error err = MB.addRequest(Token++, 1, READ_HOLD_REGISTER, 1000, 1);
  if (err != SUCCESS)
  {
    ModbusError e(err);
    Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
  }
  delay(5000);
}