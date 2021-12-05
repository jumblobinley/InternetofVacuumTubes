
#include <Adafruit_NeoPixel.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>


extern "C" {
#include "user_interface.h"
}

//This is an active low LED output on GPIO2
#define LED0 2
#define LED_ON false
#define LED_OFF true

#define flamePin 14
#define VERSION 1.0


#define TIMECOUNTERMAX 3300000

String hostName = "PUMPKIN_";
String hostIP = "";
String sIP;
uint32_t red = 50;
uint32_t green = 0;
uint32_t blue = 200;
int wait = 150;
int wait_counter = 0;
uint32_t time_counter = TIMECOUNTERMAX;
#define DST_TIME_OFFSET 5
#define MAX_WIFI_RETRY 40


/* *******************************************************************************
 *  Definitions:
 ********************************************************************************/
 
#define VACUUMTUBESETUP true     //PUMPKINSETUP is for gwarnetowrk
//#define MERCURYSETUP true     //MERCURYSETUP is for work wifi
//#define OLED_SCREEN true
//#define PIXEL_RING true

/* ******************************************************************************
 * End Definitions
 ****************************************************************************** */

#ifdef VACUUMTUBESETUP

      //************************************** PUMPKIN SETUP **************************************************************  
      //This buffer was generated via the Arduino jason assistant  https://bblanchon.github.io/ArduinoJson/assistant/
      
      #define LED_COUNT 2
      bool _flicker = true;
      const size_t bufferSize = JSON_ARRAY_SIZE(10) + JSON_OBJECT_SIZE(1) + 10*JSON_OBJECT_SIZE(4) + 320;
      const char* ssid     = "IOP_Network";
      const char* password = "xxxxxx"; 
      
      
#endif          

#ifdef PIXEL_RING
      //this is for neo pixel rings which display as RGB - not GRB
      
      Adafruit_NeoPixel lights = Adafruit_NeoPixel(LED_COUNT, flamePin, NEO_RGB + NEO_KHZ800);
#else
      
      //this is for neo pixel strips and wires
      Adafruit_NeoPixel lights = Adafruit_NeoPixel(LED_COUNT, flamePin, NEO_GRB + NEO_KHZ800);
#endif


/*  Arduino IDE Setup:
 *   
 *   --Ever Since upgrading my router to an ASUS RT-AX86U, the ESP 2.2.1 firmware update causes eratic bahaviour
 *   -- I was foced to downgrade the AI-Thinker ESP8266 firmware via these settings 
 *   
 *   Actual Board: LoLin NodeMCU lua V3  
 *   Go to File->Preferences and copy the URL below to get the ESP board manager extensions:
 *   http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *   
 *   Tools -> Board -> Generic ESP 8266 module
 *   Tools -> Flash Size -> 4M (FS: 2M ...)
 *   Tools -> CPU Frequency -> 80 Mhz
 *   Tools -> Upload Speed -> 115200
 *   Tools -> Erase flash -> "all flash contents"
 *   Tools -> ExpressiF FW -> nonos-sdk 2.2.1 (legacy)
 *   Tools -> debug level -> none
 *   
 *   Tools-->Port--> (whatever it is)
 *   
 *   //Not Used
 *   Board:  NodeMCU 1.0 (ESP-12E Module)
 *   Go to File->Preferences and copy the URL below to get the ESP board manager extensions:
 *   http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *   
 *   Tools -> Board -> NodeMCU 1.0 (ESP-12E Module)
 *   Tools -> Flash Size -> 4M (FS: 2M ...)
 *   Tools -> CPU Frequency -> 80 Mhz
 *   Tools -> Upload Speed -> 115200
 *   Tools-->Port--> (whatever it is)
 *   
 *   Mappings for pins on NodeMCU board
 *   static const uint8_t D0 = 16;
 *   static const uint8_t D1 = 5;
 *   static const uint8_t D2 = 4;
 *   static const uint8_t D3 = 0;
 *   static const uint8_t D4 = 2;
 *   static const uint8_t D5 = 14;
 *   static const uint8_t D6 = 12;
 *   static const uint8_t D7 = 13;
 *   static const uint8_t D8 = 15;
 *   static const uint8_t D9 = 3;
 *   static const uint8_t D10 = 1;
 *   
 */

