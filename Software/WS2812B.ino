//-------------------------------------------------------------------------------------
//
// defines
//
//-------------------------------------------------------------------------------------
#define WARMWHITE    WS2812B_WARMWHITE


//-------------------------------------------------------------------------------------
//
// globale Objekte
//
//-------------------------------------------------------------------------------------
int      fakeTVTimer      = 100;  
int      brightness[11]  = {0, 10, 20, 35, 47, 65, 90, 120, 155, 190, 240};


//-------------------------------------------------------------------------------------
//
// WS2812B_Init
//
//-------------------------------------------------------------------------------------
void WS2812B_Init(void)
{
  pinMode(PORT_POWERSWITCH, OUTPUT);
  digitalWrite(PORT_POWERSWITCH, 1);    

  // setze LED_ANZAHL_KANAELE von 2 auf 3 wenn ledCtrl[2] genutzt werden soll
  ledCtrl[0].pws2812b = new WS2812FX(eepData.t.ledsLese, PORT_LESELICHT, NEO_GRB + NEO_KHZ800);
  ledCtrl[1].pws2812b = new WS2812FX(eepData.t.ledsWand, PORT_WANDLICHT, NEO_GRB + NEO_KHZ800);
  ledCtrl[2].pws2812b = NULL;
  
  for (int i=0; i<LED_ANZAHL_KANAELE; i++)
  {
    ledCtrl[i].rgbColor[RGB_RED] = 0;
    ledCtrl[i].rgbColor[RGB_GREEN] = 0xFF;
    ledCtrl[i].rgbColor[RGB_BLUE] = 0;
    ledCtrl[i].color = 0;
    ledCtrl[i].brightness = 0;
    ledCtrl[i].brightnessEx = 0;
    ledCtrl[i].brightnessSetVal = 0;
    ledCtrl[i].brightnessTime = 0;
    ledCtrl[i].muteTime = 0;
    ledCtrl[i].colorState = COLOR_RED_DECR;
    ledCtrl[i].colorStep = 50;
    ledCtrl[i].effectId = 0;
    ledCtrl[i].pws2812b->init();
    ledCtrl[i].pws2812b->setColor(0x000000);
    ledCtrl[i].pws2812b->setBrightness(0);
    ledCtrl[i].pws2812b->setSpeed(2000);
    ledCtrl[i].pws2812b->setMode(FX_MODE_STATIC);
    ledCtrl[i].pws2812b->start();
  }
}


//-------------------------------------------------------------------------------------
//
// WS2812B_Service
//
//-------------------------------------------------------------------------------------
void WS2812B_Service(void)
{
  int br;
  
  for (int id=0; id<LED_ANZAHL_KANAELE; id++)
  {   
    if ( ledCtrl[id].muteTime > 1)
    {
      ledCtrl[id].muteTime--;
    }
    
    if ( ledCtrl[id].muteTime == 1 )
    {
      ledCtrl[id].muteTime = 0;
      ledCtrl[id].pws2812b->setBrightness(ledCtrl[id].brightness);
    }
    
    if ( ledCtrl[id].brightnessTime != 0 )
    { 
      ledCtrl[id].brightnessEx += ledCtrl[id].brightnessStep;
      br = round(ledCtrl[id].brightnessEx);
     
      if ( ledCtrl[id].brightnessStep > 0 )
        ledCtrl[id].brightness = begr(br, 0, brightness[ledCtrl[id].brightnessSetVal]);

      if ( ledCtrl[id].brightnessStep < 0 )
        ledCtrl[id].brightness = begr(br, brightness[ledCtrl[id].brightnessSetVal], 255);

      
      if ( ledCtrl[id].muteTime == 0 )
      {
        ledCtrl[id].pws2812b->setBrightness(ledCtrl[id].brightness);
      }
        
      if ( ledCtrl[id].color == WS2812B_WARMWHITE )
        ledCtrl[id].pws2812b->setColor(WS2812B_WARMWHITE);

      //Wenn Sollwert erreicht, dann Verstellung beenden
      if ( ledCtrl[id].brightness == brightness[ledCtrl[id].brightnessSetVal] )
      {      
        ledCtrl[id].brightnessTime = 0;
        ledCtrl[id].brightnessEx = ledCtrl[id].brightness;
      }
    }
    
    ledCtrl[id].pws2812b->service();
  }
}


