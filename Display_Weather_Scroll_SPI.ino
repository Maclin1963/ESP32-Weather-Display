/* Utilize converted including file (weatherFont.h), to display the weather info of each major
cities of Taiwan on SSD1306 OLED module */

#include <map>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <U8g2lib.h>
#include "weatherFont.h" // Converted file refering to : https://swf.com.tw/?p=1498 (ISBN 978-986-312-660-7)

/* U8G2_SSD1306_128X64_NONAME_1_4W_HW_SPI or U8G2_SSD1306_128X64_NONAME_2_4W_HW_SPI or 
U8G2_SSD1306_128X64_NONAME_Ｆ_4W_HW_SPI all functional, "1" & "2" save flash memory, "F" use all 1K capacity */
U8G2_SSD1306_128X64_NONAME_2_4W_HW_SPI  /* SSD1306 HW SPI pin connection with ESP32: GND, 3.3V, D0 (SCK) / GPIO18, D1 (MOSI) / GPIO23, */
  u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/ 16, /* reset=*/ 17);   /* RES / GPIO17, DC / GPIO16, CS / GPIO5 */

std::map<String, char> icon_map{                            // mapping of weather JSON codes and the alphabets
  {"01d", 'B'}, {"02d", 'H'}, {"03d", 'N'}, {"04d", 'Y'},
  {"09d", 'R'}, {"10d", 'Q'}, {"11d", 'P'}, {"13d", 'W'},
  {"50d", 'J'}, {"01n", 'C'}, {"02n", 'I'}, {"03n", '5'},
  {"04n", '%'}, {"09n", '8'}, {"10n", '7'}, {"11n", '6'},
  {"13n", '#'}, {"50n", 'K'}
};

const char* ssid = "Your WiFi SSID";       // SSID
const char* password = "Your WiFi router PW";
String API_KEY = "The API key get from openweathermap.org";    // https://openweathermap.org/ APIkey

HTTPClient http;

void connectWiFi() {          // Connect to Wifi

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("IP address:");
  Serial.println(WiFi.localIP());
}

String openWeather(String cityna) {     // Connect to https://openweathermap.org/
  String url = 
    "http://api.openweathermap.org/data/2.5/weather?q=" +     // check cityna info
    cityna + "&appid=" + API_KEY;
  String payload = "";

  if ((WiFi.status() != WL_CONNECTED)) {    // If not connected
    connectWiFi() ;   // re-connect
  } else {
    http.begin(url);    // elsewhile featch openweathermap.org weather info
    int httpCode = http.GET() ;     // hookup

    if (httpCode == 200) {
      payload = http.getString() ;    // get the data
      Serial.printf("Response：%s\n", payload.c_str());
    } else {
      Serial.println("HTTP request wrong...");
    }
    http.end() ;
  }
  return payload;
}

void displayWeather(String json) {      // show on SSD1306
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);     // De-serialize JSON strings
  JsonObject weather = doc["weather"][0]; 
  const char* icon = weather["icon"];   // get the weather icon name 
  const char* city = doc["name"];   // get the city name   
  JsonObject main = doc["main"];  
  float temp = (float)main["temp"] - 273.15;     // convert from F to C  
  int humid = (int)main["humidity"];    // humidity

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
        u8g2.drawUTF8(0, 16 - p, "                "); // treatment to eliminate the residual pixles of lowercase alphabets
      }                                               // when screen moving upwards
      u8g2.setFont(u8g2_font_profont12_mr);   // shift city name upwards
      u8g2.drawUTF8(0, 8 - p, city);
    
      u8g2.setFont(u8g2_font_inr16_mf);
      u8g2.setCursor(60, 36 - p);
      u8g2.print(String(temp, 1) + "\xb0");   // shift temp upwards
      
      u8g2.setCursor(60, 61 - p);
      u8g2.print(String(humid) + "%");    // shift humidity upwards
      
      u8g2.setFont(weatherFont);     
      u8g2.setCursor(0, 61 - p);        // shift weather icon upwards
      u8g2.print(icon_map[icon]);
      
      if ( icon_map[icon] == 'Q' || icon_map[icon] == 'P' || icon_map[icon] == '7' || icon_map[icon] == '6') {
        u8g2.setFont(u8g2_font_profont12_mr);
        u8g2.setCursor(0, 70 - p);        // treatment to eliminate the residual pixles of each icon
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
  connectWiFi();      // connect Wifi
  u8g2.begin();       // initiate OLED
}

void loop() {

  int citysize = 13;      // set city num. = 13 (0, 1, 2...~12)
  String cityn;           // set cache for city
  String cityName[citysize] = {"Keelung, TW", "Taipei, TW", "Taoyuan, TW", // openweathermap's cities list of Taiwan
            "Hsinchu, TW", "Miaoli, TW", "Taichung, TW", "Douliu, TW", 
            "Chiayi, TW", "Tainan, TW", "Kaohsiung, TW", "Hengchun, TW", 
            "Taitung, TW", "Hualian, TW"};

  for (int i = 0; i < 13; i++){               // show indivisual city, cityName[0] ~ cityName[12], total 13
     cityn = cityName[i];     
     String payload = openWeather(cityn);     // featch the info
        if (payload != "") {                  // do if JSON not void
          displayWeather(payload);            // go display the info
        }
  }
}
