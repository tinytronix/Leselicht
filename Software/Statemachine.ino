//-------------------------------------------------------------------------------------
//
//globale Objekte
//
//-------------------------------------------------------------------------------------
eSYSTEMSTATE      sysState;
unsigned long     sysStateTimer;
unsigned long     sysAutomDisableTimer;
eBUTTONEVENT      pushBtn;
eHTTPCMD          httpCmd;
eROOMLIGHT_EVENT  ldrEvt;
int               rotaryIncr;
            

//-------------------------------------------------------------------------------------
//
// Statemachine_Init
//
//-------------------------------------------------------------------------------------
void Statemachine_Init(void) 
{
  pushBtn    = BUTTON_NOEVENT;
  rotaryIncr = 0;
 
  sysAutomDisableTimer = TIME_SECONDS(5);
  
  if ( true == Rotary_IsButtonPressed() )
  {
    //In den Default-Zustand versetzen und neu booten
    Serial.println("Factory Reset");
    eeprom_LoadDefaults(EEP_RESET_WIFI);
    sysState = SYS_RESTART;
    sysStateTimer = TIME_SECONDS(2);
    WS2812B_Signal(RED, FX_MODE_BLINK, 200);
  }
  else if ( (eepData.t.validMarker != EEPROM_VALIDMARKER) || ((eepData.t.flags & EEPROM_FLAG_WIFI_AP) == EEPROM_FLAG_WIFI_AP) )
  {
    //Wenn das Eeprom ungültig ist (erster Start auf neuer Hardware) oder wenn 
    //der Accesspoint mode gefordert ist, dann als WLAN AP betreiben
    //LED-Signalisierung: Wandlicht blaues Lauflicht
    Wifi_APMode();
    sysState = SYS_WAITCFG;
    WS2812B_Signal(BLUE, FX_MODE_COMET, 500);
  }
  else if ((eepData.t.flags & EEPROM_FLAG_WIFI_ON) == EEPROM_FLAG_WIFI_ON )
  { 
    //Normalbetrieb als WLAN Client
    //LED-Signalisierung: erst Wandlicht blau blinken, dann für 1s grün
    Wifi_ClientMode();
    sysState = SYS_INIT;
    sysStateTimer = TIME_SECONDS(5);
    WS2812B_Signal(BLUE, FX_MODE_BLINK, 200);
  }
  else
  {
    //Normalbetrieb ohne WLAN - WLAN Modul permanent deaktiviert
    //LED-Signalisierung: Wandlicht für 1s grün 
    Wifi_Disable();
    sysStateTimer = TIME_SECONDS(1);
    WS2812B_Signal(GREEN, FX_MODE_STATIC, 2000);
    sysState = SYS_SIGNAL2NORMAL;
  }
}