//-------------------------------------------------------------------------------------
//
// WS2812B_Signal
//
//-------------------------------------------------------------------------------------
void WS2812B_Signal(int color, int fxMode, int fxSpeed)
{
  ledCtrl[LED_WANDLICHT].brightnessSetVal = DEFAULT_BRIGHTNESS;
  ledCtrl[LED_WANDLICHT].pws2812b->setColor(color);
  ledCtrl[LED_WANDLICHT].pws2812b->setBrightness(brightness[ledCtrl[LED_WANDLICHT].brightnessSetVal]);
  ledCtrl[LED_WANDLICHT].pws2812b->setSpeed(fxSpeed);
  ledCtrl[LED_WANDLICHT].pws2812b->setMode(fxMode);
}


//-------------------------------------------------------------------------------------
//
// WS2812B_PowerOn
//
//-------------------------------------------------------------------------------------
void WS2812B_PowerOn(void)
{
  digitalWrite(PORT_POWERSWITCH, 1);  
}


//-------------------------------------------------------------------------------------
//
// WS2812B_PowerOff
//
//-------------------------------------------------------------------------------------
void WS2812B_PowerOff(void)
{
  digitalWrite(PORT_POWERSWITCH, 0); 
  digitalWrite(PORT_LESELICHT, 0);
  digitalWrite(PORT_WANDLICHT, 0);
}


//-------------------------------------------------------------------------------------
//
// WS2812B_Color
//
//-------------------------------------------------------------------------------------
void WS2812B_Color(eLEDIDENT id, uint32_t color)
{
  ledCtrl[id].pws2812b->setMode(FX_MODE_STATIC);
  ledCtrl[id].pws2812b->setColor(color);
  ledCtrl[id].color = color;
}


//-------------------------------------------------------------------------------------
//
// WS2812B_Brightness
//
//-------------------------------------------------------------------------------------
void WS2812B_Brightness(eLEDIDENT id, uint8_t setVal, uint16_t timeMs)
{
  ledCtrl[id].brightnessSetVal = begr(setVal, 0, ((sizeof(brightness)/sizeof(brightness[0]))-1));;
  ledCtrl[id].brightnessTime = timeMs;
 
  //pruefen, ob sanfte Helligkeitsverstellung gefordert
  if ( ledCtrl[id].brightnessTime != 0 )
  {
    //Schrittweite berechnen, die erforderlich ist, um in der vorgegebenen Zeit den Sollwert zu erreichen
    ledCtrl[id].brightnessStep = (((float)((brightness[ledCtrl[id].brightnessSetVal] - ledCtrl[id].brightnessEx) * TASK_TIME_MS)) / ledCtrl[id].brightnessTime);
  }
  else
  {
    //es soll nicht gedimmt werden, den Sollwert direkt einstellen
    ledCtrl[id].brightnessTime = 0;
    ledCtrl[id].brightness = brightness[ledCtrl[id].brightnessSetVal];
    ledCtrl[id].brightnessEx = (float)ledCtrl[id].brightness;
    ledCtrl[id].pws2812b->setBrightness(brightness[ledCtrl[id].brightness]);
  }
}


//-------------------------------------------------------------------------------------
//
// WS2812B_Mute
//
//-------------------------------------------------------------------------------------
void WS2812B_Mute(eLEDIDENT id, uint16_t timeMs)
{
  ledCtrl[id].muteTime = timeMs+1;
  ledCtrl[id].pws2812b->setBrightness(0);
}


