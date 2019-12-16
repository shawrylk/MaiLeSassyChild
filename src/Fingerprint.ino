
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Adafruit_Fingerprint.h"
#include <Pins_Arduino.h>
#include "stdio.h"
#include "string.h"
#include <ArduinoJson.h>

#define PRINT_INFO(func, str) \
  Serial.print(#func);        \
  Serial.print(" -> ");       \
  Serial.print(str);          \
  Serial.print(" on line ");  \
  Serial.println(__LINE__);

#define SWITCH_RTC(func, rtc)                         \
  switch (rtc)                                        \
  {                                                   \
  case FINGERPRINT_OK:                                \
    PRINT_INFO(func, "FINGERPRINT_OK");               \
    break;                                            \
  case FINGERPRINT_NOFINGER:                          \
    PRINT_INFO(func, "FINGERPRINT_NOFINGER");         \
    break;                                            \
  case FINGERPRINT_PACKETRECIEVEERR:                  \
    PRINT_INFO(func, "FINGERPRINT_PACKETRECIEVEERR"); \
    break;                                            \
  case FINGERPRINT_IMAGEFAIL:                         \
    PRINT_INFO(func, "FINGERPRINT_IMAGEFAIL");        \
    break;                                            \
  default:                                            \
    PRINT_INFO(func, "default");                      \
    break;                                            \
  }

#define CHECK_RESPONSE(func) \
  {                          \
    int rtc = func;          \
    SWITCH_RTC(func, rtc)    \
  }

#define CHECK_RESPONSE_LOOP(func, value) \
  {                                      \
    int rtc = -1;                        \
    while (rtc != value)                 \
    {                                    \
      rtc = func;                        \
      SWITCH_RTC(func, rtc)              \
    }                                    \
  }

#define SSID "chidori 1"
#define PASSWORD "chidori.vn"
#define HTTP_ADDRESS "http://jsonplaceholder.typicode.com/posts"
#define HTTP_HEADER "Content-Type", "text/plain"
#define SERVER_ERROR -2
#define SEND_ERROR -1
#define SEND_OK 0
#define ENROLL 0
#define UPDATE_DB 2
#define REGISTER_FINGERPRINT 1
#define JSON_LENGTH 1200
// For UNO and others without hardware serial, we must use software serial...
// pin #13 is IN from sensor (GREEN wire)
// pin #15 is OUT from arduino(YELLOW wire)

SoftwareSerial mySerial(13, 15);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
HTTPClient http;
volatile int mode = ENROLL;
StaticJsonBuffer<(int)JSON_LENGTH> jsonBuffer;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
  };
  delay(100);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print("Connecting..");
  }
  // Set the data rate for the sensor serial port
  finger.begin(57600);
  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");
    http.begin(HTTP_ADDRESS); //Specify request destination
    http.addHeader(HTTP_HEADER);
    int httpCode = http.POST("{\"Request\": \"Mode\"}");
    if (httpCode > 0)
    {
      String payload = http.getString();
      JsonObject &root = jsonBuffer.parseObject(payload);
      if (!root.success())
      {
        Serial.println("parseObject() failed");
        return;
      }
      mode = root["Mode"];
      switch (mode)
      {
      case ENROLL:
        Serial.println("Mode: ENROLL");
        break;
      case REGISTER_FINGERPRINT:
        Serial.println("Mode: REGISTER_FINGERPRINT");
        break;
      case UPDATE_DB:
        Serial.println("Mode: UPDATE_DB");
        break;
      default:
        Serial.println("Mode: invalid");
        return;
      }
    }
  }
  else
  {
    Serial.println("Did not find fingerprint sensor :(. Press reset to start again");
    ESP.deepSleep(0);
  }
}

void loop()
{
  switch (mode)
  {
  case ENROLL:
    enrollHandler();
    break;
  case REGISTER_FINGERPRINT:
    registerFingerprintHandler();
    break;
  case UPDATE_DB:
    updateDBHandler();
    break;
  default:
    Serial.println("Mode error");
    mode = 0;
    break;
  }
}