ESP8266WebServer server(80);
WiFiClient client;

void setup(void)
{
  
  //Seting up GPIO outputs

  //This is an active low LED output on GPIO2
  pinMode(LED0, OUTPUT);
  
  digitalWrite(LED0, LED_ON);
  
  Serial.begin(115200);
  Serial.println("Starting");

  //turn on neopixels
  lights.begin();                      // Initialize all pixels to 'off'
  lights.show();                       // Initialize all pixels to 'off'

  WiFi.mode(WIFI_STA);

  delay(random(wait/2, wait));
  hostName.concat(millis()); 
  WiFi.hostname(hostName.c_str());
  Serial.print("HostName: ");
  Serial.println(hostName);
      
  //wifi_station_set_hostname((char*) hostName.c_str());  //sdk, non-Arduino function - See more at: http://www.esp8266.com/viewtopic.php?f=29&t=11124#sthash.pDHBMLJ4.dpuf
  
  //WiFi.mode(WIFI_AP_STA); //access point and server station
  WiFi.begin(ssid, password);
  //WiFi.disconnect();
  delay(100);

  
  // Wait for connection
  int wifi_counter = MAX_WIFI_RETRY;
  
  while (WiFi.status() != WL_CONNECTED)
  {
    colorWipe(lights.Color(0, 0, 0), 1);
    delay(500);
    
    Serial.print(".");
    colorWipe(lights.Color(0, 20, 0), 1);
    delay(20);
    wifi_counter--;
    if (wifi_counter <=0){ Serial.println("\n****NO WIFI FOUND ****\n"); break;}
  }

  if (wifi_counter >0)
  {
      Serial.println("");
      Serial.print("Connected to ");
      Serial.println(ssid);
      Serial.print("First IP address: ");
      Serial.println(WiFi.localIP());
      
      hostIP.concat(WiFi.localIP().toString() );
      sIP.concat( WiFi.localIP().toString() );
      
      
      if (WiFi.SSID().length() > 0) 
      {
          Serial.printf("ReConnecting to Remote AP '%s' ...\r\n", WiFi.SSID().c_str());
          WiFi.reconnect();
      }

          
            /*  Set up the server end points
            ***********************************/
            
            server.on("/", handleRoot);           // Root location - main web site
          
            server.on("/PUMPKIN", pumpkinRoot);   // Web service calls: http://192.168.1.143/PUMPKIN?color=purple
            
            server.on("/COLOR", colorRoot);       // Web service calls: http://192.168.1.143/COLOR?red=254&green=50&blue=0
          
            server.on("/TEST", testRoot);             // Test routine end point: http://192.168.1.143/TEST
          
            server.on("/API", apiRoot);             // Test routine end point: http://192.168.1.143/API

            server.on("/HEALTH", healthRoot);             // Test routine end point: http://192.168.1.143/HEALTH
          
          
          //  server.on("/inline", []()             //and example of an inline endpoint: http://192.168.1.143/inline
          //    {
          //      Pulse_Lights();
          //      server.send(200, "text/plain", "this works as well");
          //    }
          //  );
          
            server.onNotFound(handleNotFound);
          
            server.begin();
            Serial.println("HTTP server started");
   
  }   
 
  
  
  
  //flash_Lights();
  Pulse_Lights();


  //lights.setPixelColor(0, green,red blue);
  //lights.show();
  
  delay(200);  
  digitalWrite(LED0, LED_OFF); 
}

/********************************************************************
 *             LOOP
 ********************************************************************/
