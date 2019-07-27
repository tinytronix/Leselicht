#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

extern "C" 
{
  #include "user_interface.h"
}

//-------------------------------------------------------------------------------------
//
// globale Objekte
//
//-------------------------------------------------------------------------------------WIFI_HTML_START
ESP8266HTTPUpdateServer httpUpdater;
ESP8266WebServer        httpServer(80);
eHTTPCMD                httpCommand;
bool                    softwareUpdate;
bool                    httpCmdRestart;


//-------------------------------------------------------------------------------------
//
// index.html
//
//-------------------------------------------------------------------------------------
const char* INDEX_HTML_START =
R"( <html>
    <head>
      <title>ESP8266 Overview</title>
    </head>
    <body style='font: normal 90% Verdana; color: #3e3e3e; background-color: #f2f2f2;'>
      <h2>Overview</h2>
      <table>
        <colgroup>
        <col width='130'>
        <col>
        </colgroup>)";
        // -->
        // hier dynamischer Inhalt - siehe Funktion onRoot
        //<--



//-------------------------------------------------------------------------------------
//
// /WIFI (eeprom update webpage)
//
//-------------------------------------------------------------------------------------
const char* WIFI_HTML_START = 
R"( <html>
      <head>
        <title>Wifi</title>
      </head>
      <body style='font: normal 90% Verdana; color: #3e3e3e; background-color: #f2f2f2;'>
        <h2>Wifi Configuration</h2>)";


//-------------------------------------------------------------------------------------
//
// /INFO (reset info)
//
//-------------------------------------------------------------------------------------
const char* INFO_HTML_START = 
R"( <html>
      <head>
        <title>Info</title>
      </head>
      <body style='font: normal 90% Verdana; color: #3e3e3e; background-color: #f2f2f2;'>
        <h2>System Information</h2>
          <table>
            <colgroup>
            <col width='130'>
            <col>
            </colgroup>)";

        
//-------------------------------------------------------------------------------------
//
// /SET (settings webpage)
//
//-------------------------------------------------------------------------------------
const char* SETTINGS_HTML_START = 
R"( <html>
      <head>
        <title>Settings</title>
      </head>
      <body style='font: normal 90% Verdana; color: #3e3e3e; background-color: #f2f2f2;'>
        <h2>Settings</h2>)";



//-------------------------------------------------------------------------------------
//
// Common
//
//-------------------------------------------------------------------------------------
const char* HTML_END = 
R"( </body></html>)";


//-------------------------------------------------------------------------------------
//
// Http_Init
//
//-------------------------------------------------------------------------------------
void Http_Init(void)
{
  httpCmdRestart = false;
  softwareUpdate = false;
  httpCommand = HTTPCMD_NULL;

  httpServer.onNotFound(onRoot);
  httpServer.on("/",      HTTP_GET,  onRoot);
  httpServer.on("/WIFI",  HTTP_GET,  onHttpGet_Wifi);
  httpServer.on("/WIFI",  HTTP_POST, onHttpPost_Wifi);
  httpServer.on("/SET",   HTTP_GET,  onHttpGet_Settings);
  httpServer.on("/SET",   HTTP_POST, onHttpPost_Settings);
  httpServer.on("/INFO",   HTTP_GET,  onHttpGet_ResetInfo);

  if ( WIFI_STATUS_AP_ACTIVE != Wifi_GetState() )
  {
    httpServer.on("/ADC0",  HTTP_GET,   onHttpGet_OnewireSensor0);
    httpServer.on("/ADC1",  HTTP_GET,   onHttpGet_OnewireSensor1);
    httpServer.on("/ADC2",  HTTP_GET,   onHttpGet_LightSensor);
    httpServer.on("/CMD",  onHttpPost_Cmd);
  }
  
  MDNS.begin(eepData.t.mdnsName);
  
  httpUpdater.setup(&httpServer);
  //httpUpdater.registerCallback(ON_WEBPAGE, onUpdate_LoadWebpage);
  //httpUpdater.registerCallback(ON_FILEUPLOAD, onUpdate_UploadBegin);
  
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
}


