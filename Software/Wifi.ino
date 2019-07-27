//-------------------------------------------------------------------------------------
//
// globale Variablen und Konstanten
//
//-------------------------------------------------------------------------------------
IPAddress   ip;
IPAddress   gateway;
IPAddress   subnet;
int         connectTimer;  
eWIFISTATE  wifiState;



//-------------------------------------------------------------------------------------
//
// Wifi_Init
//
//-------------------------------------------------------------------------------------
void Wifi_Init(void)
{
   wifiState = WIFI_STATUS_OFF;
   connectTimer = 0;
}


//-------------------------------------------------------------------------------------
//
// Wifi_GetState
//
//-------------------------------------------------------------------------------------
eWIFISTATE Wifi_GetState(void)
{
  return wifiState;
}


//-------------------------------------------------------------------------------------
//
// Wifi_Service
//
//-------------------------------------------------------------------------------------
void Wifi_Service(void)
{
  if ( connectTimer > 0 )
    connectTimer--;
    
  switch (wifiState)
  {
    case WIFI_STATUS_OFF:
      break;
    
    case WIFI_STATUS_STA_CONNECTING:
      if ( connectTimer == 0 )
      {
        Serial.println("WIFI_STATUS_STA_CONNECTING --> WIFI_STATUS_STA_CONNECTING");
        Wifi_ClientMode();
      }
      else if ( WiFi.status() == WL_CONNECTED ) 
      {
        Serial.print("Connected to Wifi ");
        Serial.println(eepData.t.ssid);
        Serial.println(WiFi.localIP());
        connectTimer = 0;
        Serial.println("WIFI_STATUS_STA_CONNECTING --> WIFI_STATUS_STA_CONNECTED");
        wifiState = WIFI_STATUS_STA_CONNECTED;
      }
      break;

    case WIFI_STATUS_STA_CONNECTED:
      if ( WiFi.status() != WL_CONNECTED )
      {
        Wifi_ClientMode();
        Serial.println("WIFI_STATUS_STA_CONNECTED --> WIFI_STATUS_STA_CONNECTING");
      }
      break;

    case WIFI_STATUS_AP_ACTIVE:
      break;
      
  }
}



//-------------------------------------------------------------------------------------
//
// Wifi_APMode
//
//-------------------------------------------------------------------------------------
void Wifi_APMode(void)
{
  IPAddress ip(192,168,1,2);
  IPAddress gw(192,168,1,2);
  IPAddress subnet(255,255,255,0);

  Serial.println("Starting wifi AP mode");
  
  WiFi.softAPConfig(ip, gw, subnet);
  WiFi.softAP("ESP8266");
  WiFi.mode(WIFI_AP);
  
  Serial.println("AP mode");
  Serial.println("SSID: ESP8266");
  Serial.println("Password: ESP8266 ");
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP: ");
  Serial.println(myIP);

  wifiState = WIFI_STATUS_AP_ACTIVE;
}


//-------------------------------------------------------------------------------------
//
// Wifi_ClientMode
//
//-------------------------------------------------------------------------------------
void Wifi_ClientMode(void)
{
  Serial.println("Starting wifi client mode");
  
  ip = eepData.t.ip;
  gateway = eepData.t.gw;
  subnet = eepData.t.subnet;

  WiFi.mode(WIFI_STA);  
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(eepData.t.ssid, eepData.t.password);

  connectTimer = TIME_SECONDS(5);
  wifiState = WIFI_STATUS_STA_CONNECTING;
}


//-------------------------------------------------------------------------------------
//
// Wifi_Disable
//
//-------------------------------------------------------------------------------------
void Wifi_Disable(void)
{
  Serial.println("Wifi disabled");
  WiFi.mode(WIFI_OFF); 
  wifiState = WIFI_STATUS_OFF;
}