void loop(void)
{
 server.handleClient();

  if(_flicker == true)
  {
        flicker(red, green, blue);
  } 
  else
  {
    time_counter++;
    //Serial.println(time_counter);
    
    if (time_counter >= TIMECOUNTERMAX)
    {
       // Do something at a slower cycle rate than the loop here
       //Serial.println("Getting lights json file from S3");
        //digitalWrite(LED0, true);
        //digitalWrite(LED0, false);
       //time_counter = 0;
    }
  } 
}



/********************************************************************
 *              Http Server End Points
 ********************************************************************/

void handleNotFound()
{
  digitalWrite(LED0, LED_ON);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED0, LED_OFF);
}

/* This is the main entry point.  The master web page is generated here */
void handleRoot() 
{

  digitalWrite(LED0, LED_ON);
  
  String page = "<!DOCTYPE HTML> <html> <head> ";
  page.concat("<link rel=\"icon\" href=\"data:;base64,iVBORw0KGgo=\">");
  
  //begin stylesheet
  page.concat("<style> body {background-color: #");

  //Setting background color from the current light color
      String str_red = String(red, HEX);
      String str_redz = String("0"); str_redz.concat(str_red);
      if (str_red.length() == 1) {str_red = str_redz;}
      
      String str_green = String(green, HEX);
      String str_greenz = String("0"); str_greenz.concat(str_green);
      if (str_green.length() == 1) {str_green = str_greenz;}
      
      String str_blue = String(blue, HEX);
      String str_bluez = String("0"); str_bluez.concat(str_blue);
      if (str_blue.length() == 1) {str_blue = str_bluez;}
      
      String color = str_red+str_green+str_blue;
  
  page.concat(color);
  page.concat(";} "); 
  page.concat("div  {font-family: arial; font-size: 24px;} ");
  
  
  page.concat(" button {background-color: green; color: #900; font-weight: bold; border: 3px double #FC6; font-size: 50px; text-transform: uppercase; text-align:center} ");
  page.concat(" </style> \n");
  // end stylesheet

  
  page.concat(" <title>Vacuum Tube Controller");
  page.concat(hostName);
  page.concat(" Version ");
  page.concat(VERSION);
  page.concat(" </title> </head>\n");

  
  page.concat(" <body> <font face='Arial' size='16'> Pumpkin Control  </font> <br /> <font face='verdana' size='6'> " + hostName +" </font>\n"); 
  page.concat("<div id='messages'></div> ");
  page.concat(" <hr/>");
  
  page.concat(" <form method='get' action='/PUMPKIN?color=green'> <button type='submit' name='color' value='green'>green</button> </form> ");
  page.concat(" <form method='get' action='/PUMPKIN?color=red'> <button type='submit' name='color' value='red'>red</button> </form> ");
  page.concat(" <form method='get' action='/PUMPKIN?color=blue'> <button type='submit' name='color' value='blue'>blue</button> </form> ");
  page.concat(" <form method='get' action='/PUMPKIN?color=purple'> <button type='submit' name='color' value='purple'>purple</button> </form> ");
  page.concat(" <form method='get' action='/PUMPKIN?color=deep'> <button type='submit' name='color' value='deep'>deep</button> </form> ");
  page.concat(" <form method='get' action='/PUMPKIN?color=off'> <button type='submit' name='color' value='off'>off</button> </form> ");
  page.concat(" <form method='get' action='/TEST'> <button type='submit' name='color' value='TEST'>Test</button> </form> ");
  
  page.concat(" <div> </div> ");
  page.concat(" <form method='get' action='/COLOR?red=254&green=50&blue=0'> ");
  page.concat("    <input type='hidden' name='red' value='255' />  ");
  page.concat("    <input type='hidden' name='green' value='60' />  ");
  page.concat("    <input type='hidden' name='blue' value='0' />  ");
  page.concat(" <button type='submit' >Orange</button> </form><br/>");
  
  
  page.concat(" <hr/> <div id='APIS'> ");
          
  page.concat(" <A href='/'>HOME</A> <br/>");
  page.concat(" <A href='/PUMPKIN?color=red'>PUMPKIN</A> <br/>");
  page.concat(" <A href='/COLOR?red=254&green=50&blue=0'>COLOR</A> <br/>");
  page.concat(" <A href='/TEST'>TEST</A> <br/>");
  page.concat(" <A href='/API'>API</A> <br/>");

  page.concat("<br/><br/>");
  page.concat("</div> <hr/> \n");

  //page.concat(" <div> \n");
  //page.concat(" </br> </div> \n");

  page.concat("  </body> </html> ");
  
  digitalWrite(LED0, LED_OFF);

  server.send(200, "text/html", page);

  
}

