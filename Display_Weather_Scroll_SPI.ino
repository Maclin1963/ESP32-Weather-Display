/* 使用轉換天氣圖檔(weatherFont.h), 在OLED 上顯示台灣各大都市天氣, 氣溫, 濕度*/

#include <map>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <U8g2lib.h>
#include "weatherFont.h" //經轉換之天氣圖

/* U8G2_SSD1306_128X64_NONAME_1_4W_HW_SPI 或 U8G2_SSD1306_128X64_NONAME_2_4W_HW_SPI 或 U8G2_SSD1306_128X64_NONAME_Ｆ_4W_HW_SPI
 * 都能正常顯示。前2項設定節省記憶體容量一半, F則使用全部1K的記憶容量 */
U8G2_SSD1306_128X64_NONAME_2_4W_HW_SPI  /* SSD1306 HW SPI pin connection with ESP32: GND, 3.3V, D0 (SCK) / GPIO18, D1 (MOSI) / GPIO23, */
  u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/ 16, /* reset=*/ 17);   /* RES / GPIO17, DC / GPIO16, CS / GPIO5 */

std::map<String, char> icon_map{                            // 各天氣圖 JSON 編碼及代號
  {"01d", 'B'}, {"02d", 'H'}, {"03d", 'N'}, {"04d", 'Y'},
  {"09d", 'R'}, {"10d", 'Q'}, {"11d", 'P'}, {"13d", 'W'},
  {"50d", 'J'}, {"01n", 'C'}, {"02n", 'I'}, {"03n", '5'},
  {"04n", '%'}, {"09n", '8'}, {"10n", '7'}, {"11n", '6'},
  {"13n", '#'}, {"50n", 'K'}
};

const char* ssid = "Your WiFi SSID";       // SSID
const char* password = "Your WiFi router PW";
String API_KEY = "The API key get from openweathermap.org";    //openweathermap.org APIkey

HTTPClient http;

void connectWiFi() {          // 連線到Wifi

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("IP address:");
  Serial.println(WiFi.localIP());
}

String openWeather(String cityna) {     //連線到 openweathermap.org 網站
  String url = 
    "http://api.openweathermap.org/data/2.5/weather?q=" +     // 查詢 cityna 傳遞的城市天氣資訊
    cityna + "&appid=" + API_KEY;
  String payload = "";

  if ((WiFi.status() != WL_CONNECTED)) {    // 若網路斷線
    connectWiFi() ;   // 重新連線
  } else {
    http.begin(url);    // 準備連線到 openweathermap.org 網站
    int httpCode = http.GET() ;     // 開始連線

    if (httpCode == 200) {
      payload = http.getString() ;    // 取得回應資料
      Serial.printf("Response：%s\n", payload.c_str());
    } else {
      Serial.println("HTTP request wrong...");
    }
    http.end() ;
  }
  return payload;
}

void displayWeather(String json) {      // 在 OLED 顯示天氣資訊
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);     // 解析城市1 JSON 字串
  JsonObject weather = doc["weather"][0]; 
  const char* icon = weather["icon"];   // 圖示名稱  
  const char* city = doc["name"];   // 城市名稱   
  JsonObject main = doc["main"];  
  float temp = (float)main["temp"] - 273.15;     // 攝氏溫度  
  int humid = (int)main["humidity"];    // 濕度

  for ( int p =  64; p >= 0; p--) {
    u8g2.firstPage() ;
      do {
        u8g2.setFont(u8g2_font_profont12_mr);
        u8g2.drawUTF8(0, p + 8, city);
    
        u8g2.setFont(u8g2_font_inr16_mf);
        u8g2.setCursor(60, p + 36);
        u8g2.print(String(temp, 1) + "\xb0");
        u8g2.setCursor(60, p + 61);
        u8g2.print(String(humid) + "%");
        u8g2.setFont(weatherFont);
        u8g2.setCursor(0, p + 61);
        u8g2.print(icon_map[icon]);   
      } while (u8g2.nextPage()) ;
      delay(10);
  } 
  
  delay(5000);
   
  for ( int p = 0 ; p <= 64; p++) {
    u8g2.firstPage() ;
    do {
      if (p < 13) {
        u8g2.setFont(u8g2_font_profont12_mr);
        u8g2.drawUTF8(0, 16 - p, "                "); // 消除城市名稱小寫部份殘留痕跡 (因為畫面往上畫素上一位, 最底端畫素不會被蓋掉, 會殘留一條線
      }
      u8g2.setFont(u8g2_font_profont12_mr);   // 讓城市名稱漸漸升高
      u8g2.drawUTF8(0, 8 - p, city);
    
      u8g2.setFont(u8g2_font_inr16_mf);
      u8g2.setCursor(60, 36 - p);
      u8g2.print(String(temp, 1) + "\xb0");   // 讓溫度顯示漸漸升高
      
      u8g2.setCursor(60, 61 - p);
      u8g2.print(String(humid) + "%");    // 讓濕度顯示漸漸升高
      
      u8g2.setFont(weatherFont);     
      u8g2.setCursor(0, 61 - p);        // 讓天氣符號漸漸升高
      u8g2.print(icon_map[icon]);
      
      if ( icon_map[icon] == 'Q' || icon_map[icon] == 'P' || icon_map[icon] == '7' || icon_map[icon] == '6') {
        u8g2.setFont(u8g2_font_profont12_mr);
        u8g2.setCursor(0, 70 - p);        // 將部份天氣符號底下殘留消去, 底下亦同(根據符號做調整)
        u8g2.print("         ");
      }
      else if ( icon_map[icon] == 'B' || icon_map[icon] == 'J' || icon_map[icon] == 'K' || icon_map[icon] == '%') {
        u8g2.setFont(u8g2_font_profont12_mr);
        u8g2.setCursor(0, 66 - p);
        u8g2.print("         ");        
      }
      else if ( icon_map[icon] == 'N' || icon_map[icon] == '5') {
        u8g2.setFont(u8g2_font_profont12_mr);
        u8g2.setCursor(0, 64 - p);
        u8g2.print("         ");
      }
      else if ( icon_map[icon] == 'C') {
        u8g2.setFont(u8g2_font_profont12_mr);
        u8g2.setCursor(0, 60 - p);
        u8g2.print("         ");        
      }
      for ( int x = 0; x <= 123; x++) {
        u8g2.setFont(u8g2_font_profont12_mr);
        u8g2.setCursor(x, 73 - p);
        u8g2.print(" "); 
      }
    } while (u8g2.nextPage());
    delay(0);  
  } 
}

void setup() {
  Serial.begin(115200);
  connectWiFi();      //連結無線網路
  u8g2.begin();       // 啟用OLED
}

void loop() {

  int citysize = 13;      // 設定城市數 = 13 (0, 1, 2...~12)
  String cityn;           // 設定城市2名暫存名稱
  String cityName[citysize] = {"Keelung, TW", "Taipei, TW", "Taoyuan, TW", // openweathermap 網站有關台灣主要城市名稱列表
            "Hsinchu, TW", "Miaoli, TW", "Taichung, TW", "Douliu, TW", 
            "Chiayi, TW", "Tainan, TW", "Kaohsiung, TW", "Hengchun, TW", 
            "Taitung, TW", "Hualian, TW"};

  for (int i = 0; i < 13; i++){               //逐一顯示各城市資料, cityName[0] ~ cityName[12], 共13個城市
     cityn = cityName[i];     
     String payload = openWeather(cityn);     // 讀取城市天氣資料
        if (payload != "") {                  // 只要傳回的 JSON 不是空字串
          displayWeather(payload);            // 顯示天氣資料
        }
  }
}