//-------------------------------------------------------------------------------------
//
// Statemachine_Service
//
//-------------------------------------------------------------------------------------
void Statemachine_Service(void) 
{   
    httpCmd     = Http_GetCurrentCommand();
    pushBtn     = Rotary_Pushbutton();
    rotaryIncr  = Rotary_ReadEncoder();
    ldrEvt      = Ldr_RoomLightEvent();
  
    if ( sysAutomDisableTimer > 0 )
      sysAutomDisableTimer--;
      
    if ( sysStateTimer > 0 )
      sysStateTimer--;

    if ( httpCmd == HTTPCMD_RESTART )
      sysState = SYS_RESTART;

    if ( (pushBtn != BUTTON_NOEVENT) || (rotaryIncr != 0) || (httpCmd != HTTPCMD_NULL) )
      sysAutomDisableTimer = TIME_SECONDS(5);

    if ( sysAutomDisableTimer > 0 )
      ldrEvt = LIGHT_NOEVENT;
    
    //system statemachine - beinhaltet alle Betriebszustände
    switch (sysState)
    {
      case SYS_INIT: //System initialisiert sich nach PowerON
        if ( WIFI_STATUS_STA_CONNECTED == Wifi_GetState() )
        {
          WS2812B_Signal(GREEN, FX_MODE_STATIC, 2000);
          sysStateTimer = TIME_SECONDS(1);
          sysState = SYS_SIGNAL2NORMAL;
        }
        else if ( sysStateTimer == 0 )
        {
          WS2812B_Signal(RED, FX_MODE_STATIC, 2000);
          sysStateTimer = TIME_SECONDS(1);
          sysState = SYS_SIGNAL2NORMAL;
        }
        else
        {
         
        }
        break;

      case SYS_SIGNAL2NORMAL: //Zwischenstate, um PowerON Ereignisse per LED zu signalisieren
        if ( sysStateTimer == 0 )
        {
          WS2812B_SwitchOffAll();
          Serial.println("SYS_SIGNAL2NORMAL-->SYS_OFF");
          sysState = SYS_OFF;
        }
        break;

      case SYS_OFF: //alle LED Stripes aus
        if ( pushBtn == BUTTON_DOUBLECLICK )
        {
          sysStateTimer = 0;
          WS2812B_PowerOn();
          WS2812B_InitColorWheel(DEFAULT_COLORWHEEL_COLOR);
          WS2812B_Brightness(LED_WANDLICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
          Serial.println("SYS_OFF-->SYS_WANDLICHT_FARBAUSWAHL");
          sysState = SYS_WANDLICHT_FARBAUSWAHL;
        }
        else if ( pushBtn == BUTTON_SHORTPRESS )
        {
          WS2812B_PowerOn();
          WS2812B_Color(LED_LESELICHT, WS2812B_WARMWHITE);
          WS2812B_Brightness(LED_LESELICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
          Serial.println("SYS_OFF-->SYS_LESELICHT");
          sysState = SYS_LESELICHT;
        }
        else if ( httpCmd == HTTPCMD_LICHT_AN )
        {
          WS2812B_PowerOn();
          WS2812B_SwitchOnAll();
          sysStateTimer   = TIME_SECONDS(10);
          Serial.println("SYS_OFF-->SYS_REMOTE");
          sysState = SYS_REMOTE;
        }
        else if ( httpCmd == HTTPCMD_FAKETV_AN )
        {
          sysStateTimer = FUNCTION_TIMEOUT_REMOTEHTTP;
          WS2812B_PowerOn();
          ledCtrl[LED_WANDLICHT].pws2812b->setLength(15);
          FakeTv_Init();
          FakeTv_Start();
          Serial.println("SYS_OFF-->SYS_FAKETV");
          sysState = SYS_FAKETV;
        }
        else if ( rotaryIncr > 0 )
        {
          WS2812B_PowerOn();
          WS2812B_Color(LED_LESELICHT, WS2812B_WARMWHITE);
          WS2812B_Brightness(LED_LESELICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
          Serial.println("SYS_OFF-->SYS_LESELICHT");
          sysState = SYS_LESELICHT;
        }
        else if ( LIGHT_SWITCHOFF == ldrEvt )
        {
          WS2812B_PowerOn();
          WS2812B_Color(LED_LESELICHT, WS2812B_WARMWHITE);
          WS2812B_Color(LED_WANDLICHT, WS2812B_WARMWHITE);
          WS2812B_Brightness(LED_LESELICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
          WS2812B_Brightness(LED_WANDLICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
          sysStateTimer   = TIME_SECONDS(20);
          Serial.println("SYS_OFF-->SYS_AUTOMATIC_ON");
          sysState = SYS_AUTOMATIC_ON;
        }
        else
        {
          WS2812B_PowerOff();
        }
        break;
        
      case SYS_LESELICHT: //Drehdrueck bedient Leselicht
      case SYS_LESELICHT2:
        if ( pushBtn == BUTTON_DOUBLECLICK )
        {
          if ( ledCtrl[LED_WANDLICHT].brightnessSetVal != 0 )
          {
            WS2812B_Mute(LED_WANDLICHT, TIME_MILLISECONDS(200));
          }
          else
          { 
            WS2812B_InitColorWheel(DEFAULT_COLORWHEEL_COLOR);
            WS2812B_Brightness(LED_WANDLICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
          }
          
          if ( sysState == SYS_LESELICHT )
          {
            if ( ledCtrl[LED_WANDLICHT].brightnessSetVal == 0 )
              WS2812B_InitColorWheel(DEFAULT_COLORWHEEL_COLOR);
            Serial.println("SYS_LESELICHT-->SYS_WANDLICHT_FARBAUSWAHL");
            sysState = SYS_WANDLICHT_FARBAUSWAHL;
          }
          else
          {
            Serial.println("SYS_LESELICHT-->SYS_WANDLICHT_EFFEKTE");
            sysState = SYS_WANDLICHT_EFFEKTE;
          }
        }
        else if ( pushBtn == BUTTON_LONGPRESS )
        {
          WS2812B_SwitchOffAll();
          Serial.println("SYS_LESELICHT-->SYS_OFF");
          sysState = SYS_OFF;
        }
        else if ( pushBtn == BUTTON_SHORTPRESS )
        {
          WS2812B_Mute(LED_LESELICHT, TIME_MILLISECONDS(200));
        }
        else if ( rotaryIncr > 0 )
        {
          WS2812B_BrightnessIncr(LED_LESELICHT);
        }
        else if ( rotaryIncr < 0 )
        {
          WS2812B_BrightnessDecr(LED_LESELICHT);
        }
        else if ( httpCmd == HTTPCMD_LICHT_AUS )
        {
          sysStateTimer = 0;
          WS2812B_SwitchOffAll();
          Serial.println("SYS_LESELICHT-->SYS_OFF");
          sysState = SYS_OFF;
        }
        else if ( (ledCtrl[LED_WANDLICHT].brightnessSetVal == 0) && (ledCtrl[LED_LESELICHT].brightnessSetVal == 0) )
        {
          WS2812B_SwitchOffAll();
          Serial.println("SYS_LESELICHT-->SYS_OFF");
          sysState = SYS_OFF;
        }
        break;

      case SYS_WANDLICHT_FARBAUSWAHL: //Drehdrueck bedient Farbauswahl Wandlicht
        if ( pushBtn == BUTTON_DOUBLECLICK )
        {
          if ( ledCtrl[LED_LESELICHT].brightnessSetVal != 0 )
          {
            WS2812B_Mute(LED_LESELICHT, TIME_MILLISECONDS(200));
          }
          else
          {
            WS2812B_Color(LED_LESELICHT, WS2812B_WARMWHITE);
            WS2812B_Brightness(LED_LESELICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
          }
          Serial.println("SYS_WANDLICHT_FARBAUSWAHL-->SYS_LESELICHT");
          sysState = SYS_LESELICHT;
        }
        else if ( pushBtn == BUTTON_LONGPRESS )
        {
          WS2812B_SwitchOffAll();
          Serial.println("SYS_WANDLICHT_FARBAUSWAHL-->SYS_OFF");
          sysState = SYS_OFF;
        }
        else if ( pushBtn == BUTTON_SHORTPRESS )
        {
          WS2812B_WandlichtEffekt();
          Serial.println("SYS_WANDLICHT_FARBAUSWAHL-->SYS_WANDLICHT_EFFEKTE");
          sysState = SYS_WANDLICHT_EFFEKTE;
        }
        else if ( httpCmd == HTTPCMD_LICHT_AUS )
        {
          sysStateTimer = 0;
          WS2812B_SwitchOffAll();
          Serial.println("SYS_WANDLICHT_FARBAUSWAHL-->SYS_OFF");
          sysState = SYS_OFF;
        }
        else
        {
          if ( rotaryIncr > 0 )
            WS2812B_RgbColorWheel(1);

          if ( rotaryIncr < 0 )
            WS2812B_RgbColorWheel(0);
        }
        break;
        
      case SYS_WANDLICHT_EFFEKTE: //Drehdrueck kann Lichteffekte fuer Wandlicht einstellen
        if ( pushBtn == BUTTON_DOUBLECLICK )
        {      
          if ( ledCtrl[LED_LESELICHT].brightnessSetVal != 0 )
          {
            WS2812B_Mute(LED_LESELICHT, TIME_MILLISECONDS(200));
          }
          else
          {
            WS2812B_Color(LED_LESELICHT, WS2812B_WARMWHITE);
            WS2812B_Brightness(LED_LESELICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
          }
          Serial.println("SYS_WANDLICHT_EFFEKTE-->SYS_LESELICHT2");
          sysState = SYS_LESELICHT2;
        }
        else if ( pushBtn == BUTTON_LONGPRESS )
        {
          WS2812B_SwitchOffAll();
          Serial.println("SYS_WANDLICHT_EFFEKTE-->SYS_OFF");
          sysState = SYS_OFF;
        }
        else if ( pushBtn == BUTTON_SHORTPRESS )
        {
          if ( ledCtrl[LED_WANDLICHT].brightnessSetVal == 0 )
            WS2812B_Brightness(LED_WANDLICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
       
          if ( ledCtrl[LED_WANDLICHT].effectId >= ((sizeof(wx_effect_array)/sizeof(wx_effect_array[0]))-1) )
          {
            ledCtrl[LED_WANDLICHT].effectId = 0;
            uint32_t rgb = (ledCtrl[LED_WANDLICHT].rgbColor[RGB_RED]<<16)|(ledCtrl[LED_WANDLICHT].rgbColor[RGB_GREEN]<<8)|(ledCtrl[LED_WANDLICHT].rgbColor[RGB_BLUE]);
            WS2812B_Color(LED_WANDLICHT, rgb);
            Serial.println("SYS_WANDLICHT_EFFEKTE-->SYS_WANDLICHT_FARBAUSWAHL");
            sysState = SYS_WANDLICHT_FARBAUSWAHL;
          }
          else
          {
            Serial.println("SYS_WANDLICHT_EFFEKTE-->SYS_WANDLICHT_EFFEKTE");
            WS2812B_WandlichtNextEffekt();
          }
        }
        else if ( rotaryIncr > 0 )
        {
          WS2812B_BrightnessIncr(LED_WANDLICHT);
        }
        else if ( rotaryIncr < 0 )
        {
          WS2812B_BrightnessDecr(LED_WANDLICHT);
          
        }
        else if ( httpCmd == HTTPCMD_LICHT_AUS )
        {
          sysStateTimer = 0;
          WS2812B_SwitchOffAll();
          Serial.println("SYS_WANDLICHT_EFFEKTE-->SYS_OFF");
          sysState = SYS_OFF;
        }
        
        if ( (rotaryIncr != 0) && (ledCtrl[LED_WANDLICHT].brightnessSetVal == 0) && (ledCtrl[LED_LESELICHT].brightnessSetVal == 0) )
        {
          sysStateTimer = TIME_SECONDS(3);
          Serial.println("SYS_WANDLICHT_EFFEKTE-->SYS_WAITOFF");
          sysState = SYS_WAITOFF;
        }
        break;

      case SYS_WAITOFF: //warte auf Abschalten
        if ( pushBtn == BUTTON_DOUBLECLICK )
        {
          WS2812B_InitColorWheel(DEFAULT_COLORWHEEL_COLOR);
          WS2812B_Brightness(LED_WANDLICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
          sysStateTimer = 0;
          Serial.println("SYS_WAITOFF-->SYS_WANDLICHT_FARBAUSWAHL");
          sysState = SYS_WANDLICHT_FARBAUSWAHL;
        }
        else if ( pushBtn == BUTTON_SHORTPRESS )
        {
          WS2812B_Color(LED_LESELICHT, WS2812B_WARMWHITE);
          WS2812B_Brightness(LED_LESELICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
          sysStateTimer = 0;
          Serial.println("SYS_WAITOFF-->SYS_LESELICHT");
          sysState = SYS_LESELICHT;
        }
        else if ( sysStateTimer == 0 )
        {
          WS2812B_SwitchOffAll();
          Serial.println("SYS_WAITOFF-->SYS_OFF");
          sysState = SYS_OFF;
        }
        else if ( rotaryIncr > 0 )
        {
          WS2812B_BrightnessIncr(LED_WANDLICHT);
          sysStateTimer = 0;
          Serial.println("SYS_WAITOFF-->SYS_WANDLICHT_EFFEKTE");
          sysState = SYS_WANDLICHT_EFFEKTE;
        }
        break;

      case SYS_REMOTE:
        if ( (pushBtn == BUTTON_LONGPRESS) || (sysStateTimer == 0) )
        {
          sysStateTimer = 0;
          WS2812B_SwitchOffAll();
          Serial.println("SYS_REMOTE-->SYS_OFF");
          sysState = SYS_OFF;
        }
        else if ( httpCmd == HTTPCMD_LICHT_AN )
        {
          sysStateTimer = TIME_SECONDS(10); 
        }
        else if ( httpCmd == HTTPCMD_LICHT_AUS )
        {
          sysStateTimer = 0;
          WS2812B_SwitchOffAll();
          Serial.println("SYS_REMOTE-->SYS_OFF");
          sysState = SYS_OFF;
        }
        break;

      case SYS_FAKETV:
        FakeTv_Service();
        if ( (pushBtn == BUTTON_LONGPRESS) || (sysStateTimer == 0) )
        {
          sysStateTimer  = 0;
          ledCtrl[LED_WANDLICHT].pws2812b->setLength(eepData.t.ledsWand);
          WS2812B_SwitchOffAll();
          Serial.println("SYS_FAKETV-->SYS_OFF");
          sysState = SYS_OFF;
        }
        else if ( httpCmd == HTTPCMD_FAKETV_AN )
        {
          sysStateTimer = TIME_SECONDS(FUNCTION_TIMEOUT_REMOTEHTTP); 
        }
        else if ( httpCmd == HTTPCMD_FAKETV_AUS )
        {
          sysStateTimer = 0;
          ledCtrl[LED_WANDLICHT].pws2812b->setLength(eepData.t.ledsWand);
          WS2812B_SwitchOffAll();
          Serial.println("FAKE_TV-->SYS_OFF");
          sysState = SYS_OFF;
        }
        break;

      case SYS_AUTOMATIC_ON:
        if ( (pushBtn == BUTTON_LONGPRESS) || (sysStateTimer == 0) )
        {
          sysStateTimer  = 0;
          WS2812B_SwitchOffAll();
          sysAutomDisableTimer = TIME_SECONDS(1);
          Serial.println("SYS_AUTOMATIC_ON-->SYS_OFF");
          sysState = SYS_OFF;
        }
        else if ( (pushBtn == BUTTON_SHORTPRESS) || (rotaryIncr != 0) )
        {
          sysStateTimer = 0;
          WS2812B_Brightness(LED_WANDLICHT, 0, 0);
          WS2812B_Brightness(LED_LESELICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
          Serial.println("SYS_AUTOMATIC_ON-->SYS_LESELICHT");
          sysState = SYS_LESELICHT;
        }
        break;

      case SYS_WAITCFG:
        if ( pushBtn == BUTTON_SHORTPRESS )
        {
          //switch wifi permanently off and restart
          eepData.t.flags &= ~EEPROM_FLAG_WIFI_ON;
          eepData.t.flags &= ~EEPROM_FLAG_WIFI_AP;
          eeprom_Write();
          sysState = SYS_RESTART;
        }
        break;

      case SYS_RESTART:
        if ( (sysStateTimer == 0) && (false == Rotary_IsButtonPressed()) )
        {
          Serial.println("SYS_RESTART-->REBOOT");
          delay(100);
          ESP.reset();
        }
        break;
    }
}


//-------------------------------------------------------------------------------------
//
// Statemachine_GetState
//
//-------------------------------------------------------------------------------------
eSYSTEMSTATE Statemachine_GetState(void) 
{ 
  return sysState;
}