//-------------------------------------------------------------------------------------
//
// Http_Service
//
//-------------------------------------------------------------------------------------
void Http_Service(void)
{
    httpServer.handleClient();
} 


//-------------------------------------------------------------------------------------
//
// Http_GetCurrentCommand
//
//-------------------------------------------------------------------------------------
eHTTPCMD Http_GetCurrentCommand(void)
{
  eHTTPCMD cmd;

  if ( httpCmdRestart == true )
    cmd = HTTPCMD_RESTART;
  else
    cmd = httpCommand;  
    
  httpCommand = HTTPCMD_NULL;
  return cmd;
}


//-------------------------------------------------------------------------------------
//
// onRoot
//
//-------------------------------------------------------------------------------------
void onRoot(void)
{
  IPAddress ip;
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  String html = INDEX_HTML_START;
Serial.println("Test");
  html += "<tr><td>";
  html += "MDNS name";
  html += "</td><td>";
  html += eepData.t.mdnsName;
  html += "</td></tr>";

  html += "<tr><td>";
  html += "Wifi SSID";
  html += "</td><td>";
  html += eepData.t.ssid;
  html += "</td></tr>";
  
  html += "<tr><td>";
  html += "IP Address";
  html += "</td><td>";
  if ( WIFI_STATUS_AP_ACTIVE == Wifi_GetState() )
    ip = WiFi.softAPIP();
  else
    ip = WiFi.localIP();
  html += ip.toString();
  html += "</td></tr>";

  html += "<tr><td>";
  html += "Subnetmask";
  html += "</td><td>";
  html += eepData.t.subnet[0];
  html += ".";
  html += eepData.t.subnet[1];
  html += ".";
  html += eepData.t.subnet[2];
  html += ".";
  html += eepData.t.subnet[3];
  html += "</td></tr><p>";
  
  html += "<tr><td>";
  html += "RSSI";
  html += "</td><td>";
  html += WiFi.RSSI();
  html += "dBm</td></tr>";
  
  html += "<tr><td>";
  html += "Uptime";
  html += "</td><td>";
  html += hr;
  html += ":";
  html += min%60;
  html += ":";
  html += sec%60;
  html += "</td></tr>";

  html += "<p><tr><td>";
  html += "LEDs";
  html += "</td><td>";
  if ( (ledCtrl[LED_WANDLICHT].brightness > 0) || (ledCtrl[LED_LESELICHT].brightness > 0) )
  {
    html+= "an";
  }
  else
  {
    html += "aus";
  }
  html += "</td></tr>";

  html += "<tr><td>";
  html += "Beleuchtung";
  html += "</td><td>";
  if ( true == Ldr_IsRoomLight() )
  {
    html += "hell";
  }
  else
  {
    html += "dunkel";
  }
  html += "</td></tr>";

  html += "<tr><td>";
  html += "T1 (ADC0):";
  html += "</td><td>";
  if ( oneWire_GetSensor0() == DEVICE_DISCONNECTED_C )
    html += "not connected";
  else
  {
    html += oneWire_GetSensor0();
    html += "°C";
  }
  html += "</td></tr>";

  html += "<tr><td>";
  html += "T2 (ADC1):";
  html += "</td><td>";
    if ( oneWire_GetSensor1() == DEVICE_DISCONNECTED_C )
    html += "not connected";
  else
  {
    html += oneWire_GetSensor1();
    html += "°C";
  }
  html += "</td></tr>";

  html += "<tr><td>";
  html += "LDR (ADC2):";
  html += "</td><td>";
  html += Ldr_GetAdcValue();
  html += "</td></tr>";
  
  html += "</table><p><hr><small>Software:";
  html += PROGRAM_VERSION_STRING;
  html += "&nbsp&nbsp";
  html += "Build Number:";
  html += TimestampedVersion;
  html += "&nbsp&nbsp";
  html += "Build date:";
  html += __DATE__;
  html += "<br>";
  html += "<a href=\"http://";
  html += ip.toString();
  html += "/SET\">Settings</a>";
  html += "&nbsp&nbsp<a href=\"http://";
  html += ip.toString();
  html += "/WIFI\">Wifi</a>";
  html += "&nbsp&nbsp<a href=\"http://";
  html += ip.toString();
  html += "/update\">Update</a>";
  html += "&nbsp&nbsp<a href=\"http://";
  html += ip.toString();
  html += "/INFO\">Info</a>";
  html += HTML_END;
  httpServer.send(200, "text/html", html.c_str());
}

