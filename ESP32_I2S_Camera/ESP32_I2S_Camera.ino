#include "OV7670.h"

//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_ST7735.h> // Hardware-specific library

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include "BMP.h"

const int SIOD = 21; //SDA
const int SIOC = 22; //SCL

const int VSYNC = 39;
const int HREF = 36;

const int XCLK = 32;
const int PCLK = 33;

const int D0 = 13;
const int D1 = 12;
const int D2 = 14;
const int D3 = 27;
const int D4 = 26;
const int D5 = 25;
const int D6 = 35;
const int D7 = 34;


#define ssid1        "ssid1"
#define password1    "password1"
//#define ssid2        ""
//#define password2    ""

IPAddress myIP       = IPAddress(192,168,0,98);  // IP para conectarse a la red 
IPAddress myGateway = IPAddress(192,168,0, 1);



OV7670 *camera;

WiFiMulti wifiMulti;
WiFiServer server(80);

bool wifi_connect(){
  WiFi.config( myIP, myGateway, IPAddress(255,255,255,0));  
  wifiMulti.addAP(ssid1,password1);
  //wifiMulti.addAP(ssid2,password2);
  


  //Serial.println(F("Connecting Wifi..."));
  if(wifiMulti.run() == WL_CONNECTED) {

          
      /*//Serial.println(F("--- WiFi connected ---"));
      //Serial.print(F("SSID: "));
      //Serial.println( WiFi.SSID() );
      //Serial.print(F("IP Address: "));
      //Serial.println( WiFi.localIP() );*/
      
    return true;
  }
  else return false;
}

unsigned char bmpHeader[BMP::headerSize];

void serve()
{
  WiFiClient client = server.available();
  if (client) 
  {
    //Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        //Serial.write(c);
        if (c == '\n') 
        {
          if (currentLine.length() == 0) 
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print(
              "<style>body{margin: 0}\nimg{height: 75%; width: auto}</style>"
              "<img id='a' src='/camera' onload='this.style.display=\"initial\"; var b = document.getElementById(\"b\"); b.style.display=\"none\"; b.src=\"camera?\"+Date.now(); '>"
              "<img id='b' style='display: none' src='/camera' onload='this.style.display=\"initial\"; var a = document.getElementById(\"a\"); a.style.display=\"none\"; a.src=\"camera?\"+Date.now(); '>");
            client.println();
            break;
          } 
          else 
          {
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {
          currentLine += c;
        }
        
        if(currentLine.endsWith("GET /camera"))
        {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:image/bmp");
            client.println();
            
            client.write(bmpHeader, BMP::headerSize);
            client.write(camera->frame, camera->xres * camera->yres * 2);
        }
      }
    }
    // close the connection:
    client.stop();
    //Serial.println("Client Disconnected.");
  }  
}

void setup() 
{
       
  if(wifi_connect()){ 
      server.begin();     
  }
  
  camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
  BMP::construct16BitHeader(bmpHeader, camera->xres, camera->yres);
  
}

void displayY8(unsigned char * frame, int xres, int yres)
{

  int i = 0;
  for(int x = 0; x < xres; x++)
    for(int y = 0; y < yres; y++)
    {
      i = y * xres + x;
      unsigned char c = frame[i];
      unsigned short r = c >> 3;
      unsigned short g = c >> 2;
      unsigned short b = c >> 3;

    }  
}

void displayRGB565(unsigned char * frame, int xres, int yres)
{
  int i = 0;
  for(int x = 0; x < xres; x++)
    for(int y = 0; y < yres; y++)
    {
      i = (y * xres + x) << 1;
    }  
}

void loop()
{
  camera->oneFrame();
  serve();
  displayRGB565(camera->frame, camera->xres, camera->yres);
}
