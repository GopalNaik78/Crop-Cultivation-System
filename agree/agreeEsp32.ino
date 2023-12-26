#include <WiFi.h>
#include <WiFiClient.h>
#include <FirebaseESP32.h>
#include <TinyGPSPlus.h>
TinyGPSPlus gps;

// Firebase Credentials
#define FIREBASE_HOST "farmerfriend-50b62-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "2QzYBkLlZUaiuZThLHE9BIaeghJpFF7WO4V3dDdO"

#define Buzz 25
// WiFi Credentials
#define WIFI_SSID "dontTry"
#define WIFI_PASSWORD "12341234"

//Define FirebaseESP32 data object
FirebaseData firebaseData;
FirebaseJson json;
 
float lati, longi;
char buf1[9];
char buf2[9];

const byte nitro_inquiry_frame[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos_inquiry_frame[] = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota_inquiry_frame[] = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

byte values[11];

byte N, P, K;

void setup() {
  pinMode(Buzz, OUTPUT);
  Serial.begin(9600);
  Serial2.begin(9600);
  delay(3000);
#ifndef ESP32
  while (!Serial);
#endif
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

}

void loop() {
  if (WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
    digitalWrite(Buzz, HIGH);
    delay(5000);
    digitalWrite(Buzz, LOW);
  } else {
    if (Firebase.getString(firebaseData, "/Appliances/Read")) {
      if (firebaseData.stringData() == "1")
      {
        for (int i = 0; i < 5; i++) {
          N = nitrogen();
          delay(250);
          P = phosphorous();
          delay(250);
          K = potassium();
          delay(250);
          delay(250);
        }
        lati = 0.0;
        longi = 0.0;
        boolean newData = false;
        for (unsigned long start = millis(); millis() - start < 1000;)
        {
          while (Serial2.available())
          {
            if (gps.encode(Serial2.read()))
            {
              newData = true;
            }
          }
        }
        if (newData == true)
        {
          newData = false;
          UpdateGPS();
        }
        else
        {
          json.set("/lati", "0");
          Firebase.updateNode(firebaseData, "/Appliances", json);
          json.set("/lon", "0");
          Firebase.updateNode(firebaseData, "/Appliances", json);
        }
        json.set("/n", N);
        Firebase.updateNode(firebaseData, "/Appliances", json);
        json.set("/p", P);
        Firebase.updateNode(firebaseData, "/Appliances", json);
        json.set("/k", K);
        Firebase.updateNode(firebaseData, "/Appliances", json);
        json.set("/Read", "0");
        Firebase.updateNode(firebaseData, "/Appliances", json);
      }
    }
  }
}

byte nitrogen() {
  delay(10);
  if (Serial.write(nitro_inquiry_frame, sizeof(nitro_inquiry_frame)) == 8) {
    for (byte i = 0; i < 7; i++) {
      values[i] = Serial.read();
    }
  }
  return values[4]; // returns the Nitrogen value only, which is stored at location 4 in the array
}

byte phosphorous() {
  delay(10);
  if (Serial.write(phos_inquiry_frame, sizeof(phos_inquiry_frame)) == 8) {
    for (byte i = 0; i < 7; i++) {
      values[i] = Serial.read();
    }
  }
  return values[4];
}

byte potassium() {
  delay(10);
  if (Serial.write(pota_inquiry_frame, sizeof(pota_inquiry_frame)) == 8) {
    for (byte i = 0; i < 7; i++) {
      values[i] = Serial.read();
    }
  }
  return values[4];
}


void UpdateGPS() {
  if (gps.location.isValid()) {

    lati = gps.location.lat();
    dtostrf(lati, 9, 6, buf1);
    json.set("/lati", buf1);
    Firebase.updateNode(firebaseData, "/Appliances", json);

    longi = gps.location.lng();
    dtostrf(longi, 9, 6, buf2);
    json.set("/lon", buf2);
    Firebase.updateNode(firebaseData, "/Appliances", json);

    json.set("/errorMsg", "gps ok");
    Firebase.updateNode(firebaseData, "/Appliances", json);
  } else {
    json.set("/errorMsg", "invalid gps location");
    Firebase.updateNode(firebaseData, "/Appliances", json);
    json.set("/lati", "0");
    Firebase.updateNode(firebaseData, "/Appliances", json);
    json.set("/lon", "0");
    Firebase.updateNode(firebaseData, "/Appliances", json);
  }
}