//-------------------------------------------------------------------------------------
//
// WS2812B_SwitchOnAll
//
//-------------------------------------------------------------------------------------
void WS2812B_SwitchOnAll(void)
{
  WS2812B_Color(LED_LESELICHT, WS2812B_WARMWHITE);
  WS2812B_Color(LED_WANDLICHT, WS2812B_WARMWHITE);
  WS2812B_Brightness(LED_LESELICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
  WS2812B_Brightness(LED_WANDLICHT, DEFAULT_BRIGHTNESS, BRIGHTNESS_RISETIME_ON);
}


//-------------------------------------------------------------------------------------
//
// WS2812B_SwitchOffAll
//
//-------------------------------------------------------------------------------------
void WS2812B_SwitchOffAll(void)
{
  WS2812B_Brightness(LED_LESELICHT, 0, 0);
  WS2812B_Brightness(LED_WANDLICHT, 0, 0);
}


//-------------------------------------------------------------------------------------
//
// WS2812B_BrightnessIncr
//
//-------------------------------------------------------------------------------------
void WS2812B_BrightnessIncr(eLEDIDENT id)
{
  if ( ledCtrl[id].brightnessSetVal < ((sizeof(brightness)/sizeof(int))-1) )
    ledCtrl[id].brightnessSetVal++;

  WS2812B_Brightness(id, ledCtrl[id].brightnessSetVal, BRIGHTNESS_RISETIME_ENC);
}


//-------------------------------------------------------------------------------------
//
// WS2812B_BrightnessDecr
//
//-------------------------------------------------------------------------------------
void WS2812B_BrightnessDecr(eLEDIDENT id)
{
  if ( ledCtrl[id].brightnessSetVal > 0 )
    ledCtrl[id].brightnessSetVal--;

  WS2812B_Brightness(id, ledCtrl[id].brightnessSetVal, BRIGHTNESS_RISETIME_ENC);
}


//-------------------------------------------------------------------------------------
//
// WS2812B_WandlichtEffekt
//
//-------------------------------------------------------------------------------------
void WS2812B_WandlichtEffekt(void)
{ 
  ledCtrl[LED_WANDLICHT].effectId = 0;
  ledCtrl[LED_WANDLICHT].pws2812b->setMode(wx_effect_array[ledCtrl[LED_WANDLICHT].effectId][0]);
  ledCtrl[LED_WANDLICHT].pws2812b->setSpeed(wx_effect_array[ledCtrl[LED_WANDLICHT].effectId][1]);
  ledCtrl[LED_WANDLICHT].pws2812b->setColor(WS2812B_WARMWHITE);
  ledCtrl[LED_WANDLICHT].color = WS2812B_WARMWHITE;
}


//-------------------------------------------------------------------------------------
//
// WS2812B_WandlichtNextEffekt
//
//-------------------------------------------------------------------------------------
void WS2812B_WandlichtNextEffekt(void)
{ 
  ledCtrl[LED_WANDLICHT].effectId++;
  if ( ledCtrl[LED_WANDLICHT].effectId >= (sizeof(wx_effect_array)/sizeof(wx_effect_array[0])) )
    ledCtrl[LED_WANDLICHT].effectId = 0;

  ledCtrl[LED_WANDLICHT].pws2812b->setMode(wx_effect_array[ledCtrl[LED_WANDLICHT].effectId][0]);
  ledCtrl[LED_WANDLICHT].pws2812b->setSpeed(wx_effect_array[ledCtrl[LED_WANDLICHT].effectId][1]); 
  int rgb =  (ledCtrl[LED_WANDLICHT].rgbColor[RGB_RED]<<16)|(ledCtrl[LED_WANDLICHT].rgbColor[RGB_GREEN]<<8)|(ledCtrl[LED_WANDLICHT].rgbColor[RGB_BLUE]);
  ledCtrl[LED_WANDLICHT].pws2812b->setColor(rgb); 
  ledCtrl[LED_WANDLICHT].color = rgb;
}


//-------------------------------------------------------------------------------------
//
// WS2812B_InitColorWheel
//
//-------------------------------------------------------------------------------------
void WS2812B_InitColorWheel(uint32_t rgb)
{
  WS2812B_Color(LED_WANDLICHT, DEFAULT_COLORWHEEL_COLOR);
  ledCtrl[LED_WANDLICHT].rgbColor[RGB_RED] = ((DEFAULT_COLORWHEEL_COLOR&0xff0000)>>16);
  ledCtrl[LED_WANDLICHT].rgbColor[RGB_GREEN] = ((DEFAULT_COLORWHEEL_COLOR&0x00ff00)>>8);
  ledCtrl[LED_WANDLICHT].rgbColor[RGB_BLUE] = (DEFAULT_COLORWHEEL_COLOR&0xff);
  ledCtrl[LED_WANDLICHT].colorState = COLOR_RED_DECR;
}
//-------------------------------------------------------------------------------------
//
// WS2812B_RgbColorWheel
//
//-------------------------------------------------------------------------------------
void WS2812B_RgbColorWheel(int dir)
{
  int       rgb = 0;
  int       nextRgb = 0;
  int       step = 0;
  eLEDSTATE nextState = ledCtrl[LED_WANDLICHT].colorState;

  if ( dir > 0 )
  {
    switch (ledCtrl[LED_WANDLICHT].colorState)
    {
      case COLOR_RED_DECR:
        nextState = COLOR_GREEN_INCR;
        step = ledCtrl[LED_WANDLICHT].colorStep * (-1);
        rgb = RGB_RED;
        nextRgb = RGB_GREEN;
        break;

      case COLOR_GREEN_INCR:
        nextState = COLOR_BLUE_DECR;
        step = ledCtrl[LED_WANDLICHT].colorStep;
        rgb = RGB_GREEN;
        nextRgb = RGB_BLUE;
        break;

      case COLOR_BLUE_DECR:
        nextState = COLOR_RED_INCR;
        step = ledCtrl[LED_WANDLICHT].colorStep * (-1);
        rgb = RGB_BLUE;
        nextRgb = RGB_RED;
        break;

      case COLOR_RED_INCR:
        nextState = COLOR_GREEN_DECR;
        step = ledCtrl[LED_WANDLICHT].colorStep;
        rgb = RGB_RED;
        nextRgb = RGB_GREEN;
        break;

      case COLOR_GREEN_DECR:
        nextState = COLOR_BLUE_INCR;
        step = ledCtrl[LED_WANDLICHT].colorStep * (-1);
        rgb = RGB_GREEN;
        nextRgb = RGB_BLUE;
        break;

      case COLOR_BLUE_INCR:
        nextState = COLOR_RED_DECR;
        step = ledCtrl[LED_WANDLICHT].colorStep;
        rgb = RGB_BLUE;
        nextRgb = RGB_RED;
        break;
    }
  }
  else
  { 
    switch (ledCtrl[LED_WANDLICHT].colorState)
    {
      case COLOR_RED_DECR:
        nextState = COLOR_BLUE_INCR;
        step = ledCtrl[LED_WANDLICHT].colorStep * (-1);
        rgb = RGB_RED;
        nextRgb = RGB_BLUE;
        break;

      case COLOR_BLUE_INCR:
        nextState = COLOR_GREEN_DECR;
        step = ledCtrl[LED_WANDLICHT].colorStep;
        rgb = RGB_BLUE;
        nextRgb = RGB_GREEN;
        break;

      case COLOR_GREEN_DECR:
        nextState = COLOR_RED_INCR;
        step = ledCtrl[LED_WANDLICHT].colorStep * (-1);
        rgb = RGB_GREEN;
        nextRgb = RGB_RED;
        break;

      case COLOR_RED_INCR:
        nextState = COLOR_BLUE_DECR;
        step = ledCtrl[LED_WANDLICHT].colorStep;
        rgb = RGB_RED;
        nextRgb = RGB_BLUE;
        break;

      case COLOR_BLUE_DECR:
        nextState = COLOR_GREEN_INCR;
        step = ledCtrl[LED_WANDLICHT].colorStep * (-1);
        rgb = RGB_BLUE;
        nextRgb = RGB_GREEN;
        break;

      case COLOR_GREEN_INCR:
        nextState = COLOR_RED_DECR;
        step = ledCtrl[LED_WANDLICHT].colorStep;
        rgb = RGB_GREEN;
        nextRgb = RGB_RED;
        break;
    }
  }

  if ( dir == 0 )
    step = step * (-1);
    
  ledCtrl[LED_WANDLICHT].rgbColor[rgb] += step;


 if ( (ledCtrl[LED_WANDLICHT].rgbColor[rgb] <= 0) || (ledCtrl[LED_WANDLICHT].rgbColor[rgb] >= 255) )
  {
    ledCtrl[LED_WANDLICHT].colorState = nextState;
    ledCtrl[LED_WANDLICHT].rgbColor[rgb] = begr(ledCtrl[LED_WANDLICHT].rgbColor[rgb], 0, 255);
  }
/*
  Serial.print("Dir: ");
  Serial.print(dir);
  Serial.print("   State: ");
  Serial.print(ledCtrl[LED_WANDLICHT].colorState); 
  Serial.print("   Next: ");
  Serial.print(nextState); 
  Serial.print("   R:");
  Serial.print(ledCtrl[LED_WANDLICHT].rgbColor[RGB_RED], HEX);
  Serial.print("   G:");
  Serial.print(ledCtrl[LED_WANDLICHT].rgbColor[RGB_GREEN], HEX);
  Serial.print("   B:");
  Serial.println(ledCtrl[LED_WANDLICHT].rgbColor[RGB_BLUE], HEX);
*/  
  
  rgb =  (ledCtrl[LED_WANDLICHT].rgbColor[RGB_RED]<<16)|(ledCtrl[LED_WANDLICHT].rgbColor[RGB_GREEN]<<8)|(ledCtrl[LED_WANDLICHT].rgbColor[RGB_BLUE]);
  ledCtrl[LED_WANDLICHT].pws2812b->setColor(rgb);
}


