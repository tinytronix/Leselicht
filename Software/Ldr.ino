//-------------------------------------------------------------------------------------
//
// globale Objekte
//
//-------------------------------------------------------------------------------------
#define GRADIENT_ARR    5
#define LDR_ZUST_HELL       1
#define LDR_ZUST_DUNKEL     2
#define LDR_ZUST_UNBEKANNT  3
#define LDR_EVAL_TIME       150 //milliseconds

eROOMLIGHT_EVENT  roomLightEvent;
int               lastAdc;
int               lichtZustand;
int               ldrEvalTimer;

//-------------------------------------------------------------------------------------
//
// Ldr_Init
//
//-------------------------------------------------------------------------------------
void Ldr_Init(void)
{
  ldrEvalTimer = TIME_MILLISECONDS(LDR_EVAL_TIME);
  roomLightEvent = LIGHT_NOEVENT;
  lastAdc = analogRead(A0);
  if ( lastAdc > LDR_HELLDUNKELGRENZE )
    lichtZustand = LDR_ZUST_HELL;
  else
    lichtZustand = LDR_ZUST_DUNKEL;
  
}


//-------------------------------------------------------------------------------------
//
// Ldr_Service
//
//-------------------------------------------------------------------------------------
void Ldr_Service(void)
{
  int adc;

  if ( ldrEvalTimer > 0 )
  {
    ldrEvalTimer--;
  }
  else
  {
    ldrEvalTimer = TIME_MILLISECONDS(LDR_EVAL_TIME);
    adc = analogRead(A0);
    if ( (lichtZustand == LDR_ZUST_HELL) && (adc < eepData.t.ldrSchwelle) && ((adc - lastAdc) < eepData.t.ldrGradient) )
    {
      roomLightEvent = LIGHT_SWITCHOFF;
      lichtZustand = LDR_ZUST_DUNKEL;
    }
  
    if ( adc > LDR_HELLDUNKELGRENZE )
      lichtZustand = LDR_ZUST_HELL;
      
    lastAdc = adc; 
  }
}


//-------------------------------------------------------------------------------------
//
// Ldr_IsRoomLight
//
//-------------------------------------------------------------------------------------
bool Ldr_IsRoomLight(void)
{
  if ( lichtZustand == LDR_ZUST_DUNKEL )
    return false;
    else
    return true;
}


//-------------------------------------------------------------------------------------
//
// Ldr_GetAdcValue
//
//-------------------------------------------------------------------------------------
int Ldr_GetAdcValue(void)
{
  return analogRead(A0);
}


//-------------------------------------------------------------------------------------
//
// Ldr_RoomLightEvent
//
//-------------------------------------------------------------------------------------
eROOMLIGHT_EVENT Ldr_RoomLightEvent(void)
{
  eROOMLIGHT_EVENT rv;
  rv = roomLightEvent;
  roomLightEvent = LIGHT_NOEVENT;
  return rv;
}

