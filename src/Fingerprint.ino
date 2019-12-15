
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Adafruit_Fingerprint.h"
#include <Pins_Arduino.h>
#include "stdio.h"
#include "string.h"

#define PRINT_INFO(func, str) \
  Serial.print(#func);        \
  Serial.print(" -> ");  \
  Serial.print(str);          \
  Serial.print(" on line ");    \
  Serial.println(__LINE__);

#define CHECK_RESPONSE(func)                             \
  {                                                      \
    int rtc = func;                                      \
    switch (rtc)                                         \
    {                                                    \
    case FINGERPRINT_OK:                                 \
      PRINT_INFO(func, "FINGERPRINT_OK");               \
      break;                                             \
    case FINGERPRINT_NOFINGER:                           \
      PRINT_INFO(func, "FINGERPRINT_NOFINGER");         \
      break;                                             \
    case FINGERPRINT_PACKETRECIEVEERR:                   \
      PRINT_INFO(func, "FINGERPRINT_PACKETRECIEVEERR"); \
      break;                                             \
    case FINGERPRINT_IMAGEFAIL:                          \
      PRINT_INFO(func, "FINGERPRINT_IMAGEFAIL");        \
      break;                                             \
    default:                                             \
      PRINT_INFO(func, "default");                      \
      break;                                             \
    }                                                    \
  }

#define CHECK_RESPONSE_LOOP(func, value)                   \
  {                                                        \
    int rtc = -1;                                          \
    while (rtc != value)                                   \
    {                                                      \
      rtc = func;                                          \
      switch (rtc)                                         \
      {                                                    \
      case FINGERPRINT_OK:                                 \
        PRINT_INFO(func, "FINGERPRINT_OK");               \
        break;                                             \
      case FINGERPRINT_NOFINGER:                           \
        PRINT_INFO(func, "FINGERPRINT_NOFINGER");         \
        break;                                             \
      case FINGERPRINT_PACKETRECIEVEERR:                   \
        PRINT_INFO(func, "FINGERPRINT_PACKETRECIEVEERR"); \
        break;                                             \
      case FINGERPRINT_IMAGEFAIL:                          \
        PRINT_INFO(func, "FINGERPRINT_IMAGEFAIL");        \
        break;                                             \
      default:                                             \
        PRINT_INFO(func, "default");                      \
        break;                                             \
      }                                                    \
    }                                                      \
  }

// TODO: Configure interrupt pins
// Pin SCAN_FINGER_BUTTON is interrupt no. 0
#define SCAN_FINGER_BUTTON 2
// Pin UPDATE_DB_BUTTON is interrupt no. 1
#define UPDATE_DB_BUTTON 3
#define SCAN_FINGER_INTTERUPT 0
#define UPDATE_DB_INTERRUPT 1
#define SSID "chidori 1"
#define PASSWORD "chidori.vn"
#define HTTP_ADDRESS "http://jsonplaceholder.typicode.com/posts"
#define HTTP_HEADER "Content-Type", "text/plain"
#define SERVER_ERROR -2
#define SEND_ERROR -1
#define SEND_OK 0
#define ENROLL 0
#define UPDATE_DB 1
#define REGISTER_FINGERPRINT 2
// For UNO and others without hardware serial, we must use software serial...
// pin #13 is IN from sensor (GREEN wire)
// pin #15 is OUT from arduino(YELLOW wire)

SoftwareSerial mySerial(13, 15);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
HTTPClient http;
volatile int mode = ENROLL;
void setup()
{
  // When a button is pressed, the interrupt pin is drived to LOW to signal
  // that an event occurrs
  // Place a button between SCAN_FINGER_BUTTON and GND
  pinMode(SCAN_FINGER_BUTTON, INPUT_PULLUP);
  attachInterrupt(SCAN_FINGER_INTTERUPT, registerFingerprint, FALLING);
  // Place a button  between UPDATE_DB_BUTTON and GND
  pinMode(UPDATE_DB_BUTTON, INPUT_PULLUP);
  attachInterrupt(UPDATE_DB_INTERRUPT, updateDB, FALLING);
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
    break;
  }
}