//-------------------------------------------------------------------------------------
//
// onUpdate_UploadBegin
//
//-------------------------------------------------------------------------------------
bool Http_IsSoftwareUpdate(void)
{
  return softwareUpdate;
}


//-------------------------------------------------------------------------------------
//
// onUpdate_UploadBegin
//
//-------------------------------------------------------------------------------------
bool onUpdate_UploadBegin(void)
{
  softwareUpdate = true;
  return true;
}


//-------------------------------------------------------------------------------------
//
// onUpdate_LoadWebpage
//
//-------------------------------------------------------------------------------------
bool onUpdate_LoadWebpage(void)
{
  return true;
}


//-------------------------------------------------------------------------------------
//
// onHttpPost_Cmd
//
//-------------------------------------------------------------------------------------
void onHttpPost_Cmd(void)
{
  String key;
  String strVal;
  int value;
  bool rv = false;

  if ( 1 == httpServer.args() )
  { 
    key = httpServer.argName(0);
    strVal = httpServer.arg(0);
    value = strVal.toInt();
  
    if ( (value == 0) || (value == 1) )
    {
      if ( key == "LED" )
      { 
        rv = true;
        if ( value == 0 )
          httpCommand = HTTPCMD_LICHT_AUS;
        else
          httpCommand = HTTPCMD_LICHT_AN;
      }
      else if ( key == "FAKETV" )
      { 
        rv = true;
        if ( value == 0 )
        {
          httpCommand = HTTPCMD_FAKETV_AUS;
        }
        else
        {
          httpCommand = HTTPCMD_FAKETV_AN;
        }
      }
    }
  }
  
  if ( true == rv )
    httpServer.send(200, "text/plain", "ACK");
  else
    httpServer.send(200, "text/plain", "NAK");
}


//-------------------------------------------------------------------------------------
//
// onHttpGet_OnewireSensor0
//
//-------------------------------------------------------------------------------------
void onHttpGet_OnewireSensor0(void)
{
  SendOnewireSensor(oneWire_GetSensor0());
}


//-------------------------------------------------------------------------------------
//
// onHttpGet_OnewireSensor1
//
//-------------------------------------------------------------------------------------
void onHttpGet_OnewireSensor1(void)
{
  SendOnewireSensor(oneWire_GetSensor1());
}


//-------------------------------------------------------------------------------------
//
// SendOnewireSensor
//
//-------------------------------------------------------------------------------------
void SendOnewireSensor(float t)
{
  int val;

  if ( DEVICE_DISCONNECTED_C != t )
  {
    val = t * 100; //auf Zehntel skalieren, verbleibende Nachkommastellen aufrunden
    String answer = String(val, DEC);
    httpServer.send(200, "text/plain", answer.c_str());
  }
  else
  {
    httpServer.send(200, "text/plain", "NAK");
  }
}


//-------------------------------------------------------------------------------------
//
// onHttpGet_LightSensor
//
//-------------------------------------------------------------------------------------
void onHttpGet_LightSensor(void)
{
  if ( false == Ldr_IsRoomLight() )
  {
    //httpServer.send(200, "text/plain", "0");
  }
  else
  {
    //httpServer.send(200, "text/plain", "1");
  }

  String s = String(Ldr_GetAdcValue(), DEC);
  httpServer.send(200, "text/plain", s.c_str());
}


