// Program: Teste Display OLED SSD1306 + Medidor temperatura DHT22
// Autor: André Romero - 2015
// e-mail: andretec@ifsc.usp.br
//-------------------------------------------------------------------------------- 
#include "U8glib.h"
#include "DHT.h"
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHTPIN 4     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Data wire is plugged into port 10 on the Arduino
#define ONE_WIRE_BUS 10
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
//--------------------------------------------------------------------------------
// Set up input pins
uint8_t uiKeyPrev = 7;
uint8_t uiKeyNext = 3;
uint8_t uiKeySelect = 2;
uint8_t uiKeyBack = 8;
//--------------------------------------------------------------------------------
float tempC, humidity, tmaxim, tminim, hmaxim, hminim ; 
float arraytemp [127];
float arrayhum [127];
int eeAddress = 0; 
float tempEEPROM;
int sensorValue;
float voltage;
int opcao = 1;
int buttonState = 0;     // variable for reading the pushbutton status
int ligou = 0;
//--------------------------------------------------------------------------------
unsigned long previousMillis;
unsigned long currentMillis;
unsigned long deltaMillis;

unsigned long timePrevious;
unsigned long timeCurrent;
unsigned long timeDelta;

char Str; // Receive instructions from USB

unsigned long timeInterval = 1;
//timeInterval = 60000;  // 2h 
//timeInterval = 171250; // 6h
//timeInterval = 342500; // 12h
//timeInterval = 685000; // 24h

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

// setup u8g object, please remove comment from one of the following constructor calls
// IMPORTANT NOTE: The following list is incomplete. The complete list of supported 
// devices with all constructor calls is here: http://code.google.com/p/u8glib/wiki/device
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Display which does not send AC