/* This endpoint describes the API'S */
void apiRoot() 
{

  digitalWrite(LED0, LED_ON);
  String page = "<!DOCTYPE HTML> <html> <head> ";
  page.concat("<style> body {background-color: LIGHTSALMON;} div  {font-family: arial; font-size: 14px;} "); 
  page.concat(" button {background-color: green; color: #900; font-weight: bold; border: 3px double #FC6; font-size: 150%; text-transform: uppercase; text-align:center} ");
  page.concat(" div.apititle  {font-family: arial; font-size: 18px; font-weight: bold; color: blue;} ");
  page.concat(" </style>");
  page.concat("<link rel=\"icon\" href=\"data:;base64,iVBORw0KGgo=\">");
  page.concat(" <title>Vacuum Tube Controller</title>");
  page.concat(" </head>");
  page.concat(" <body> <font face='Arial'> <h1>Pumpkin CONTROL - API Definitions</h1> ");
  
  page.concat("  <br/>");
  
  
  page.concat("<div id='APIS'> ");
  page.concat("<div class='apititle'>Light Control Micro service</div> <br/>");
  page.concat("Set all lights to one color via json POST <br/>");
  page.concat(" POST to: <i> Device_IP/COLOR</i> <br/>");
  page.concat("JSON Body = <br/>");
  page.concat("{ <br/>");
  page.concat(" &nbsp; &nbsp; &nbsp; &quot;RED&quot;: &quot;254&quot;<br/>");
  page.concat(" &nbsp; &nbsp; &nbsp; &quot;GREEN&quot;: &quot;30&quot;<br/>");
  page.concat(" &nbsp; &nbsp; &nbsp; &quot;BLUE&quot;: &quot;100&quot;<br/>");
  page.concat("} <br/>");
  page.concat("ex: 192.168.1.154/COLOR");
  page.concat("</div> ");
  page.concat("<hr/> <br/>");

  page.concat("<div id='APIS'> ");
  page.concat("<div class='apititle'>Light Control Micro service </div> <br/>");
  page.concat("Set all lights to one color via query string GET <br/>");
  page.concat(" GET to: <i> Device_IP/COLOR</i> <br/>");
  page.concat("query string parameters = <br/>");
  page.concat("?red=254&green=50&blue=0 <br/>");
  page.concat("ex: 192.168.1.154/COLOR?red=254&green=50&blue=0");
  page.concat("</div> ");
  page.concat("<hr/> <br/>");


  page.concat("</font>  </body> </html> ");

  server.send(200, "text/html", page);
  digitalWrite(LED0, LED_ON);

}


void pumpkinRoot() 
{
  Serial.println("PUMPKIN:Root - pumpkin has been called");
   _flicker = true;
  
  String command = server.argName(0)+" "+server.arg(0);
  Serial.println(command);
  String text;
  
  if (command.equals("color green"))
  {
    text = "Pumpkin color green - "+command;

    red = 0;
    green = 250;
    blue = 10;
 
  }else if (command.equals("color blue"))
  {
    text = "Pumpkin - "+command;


    red = 0;
    green = 0;
    blue = 255;
  }else if (command.equals("color purple"))
  {
    text = "Pumpkin - "+command;


    red = 255;
    green = 0;
    blue = 255;
  }else if (command.equals("color red"))
  {
    text = "Pumpkin - "+command;


    red = 255;
    green = 0;
    blue = 0;
  }else if (command.equals("color deep"))
  {
    text = "Pumpkin - "+command;
 

    red = 0;
    green = 10;
    blue = 20;
  }else if (command.equals("color off"))
  {
    text = "Pumpkin - "+command;

    red = 0;
    green = 0;
    blue = 0;
  }
  else 
  {
    text = "Pumpkin - "+command;

    red = 55;
    green = 5;
    blue = 0;
  }

  String ret = returnHTML(text);
  server.send(200, "text/html", ret);
  
}

