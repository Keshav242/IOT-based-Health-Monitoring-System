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
#include <Adafruit_AHT10.h>

//extern "C" {
//#include "user_interface.h"
//#include "wpa2_enterprise.h"
//#include "c_types.h"
//}
float filt_temp;

float prev_val = 92.00;
#define REPORTING_PERIOD_MS     50
int pin = A0;                   // flex sensor connected
int z = 0;
bool resp = 0;
int data, avg, reading, a=0,previous_a=0;
const int WINDOW_SIZE1 = 1000,WINDOW_SIZE2=50;
int INDEX1 = 0,INDEX2 = 0;
int SUM1 = 0,SUM2 = 0;
int READINGS1[WINDOW_SIZE1],READINGS2[WINDOW_SIZE2];
int AVERAGED = 0;

//=================================================================================
// declaration of objects
//=================================================================================
PeakDetection peakDetection;
PulseOximeter pox;
uint32_t tsLastReport = 0;
Adafruit_AHT10 aht;
MAX30100 sensor;


int previous_peak = 0,flag_mila = 0;
double t = 0,inhale_time = 1,exhale_time = 1,breadth_time= 1,w = 0.3, rr = 1;


//=================================================================================
// wifi information
//=================================================================================
// SSID to connect to
//char ssid[] = "IITR_WIFI";
//char username[] = "18117100";
//char identity[] = "18117100";
//char password[] = "shubiitr";

char ssid[] = "IITR_IoT";
char password[] = "microprocessorlab";

//=================================================================================
// IP information
//=================================================================================
WiFiClient  client;
char PatientID[] = "ANK_S19";
int Spo2;
float hrate;
int port = 4000;
//char ip[] = "103.37.200.133";
//char ip[] = "192.168.1.6";
char ip[]= "10.12.0.65";
char data_rcvd;

//====================================================================================
void onBeatDetected()
{
    Serial.println("Beat!");
//    digitalWrite(LED_BUILTIN, LOW);
//    delay(20);
//    digitalWrite(LED_BUILTIN, HIGH);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
//-----------------------------
// Serial begin, wifi config, pin config
//-----------------------------
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    pinMode(pin, INPUT);
    peakDetection.begin(17, 2,0.9);
    t = millis();

    delay(1000);

//-----------------------------
// Connect or reconnect to WiFi
//-----------------------------
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
        digitalWrite(LED_BUILTIN, LOW);
        delay(2500);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(2500);
        
      }
      Serial.println("\nConnected.");
    }

//-----------------------------
// connecting to IP
//-----------------------------
    if(client.connect(ip, port))
    {
      Serial.print("connected to server");
      Serial.println(String(port));
      client.print("client device_6_id");
    }

//-----------------------------
// initiallizing pox
//-----------------------------
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

//-----------------------------
// Initiallizing AHT10
//-----------------------------
    Serial.println("Initializing Adafruit AHT10");
  
    if (! aht.begin()) {
      Serial.println("Could not find AHT10? Check wiring");
      while (1) delay(10);
    }
    Serial.println("AHT10 found");

}

void loop()
{
//------------------------------------------
// runs only when connected to server
//------------------------------------------
  while(client.connected())
  {
    if(resp == 0)
    {
      //-----------------------------
      // POX reading
      //-----------------------------
      // Make sure to call update as fast as possible
      pox.update();
      sensor.update();
      if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
          hrate = pox.getHeartRate();
          Spo2 = pox.getSpO2();
          z = z+1;
          tsLastReport = millis();
          if(z>=50 && (Spo2>70 && Spo2<100) && (hrate>50 && hrate<110)) 
          {
            Serial.print("Heart Rate:");
            Serial.print(hrate);
            Serial.print(" Spo2:");
            Serial.println(Spo2);
            resp = 1;
            z = 0;
            pox.shutdown();
          }
      }
    }
    else
    {
//      delay(1000);
//-----------------------------
// Respiration data
//-----------------------------
        data = analogRead(A0);
        avg = filter(data, 1);
        reading = filter(data,2);
        Serial.print(data);
        Serial.print(",");
        Serial.print(reading);
        Serial.print(",");
        Serial.print(avg);
        Serial.print(",");
        Serial.println(a);
        if(reading>avg)
          a = 1;
        else 
          a = -1;
        if(previous_a<0&&a>0)
        {
        rr = 60000/(millis()-t);
        if(rr<35&&rr>10){
        Serial.println(rr);
        z += 1;
        }
        t = millis();
        }
        //Serial.print(a);
        previous_a = a;
      
        delay(10);
        if(z>0)
        {
        //if((Spo2>70 && Spo2<105) && (hrate>50 && hrate<110))// && (temperature>25 && temperature<45))
        //-----------------------------
        // Temperature reading
        //-----------------------------
          sensors_event_t humidity, temp;
          aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
          Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
          Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

          client.print(String(Spo2)+","+String(abs(hrate))+","+String(rr)+","+String(temp.temperature) );
          Serial.println("data_sent");
          digitalWrite(LED_BUILTIN, LOW);
          delay(1000);
          digitalWrite(LED_BUILTIN, HIGH);
          z = 0;
          resp = 0;
          pox.resume();
        }
    }
  }
}