void enrollHandler()
{
  int rtc;
  char id[8];
  memset(&id, 0x00, sizeof(id));
  // To hold string header + template + null terminated character
  String myID;
  myID.reserve(JSON_LENGTH);
#ifdef __STUB__
  delay(5);
#endif
  Serial.println("Please place finger");
  // Get image of fingerprint
  CHECK_RESPONSE_LOOP(finger.getImage(), FINGERPRINT_OK);
  // Convert image to character file and store to CharBuffer1
  CHECK_RESPONSE(finger.image2Tz());
  // Search this fingerprint in ROM
  rtc = finger.fingerFastSearch();
  // Convert finger ID to string
  sprintf(id, "%d", finger.fingerID);
  if (rtc == FINGERPRINT_OK)
  {
    myID = "{\"ID\": \"";
    myID += id;
    myID += "\", \"Enroll\": \"true\", ";
    // Send template to server
    rtc = sendInfoToServer(&myID);
    // TODO: handle abnormal case
    // switch (rtc)
    // {
    // case SEND_ERROR:
    //   break;
    // case SEND_OK:
    //   break;
    // default:
    //   break;
    // }
  }
  else
  {
    myID = "{\"ID\": \"";
    myID += id;
    myID += "\", \"Enroll\": \"false\", ";
    // Send template to server
    rtc = sendInfoToServer(&myID);
    // TODO: handle abnormal case
    // switch (rtc)
    // {
    // case SEND_ERROR:
    //   break;
    // case SEND_OK:
    //   break;
    // default:
    //   break;
    // }
  }
}
/**************************************************************************/
/*!
    @brief  Start scanning fingerprint, then send the template to server,
      notice that this fingerprint is not save to sensors ROM
    @param  None
*/
/**************************************************************************/
void registerFingerprintHandler()
{
  int rtc = -1;
  // To hold string header + template + null terminated character
  String myTemplate;
  myTemplate.reserve(JSON_LENGTH);

  Serial.println("Please place finger");
  // Get image of fingerprint
  CHECK_RESPONSE_LOOP(finger.getImage(), FINGERPRINT_OK);
  // Convert image to character file and store to CharBuffer1
  CHECK_RESPONSE(finger.image2Tz(1));
  Serial.println("Remove finger");
  delay(1000);
// Remove finger to prepare for next step
#ifdef __STUB__
  delay(5);
#else
  CHECK_RESPONSE_LOOP(finger.getImage(), FINGERPRINT_NOFINGER);
#endif
  Serial.println("Place same finger again");
  // Get image of fingerprint, again
  CHECK_RESPONSE_LOOP(finger.getImage(), FINGERPRINT_OK);
  // Convert image to character file and store to CharBuffer2
  CHECK_RESPONSE(finger.image2Tz(2));
  // Create template from CharBuffer1 and CharBuffer2, then store back to
  // both CharBuffer1 and CharBuffer2 (templates are same on 2 buffer)
  CHECK_RESPONSE(finger.createModel());
  // Get template from CharBuffer1 and store it to internal buffer
  CHECK_RESPONSE(finger.getModel());
  // Convert array of raw bytes to array of hexa characer for human reading purpose
  convertTemplateToString(finger.fingerTemplate, &myTemplate);
  // Send template to server
  rtc = sendInfoToServer(&myTemplate);
  // TODO: handle abnormal case
  // switch (rtc)
  // {
  // case SEND_ERROR:
  //   break;
  // case SEND_OK:
  //   break;
  // default:
  //   break;
  // }
}

/**************************************************************************/
/*!
    @brief  Convert array of bytes to array of character
    @param  None
*/
/**************************************************************************/
void convertTemplateToString(char *bytesTemplate, String *strTemplate)
{
  char charTemplate[1025];
  for (int i = 0; i < 512; i++)
  {
    sprintf(&charTemplate[i * 2], "%02X", bytesTemplate[i]);
  }
  *strTemplate = "{\"Template\": \"";
  *strTemplate += charTemplate;
  *strTemplate += "\", ";
}

/**************************************************************************/
/*!
    @brief  Send template to HTTP Server
    @param  None
*/
/**************************************************************************/
int sendInfoToServer(String *strTemplate)
{
  int rtc = SEND_ERROR;
  if (WiFi.status() == WL_CONNECTED)
  {
    *strTemplate += "\"MAC\": \"";
    *strTemplate += WiFi.macAddress().c_str();
    *strTemplate += "\"}";
    int httpCode = http.POST(strTemplate->c_str());
    if (httpCode > 0)
    {
      //Get the response payload
      String payload = http.getString();
      //Print the response payload
      Serial.println(payload);
      // TODO: check payload to decide whether it OK or NG, we assume it OK here
      rtc = SEND_OK;
    }
    return rtc;
  }
}

/**************************************************************************/
/*!
    @brief  Request HTTP server to send back fingerprint DB to save to ROM
    @param  None
*/
/**************************************************************************/
void updateDBHandler()
{
  Serial.println("Update DataBase");
#ifdef __STUB__
  delay(5);
  ESP.deepSleep(0);
#endif
  // int httpCode = http.POST("{\"Request\": \"Mode\"}");
  // if (httpCode > 0)
  // {
  //   String payload = http.getString();
  //   JsonObject &root = jsonBuffer.parseObject(payload);
  //   if (!root.success())
  //   {
  //     Serial.println("parseObject() failed");
  //     return;
  //   }
  //   mode = root["Mode"];
  //   switch (mode)
  //   {
  //   case ENROLL:
  //     Serial.println("Mode: ENROLL");
  //     break;
  //   case REGISTER_FINGERPRINT:
  //     Serial.println("Mode: REGISTER_FINGERPRINT");
  //     break;
  //   case UPDATE_DB:
  //     Serial.println("Mode: UPDATE_DB");
  //     break;
  //   default:
  //     Serial.println("Mode: invalid");
  //     return;
  //   }
  // }
}
