#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <FirebaseESP32.h>

// WiFi credentials
#define WIFI_SSID "Wokwi-GUEST" // Thay bằng SSID WiFi của bạn
#define WIFI_PASSWORD "" // Thay bằng mật khẩu WiFi

// Firebase credentials
#define FIREBASE_HOST "https://iot-web-a76b6-default-rtdb.asia-southeast1.firebasedatabase.app/" // Ví dụ: https://your-project-id.firebaseio.com
#define FIREBASE_AUTH "AIzaSyBe2Jz-2XUr59QTrlG-oLmvpEXayDraz84" // API Key từ Firebase Console

// DHT22 sensor configuration
#define DHTPIN_LIVING 4  // Phòng khách
#define DHTPIN_BEDROOM 5 // Phòng ngủ
#define DHTTYPE DHT22
DHT dhtLiving(DHTPIN_LIVING, DHTTYPE);
DHT dhtBedroom(DHTPIN_BEDROOM, DHTTYPE);

// LDR configuration
#define LDR_PIN_LIVING 34  // GPIO34 (ADC1) - Phòng khách
#define LDR_PIN_BEDROOM 35 // GPIO35 (ADC1) - Phòng ngủ

// Relay pins
#define RELAY_LIGHT_LIVING 14 // GPIO14 - Đèn Phòng khách
#define RELAY_FAN_LIVING 27   // GPIO27 - Quạt Phòng khách
#define RELAY_AC_LIVING 26    // GPIO26 - Điều hòa Phòng khách
#define RELAY_LIGHT_BEDROOM 13 // GPIO13 - Đèn Phòng ngủ
#define RELAY_FAN_BEDROOM 12   // GPIO12 - Quạt Phòng ngủ
#define RELAY_AC_BEDROOM 15    // GPIO15 - Điều hòa Phòng ngủ

float temperatureLiving, humidityLiving, lightLevelLiving;
float temperatureBedroom, humidityBedroom, lightLevelBedroom;
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

void setup() {
  Serial.begin(9600);

  // Khởi tạo chân relay
  pinMode(RELAY_LIGHT_LIVING, OUTPUT);
  pinMode(RELAY_FAN_LIVING, OUTPUT);
  pinMode(RELAY_AC_LIVING, OUTPUT);
  pinMode(RELAY_LIGHT_BEDROOM, OUTPUT);
  pinMode(RELAY_FAN_BEDROOM, OUTPUT);
  pinMode(RELAY_AC_BEDROOM, OUTPUT);
  digitalWrite(RELAY_LIGHT_LIVING, LOW);
  digitalWrite(RELAY_FAN_LIVING, LOW);
  digitalWrite(RELAY_AC_LIVING, LOW);
  digitalWrite(RELAY_LIGHT_BEDROOM, LOW);
  digitalWrite(RELAY_FAN_BEDROOM, LOW);
  digitalWrite(RELAY_AC_BEDROOM, LOW);

  // Khởi tạo DHT
  dhtLiving.begin();
  dhtBedroom.begin();

  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Connected to WiFi!");
  Serial.println(WiFi.localIP());

  // Khởi tạo Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void sendSensorData() {
  // Đọc cảm biến DHT22 - Phòng khách
  humidityLiving = dhtLiving.readHumidity();
  temperatureLiving = dhtLiving.readTemperature();

  // Đọc cảm biến LDR - Phòng khách
  int ldrValueLiving = analogRead(LDR_PIN_LIVING); // 0-4095
  lightLevelLiving = map(ldrValueLiving, 4095, 0, 0, 1000); // Chuyển thành 0-1000 lux

  // Đọc cảm biến DHT22 - Phòng ngủ
  humidityBedroom = dhtBedroom.readHumidity();
  temperatureBedroom = dhtBedroom.readTemperature();

  // Đọc cảm biến LDR - Phòng ngủ
  int ldrValueBedroom = analogRead(LDR_PIN_BEDROOM); // 0-4095
  lightLevelBedroom = map(ldrValueBedroom, 4095, 0, 0, 1000); // Chuyển thành 0-1000 lux

  // Kiểm tra lỗi cảm biến
  if (isnan(humidityLiving) || isnan(temperatureLiving)) {
    Serial.println("Failed to read from DHT sensor (Living Room)!");
  } else {
    Serial.print("Living Room - Temperature: ");
    Serial.print(temperatureLiving);
    Serial.print(" *C\tHumidity: ");
    Serial.print(humidityLiving);
    Serial.print(" %\tLight: ");
    Serial.print(lightLevelLiving);
    Serial.println(" lux");
  }

  if (isnan(humidityBedroom) || isnan(temperatureBedroom)) {
    Serial.println("Failed to read from DHT sensor (Bedroom)!");
  } else {
    Serial.print("Bedroom - Temperature: ");
    Serial.print(temperatureBedroom);
    Serial.print(" *C\tHumidity: ");
    Serial.print(humidityBedroom);
    Serial.print(" %\tLight: ");
    Serial.print(lightLevelBedroom);
    Serial.println(" lux");
  }

  // Gửi dữ liệu cảm biến lên Firebase - Phòng khách
  String sensorPathLiving = "/sensors/dashboard0/";
  if (!isnan(temperatureLiving) && Firebase.setFloat(firebaseData, sensorPathLiving + "temperature", temperatureLiving)) {
    Serial.println("Sent Living Room temperature successfully");
  } else {
    Serial.println("Failed to send Living Room temperature: " + firebaseData.errorReason());
  }
  if (!isnan(humidityLiving) && Firebase.setFloat(firebaseData, sensorPathLiving + "humidity", humidityLiving)) {
    Serial.println("Sent Living Room humidity successfully");
  } else {
    Serial.println("Failed to send Living Room humidity: " + firebaseData.errorReason());
  }
  if (Firebase.setFloat(firebaseData, sensorPathLiving + "light", lightLevelLiving)) {
    Serial.println("Sent Living Room light successfully");
  } else {
    Serial.println("Failed to send Living Room light: " + firebaseData.errorReason());
  }

  // Gửi dữ liệu cảm biến lên Firebase - Phòng ngủ
  String sensorPathBedroom = "/sensors/dashboard1/";
  if (!isnan(temperatureBedroom) && Firebase.setFloat(firebaseData, sensorPathBedroom + "temperature", temperatureBedroom)) {
    Serial.println("Sent Bedroom temperature successfully");
  } else {
    Serial.println("Failed to send Bedroom temperature: " + firebaseData.errorReason());
  }
  if (!isnan(humidityBedroom) && Firebase.setFloat(firebaseData, sensorPathBedroom + "humidity", humidityBedroom)) {
    Serial.println("Sent Bedroom humidity successfully");
  } else {
    Serial.println("Failed to send Bedroom humidity: " + firebaseData.errorReason());
  }
  if (Firebase.setFloat(firebaseData, sensorPathBedroom + "light", lightLevelBedroom)) {
    Serial.println("Sent Bedroom light successfully");
  } else {
    Serial.println("Failed to send Bedroom light: " + firebaseData.errorReason());
  }
}

