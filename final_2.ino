#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Firebase.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE];  
byte rateSpot = 0;
long lastBeat = 0; 

#define _SSID "ROOT32"
#define _PASSWORD "asd12345#"

#define REFERENCE_URL "" 

Firebase firebase(REFERENCE_URL);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    loading();

    wifiConnect();


    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) 
    {
        Serial.println("MAX30105 was not found. Please check wiring/power. ");
        while (1);
    }
    Serial.println("Place your index finger on the sensor with steady pressure.");

    particleSensor.setup();                                          
    particleSensor.setPulseAmplitudeRed(0x0A);                     
    particleSensor.setPulseAmplitudeGreen(0);                       

    title_graphics();
    delay(2000);
}

void loop()
{
    long irValue = particleSensor.getIR();

    if (irValue < 50000) 
    {
        
        Serial.println("No finger detected. Entering sleep mode...");
        delay(5000); 
              
    }
    else
    {
        
        if (checkForBeat(irValue))
        {
          
            long delta = millis() - lastBeat;
            lastBeat = millis();

            float beatsPerMinute = 60 / (delta / 1000.0);

            if (beatsPerMinute < 255 && beatsPerMinute > 20)
            {
                rates[rateSpot++] = (byte)beatsPerMinute; 
                rateSpot %= RATE_SIZE;                    

               
                int beatAvg = 0;
                for (byte x = 0; x < RATE_SIZE; x++)
                    beatAvg += rates[x];
                beatAvg /= RATE_SIZE;

                display.clearDisplay();
                display.setTextSize(2);
                display.setTextColor(WHITE);
                display.setCursor(15, 10);
                display.print("BPM:");
                display.setTextSize(2);
                display.setCursor(50, 30);
                display.print(beatsPerMinute);
                display.display();

                Serial.print("IR=");
                Serial.print(irValue);
                Serial.print(", BPM=");
                Serial.print(beatsPerMinute);
                Serial.print(", Avg BPM=");
                Serial.println(beatAvg);

                firebase.setInt("Node1/HeartRate", beatsPerMinute);
                firebase.setInt("Node1/Temperature", beatAvg);
            }
        }
    }

 
}

void wifiConnect()
{
    Serial.println("Connecting to: ");
    Serial.println(_SSID);
    WiFi.begin(_SSID, _PASSWORD);

    unsigned long wifiTimeout = millis() + 30000; 

    while (WiFi.status() != WL_CONNECTED && millis() < wifiTimeout)
    {
        flash();
        Serial.print("-");
        delay(500); 
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("");
        Serial.println("WiFi Connected Successfully");
        digitalWrite(LED_BUILTIN, HIGH);

        Serial.print("IP Address: ");
        Serial.print("http://");
        Serial.print(WiFi.localIP());
        Serial.println("/");
    }
    else
    {
        Serial.println("");
        Serial.println("Failed to connect to WiFi. Please check your credentials and connection.");
    }
}

void flash()
{
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
}

void loading()
{
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);

    String loadingText = "LOADING";
    int textWidth = loadingText.length() * 6;            
    int centerX = (display.width() - textWidth) / 2;    
    display.setCursor(centerX, 32);
    display.print(loadingText);

    
    display.drawRect(10, 45, 100, 10, WHITE);


    for (int i = 0; i <= 100; i++)
    {
       
        int barWidth = map(i, 0, 100, 0, 100);

       
        display.fillRect(12, 47, barWidth, 6, WHITE);

       
        display.display();

        delay(50);

        display.fillRect(12, 47, barWidth, 6, BLACK);
    }
}

void title_graphics()
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(15, 10);
    display.print("HOSPITAL");
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    display.setCursor(10, 30);
    display.print("PATIENT MONITORING");
    display.setCursor(10, 40);
    display.print(" HEALTHCARE SYSTEM");
    display.display();
}