void registerFingerprint()
{
  if (mode != REGISTER_FINGERPRINT)
  {
    mode = REGISTER_FINGERPRINT;
    // TODO: indicate LED to notify system is in REGISTER_FINGERPRINT mode
  }
  else
  {
    mode = ENROLL;
  }
}

void updateDB()
{
  if (mode != UPDATE_DB)
  {
    mode = UPDATE_DB;
    // TODO: indicate LED to notify system is in UPDATE_DB mode
  }
  else
  {
    mode = ENROLL;
  }
}

void enrollHandler()
{
  int rtc;
  // To hold string header + template + null terminated character
  char myTemplate[1043];
  memset(myTemplate, 0x24, sizeof(myTemplate));
  // Get image of fingerprint
  CHECK_RESPONSE_LOOP(finger.getImage(), FINGERPRINT_OK);
  // Convert image to character file and store to CharBuffer1
  CHECK_RESPONSE(finger.image2Tz());
  // Search this fingerprint in ROM
  rtc = finger.fingerFastSearch();
  if (rtc == FINGERPRINT_OK)
  {
    // Load finger template from ID
    CHECK_RESPONSE(finger.loadModel(finger.fingerID));
    // Get template from CharBuffer1 and store it to internal buffer
    CHECK_RESPONSE(finger.getModel());
    // Convert array of raw bytes to array of hexa characer for human reading purpose
    convertTemplateToString(finger.fingerTemplate, myTemplate);
    // Send template to server
    rtc = sendTemplateToServer(&myTemplate[0]);
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
    rtc = sendTemplateToServer("ERROR");
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
  char myTemplate[1043];
  memset(myTemplate, 0x24, sizeof(myTemplate));
  // Get image of fingerprint
  CHECK_RESPONSE_LOOP(finger.getImage(), FINGERPRINT_OK);
  // Convert image to character file and store to CharBuffer1
  CHECK_RESPONSE(finger.image2Tz(1));
  Serial.println("Remove finger");
  delay(1000);
  // Get image of fingerprint, again
  CHECK_RESPONSE_LOOP(finger.getImage(), FINGERPRINT_NOFINGER);
  Serial.println("Place same finger again");
  CHECK_RESPONSE_LOOP(finger.getImage(), FINGERPRINT_OK);
  // Convert image to character file and store to CharBuffer2
  CHECK_RESPONSE(finger.image2Tz(2));
  // Create template from CharBuffer1 and CharBuffer2, then store back to
  // both CharBuffer1 and CharBuffer2 (templates are same on 2 buffer)
  CHECK_RESPONSE(finger.createModel());
  // Get template from CharBuffer1 and store it to internal buffer
  CHECK_RESPONSE(finger.getModel());
  // Convert array of raw bytes to array of hexa characer for human reading purpose
  convertTemplateToString(finger.fingerTemplate, myTemplate);
  // Send template to server
  rtc = sendTemplateToServer(&myTemplate[0]);
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
void convertTemplateToString(char *bytesTemplate, char *strTemplate)
{
  for (int i = 0; i < 512; i++)
  {
    sprintf(&strTemplate[i * 2 + 18], "%02X", bytesTemplate[i]);
  }
  strTemplate[1025] = '\0';
}

/**************************************************************************/
/*!
    @brief  Send template to HTTP Server
    @param  None
*/
/**************************************************************************/
int sendTemplateToServer(char *strTemplate)
{
  int rtc = SEND_ERROR;
  if (WiFi.status() == WL_CONNECTED)
  {
    memcpy(&strTemplate[0], WiFi.macAddress().c_str(), 17);
    strTemplate[17] = ',';
    int httpCode = http.POST(strTemplate);
    if (httpCode > 0)
    {
      //Get the request response payload
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
}
