#include <OneWire.h>
#include <DallasTemperature.h>

//-------------------------------------------------------------------------------------
//
// globale Objekte
//
//-------------------------------------------------------------------------------------
OneWire           oneWire(PORT_ONEWIRE);
DallasTemperature sensors(&oneWire);

float             tempC[2];
uint8_t           owAddr[2][8];
bool              owConnected[2];

int               intervall;
int               owState;

 
//-------------------------------------------------------------------------------------
//
// onewire_Init
//
//-------------------------------------------------------------------------------------
void onewire_Init(void)
{
  intervall     = TIME_SECONDS(2);
  owState       = 0; 

  owConnected[0] = false;
  owConnected[1] = false;
  tempC[0] = DEVICE_DISCONNECTED_C;
  tempC[1] = DEVICE_DISCONNECTED_C;
  
  sensors.begin();
  
  sensors.requestTemperatures();
  delay(800);
  
  if ( sensors.getAddress(owAddr[0], 0) )
  {
    owConnected[0] = true;
    tempC[0] = sensors.getTempC(owAddr[0]);
    Serial.println("Sensor 0 Address: ");
    Serial.print(owAddr[0][0], HEX);
    Serial.print(owAddr[0][1], HEX);
    Serial.print(owAddr[0][2], HEX);
    Serial.print(owAddr[0][3], HEX);
    Serial.print(owAddr[0][4], HEX);
    Serial.print(owAddr[0][5], HEX);
    Serial.print(owAddr[0][6], HEX);
    Serial.println(owAddr[0][7], HEX);
  }
  if ( sensors.getAddress(owAddr[1], 1) )
  {
    owConnected[1] = true;
    tempC[1] = sensors.getTempC(owAddr[1]);
    Serial.println("Sensor 1 Address: ");
    Serial.print(owAddr[1][0], HEX);
    Serial.print(owAddr[1][1], HEX);
    Serial.print(owAddr[1][2], HEX);
    Serial.print(owAddr[1][3], HEX);
    Serial.print(owAddr[1][4], HEX);
    Serial.print(owAddr[1][5], HEX);
    Serial.print(owAddr[1][6], HEX);
    Serial.println(owAddr[1][7], HEX);
  }
}


//-------------------------------------------------------------------------------------
//
// onewire_Service
//
//-------------------------------------------------------------------------------------
void onewire_Service(void)
{
  if ( intervall > 0 )
  {
    intervall--;
    return;
  }

  switch (owState)
  {
    case 0:
      sensors.setWaitForConversion(false);  //Temperaturerfassung asynchron starten 
      sensors.requestTemperatures();
      sensors.setWaitForConversion(true);
      intervall = TIME_SECONDS(2);
      owState = 1;
      break;

    case 1:
      intervall = TIME_SECONDS(2);
      if ( owConnected[0] == true )
        tempC[0] = sensors.getTempC(owAddr[0]);
      owState = 2;
      break;

    case 2:
      intervall = TIME_SECONDS(6);
      if ( owConnected[1] == true )
        tempC[1] = sensors.getTempC(owAddr[1]);
      owState = 0;
      break;
  }
}


//-------------------------------------------------------------------------------------
//
// oneWire_GetSensor0
//
//-------------------------------------------------------------------------------------
float oneWire_GetSensor0(void)
{
  //wenn Sensor nicht verbunden oder fehlerhaft, dann wird DEVICE_DISCONNECTED_C
  //zurückgegeben. Dieser Wert entspr. einer negativen Temperatur ausserhalb
  //des Messbereiches.
   if ( (eepData.t.flags & EEPROM_FLAG_SWAP_1WIRE) == EEPROM_FLAG_SWAP_1WIRE )
    return tempC[1];
   else
    return tempC[0];
}


//-------------------------------------------------------------------------------------
//
// oneWire_GetSensor1
//
//-------------------------------------------------------------------------------------
float oneWire_GetSensor1(void)
{
  //wenn Sensor nicht verbunden oder fehlerhaft, dann wird DEVICE_DISCONNECTED_C
  //zurückgegeben. Dieser Wert entspr. einer negativen Temperatur ausserhalb
  //des Messbereiches.
  if ( (eepData.t.flags & EEPROM_FLAG_SWAP_1WIRE) == EEPROM_FLAG_SWAP_1WIRE )
    return tempC[0];
   else
    return tempC[1];
}
