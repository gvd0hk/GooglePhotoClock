/*******************************************************************************
 * ESP32 Photo Frame
 * This is a simple IoT photo frame sample
 * Please find more details at instructables:
 * https://www.instructables.com/id/Face-Aware-OSD-Photo-Frame/
 * 
 * Setup steps:
 * 1. Fill your own SSID_NAME, SSID_PASSWORD and URL_TEMPLATE
 * 2. Change your LCD parameters in Arduino_GFX setting
 ******************************************************************************/

/* WiFi settings */
#define SSID_NAME "YourAP"
#define SSID_PASSWORD "PleaseInputYourPasswordHere"
/* Google Photo settings */
// replace your Google Photo share link
#define GOOGLE_PHOTO_SHARE_LINK "https://photos.app.goo.gl/ABCDefGhijklMNoPQ"
#define PHOTO_URL_PREFIX "https://lh3.googleusercontent.com/"
#define PHOTO_LIMIT 100                                    // first 100 photos add to the list
#define PHOTO_ID_SIZE 141                                  // the photo ID should be 140 charaters long and then add a zero-tail
#define PHOTO_FILE_BUFFER 99000                            // 99 kB, a 320 x 480 Google compressed photo should be aroud 60-90 kB
#define HTTP_TIMEOUT 60000                                 // in ms, wait a while for server processing
#define PHOTO_URL_TEMPLATE PHOTO_URL_PREFIX "%s=w%d-h%d-c" // photo id, display width and height
/* NTP settings */
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 28800L  // Timezone +0800
#define DAYLIGHT_OFFSET_SEC 0L // no daylight saving

/*******************************************************************************
 * Start of Arduino_GFX setting
 ******************************************************************************/
#include "SPI.h"
#include "Arduino_HWSPI.h"
#include "Arduino_ESP32SPI.h"
#include "Arduino_SWSPI.h"
#include "Arduino_GFX.h"            // Core graphics library
#include "Arduino_Canvas.h"         // Canvas (framebuffer) library
#include "Arduino_Canvas_Indexed.h" // Indexed Color Canvas (framebuffer) library
#include "Arduino_HX8347C.h"        // Hardware-specific library for HX8347C
#include "Arduino_HX8352C.h"        // Hardware-specific library for HX8352C
#include "Arduino_HX8357B.h"        // Hardware-specific library for HX8357B
#include "Arduino_ILI9225.h"        // Hardware-specific library for ILI9225
#include "Arduino_ILI9341.h"        // Hardware-specific library for ILI9341
#include "Arduino_ILI9481_18bit.h"  // Hardware-specific library for ILI9481
#include "Arduino_ILI9486_18bit.h"  // Hardware-specific library for ILI9486
#include "Arduino_SEPS525.h"        // Hardware-specific library for SEPS525
#include "Arduino_SSD1283A.h"       // Hardware-specific library for SSD1283A
#include "Arduino_SSD1331.h"        // Hardware-specific library for SSD1331
#include "Arduino_SSD1351.h"        // Hardware-specific library for SSD1351
#include "Arduino_ST7735.h"         // Hardware-specific library for ST7735
#include "Arduino_ST7789.h"         // Hardware-specific library for ST7789
#include "Arduino_ST7796.h"         // Hardware-specific library for ST7796

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
#define TFT_BL 32
#include "Arduino_ILI9341_M5STACK.h"
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(27 /* DC */, 14 /* CS */, SCK, MOSI, MISO);
Arduino_ILI9341_M5STACK *gfx = new Arduino_ILI9341_M5STACK(bus, 33 /* RST */, 1 /* rotation */);
#elif defined(ARDUINO_ODROID_ESP32)
#define TFT_BL 14
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(21 /* DC */, 5 /* CS */, SCK, MOSI, MISO);
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, -1 /* RST */, 3 /* rotation */);
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus,  -1 /* RST */, 1 /* rotation */, true /* IPS */);
#elif defined(ARDUINO_T) // TTGO T-Watch
#define TFT_BL 12
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(27 /* DC */, 5 /* CS */, 18 /* SCK */, 19 /* MOSI */, -1 /* MISO */);
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240, 240, 0, 80);
#else /* not a specific hardware */

