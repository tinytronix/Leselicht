#include <EEPROM.h>

//-------------------------------------------------------------------------------------
//
// eeprom_Write
//
//-------------------------------------------------------------------------------------
void eeprom_Write(void)
{
  eepData.t.validMarker = EEPROM_VALIDMARKER;
  eepData.t.version = EEPROM_CURRENT_VERSION;
  
  EEPROM.begin(sizeof(eepData));
  for (int i=0; i<sizeof(eepData); i++)
  {
    EEPROM.write(i, eepData.b[i]);
  }

  EEPROM.end();
}


//-------------------------------------------------------------------------------------
//
// eeprom_Read
//
//-------------------------------------------------------------------------------------
void eeprom_Read(void)
{
  bool useDefaults = true;
  
  //Version und Validmarker auslesen (liegen bei allen Eeprom Layout Versionen an der selben Stelle)
  EEPROM.begin(8);
  for (int i=0; i<8; i++)
  {
    eepData.b[i] = EEPROM.read(i);
  }
  EEPROM.end();
  
  //Serial.print("Validmarker: ");
  //Serial.println(eepData.t.validMarker, HEX);
  //Serial.print("Version: ");
  //Serial.println(eepData.t.version, HEX);
  
  if ( eepData.t.validMarker == EEPROM_VALIDMARKER )
  {
    if ( eepData.t.version == EEPROM_CURRENT_VERSION )
    {
      useDefaults = false;
      EEPROM.begin(sizeof(eepData));
      for (int i=0; i<sizeof(eepData); i++)
      {
        eepData.b[i] = EEPROM.read(i);
      }
      EEPROM.end();
    }
    else if ( eepData.t.version == EEPROM_VERSION_t1 )
    {
      useDefaults = false;
      EEPROM.begin(EEPROM_SIZE_t1);
      for (int i=0; i<EEPROM_SIZE_t1; i++)
      {
        eepData.b[i] = EEPROM.read(i);
      }
      EEPROM.end();

      eepData.t.ledsLese = 50;
      eepData.t.ledsWand = 8;
      eeprom_Write();
    }
    else
    {
      Serial.println("EEP found older version but no transformation routine implemented.");
    }
  }

  if ( useDefaults == true )
  {
    eeprom_LoadDefaults(EEP_RESET_ALL);
  } 

  Serial.print("Eep-Flags: ");
  Serial.println(eepData.t.flags, HEX);
}


//-------------------------------------------------------------------------------------
//
// eeprom_LoadDefaults
//
//-------------------------------------------------------------------------------------
void eeprom_LoadDefaults(eEEPRESETMODE mode)
{
    //Serial.println("eeprom_LoadDefaults NEIN");
    //return;
    Serial.println("EEP invalid, use defaults");

    eepData.t.validMarker = EEPROM_VALIDMARKER; //mark as invalid
    strcpy(eepData.t.mdnsName, "esp8266");
    strcpy(eepData.t.ssid, "ESP8266");
    strcpy(eepData.t.password, "ESP8266");
    
    eepData.t.ip[0] = 192;
    eepData.t.ip[1] = 168;
    eepData.t.ip[2] = 1;
    eepData.t.ip[3] = 2;

    eepData.t.gw[0] = 192;
    eepData.t.gw[1] = 168;
    eepData.t.gw[2] = 1;
    eepData.t.gw[3] = 2;

    eepData.t.subnet[0] = 255;
    eepData.t.subnet[1] = 255;
    eepData.t.subnet[2] = 255;
    eepData.t.subnet[3] = 0;
    eepData.t.flags = (EEPROM_FLAG_WIFI_AP | EEPROM_FLAG_WIFI_ON);//Wifi im Accesspoint Modus

    if ( mode == EEP_RESET_ALL )
    {
      eepData.t.ldrSchwelle = 150;
      eepData.t.ldrGradient = -10;
  
      eepData.t.ledsLese = 20;
      eepData.t.ledsWand = 20;
    }
    
    eeprom_Write();
}