//--------------------------------------------------------------------------------
void draw() 
{
  // Draw the Screen
  // Border
  u8g.drawRFrame(0,10, 128, 45, 0);

  // Print T ou H
  if (opcao == 1)
  {
    //u8g.setFont(u8g_font_courB08);
    u8g.setFont(u8g_font_6x12);
    u8g.setPrintPos(5,8); // X Y
    u8g.print("Temp *C");
    
  ///---------------------------------- Temperatura
  //u8g.setFont(u8g_font_courB08);
  //u8g.setFont(u8g_font_fub30);
  u8g.setFont(u8g_font_6x12);
  u8g.setPrintPos(0,64); // X Y
  u8g.print(tempC);
  
  // Scale Max e Min
  //u8g.setFont(u8g_font_8x13B);
  u8g.setFont(u8g_font_6x12);
  u8g.setPrintPos(95,7); // X Y
  u8g.print(tmaxim); // print max
  //u8g.setFont(u8g_font_unifont);
  u8g.setPrintPos(95,64); // X Y
  u8g.print(tminim); // print min
  }
    else
  {
    //---------------------------------- Humidity
    //u8g.setFont(u8g_font_courB08);
    u8g.setFont(u8g_font_6x12);
    u8g.setPrintPos(5,8); // X Y
    u8g.print("Humidity %");

  // Humidity
  //u8g.setFont(u8g_font_courB08);
  //u8g.setFont(u8g_font_fub30);
  u8g.setFont(u8g_font_6x12);
  u8g.setPrintPos(0,64); // X Y
  u8g.print(humidity);
  
  // Scale Max and Min
  //u8g.setFont(u8g_font_8x13B);
  u8g.setFont(u8g_font_6x12);
  u8g.setPrintPos(95,7); // X Y
  u8g.print(hmaxim); // print max value
  //u8g.setFont(u8g_font_unifont);
  u8g.setPrintPos(95,64); // X Y
  u8g.print(hminim); // print min value
  }// else
//--------------------------------------------------  
 if (opcao == 1)  // temperature
  {
   for (int i=0; i <= 125; i++)
   {
      float valtmp = mapfloat(arraytemp[i], tminim, tmaxim, 51, 13);
      float valtmp2 = mapfloat(arraytemp[i+1], tminim, tmaxim, 51, 13);
      u8g.drawLine(i+1, valtmp, i+2, valtmp2);
   } 
  }
  else    // humidity
  {
    for (int i=0; i <= 125; i++)
   {
      float valtmp = mapfloat(arrayhum[i], hminim, hmaxim, 51, 13);
      float valtmp2 = mapfloat(arrayhum[i+1], hminim, hmaxim, 51, 13);
      u8g.drawLine(i+1, valtmp, i+2, valtmp2);
   } 
  }// else
}// Draw
//--------------------------------------------------------------------------------
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}// mapfloat
//--------------------------------------------------------------------------------
void setup(void) 
{
  pinMode(13, OUTPUT);
  pinMode(uiKeyPrev, INPUT_PULLUP);           // set pin to input with pullup
  pinMode(uiKeyNext, INPUT_PULLUP);           // set pin to input with pullup
  pinMode(uiKeySelect, INPUT_PULLUP);         // set pin to input with pullup
  pinMode(uiKeyBack, INPUT_PULLUP);           // set pin to input with pullup
  
  Serial.begin(9600);

  dht.begin();

  delay(3000);
  
//--------------------------------- Read EEPROM - Temperature
   eeAddress = 0;
   for (int i=0; i <= 126; i++)
   {
       EEPROM.get(eeAddress, tempEEPROM);
       arraytemp[i] = tempEEPROM;;
       eeAddress += sizeof(float);
   }
//--------------------------------- Read EEPROM - Humidity 
 for (int i=0; i <= 126; i++)
   {
       EEPROM.get(eeAddress, tempEEPROM);
       arrayhum[i] = tempEEPROM;
       eeAddress += sizeof(float);
   }
//-------------------------------------------------
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
}// setup
//------------------------------------------------------------------ main loop 
void loop(void) 
{
  Str = Serial.read();
//----------------------- temperature
  if (Str == 'D') // Download data
    {
        eeAddress = 0;     
        for (int i=0; i <= 126; i++)
          {
              EEPROM.get(eeAddress, tempEEPROM);
              Serial.print("[");
              Serial.print(tempEEPROM);
              Serial.println("]");
              eeAddress += sizeof(float);
          }//for
//-------------------------------- humidity
        for (int i=0; i <= 126; i++)
          {
              EEPROM.get(eeAddress, tempEEPROM);
              Serial.print("[");
              Serial.print(tempEEPROM);
              Serial.println("]");
              eeAddress += sizeof(float);
          }//for
     Serial.println("end");
    }//if
//------------------------------------------------
timeCurrent = millis();
//============================================================
if (timeCurrent - timePrevious >= timeInterval || ligou == 0)
{
  ligou = 1;
  
  timePrevious = timeCurrent;
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    //Serial.println("Failed to read from DHT sensor!");
    return;
  }
//------------------- temperature ---------------------------------------->
  tempC = t;
  humidity = h;
  tmaxim = tempC + 1;
  tminim = tempC - 1;
  hmaxim = humidity + 1;
  hminim = humidity - 1;
//------------------------------------------------  
   for (int i=0; i <= 125; i++)
   {
       arraytemp[i] = arraytemp[i+1];
//----- Rotina de Maximos e Minimos do Array -----
       if (arraytemp[i] > tmaxim)
       {
          tmaxim = arraytemp[i];
       }
       
       if (arraytemp[i] < tminim)
       {
           tminim = arraytemp[i];
       }
   } // for
//-------------------------------------------------     
    arraytemp[126] = tempC; // temperatura atual
//-------------------- humidity ---------------------------------------->
  
   for (int i=0; i <= 125; i++)
   {
       arrayhum[i] = arrayhum[i+1];
//----- Rotina de Maximos e Minimos do Array -----
       if (arrayhum[i] > hmaxim)
       {
          hmaxim = arrayhum[i];
       }
       
       if (arrayhum[i] < hminim)
       {
           hminim = arrayhum[i];
       }
   } // for
//-------------------------------------------------     
    arrayhum[126] = humidity; // humidade atual
//-------------------------------------------------

//--------- Atualiza EEPROM Temperature
   eeAddress = 0;
   for (int i=0; i <= 126; i++)
   {
       tempEEPROM = arraytemp[i];
       EEPROM.put(eeAddress, tempEEPROM);
       eeAddress += sizeof(float);
   }
//--------- Atualiza EEPROM Hunidity
    for (int i=0; i <= 126; i++)
   {
       tempEEPROM = arrayhum[i];
       EEPROM.put(eeAddress, tempEEPROM);
       eeAddress += sizeof(float);
   }
//-------------------------------------------------
  u8g.firstPage();  
  do
  {
    draw();
  } while( u8g.nextPage() );
  
}//if (timeCurrent - timePrevious >= timeInterval)
//============================================================
   if ( digitalRead(uiKeyPrev) == LOW )
    {
      opcao = 1;
      u8g.firstPage();  
      do
        {
          draw();
        } while( u8g.nextPage() );
  
    }
   else if ( digitalRead(uiKeyNext) == LOW )
    {
      opcao = 2;
      u8g.firstPage();  
     do
        {
          draw();
        } while( u8g.nextPage() ); 
    }//else if

  digitalWrite(13, !digitalRead(13)); //Led indicador de atividade
}
//------------------------------------------------------------------ main loop