#if defined(ESP32)
#define TFT_CS 5
// #define TFT_CS -1 // for display without CS pin
#define TFT_DC 16
// #define TFT_DC 27
// #define TFT_DC -1 // for display without DC pin (9-bit SPI)
#define TFT_RST 17
// #define TFT_RST 33
#define TFT_BL 22
#elif defined(ESP8266)
#define TFT_CS 15
#define TFT_DC 5
#define TFT_RST -1
// #define TFT_BL 4
#else
#define TFT_CS 20
#define TFT_DC 19
#define TFT_RST 18
#define TFT_BL 10
#endif

/*
 * Step 1: Initize one databus for your display
*/

// General software SPI
// Arduino_DataBus *bus = new Arduino_SWSPI(TFT_DC, TFT_CS, 18 /* SCK */, 23 /* MOSI */, -1 /* MISO */);

// General hardware SPI
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);

// ESP32 hardware SPI, more customizable parameters
// Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, 18 /* SCK */, 23 /* MOSI */, -1 /* MISO */, VSPI /* spi_num */);

/*
 * Step 2: Initize one driver for your display
*/

// Canvas (framebuffer)
// Arduino_ST7789 *output_display = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */);
// 16-bit color Canvas (240x320 resolution only works for ESP32 with PSRAM)
// Arduino_Canvas *gfx = new Arduino_Canvas(240, 320, output_display);
// Indexed color Canvas, mask_level: 0-2, larger mask level mean less color variation but can have faster index mapping
// Arduino_Canvas_Indexed *gfx = new Arduino_Canvas_Indexed(240, 320, output_display, MAXMASKLEVEL /* mask_level */);

// HX8347C IPS LCD 240x320
// Arduino_HX8347C *gfx = new Arduino_HX8347C(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// HX8352C IPS LCD 240x400
// Arduino_HX8352C *gfx = new Arduino_HX8352C(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// HX8357B IPS LCD 320x480
// Arduino_HX8357B *gfx = new Arduino_HX8357B(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// ILI9225 LCD 176x220
// Arduino_ILI9225 *gfx = new Arduino_ILI9225(bus, TFT_RST);

// ILI9341 LCD 240x320
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, TFT_RST);

// ILI9481 LCD 320x480
// Arduino_ILI9481_18bit *gfx = new Arduino_ILI9481_18bit(bus, TFT_RST);

// ILI9486 LCD 320x480
// Arduino_ILI9486_18bit *gfx = new Arduino_ILI9486_18bit(bus, TFT_RST);

// SEPS525 OLED 160x128
// Arduino_SEPS525 *gfx = new Arduino_SEPS525(bus, TFT_RST);

// SSD1283A OLED 130x130
// Arduino_SSD1283A *gfx = new Arduino_SSD1283A(bus, TFT_RST);

// SSD1331 OLED 96x64
// Arduino_SSD1331 *gfx = new Arduino_SSD1331(bus, TFT_RST);

// SSD1351 OLED 128x128
// Arduino_SSD1351 *gfx = new Arduino_SSD1351(bus, TFT_RST);

// ST7735 LCD
// 1.8" REDTAB 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST);
// 1.8" BLACKTAB 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 2 /* col offset 1 */, 1 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */, false /* BGR */);
// 1.8" GREENTAB A 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 2 /* col offset 1 */, 1 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */);
// 1.8" GREENTAB B 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 2 /* col offset 1 */, 3 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */);
// 1.8" Wide angle LCD 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 2 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 0 /* row offset 2 */, false /* BGR */);
// 1.5" GREENTAB B 128x128
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 128 /* height */, 2 /* col offset 1 */, 3 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */);
// 1.5" GREENTAB C 128x128
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 128 /* height */, 0 /* col offset 1 */, 32 /* row offset 1 */);
// 0.96" IPS LCD 80x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 3 /* rotation */, true /* IPS */, 80 /* width */, 160 /* height */, 26 /* col offset 1 */, 1 /* row offset 1 */, 26 /* col offset 2 */, 1 /* row offset 2 */);