void syncDevices() {
  // Đồng bộ thiết bị từ Firebase - Phòng khách
  String devicePathLiving = "/devices/dashboard0/";
  if (Firebase.getInt(firebaseData, devicePathLiving + "light")) {
    int state = firebaseData.intData();
    digitalWrite(RELAY_LIGHT_LIVING, state ? HIGH : LOW);
    Serial.println("Living Room Light state from Firebase: " + String(state));
  } else {
    Serial.println("Failed to get Living Room Light state: " + firebaseData.errorReason());
  }
  if (Firebase.getInt(firebaseData, devicePathLiving + "fan")) {
    int state = firebaseData.intData();
    digitalWrite(RELAY_FAN_LIVING, state ? HIGH : LOW);
    Serial.println("Living Room Fan state from Firebase: " + String(state));
  } else {
    Serial.println("Failed to get Living Room Fan state: " + firebaseData.errorReason());
  }
  if (Firebase.getInt(firebaseData, devicePathLiving + "ac")) {
    int state = firebaseData.intData();
    digitalWrite(RELAY_AC_LIVING, state ? HIGH : LOW);
    Serial.println("Living Room AC state from Firebase: " + String(state));
  } else {
    Serial.println("Failed to get Living Room AC state: " + firebaseData.errorReason());
  }

  // Đồng bộ thiết bị từ Firebase - Phòng ngủ
  String devicePathBedroom = "/devices/dashboard1/";
  if (Firebase.getInt(firebaseData, devicePathBedroom + "light")) {
    int state = firebaseData.intData();
    digitalWrite(RELAY_LIGHT_BEDROOM, state ? HIGH : LOW);
    Serial.println("Bedroom Light state from Firebase: " + String(state));
  } else {
    Serial.println("Failed to get Bedroom Light state: " + firebaseData.errorReason());
  }
  if (Firebase.getInt(firebaseData, devicePathBedroom + "fan")) {
    int state = firebaseData.intData();
    digitalWrite(RELAY_FAN_BEDROOM, state ? HIGH : LOW);
    Serial.println("Bedroom Fan state from Firebase: " + String(state));
  } else {
    Serial.println("Failed to get Bedroom Fan state: " + firebaseData.errorReason());
  }
  if (Firebase.getInt(firebaseData, devicePathBedroom + "ac")) {
    int state = firebaseData.intData();
    digitalWrite(RELAY_AC_BEDROOM, state ? HIGH : LOW);
    Serial.println("Bedroom AC state from Firebase: " + String(state));
  } else {
    Serial.println("Failed to get Bedroom AC state: " + firebaseData.errorReason());
  }
}

void loop() {
  sendSensorData();
  syncDevices();
  delay(1000); // Gửi và đọc dữ liệu mỗi giây
}