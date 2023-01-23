
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
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#include <math.h> 
#include <stdio.h> 
#include <string.h>

//Veritabanına düzgün bağlanmak için gerekli olan ilaveler
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Ağ bilgilerinizi girin
#define WIFI_SSID "Mehmetakif"
#define WIFI_PASSWORD "aaaaaaaa"

// Firebase proje API Key'i girin
#define API_KEY "AIzaSyDE76wY-NuBYIni7FRr8vVYJYy6eCmLixE"

// Veritabanı URL'sini girin */
#define DATABASE_URL "https://wateranalysis-6a3af-default-rtdb.europe-west1.firebasedatabase.app"

//Bir Firebase veri objesi oluşturalım
FirebaseData fbdo;
//yetki ve ayar nesneleri oluşturalım
FirebaseAuth auth;
FirebaseConfig config;
//gerekli değişken tanımları
unsigned long sendDataPrevMillis = 0;
//unsigned long getDataPrevMillis = 0;
bool signupOK = false;

float pHDegeri;
float clorDegeri;
float sicaklikDegeri;

void handleData(ModbusMessage response, uint32_t token)
{
  Serial.printf("Response: serverID=%d, FC=%d, Token=%08X, length=%d:\n", response.getServerID(), response.getFunctionCode(), token, response.size());

  // for (auto &byte : response)
  // {
  //   Serial.printf("%02X ", byte);
  // }
  uint16_t pH = 0;
  pH = response[3];         // 1 Dec
  pH <<= 8;                 // Left shit 8 bits, so from 1 Dec it's not 256 Dec. From 0x01 to 0x100;
  pH = pH | response[4];    // OR operation, basically 0x100 merged with 0x71, which will result in 0x171
  Serial.println(pH, DEC);
  pHDegeri = (float) pH / 100;
  //pHDegeri=pH/100;
  Serial.println(pHDegeri);
  Serial.println("");
  
  

  uint16_t clor = 0;
  clor = response[5];         // 1 Dec
  clor <<= 8;                 // Left shit 8 bits, so from 1 Dec it's not 256 Dec. From 0x01 to 0x100;
  clor = clor | response[6]; // OR operation, basically 0x100 merged with 0x71, which will result in 0x171
  Serial.println(clor, DEC);
  clorDegeri = (float) clor / 100;
  //clorDegeri = clor/10;
  Serial.println(clorDegeri);
  Serial.println("");

  uint16_t sicaklik = 0;
  sicaklik = response[9];         // 1 Dec
  sicaklik <<= 8;                 // Left shit 8 bits, so from 1 Dec it's not 256 Dec. From 0x01 to 0x100;
  sicaklik = sicaklik | response[10]; // OR operation, basically 0x100 merged with 0x71, which will result in 0x171
  Serial.println(sicaklik, DEC);
  sicaklikDegeri = (float) sicaklik / 10;
  //sicaklikDegeri = sicaklik/10;
  Serial.println(sicaklikDegeri);
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
  Serial2.begin(9600, SERIAL_8N1, GPIO_NUM_18, GPIO_NUM_19); //RX-TX

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Ağa bağlaniyor");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Bağlandi. IP Adresi: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* yukarıdaki API keyi ayarlara atayalım */
  config.api_key = API_KEY;

  /* veritabanı URL'sini ayarlara atayalım */
  config.database_url = DATABASE_URL;
  
  /* giriş yapalım */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* token'in geçerlilik durumu kontrolü için gerekli */
  config.token_status_callback = tokenStatusCallback; 
//bağlantıyı başlatalım
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Set up ModbusRTU client.
  // - provide onData handler function
  MB.onDataHandler(&handleData);
  // - provide onError handler function
  MB.onErrorHandler(&handleError);
  // Set message timeout to 2000ms
  MB.setTimeout(2000);
  // Start ModbusRTU background task
  MB.begin();

  
  
  

  //Serial.begin(115200);
  



}

// loop() - nothing done here today!
void loop()
{

  Error err = MB.addRequest(Token++, 1, READ_HOLD_REGISTER, 1000, 6);
  if (err != SUCCESS)
  {
    ModbusError e(err);
    Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
  }
  delay(1000);

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 60000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    
    if (Firebase.RTDB.setFloat(&fbdo, "data/klor", clorDegeri) && Firebase.RTDB.setFloat(&fbdo, "data/ph", pHDegeri) && Firebase.RTDB.setFloat(&fbdo, "data/sicaklik", sicaklikDegeri)) {
      Serial.println("YAZMA TAMAM");
     // Serial.println("DİZİN: " + fbdo.dataPath());
      //Serial.println("VERİ TİPİ: " + fbdo.dataType());
    }
    else {
      Serial.println("HATA");
      //Serial.println("HATA SEBEBİ: " + fbdo.errorReason());
    }
    
  }
}