// ST7789 LCD
// 2.4" LCD 240x320
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST);
// 2.4" IPS LCD 240x320
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */);
// 1.3"/1.5" square IPS LCD 240x240
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST, 2 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);
// 1.14" IPS LCD 135x240 TTGO T-Display
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */, 135 /* width */, 240 /* height */, 53 /* col offset 1 */, 40 /* row offset 1 */, 52 /* col offset 2 */, 40 /* row offset 2 */);

// ST7796 LCD
// 4" LCD 320x480
// Arduino_ST7796 *gfx = new Arduino_ST7796(bus, TFT_RST);
// 4" IPS LCD 320x480
// Arduino_ST7796 *gfx = new Arduino_ST7796(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

#endif /* not a specific hardware */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

/* WiFi and HTTPS */
#if defined(ESP32)
#include "esp_jpg_decode.h"
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
// HTTPS howto: https://techtutorialsx.com/2017/11/18/esp32-arduino-https-get-request/
const char *rootCACertificate =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n"
    "A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n"
    "Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n"
    "MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n"
    "A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n"
    "hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n"
    "v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n"
    "eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n"
    "tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n"
    "C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n"
    "zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n"
    "mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n"
    "V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n"
    "bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n"
    "3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n"
    "J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n"
    "291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n"
    "ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n"
    "AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n"
    "TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n"
    "-----END CERTIFICATE-----\n";
WiFiMulti WiFiMulti;
WiFiClientSecure *client = new WiFiClientSecure;
#else // ESP8266
#include "esp_jpg_decode.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
const uint8_t photos_app_fingerprint[20] = {0xC8, 0x23, 0xC6, 0x3C, 0x59, 0x23, 0x1A, 0x3E, 0x33, 0x67, 0xFD, 0xFC, 0xD5, 0x83, 0x64, 0xFE, 0x41, 0xAB, 0xA3, 0x57};
const uint8_t google_com_fingerprint[20] = {0x27, 0xE9, 0x1B, 0x9A, 0xD1, 0x94, 0x8D, 0x27, 0x40, 0x91, 0xA8, 0x87, 0x12, 0x55, 0x3B, 0x63, 0xD6, 0x05, 0xD3, 0x1F};
const uint8_t lh3_google_fingerprint[20] = {0x40, 0x50, 0xD7, 0xCA, 0x73, 0x60, 0xA7, 0x29, 0x14, 0x4B, 0x9D, 0xD4, 0x03, 0x01, 0x50, 0xB6, 0x1D, 0xA4, 0x1A, 0x21};
ESP8266WiFiMulti WiFiMulti;
std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
#endif

/* time library */
#include <time.h>
static time_t now;

/* HTTP */
const char *headerkeys[] = {"Location"};
HTTPClient https;
WiFiClient *httpsStream;

/* Google photo */
char photoIdList[PHOTO_LIMIT][PHOTO_ID_SIZE];
uint8_t *photoBuf;

/* variables */
static int w, h, timeX, timeY, len, offset, photoCount = 0;
static unsigned long next_show_millis = 0;

void ntpGetTime()
{
  // Initialize NTP settings
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print('.');
    yield();
    nowSecs = time(nullptr);
  }
  Serial.print(F(" done."));
}

void printTime()
{
  now = time(nullptr);
  const tm *tm = localtime(&now);
  int hour = tm->tm_hour;
  int min = tm->tm_min;
  // print time with border
  gfx->setCursor(timeX - 2, timeY - 2);
  gfx->setTextColor(DARKGREY);
  gfx->printf("%02d:%02d", hour, min);
  gfx->setCursor(timeX + 2, timeY + 2);
  gfx->setTextColor(BLACK);
  gfx->printf("%02d:%02d", hour, min);
  gfx->setCursor(timeX, timeY);
  gfx->setTextColor(WHITE);
  gfx->printf("%02d:%02d", hour, min);
}