void colorRoot() 
{
  Serial.println ("COLOR:Root ");
  _flicker = true;
  String ret = "";
  
  if (server.method() == HTTP_POST)
  {
    Serial.println("received a post");
    Serial.println("COLOR: REST API has been called");
    
    String command = server.argName(0);
    String json_value = server.arg(0);
    Serial.println("Command " + command);
    Serial.println("Value " + json_value);

    //StaticJsonBuffer<200> jsonBuffer;
    StaticJsonDocument<200> doc;
    
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, json_value);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    
    JsonObject root = doc.to<JsonObject>();
    //const char* red = root["RED"];
    
    String Sred((const char*) root["RED"]);
    String Sgreen((const char*) root["GREEN"]);
    String Sblue((const char*) root["BLUE"]);
    
    red = Sred.toInt();
    green = Sgreen.toInt();
    blue = Sblue.toInt();
    
    ret = returnHTML("setting color to "+Sred+ " "+Sgreen+" "+Sblue);
    
  }
  else
  {
    Serial.println("received a get");
    Serial.println("COLOR: HTTP API has been called");
  
    String command_red = server.argName(0);
    String value_red = server.arg(0);

    String command_green = server.argName(1);
    String value_green = server.arg(1);

    String command_blue = server.argName(2);
    String value_blue = server.arg(2);
  
    Serial.println(command_red + " " + value_red);
    Serial.println(command_green + " " + value_green);
    Serial.println(command_blue + " " + value_blue);
  
    red = value_red.toInt();
    green = value_green.toInt();
    blue = value_blue.toInt();
    ret = returnHTML("setting color to "+value_red+ " "+value_green+" "+value_blue);
  } 
  
  server.send(200, "text/html", ret);
  
}

void testRoot()
{
  
  String command = server.argName(0)+" "+server.arg(0);
 
  Serial.println("TEST:Root - test has been called");
  _flicker = true;
   Serial.println(command);
  

  flash_Lights();

  String ret = returnHTML("Get flashed boi");
  
  server.send(200, "text/html", ret );

}

String returnHTML( String inText)
{
  String page = "<!DOCTYPE HTML> <html> <head> \n";
  page.concat("<link rel=\"icon\" href=\"data:;base64,iVBORw0KGgo=\"> \n");
  page.concat("<style> \n body {background-color: LIGHTSALMON;} div  {font-family: arial; font-size: 14px;} \n"); 
  page.concat(" button {background-color: green; color: #900; font-weight: bold; border: 3px double #FC6; font-size: 150%; text-transform: uppercase; text-align:center} ");
  page.concat(" div.response  {font-family: arial; font-size: 12px; font-weight: normal; color: black;} \n");
  page.concat(" </style>\n");

  page.concat(" <script> \n");
  page.concat(" function goBack() { \n");
  page.concat("   window.history.back(); \n");
  page.concat(" } \n");
  page.concat(" </script> \n");
  
  page.concat(" <title>Vacuum Tube Controller</title>");
  page.concat(" </head>");
  
  page.concat(" <body onload=\"window.history.back()\"> ");
  
  page.concat("  <br/>");

  page.concat("          <font face='Arial'>");
  page.concat("          <div class='response'>\n");
  page.concat(inText);

  page.concat("  <br/> <br/> <button onclick=\"goBack()\">Go Back</button> \n");
  
  page.concat( "         </div> ");
  
  page.concat("       <br/><br/>");
  page.concat("            <form method='get' action='/?page=home'> ");
  page.concat("               <button type='submit' name='page' value='home' >Back to the controller</button> ");
  page.concat("            </form>");
  page.concat("     ");
  page.concat("            <hr/> ");
  page.concat("           </font>  ");
  page.concat("     </body> ");
  page.concat("</html>");


  return page;
}

