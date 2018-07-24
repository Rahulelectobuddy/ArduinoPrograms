#include <Wire.h>
#include "DS3231.h"
#include <Arduino.h>
#include <U8x8lib.h>
#include <SPI.h>
#include "mcp_can.h"
#include <SD.h>

const int SPI_CS_PIN_CAN = 4;
const int SPI_CS_PIN_SD = 5;

DS3231 Clock;
U8X8_KS0108_128X64 u8x8(27,29,31,33,35,37,39,41,/*enable=*/ 25, /*dc=*/ 23, /*cs0=*/ 43, /*cs1=*/ 45, /*cs2=*/ U8X8_PIN_NONE, /* reset=*/  U8X8_PIN_NONE);  // Set R/W to low!
MCP_CAN CAN(SPI_CS_PIN_CAN);


#define CAN_ID_PID          0x7DF

// Variables for CAN
unsigned char PID_INPUT,getPid    = 0;

int i,flag=1;
unsigned char len = 0,buf[8];

// Variables for real time clock
bool h12=true,PM=true,Century=true;
byte Year,Month,Date,Hour,Minute,Second;

void GetDateStuff() {
  // Call this if you notice something coming in on 
  // the serial port. The stuff coming in should be in 
  // the order YYMMDDHHMMSS, with an 'x' at the end.
  boolean GotString = false;
  char InChar;
  byte Temp1, Temp2;
  char InString[20];

  byte j=0;
  while (!GotString) {
    if (Serial.available()) {
      InChar = Serial.read();
      InString[j] = InChar;
      j += 1;
      if (InChar == 'x') {
        GotString = true;
      }
    }
  }
  if(InChar == 'x')
  {
    Serial.println(InString);
    // Read Year first
    Temp1 = (byte)InString[0] -48;
    Temp2 = (byte)InString[1] -48;
    Year = Temp1*10 + Temp2;
    // now month
    Temp1 = (byte)InString[2] -48;
    Temp2 = (byte)InString[3] -48;
    Month = Temp1*10 + Temp2;
    // now date
    Temp1 = (byte)InString[4] -48;
    Temp2 = (byte)InString[5] -48;
    Date = Temp1*10 + Temp2;
    // now Hour
    Temp1 = (byte)InString[6] -48;
    Temp2 = (byte)InString[7] -48;
    Hour = Temp1*10 + Temp2;
    // now Minute
    Temp1 = (byte)InString[8] -48;
    Temp2 = (byte)InString[9] -48;
    Minute = Temp1*10 + Temp2;
    // now Second
    Temp1 = (byte)InString[10] -48;
    Temp2 = (byte)InString[11] -48;
    Second = Temp1*10 + Temp2;
  }
}

void setup(void)
{
  Serial.begin(9600);
  Wire.begin();
  // boot sequence
  u8x8.begin();    
  u8x8.clear();
  u8x8.setFont(u8x8_font_saikyosansbold8_u); 
  u8x8.setCursor(0,1);
  u8x8.print("  DRU ELECTRIC ");
  u8x8.setCursor(0,2); 
  u8x8.print("  WELCOMES YOU ");
  delay(500);
  u8x8.clear();
  u8x8.setCursor(0,1);
  for( i = 1; i < 4; i++ )
  {
    switch (i) 
    {
      case 1:
        Clock.enable32kHz(true);
        Clock.enableOscillator(true, false, 0);
        u8x8.setCursor(1,1);
        u8x8.print("RTC OK");
        break;
      case 2:
        while (CAN_OK != CAN.begin(CAN_500KBPS))    // init can bus : baudrate = 500k
        {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
        }
        Serial.println("CAN BUS Shield init ok!");
        u8x8.setCursor(1,1);
        u8x8.print("CAN OK");
        break;
      case 3:
        Serial.print("/n Initializing SD card...");
        if (!SD.begin(SPI_CS_PIN_SD)) 
        {
            Serial.println("Card failed, or not present");
            return;
        }
        Serial.println("card initialized.");
        u8x8.setCursor(1,1);
        u8x8.print("SD CARD OK");
        break;
      default:
        u8x8.setCursor(1,1);
        u8x8.print("Error");
    }
    delay(1000);
  }
  attachInterrupt(digitalPinToInterrupt(2), UPDATE, CHANGE);
  u8x8.clear();
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
}

void loop()
{
  if(flag==1)
  {
      u8x8.setCursor(0,0);
      u8x8.print(Clock.getDate(), DEC);
      u8x8.print("-");
      u8x8.print(Clock.getMonth(Century), DEC);
      u8x8.print("-");
      
      u8x8.print(Clock.getYear(), DEC);
      u8x8.print(" ");
      u8x8.print(Clock.getHour(h12,PM), DEC); //24-hr
      u8x8.print(":");
      u8x8.print(Clock.getMinute(), DEC);
      u8x8.print(":");
      u8x8.println(Clock.getSecond(), DEC);
      flag=0;
   }
   if(CAN_MSGAVAIL == CAN.checkReceive())                   // check if get data
    {    
        String dataString_id = "";
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
        u8x8.clear();
        File dataFile = SD.open("datalog2.txt", FILE_WRITE);
        //Serial.println("\r\n------------------------------------------------------------------");
        //Serial.print("Get Data From id: ");
        u8x8.setCursor(1,1);
        u8x8.print("ID : ");
        //Serial.println(CAN.getCanId(), HEX);
        u8x8.print(CAN.getCanId(), DEC);
        //dataString_id = (CAN.getCanId(), HEX);
        //Serial.print("0x");
        Serial.println(buf[1]*256+buf[0],DEC);
        u8x8.setCursor(1,2);
        u8x8.print("Data : ");
        u8x8.print(buf[2], DEC);
        //Serial.print("\t");
        if (dataFile) {
          //DATE
        dataFile.println();
        dataFile.print("|   DATE :");
        dataFile.print(Clock.getDate(), DEC);
        dataFile.print("-");
        dataFile.print(Clock.getMonth(Century), DEC);
        dataFile.print("-");
        dataFile.print(Clock.getYear(), DEC);
          //TIME
        dataFile.print("|   TIME :");
        dataFile.print(Clock.getHour(h12,PM), DEC);
        dataFile.print(":");
        dataFile.print(Clock.getMinute(), DEC);
        dataFile.print(":");
        dataFile.print(Clock.getSecond(), DEC);
        if ((Clock.getSecond()) < 10) 
          {
            dataFile.print(" ");
          }
          //SENDER ID
        dataFile.print("|   SENDER-ID :");
        dataFile.print(dataString_id);
          //Message
        
        for(int i = 0; i<len; i++)    // print the data
        {
            dataFile.print("|   MBYTE");
            dataFile.print(i+1);
            dataFile.print(" :");
            dataFile.print("0x");
            dataFile.print(buf[i], HEX);
            if((buf[i])<10)
            {
              dataFile.print(" ");
            }
        }   
        dataFile.close();
                    }
        else {
           Serial.println("error opening datalog.txt");
          }
        Serial.println();
    }
    if (Serial.available()) 
    {
      GetDateStuff();
      Clock.setClockMode(false);  // set to 24h
      Clock.setYear(Year);
      Clock.setMonth(Month);
      Clock.setDate(Date);
      Clock.setHour(Hour);
      Clock.setMinute(Minute);
      Clock.setSecond(Second);
      delay(500);
    }
   
}
void UPDATE()
{
  flag=1;
}