void getPhotoList(const char *url)
{
#if defined(ESP32)
#else // ESP8266
  // hack: url tricks to select fingerprint
  if (url[8] == 'p')
  {
    client->setFingerprint(photos_app_fingerprint);
  }
  else
  {
    client->setFingerprint(google_com_fingerprint);
  }
#endif
  Serial.printf("[HTTPS] begin...\n");
  https.begin(*client, url);

  Serial.printf("[HTTPS] GET...\n");
  int httpCode = https.GET();

  Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
  if (httpCode <= 0)
  {
    Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    https.end();
    return;
  }

  // redirect
  if (httpCode == HTTP_CODE_FOUND)
  {
    char redirectUrl[1024];
    strcpy(redirectUrl, https.header((size_t)0).c_str());
    Serial.printf("redirectUrl: %s\r\n", redirectUrl);
    https.end();
    getPhotoList(redirectUrl);
    return;
  }

  if (httpCode != HTTP_CODE_OK)
  {
    Serial.printf("[HTTPS] Not OK!\n");
    https.end();
    return;
  }

  // HTTP header has been send and Server response header has been handled

  //find photo ID leading pattern: ",["https://lh3.googleusercontent.com/
  photoCount = 0;
  int reads = 0;
  int wait_count = 0;
  WiFiClient *stream = https.getStreamPtr();
  while ((stream->available()) || (wait_count < 5))
  {
    if (!stream->available())
    {
      delay(200); // wait some time to receive next truck
      ++wait_count;
      Serial.printf("reads: %d, wait_count: %d\n", reads, wait_count);
    }
    else
    {
      String line = stream->readStringUntil('\n');
      reads += line.length();
      ++reads; // \n
      int key_idx = line.indexOf(F("\",[\"" PHOTO_URL_PREFIX));
      if (key_idx >= 0)
      {
        int val_start_idx = key_idx + 38;
        int val_end_idx = line.indexOf('\"', val_start_idx);
        String photoId = line.substring(val_start_idx, val_end_idx);
        strcpy(photoIdList[photoCount], photoId.c_str());

        ++photoCount;
      }
    }

#if defined(ESP32)
    // notify WDT still working
    feedLoopWDT();
#else // ESP8266
    yield();
#endif
  }
  https.end();

  Serial.printf("reads: %d, %d photo ID added.\n", reads, photoCount);
  return;
}

void setup()
{
  Serial.begin(115200);

  // init display
  gfx->begin();
  gfx->fillScreen(BLACK);
  w = gfx->width();
  h = gfx->height();
  uint8_t textSize = w / 6 / 5;
  Serial.printf("textSize: %d\n", textSize);
  gfx->setTextSize(textSize);
  timeX = (w - (textSize * 5 * 6)) / 2;
  timeY = h - (textSize * 8) - 10;

  // init WiFi
  Serial.print(F("Connecting to WiFi: "));
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(SSID_NAME, SSID_PASSWORD);
  while ((WiFiMulti.run() != WL_CONNECTED))
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F(" done."));

  // init time
  ntpGetTime();

  // init HTTPClient
#if defined(ESP32)
  client->setCACert(rootCACertificate);

  // set WDT timeout a little bit longer than HTTP timeout
  esp_task_wdt_init((HTTP_TIMEOUT / 1000) + 1, true);
  enableLoopWDT();
#else // ESP8266
#endif
  https.collectHeaders(headerkeys, sizeof(headerkeys) / sizeof(char *));

  // allocate photo file buffer
  photoBuf = (uint8_t *)malloc(PHOTO_FILE_BUFFER);

#ifdef TFT_BL
  // finally turn on display backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif
}