/*
 * Health Root
 * The main edge controller will be calling /HEALTH and expects "working" as response
 * test/plain with a status code of 200 is expected
 */
void healthRoot()
{
  digitalWrite(LED0, LED_ON);
  String message = "working";
  server.send(200, "text/plain", message);
  digitalWrite(LED0, LED_OFF);
}



//--------------------------------------------- utilities -------------------------------------------------------------


void light_Pixel(int pixel, int ired, int igreen, int iblue)
{

      uint32_t c = lights.Color(igreen, ired, iblue);
      lights.setPixelColor(pixel, c);
      lights.show();
}


void flash_Lights()
{

    for (int cnt=1; cnt<6; cnt++)
    {
      /*
      digitalWrite(led15_red, 0);
      digitalWrite(led12_green, 1);
      delay(50);
      digitalWrite(led15_red, 1);
      digitalWrite(led12_green, 0);
      delay(50);

      digitalWrite(led15_red, 0);
      digitalWrite(led12_green, 0);
     */
     
     colorWipe(lights.Color(0, 254, 0), 30);
     delay(50);
  
     colorWipe(lights.Color(0, 0, 254), 30);
     delay(50);
  
     colorWipe(lights.Color(254, 0, 0), 30);
     delay(50);

      
    }
}

void Pulse_Lights()
{

    for (int cnt=1; cnt<2; cnt++)
    {
     
     colorWipe(lights.Color(254, 254, 0), 30);
     delay(20);
  
     colorWipe(lights.Color(0, 0, 254), 30);
     delay(20);
  
     colorWipe(lights.Color(0, 0, 0), 30);
     delay(30);

      
    }
}

void flicker(unsigned int red, unsigned int green, unsigned int blue) 
{
  
  if (wait_counter == 0)
  {
    wait_counter=random(wait/2, wait);
    //Serial.println(wait_counter);
    
    int pixel=random(0,LED_COUNT);
    int redlvl = random(red/2,red);
    int greenlvl = random(green/2,green);
    int bluelvl = random(blue/2, blue);
  
    uint32_t c = lights.Color(greenlvl, redlvl, bluelvl);
    lights.setPixelColor(pixel, c);
    lights.show();
  }else
  {
    wait_counter--;
    delay(1);
  }
    
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<lights.numPixels(); i++) {
      lights.setPixelColor(i, c);
      lights.show();
      delay(wait);
     
  }
}

/*
 * *
 * * GitHub Examples
 * * This is for me, cause I cant always remember the git flow syntax
 *  
 *  New Git Repo - do this one time
 *  echo "# Internet of Vacuum Tubes" >> README.md
 *  git init
 *  git add README.md
 *  git commit -m "first commit"
 *  git branch -M main
 *  git remote add origin git@github.com:jumblobinley/LWSC_Auto_Rob_V1.git
 *  ----- Note: before doing this  you need a public key for the account in git hub
 *  https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent
 *  
 *  git push -u origin main
 *  git add InternetofVacuumTubes.ino
 *  git commit -m "Added core arduino code file v1"
 *  git push -u origin main
 *  
 *  
 *  ---Write code
 *  ---Add file to repo:
 *  git add <filename>
 *   
 *  ---When done writing code, commit your code and push to github in the cloud
 *  
 *  cd ~/Documents/SketchBook/InternetofVacuumTubes/InternetofVacuumTubes
 *  git add InternetofVacuumTubes.ino
 *  git commit -m "comment"
 *  
 *  git push -u origin main
 *  
 */
