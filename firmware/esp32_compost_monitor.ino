/*
  DS18B20 Composting Monitor with Deep Sleep + Google Sheets + Telegram
  المدى المثالي: 55 - 65 درجة مئوية
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//  Config 
#define WIFI_SSID     ""
#define WIFI_PASS     ""
#define BOT_TOKEN     ""
#define CHAT_ID       ""
#define SHEETS_URL    ""

#define ONE_WIRE_BUS  4  
#define BUZZER_PIN    5

const float TEMP_MIN = 55.0;  // أقل حرارة للتخمير
const float TEMP_MAX = 65.0;  // أعلى حرارة عشان الميكروبات متبوزش

const uint64_t SLEEP_NORMAL_US = 60ULL * 60ULL * 1000000ULL; // ساعة
const uint64_t SLEEP_RETRY_US  = 5ULL  * 60ULL * 1000000ULL; 

// بيفضلوا محفوظين في RTC
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int wifiFailStreak = 0;
RTC_DATA_ATTR bool alertSent = false; 

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void buzzPattern(int beeps, int onMs, int offMs) {
  for (int i = 0; i < beeps; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(onMs);
    digitalWrite(BUZZER_PIN, LOW);
    delay(offMs);
  }
}

bool connectWifi(unsigned long timeoutMs) {
  WiFi.disconnect(true); // مسح الاتصال القديم
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  for(int i=0; i<3; i++){ // 3 محاولات
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs/3) delay(300);
    if(WiFi.status() == WL_CONNECTED) return true;
    delay(1000);
  }
  return false;
}

void logToSheets(float t, String status) {
  if(WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.setTimeout(5000); 
  String url = String(SHEETS_URL) + "?temp=" + String(t, 1) + "&status=" + status;
  http.begin(url);
  int httpCode = http.GET(); 
  if(httpCode > 0) Serial.println("تم الإرسال للشيت");
  else Serial.println("فشل الإرسال للشيت: " + String(httpCode));
  http.end();
}

void sendTelegram(String msg){
  if(WiFi.status() != WL_CONNECTED) return;
  bool sent = bot.sendMessage(CHAT_ID, msg, "");
  if(sent) Serial.println("تم إرسال تلجرام");
  else Serial.println("فشل إرسال تلجرام");
}

void goToSleep(uint64_t microseconds) {
  esp_sleep_enable_timer_wakeup(microseconds);
  Serial.println("جاري الدخول في deep sleep...");
  Serial.flush();
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  bootCount++;

  sensors.begin();
  delay(1000);
  sensors.requestTemperatures(); 
  delay(750); // DS18B20 محتاج 750ms للقراءة بدقة 12bit
  float t = sensors.getTempCByIndex(0);

  bool wifiOk = connectWifi(15000);
  if (!wifiOk) {
    wifiFailStreak++;
    buzzPattern(3, 400,  200); // تنبيه: مفيش نت
    Serial.println("فشل الاتصال بالواي فاي، هينام وهيحاول تاني بعد 5 دقايق.");
    goToSleep(SLEEP_RETRY_US);
    return;
  }

  wifiFailStreak = 0;
  secured_client.setInsecure(); 

  if (t == DEVICE_DISCONNECTED_C) {
    Serial.println("فشل قراءة DS18B20!");
    sendTelegram("⚠️ فشلت قراءة حساس الحرارة DS18B20 في Boot #" + String(bootCount));
    goToSleep(SLEEP_NORMAL_US);
    return;
  }

  Serial.printf("Temp: %.1f°C\n", t);

  bool outOfRange = (t < TEMP_MIN || t > TEMP_MAX);
  String status = outOfRange ? "ALERT" : "OK";

  logToSheets(t, status); // ابعت للشيت الأول

  
  if (outOfRange) {
    if(!alertSent){ // ابعت تنبيه أول مرة بس
      buzzPattern(5, 100, 100);
      String msg = "⚠️ تنبيه: حرارة السماد خارج المدى!\n";
      msg += "🌡️ الحرارة: " + String(t) + "°C\n";
      if(t > TEMP_MAX) msg += "الحل: قلب الكومة فوراً للتهوية";
      else msg += "الحل: غطي الكومة وزود رطوبة خفيفة";
      sendTelegram(msg);
      alertSent = true;
    }
  } else {
    if(alertSent){ 
      sendTelegram("✅ الحرارة رجعت للمدى الطبيعي: " + String(t) + "°C");
      alertSent = false;
    }
    
  }

  goToSleep(SLEEP_NORMAL_US);
}

void loop() {
  
}