void loop()
{
  if (WiFiMulti.run() != WL_CONNECTED)
  {
    // wait for WiFi connection
    delay(500);
  }
  else if (millis() < next_show_millis)
  {
    delay(1000);
  }
  else
  {
    next_show_millis = ((millis() / 60000L) + 1) * 60000L; // next minute

#if defined(ESP32)
#else // ESP8266
      wdt_disable();
#endif

    if (!photoCount)
    {
      getPhotoList(GOOGLE_PHOTO_SHARE_LINK);
    }

    if (photoCount)
    {
      char photoUrl[1024];
      // UNCOMMENT FOR DEBUG PHOTO LIST
      // for (int i = 0; i < photoCount; i++)
      // {
      //   sprintf(photoUrl, PHOTO_URL_TEMPLATE, photoIdList[i], w, h);
      //   Serial.printf("%d: %s\n", i, photoUrl);
      // }

      // setup url query value with LCD dimension
      int randomIdx = random(photoCount);
      sprintf(photoUrl, PHOTO_URL_TEMPLATE, photoIdList[randomIdx], w, h);
      Serial.printf("Random selected photo #%d: %s\n", randomIdx, photoUrl);

#if defined(ESP32)
#else // ESP8266
      client->setFingerprint(lh3_google_fingerprint);
#endif
      Serial.print("[HTTP] begin...\n");
      https.begin(*client, photoUrl);
      https.setTimeout(HTTP_TIMEOUT);
      Serial.print("[HTTP] GET...\n");
      int httpCode = https.GET();

      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      // HTTP header has been send and Server response header has been handled
      if (httpCode <= 0)
      {
        Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      else
      {
        if (httpCode != HTTP_CODE_OK)
        {
          Serial.printf("[HTTP] Not OK!\n");
          delay(9000); // don't repeat the wrong thing too fast
        }
        else
        {
          // get lenght of document (is -1 when Server sends no Content-Length header)
          len = https.getSize();
          Serial.printf("[HTTP] size: %d\n", len);

          if (len <= 0)
          {
            Serial.printf("[HTTP] Unknow content size: %d\n", len);
          }
          else
          {
            // get tcp stream
            httpsStream = https.getStreamPtr();
            // JPG decode option 1: http_stream_reader, decode on the fly but slower
            // esp_jpg_decode(len, JPG_SCALE_NONE, http_stream_reader, tft_writer, NULL /* arg */);

            // JPG decode option 2: buffer_reader, use much more memory but faster
            size_t r = 0;
            while (r < len)
            {
              r += httpsStream->readBytes(photoBuf + r, (len - r));
              Serial.printf("Photo buffer read: %d/%d\n", r, len);
            }
            esp_jpg_decode(len, JPG_SCALE_NONE, buffer_reader, tft_writer, NULL /* arg */);
          }
        }
      }
      https.end();

#if defined(ESP32)
#else // ESP8266
      wdt_enable(8000);
#endif
    }

    // overlay current time on the photo
    printTime();
  }

#if defined(ESP32)
  // notify WDT still working
  feedLoopWDT();
#else // ESP8266
  yield();
#endif
}

static size_t http_stream_reader(void *arg, size_t index, uint8_t *buf, size_t len)
{
  if (buf)
  {
    Serial.printf("[HTTP] read: %d\n", len);
    size_t a = httpsStream->available();
    size_t r = 0;
    while (r < len)
    {
      r += httpsStream->readBytes(buf + r, min((len - r), a));
      delay(50);
    }

    return r;
  }
  else
  {
    // Serial.printf("[HTTP] skip: %d\n", len);
    int l = len;
    while (l--)
    {
      httpsStream->read();
    }
    return len;
  }
}

static size_t buffer_reader(void *arg, size_t index, uint8_t *buf, size_t len)
{
  if (buf)
  {
    memcpy(buf, photoBuf + index, len);
  }
  return len;
}

static bool tft_writer(void *arg, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *data)
{
  if (data)
  {
    // Serial.printf("%d, %d, %d, %d\n", x, y, w, h);
    gfx->draw24bitRGBBitmap(x, y, data, w, h);
  }

#if defined(ESP32)
  // notify WDT still working
  feedLoopWDT();
#else // ESP8266
  yield();
#endif

  return true; // Continue to decompression
}
