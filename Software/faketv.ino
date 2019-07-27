#include "faketv.h" 

#define FAKETV_GET_RGB              0
#define FAKETV_COLOR_TRANSITION     1
#define FAKETV_COLOR_FREEZE         2
#define FAKETV_AVERAGE              35

  
typedef struct _tagFAKETV
{
  int       state;
  uint16_t  nr;
  uint16_t  ng;
  uint16_t  nb;
  uint16_t  avr[FAKETV_AVERAGE];
  uint16_t  avg[FAKETV_AVERAGE];
  uint16_t  avb[FAKETV_AVERAGE];
  uint16_t  avIdx;
  uint32_t  rgb;
  uint16_t  fadeTime;
  uint16_t  holdTime;
  uint16_t  pixelNum;
  uint8_t   r;
  uint8_t   g;
  uint8_t   b;
  
}FAKETV;

FAKETV tv;


//-------------------------------------------------------------------------------------
//
// FakeTv_Init
//
//-------------------------------------------------------------------------------------
void FakeTv_Init(void) 
{
  tv.state = FAKETV_GET_RGB;
  tv.holdTime = 0;
  tv.fadeTime = 0;
  tv.pixelNum = 0;

  tv.avIdx = 0;
  
  for (int i=0; i < FAKETV_AVERAGE; i++)
  { 
    tv.avr[i] = 0;
    tv.avg[i] = 0;
    tv.avb[i] = 0;
  }
}


//-------------------------------------------------------------------------------------
//
// FakeTv_Start
//
//-------------------------------------------------------------------------------------
void FakeTv_Start(void) 
{
  WS2812B_Color(LED_WANDLICHT, 0);
  WS2812B_Brightness(LED_WANDLICHT, DEFAULT_BRIGHTNESS, 0);
}


//-------------------------------------------------------------------------------------
//
// FakeTv_GetNextRgbValue
//
//-------------------------------------------------------------------------------------
void FakeTv_GetNextRgbValue(void)
{
  uint8_t  hi, lo, r, g, b;
  
  // Read next 16-bit (5/6/5) color
  hi = pgm_read_byte(&colors[tv.pixelNum * 2    ]);
  lo = pgm_read_byte(&colors[tv.pixelNum * 2 + 1]);

  tv.pixelNum++;
  if ( tv.pixelNum >= (sizeof(colors) / sizeof(colors[0])) )
    tv.pixelNum = 0; 
    
  // Expand to 24-bit (8/8/8)
  r = (hi & 0xF8) | (hi >> 5);
  g = (hi << 5) | ((lo & 0xE0) >> 3) | ((hi & 0x06) >> 1);
  b = (lo << 3) | ((lo & 0x1F) >> 2);

  // Apply gamma correction, further expand to 16/16/16
  tv.nr = (uint8_t)pgm_read_byte(&gamma8[r]) * 257; // New R/G/B
  tv.ng = (uint8_t)pgm_read_byte(&gamma8[g]) * 257;
  tv.nb = (uint8_t)pgm_read_byte(&gamma8[b]) * 257;
}


//-------------------------------------------------------------------------------------
//
// FakeTv_ColorTransition
//
//-------------------------------------------------------------------------------------
void FakeTv_ColorTransition(void)
{
  unsigned long  r, g, b;
  
  if ( tv.fadeTime > 0 )
  {
    tv.avr[tv.avIdx] = tv.nr;
    tv.avg[tv.avIdx] = tv.ng;
    tv.avb[tv.avIdx] = tv.nb;
    tv.avIdx++;
    
    r = 0;
    g = 0;
    b = 0;
    
    for (int i=0; i < tv.fadeTime; i++)
    { 
      r += tv.avr[i];
      g += tv.avg[i];
      b += tv.avb[i];
    }
    
    r /= tv.fadeTime; 
    g /= tv.fadeTime;
    b /= tv.fadeTime;
  }
  else
  {
    r = tv.nr;
    g = tv.ng;
    b = tv.nb;
  }

  r = r >> 8; // Quantize to 8-bit
  g = g >> 8;
  b = b >> 8;

  tv.r = r;
  tv.g = g;
  tv.b = b;

  tv.rgb = 0;
  tv.rgb = (r<<16)|(g<<8)|(b);
}


//-------------------------------------------------------------------------------------
//
// FakeTv_Service
//
//-------------------------------------------------------------------------------------
void FakeTv_Service(void) 
{
  int rnd; 
  uint32_t mw;
  
  switch ( tv.state )
  {
    case FAKETV_GET_RGB:
      FakeTv_GetNextRgbValue();

      tv.avIdx = 0;
      rnd = random(10);

      if ( rnd < 3 )
        tv.fadeTime  = random(5, 10);
      else if ( rnd < 6 )
        tv.fadeTime  = random(20, 30);
      else
        tv.fadeTime  = 0;
      
      //Serial.println("");
      //Serial.print("FAKETV_GET_RGB          -> FAKETV_COLOR_TRANSITION  ");
      //Serial.println(tv.fadeTime);
      tv.state = FAKETV_COLOR_TRANSITION;
      break;

    case FAKETV_COLOR_TRANSITION:
      FakeTv_ColorTransition();

      ledCtrl[LED_WANDLICHT].pws2812b->setColor(tv.rgb);
      
      mw = (tv.r+tv.g+tv.b)/3;
      if ( (tv.r > 220) && (tv.g > 220) && (tv.b > 220) )
        ledCtrl[LED_WANDLICHT].pws2812b->setBrightness(170);
      else if ( (tv.r < 40) && (tv.g < 40) && (tv.b < 40) )
        ledCtrl[LED_WANDLICHT].pws2812b->setBrightness(230);
      else
        ledCtrl[LED_WANDLICHT].pws2812b->setBrightness(200);
      //ledCtrl[LED_WANDLICHT].pws2812b->setBrightness(map(mw, 0, 255, 240, 140));
     
      if ( tv.avIdx == tv.fadeTime )
      { 
        if ( tv.fadeTime == 0 )
        {
          tv.holdTime  = random(TIME_MILLISECONDS(500), TIME_MILLISECONDS(1500));
        }
        else
        {
          tv.holdTime = 0;
        }
        //Serial.print("FAKETV_COLOR_TRANSITION -> FAKETV_COLOR_FREEZE  ");
        //Serial.println(tv.holdTime);
        tv.state = FAKETV_COLOR_FREEZE; 
      }
      break;

    case FAKETV_COLOR_FREEZE:
      if ( tv.holdTime > 0 )
      { 
        //Bild unveraendert halten 
        tv.holdTime--;
      }
      else
      {
        //Filter fuer die naechste Runde initialisieren
        for (int i=0; i < FAKETV_AVERAGE; i++)
        { 
          tv.avr[i] = tv.nr;
          tv.avg[i] = tv.ng;
          tv.avb[i] = tv.nb;
        }

        //Serial.println("FAKETV_COLOR_FREEZE   -> FAKETV_GET_RGB");
        tv.state = FAKETV_GET_RGB;  
      }
      
    default: 
      break;
  }
}