//-------------------------------------------------------------------------------------
//
// onHttpGet_ResetInfo
//
//-------------------------------------------------------------------------------------
void onHttpGet_ResetInfo(void)
{
  struct rst_info *rtc_info = system_get_rst_info();   
  String html = INFO_HTML_START;
  IPAddress ip;
  
  html += "<tr><td>";
  html += "Reset Reason";
  html += "</td><td>";
  switch ( rtc_info->reason )
  {
    case 0:
      html += "Power reboot";
      break;

    case 1:
      html += "Hardware WDT reset";
      break;

    case 2:
      html += "Fatal exception";
      break;

    case 3:
      html += "Software watchdog reset";
      break;

    case 4:
      html += "Software reset";
      break;

    case 5:
      html += "Deep-sleep";
      break;

    case 6:
      html += "Hardware reset";
      break;

    default:
      html += rtc_info->reason;
      break;
  }
  html += "</td></tr>";

  html += "<tr><td>";
  html += "Exception";
  html += "</td><td>";
  switch ( rtc_info->exccause )
  {
    case 0:
      html += "Invalid command";
      break;

    case 6:
      html += "Division by zero";
      break;

    case 9:
      html += "Unaligned read/write";
      break;

    case 28:
    case 29:
      html += "Access to invalid address";
      break;
      
    default:
      html += rtc_info->exccause;
      break;
  }
  html += "</td></tr>";

  html += "<tr><td>";
  html += "epc1";
  html += "</td><td>";
  html += "0x";
  html += String(rtc_info->epc1, HEX);
  html += "</td></tr>";

  html += "<tr><td>";
  html += "epc2";
  html += "</td><td>";
  html += "0x";
  html += String(rtc_info->epc2, HEX);
  html += "</td></tr>";

  html += "<tr><td>";
  html += "epc3";
  html += "</td><td>";
  html += "0x";
  html += String(rtc_info->epc3, HEX);
  html += "</td></tr>";

  html += "<tr><td>";
  html += "excvaddr";
  html += "</td><td>";
  html += "0x";
  html += String(rtc_info->excvaddr, HEX);
  html += "</td></tr>";

  html += "<tr><td>";
  html += "depc";
  html += "</td><td>";
  html += "0x";
  html += String(rtc_info->depc, HEX);
  html += "</td></tr>";
  
  html += "<tr><td>";
  html += "sysState";
  html += "</td><td>";

  switch ( Statemachine_GetState() )
  {
    case SYS_INIT:
      html += "SYS_INIT";
      break;

    case SYS_SIGNAL2NORMAL:
      html += "SYS_SIGNAL2NORMAL";
      break;

    case SYS_LESELICHT:
      html += "SYS_LESELICHT";
      break;

    case SYS_LESELICHT2:
      html += "SYS_LESELICHT2";
      break;

    case SYS_WANDLICHT_FARBAUSWAHL:
      html += "SYS_WANDLICHT_FARBAUSWAHL";
      break;

    case SYS_WANDLICHT_EFFEKTE:
      html += "SYS_WANDLICHT_EFFEKTE";
      break;

    case SYS_UPDATE:
      html += "SYS_UPDATE";
      break;

    case SYS_OFF:
      html += "SYS_OFF";
      break;

    case SYS_WAITOFF:
      html += "SYS_WAITOFF";
      break;

    case SYS_REMOTE:
      html += "SYS_REMOTE";
      break;

    case SYS_FAKETV:
      html += "SYS_FAKETV";
      break;

    case SYS_AUTOMATIC_ON:
      html += "SYS_AUTOMATIC_ON";
      break;

    case SYS_WAITCFG:
      html += "SYS_WAITCFG";
      break;

    case SYS_RESTART:
      html += "SYS_RESTART";
      break;
      
    default:
      html += Statemachine_GetState();
      break;
  }
  html += "</td></tr>";
  
  html += "</table>";

  html += "<p><hr><small>";
  html += "Software:";
  html += PROGRAM_VERSION_STRING;
  html += "&nbsp&nbsp";
  html += "Build Number:";
  html += TimestampedVersion;
  html += "&nbsp&nbsp";
  html += "Build date:";
  html += __DATE__;
  if ( WIFI_STATUS_AP_ACTIVE == Wifi_GetState() )
    ip = WiFi.softAPIP();
  else
    ip = WiFi.localIP();
  html += "<br><a href=\"http://";
  html += ip.toString();
  html += "\">Overview</a>";
  
  html += HTML_END;

  httpServer.send(200, "text/html", html.c_str());
}

