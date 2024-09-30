// เขียนโปรแกรมโดย เด็กชายกรวิทย์ สุพร ม.3/6 เลขที่ 1 รหัสนักเรียน 44478 โรงเรียนสิงห์บุรี
// เขียนเพื่อแข่งขันศิลปหัตกรรมในการแข่งขันผลงานสิ่งประดิษฐ์ทางวิทยาศาสตร์ ครั้งที่ 71 ระดับมัธยมศึกษาตอนต้น
// ต้องลงไลบรารี Firebase Arduino Client Library เพื่อใช้งาน


unsigned long Movement_State = 0;  // in firebase set, get
unsigned long Movement_Maxround = 0;
unsigned long Movement_Bed_Point = 0;


#define IRL D7
#define IRR D6


int IRLread;
int IRRread;


#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4


#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif


#include <Firebase_ESP_Client.h>


#include <addons/TokenHelper.h>


#include <addons/RTDBHelper.h>
 
#define WIFI_SSID "" //ใช้ชื่อ wifiที่ต้องเชื่อต่อ
#define WIFI_PASSWORD "" //ใช้รหัส wifi ที่ต้องเชื่มต่อ
 
#define API_KEY "" //ใช้ Api ของ Firebase ที่สร้าง


#define DATABASE_URL "" //ใช้ลิ่งของ Firebase ที่สร้าง


#define USER_EMAIL "" //ใช้ชื่อของ fIREBASE เพื่อเข้าถึง
#define USER_PASSWORD "" //ใช้รหัสของ fIREBASE เพื่อเข้าถึง


FirebaseData fbdo;


FirebaseAuth auth;
FirebaseConfig config;


unsigned long sendDataPrevMillis = 0;


unsigned long count = 0;


int i = 0;


#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif


void setup() {


  pinMode(IRL, INPUT);
  pinMode(IRL, INPUT);


  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);


  Serial.begin(115200);


  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);


#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  multi.addAP(WIFI_SSID, WIFI_PASSWORD);
  multi.run();
#else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif


  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    if (millis() - ms > 10000)
      break;
#endif
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();


  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);


  config.api_key = API_KEY;


  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;


  config.database_url = DATABASE_URL;


  config.token_status_callback = tokenStatusCallback;


  Firebase.reconnectNetwork(true);


  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);


  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);


  Firebase.begin(&config, &auth);


  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP();
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif


  Firebase.setDoubleDigits(5);


  config.timeout.serverResponse = 10 * 1000;
}


void loop() {
  if (Firebase.ready()) {
    if (count == 1) {
      Firebase.RTDB.setInt(&fbdo, F("/2Movement/0State"), 0);
      Firebase.RTDB.setBool(&fbdo, F("/2Movement/3Pointstatus"), true);
      count = 0;
    } else if (count == 2) {
      Firebase.RTDB.setInt(&fbdo, F("/2Movement/0State"), 0);
      Firebase.RTDB.setBool(&fbdo, F("/2Movement/3Pointstatus"), false);
      count = 0;
    }
    Movement_State = Firebase.RTDB.getInt(&fbdo, F("/2Movement/0State")) ? fbdo.to<int>() : 0;
    Movement_Maxround = Firebase.RTDB.getInt(&fbdo, F("/2Movement/1Maxround")) ? fbdo.to<int>() : 0;
    Movement_Bed_Point = Firebase.RTDB.getInt(&fbdo, F("/2Movement/2Bed-Point")) ? fbdo.to<int>() : 0;


    if (Movement_State == 1) {
      for (i = 0; i < Movement_Bed_Point; i++) {
        Serial.printf("Movement State : %d\n", Movement_State);
        goingtopoint();
      }
      count = 1;
    } else if (Movement_State == 2) {
      for (i; i < Movement_Maxround; i++) {
        Serial.printf("Movement State : %d\n", Movement_State);
        goingtopoint();
        delay(3000);
      }
      count = 2;
    } else {
      Serial.printf("Movement State : %d\n", Movement_State);
      Serial.printf("Movement Point : %d\n", Movement_Bed_Point);
      Serial.printf("Movement Round : %d\n", Movement_Maxround);
      Serial.printf("         Count : %d\n", count);
      delay(3000);
    }
    delay(15000);
  }
}


void goingtopoint() {
  while (1) {
    IRLread = digitalRead(IRL);
    IRRread = digitalRead(IRR);


    if (IRLread == 1 && IRRread == 1) {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
    } else if (IRLread == 0 || IRLread == 1) {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, HIGH);
      break;
    } else if (IRRread == 0 || IRRread == 1) {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, HIGH);
      break;
    }
    delay(20);
  }


  while (1) {
    IRLread = digitalRead(IRL);
    IRRread = digitalRead(IRR);


    if (IRLread == 0) {
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
    } else if (IRLread == 1) {
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
    }


    if (IRRread == 0) {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
    } else if (IRRread == 1) {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
    }


    if (IRLread == 1 && IRRread == 1) {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, HIGH);
      break;
    }
    delay(20);
  }
}

