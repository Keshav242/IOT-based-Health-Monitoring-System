#include <CircularBuffer.h>
#include <MAX30100.h>
#include <MAX30100_BeatDetector.h>
#include <MAX30100_Filters.h>
#include <MAX30100_PulseOximeter.h>
#include <MAX30100_Registers.h>
#include <MAX30100_SpO2Calculator.h>
#include <PeakDetection.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <ESP8266WiFi.h>

//extern "C" {
//#include "user_interface.h"
//#include "wpa2_enterprise.h"
//#include "c_types.h"
//}
float filt_temp;
float prev_val = 92.00;
#define REPORTING_PERIOD_MS     50
int pin = A0;                   // Temp sensor pin wire connected to analog pin 1
float RawTemp;                        // holds incoming raw temp data
float TEMP = 0;                         // holds temperature in F
int peak;
double filtered;

PeakDetection peakDetection;
int previous_peak = 0,flag_mila = 0;
double t = 0,inhale_time = 1,exhale_time = 1,breadth_time= 1,w = 0.3, rr = 1;

// SSID to connect to
//char ssid[] = "IITR_WIFI";
//char username[] = "18117100";
//char identity[] = "18117100";
//char password[] = "shubiitr";

char ssid[] = "chori krna paap h";
char password[] = "orangreen";

WiFiClient  client;
char PatientID[] = "ANK_S19";
int Spo2;
float rate;
int port = 4000;
char ip[] = "103.37.200.133";
char data_rcvd;

PulseOximeter pox;
uint32_t tsLastReport = 0;
 
void onBeatDetected()
{
    Serial.println("Beat!");
}
MAX30100 sensor;
void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    pinMode(pin, INPUT);
    peakDetection.begin(17, 2,0.9);
    t = millis();

    delay(1000);
    // Connect or reconnect to WiFi
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
        Serial.print(".");
        delay(5000);
      }
      Serial.println("\nConnected.");
    }
//    Serial.setDebugOutput(true);
//    Serial.printf("SDK version: %s\n", system_get_sdk_version());
//    Serial.printf("Free Heap: %4d\n",ESP.getFreeHeap());
//    
//    // Setting ESP into STATION mode only (no AP mode or dual mode)
//    wifi_set_opmode(STATION_MODE);
//  
//    struct station_config wifi_config;
//  
//    memset(&wifi_config, 0, sizeof(wifi_config));
//    strcpy((char*)wifi_config.ssid, ssid);
//    strcpy((char*)wifi_config.password, password);
//  
//    wifi_station_set_config(&wifi_config);
//  //  wifi_set_macaddr(STATION_IF,target_esp_mac);
//    
//  
//    wifi_station_set_wpa2_enterprise_auth(1);
//  
//    // Clean up to be sure no old data is still inside
//    wifi_station_clear_cert_key();
//    wifi_station_clear_enterprise_ca_cert();
//    wifi_station_clear_enterprise_identity();
//    wifi_station_clear_enterprise_username();
//    wifi_station_clear_enterprise_password();
//    wifi_station_clear_enterprise_new_password();
//    
//    wifi_station_set_enterprise_identity((uint8*)identity, strlen(identity));
//    wifi_station_set_enterprise_username((uint8*)username, strlen(username));
//    wifi_station_set_enterprise_password((uint8*)password, strlen((char*)password));
//  
//    
//    wifi_station_connect();
//    while (WiFi.status() != WL_CONNECTED) {
//      delay(1000);
//      Serial.print(".");
//    }
//  
//    Serial.println("WiFi connected");
//    Serial.println("IP address: ");
//    Serial.println(WiFi.localIP());

    if(client.connect(ip, port))
    {
      Serial.print("connected to server");
      Serial.println(String(port));
      client.print("client device_1_id");
    }

    Serial.print("Initializing pulse oximeter..");
 
    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
     pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
 
    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);

}

int z = 0;
bool resp = 0;
double data;

void loop()
{
  while(client.connected())
  {
    if(resp == 0)
    {
      // Make sure to call update as fast as possible
      pox.update();
      sensor.update();
      if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
          Serial.print("Heart Rate:");
          rate = pox.getHeartRate();
          Serial.print(pox.getHeartRate());
          Serial.print(" Spo2:");
          Spo2 = pox.getSpO2();
          Serial.print(pox.getSpO2());
          Serial.print(" temp:");
          RawTemp = analogRead(pin);
          float mv = (RawTemp/1023.0)*1250;
          float celcius = mv/10;
          TEMP = (celcius*9/5) + 32;
          filt_temp = (5*prev_val+TEMP)/6;
          Serial.print(filt_temp);
          Serial.println("F");
          prev_val = prev_val + filt_temp;
          z = z+1;
          tsLastReport = millis();
          if(z>=200 && (Spo2>70 && Spo2<105) && (rate>50 && rate<110)) 
          {
            resp = 1;
            z = 0;
            pox.shutdown();
          }
      }
    }
    else
    {
//      delay(1000);
      data = (double)analogRead(pin);
      peakDetection.add(data);
      peak = peakDetection.getPeak();
      filtered = peakDetection.getFilt();
      if((previous_peak*peak == -1 )&& peak ==-1){
      flag_mila = 1;
      inhale_time = w*exhale_time+ (1-w)*(millis()-t)/1000;
      t = millis();
      }
      else if((previous_peak*peak == -1)&& peak ==1){
        flag_mila = 1;
        exhale_time = w*exhale_time+ (1-w)*(millis()-t)/1000;
        t = millis();
      }
      else
      {
        flag_mila = 0;
        breadth_time = inhale_time +exhale_time;
        rr = 1/breadth_time;
//        Serial.print(data);
//        Serial.print(",");
//        Serial.print(100*peak+600);
//        Serial.print(",");
//        Serial.print(100*flag_mila+600);
//        Serial.print(",");
//        Serial.print(filtered);
//        Serial.print(",");
        Serial.print(inhale_time+600);
        Serial.print(",");
        Serial.print(exhale_time+600);
        Serial.print(",");
        Serial.print(breadth_time);
        Serial.print(",");
        Serial.println(rr);
        rr = rr*60;
      
        if(abs(peak)>0) previous_peak = peak;
        z += 1;
        delay(10);
        if(z>500)
        {
        //if((Spo2>70 && Spo2<105) && (rate>50 && rate<110))// && (temperature>25 && temperature<45))
        
          client.print(String(Spo2)+","+String(abs(rate))+","+String(filt_temp));
          Serial.println("data_sent");
          z = 0;
          resp = 0;
          pox.resume();
        }
      }
    }
  }
}