//-------------------------------------------------------------------------------------
//
// onHttpGet_Wifi
//
//-------------------------------------------------------------------------------------
void onHttpGet_Wifi(void)
{
  String html = WIFI_HTML_START;
  IPAddress ip;

  html += "<FORM action='/WIFI' method='post'>";
  html += "<table><colgroup><col width='130'><col></colgroup>";
  
  html += "<tr><td>MDNS name</td><td><input type='text' name='mdns' value='";
  html += eepData.t.mdnsName;
  html += "'>";

  html += "<tr><td>SSID</td><td><input type='text' name='ssid' value='";
  html += eepData.t.ssid;
  html += "'>";

  html += "<tr><td>Password</td><td><input type='text' name='pw' placeholder='";
  html += "please enter password";
  html += "'>";

  html += "<tr><td>IP Address</td><td><input type='text' name='ip' value='";
  html += eepData.t.ip[0];
  html += ".";
  html += eepData.t.ip[1];
  html += ".";
  html += eepData.t.ip[2];
  html += ".";
  html += eepData.t.ip[3];
  html += "'>";

  html += "<tr><td>Subnet mask</td><td><input type='text' name='sn' value='";
  html += eepData.t.subnet[0];
  html += ".";
  html += eepData.t.subnet[1];
  html += ".";
  html += eepData.t.subnet[2];
  html += ".";
  html += eepData.t.subnet[3];
  html += "'>";

  html += "</td></tr>";
  html += "</table><p><button style='border: 2px solid #6cae1e;' type='submit'>Write Eeprom</button></FORM><p><hr><small>";
  html += "Software:";
  html += PROGRAM_VERSION_STRING;
  html += "&nbsp&nbsp";
  html += "Build Number:";
  html += TimestampedVersion;
  html += "&nbsp&nbsp";
  html += "Build date:";
  html += __DATE__;
  if ( WIFI_STATUS_AP_ACTIVE == Wifi_GetState() )
    ip = WiFi.softAPIP();
  else
    ip = WiFi.localIP();
  html += "<br><a href=\"http://";
  html += ip.toString();
  html += "\">Overview</a>";
  
  html += HTML_END;

  httpServer.send(200, "text/html", html.c_str());
}


//-------------------------------------------------------------------------------------
//
// onHttpPost_Wifi
//
//-------------------------------------------------------------------------------------
void onHttpPost_Wifi(void)
{
  bool rv = false;
  if ( httpServer.hasArg("mdns") )
  { 
    String name = httpServer.arg("mdns");
    if ( (name.length() > 1) && (name.length() < 30) )
    { 
      if ( httpServer.hasArg("ssid") )
      { 
        String ssid = httpServer.arg("ssid");

        if ( (ssid.length() > 1) && (ssid.length() < 30) )
        { 
          if ( httpServer.hasArg("pw") )
          { 
            String pw = httpServer.arg("pw");
    
            if ( (pw.length() > 3) && (pw.length() < 20)  )
            { 
              if ( httpServer.hasArg("ip") )
              {
              
                if ( httpServer.hasArg("sn") )
                { 
                  strcpy(eepData.t.mdnsName, name.c_str());
                  strcpy(eepData.t.password, pw.c_str());
                  strcpy(eepData.t.ssid, ssid.c_str());
                  
                  IPAddress ip;
                  ip.fromString(httpServer.arg("ip"));
                  eepData.t.ip[0] = ip[0];
                  eepData.t.ip[1] = ip[1];
                  eepData.t.ip[2] = ip[2];
                  eepData.t.ip[3] = ip[3];
                              
                  ip.fromString(httpServer.arg("sn"));
                  eepData.t.subnet[0] = ip[0];
                  eepData.t.subnet[1] = ip[1];
                  eepData.t.subnet[2] = ip[2];
                  eepData.t.subnet[3] = ip[3];

                  rv = true;
                }
              }
            }
          }
        }
      }
    }
  }

  if ( rv == true )
  {
    eepData.t.flags |= EEPROM_FLAG_WIFI_ON;
    eepData.t.flags &= ~EEPROM_FLAG_WIFI_AP;
    eeprom_Write();
    httpCmdRestart = true;
    httpServer.send(200, "text/plain", "OK, eeprom data written. Rebooting now. Please wait 10s...");
  }
  else 
  {
    httpServer.send(200, "text/plain", "ERROR, one or more parameter missing or invalid.");
  }
}



//-------------------------------------------------------------------------------------
//
// onHttpGet_Settings
//
//-------------------------------------------------------------------------------------
void onHttpGet_Settings(void)
{
  String html = SETTINGS_HTML_START;
  IPAddress ip;

  html += "<FORM action='/SET' method='post'>";
  html += "<table><colgroup><col width='130'><col></colgroup>";
  
  html += "<tr><td>LDR Threashold</td><td><input type='text' name='th' value='";
  html += eepData.t.ldrSchwelle;
  html += "'>";

  html += "<tr><td>LDR Gradient</td><td><input type='text' name='grd' value='";
  html += eepData.t.ldrGradient;
  html += "'>";

  html += "<tr><td>LEDs Lese</td><td><input type='text' name='ledsl' value='";
  html += eepData.t.ledsLese;
  html += "'>";

  html += "<tr><td>LEDs Wand</td><td><input type='text' name='ledsw' value='";
  html += eepData.t.ledsWand;
  html += "'>";

  html += "<tr><td>1W Tausch</td><td><input type='text' name='tausch' value='";
  if ( (eepData.t.flags & EEPROM_FLAG_SWAP_1WIRE) == EEPROM_FLAG_SWAP_1WIRE )
    html += "ja";
  else
    html += "nein";
  html += "'>";
  
  html += "</td></tr>";
  html += "</table><p><button style='border: 2px solid #6cae1e;' type='submit'>Write Eeprom</button></FORM><p><hr><small>";
  html += "Software:";
  html += PROGRAM_VERSION_STRING;
  html += "&nbsp&nbsp";
  html += "Build Number:";
  html += TimestampedVersion;
  html += "&nbsp&nbsp";
  html += "Build date:";
  html += __DATE__;
  if ( WIFI_STATUS_AP_ACTIVE == Wifi_GetState() )
    ip = WiFi.softAPIP();
  else
    ip = WiFi.localIP();
  html += "<br><a href=\"http://";
  html += ip.toString();
  html += "\">Overview</a>";
  html += HTML_END;
  
  httpServer.send(200, "text/html", html.c_str());
}


//-------------------------------------------------------------------------------------
//
// onHttpPost_Settings
//
//-------------------------------------------------------------------------------------
void onHttpPost_Settings(void)
{
  String argStr;
  int argInt;
  
  if ( httpServer.hasArg("th") )
  {  
    argStr = httpServer.arg("th");
    argInt = atoi(argStr.c_str());
    eepData.t.ldrSchwelle =  begr(argInt, 5, 1024);
  } 

  if ( httpServer.hasArg("grd") )
  {  
    argStr = httpServer.arg("grd");
    argInt = atoi(argStr.c_str());
    eepData.t.ldrGradient =  begr(argInt, -400, 400);
  } 

  if ( httpServer.hasArg("ledsl") )
  {  
    argStr = httpServer.arg("ledsl");
    argInt = atoi(argStr.c_str());
    eepData.t.ledsLese =  begr(argInt, 1, 200);
  } 

  if ( httpServer.hasArg("ledsw") )
  {  
    argStr = httpServer.arg("ledsw");
    argInt = atoi(argStr.c_str());
    eepData.t.ledsWand =  begr(argInt, 1, 200);
  } 
  
  if ( httpServer.hasArg("tausch") )
  {  
    argStr = httpServer.arg("tausch");
    argStr.toLowerCase() ;
    if ( 0 == argStr.compareTo("ja") )
      eepData.t.flags |= EEPROM_FLAG_SWAP_1WIRE;
    else
      eepData.t.flags &= ~EEPROM_FLAG_SWAP_1WIRE;
  }
   
  eeprom_Write();
  httpCmdRestart = true;
  httpServer.send(200, "text/plain", "OK, eeprom data written. Rebooting now. Please wait 10s...");
}


