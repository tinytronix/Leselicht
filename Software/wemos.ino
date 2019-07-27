//-------------------------------------------------------------------------------------------------------------------------------------
//  
//  * Funktions-Beschreibung:
//  -------------------------
//  Diese Steuerung ist dazu da, um am Bett zwei LED Stripes Typ WS2812b zu steuern.
//    - Stripe 1: Leselicht (an, aus, Helligkeit, nur weißes Licht)
//    - Stripe 2: Wandlicht (an, aus, Helligkeit, Farb-Effekte)
//
//  Die Stripes werden mittels Drehdrücksteller gesteuert. Die Steuerung ist in einer 
//  Statemachine (Variable sysState, siehe Statemachine.ino) zusammengefasst. 
//  Über WLAN kann die Beleuchtung ferngesteuert werden. Die Fernsteuerung läuft über HTTP-Requests.
//  Beispiel: http://192.168.1.44/CMD?LED=1 -> schaltet das Licht an
//  Folgende Funktionen sind fernsteuerbar:
//    - Licht An, Aus
//    - Fernsehsimulation http://192.168.1.44/CMD?FAKETV=1
//  Die entsprechenden http-Kommandos müssen zyklisch gesendet werden, sonst wird der Fernsteuermodus wieder 
//  verlassen und alle LEDs werden ausgeschaltet.
//  
//  Es gibt zwei 1wire Sensoren:
//    - Temperatur Netzteil
//    - Temperatur Raum
//  Beide Temperaturen können per http abgefragt werden.
//
//  Die Updatefunktion wird aktiviert, indem die Upload-Webseite aufgerufen wird.
//  Beispiel: http://192.168.1.49/update
//    
//  Wenn ein Lichtsensor (LDR) angeschlossen ist, aktiviert sich die Beleuchtung automatisch für eine bestimmte Zeit (ca. 15s), 
//  wenn es im Zimmer dunkel wird. 
//  
//  Die WLAN Einstellungen können per Browser vorgenommen werden Die WLAN-Einstellungen können jederzeit geändert werden.
//  Der Betrieb ohne WLAN ist ebenfalls möglich. Näheres siehe in den Kapiteln "WLAN-Einstellungen" und "Default-Einstellungen"
//
//
//  * WLAN-Einstellungen
//  ---------------------
//  Im Initialzustand (Default) wird die Steuerung im Accesspoint-Modus betrieben.
//  SSID: ESP8266
//  Passwort: ESP8266
//  IP: 192.168.1.2
//
//  Der  Accesspoint-Modus kann jederzeit aktiviert werden, indem der Drehdrücksteller gehalten wird während 
//  die Spannungsversorgung eingeschaltet wird.
//
//  Dieser Betriebsmodus wird durch ein blaues Lauflicht (Komet) signalisiert. Er kann auf zwei Arten beendet werden:
//    1. Webseite http://192.168.1.2, dann dort zu den Wifi Einstellungen und SSID, IP, Passwort etc eingeben und OK drücken.
//  
//    2. Drehdrücksteller kurz drücken. 
//       Dann startet die Steuerung ebenfalls neu und leuchtet dabei für 1s grün. 
//       Das WLAN ist nun vollständig deaktiviert, kann aber später erneut aktiviert werden. 
//
//
//
//  * Eeprom:
//  ---------
//  Im Eeprom werden Netzwerk-Connectivity Daten gespeichert:
//    - Bettname
//    - die IP Adresse 
//    - Gateway
//    - Subnetzmaske
//    eepData- Wifi SSID
//    - Wifi Passwort
//  Das Eeprom kann via http Request aktualisiert werden.
//  Beispiel: http://192.168.1.50/EEP?name=Bett-Julius&ip=192.168.1.50&gw=192.168.1.2&sn=255.255.255.0&ssid=3-LTE-3FF2C0&pw=14et3010
// 
// 
//  * Default-Einstellungen
//  ------------------------
//    Die Default-Einstellungen sind:
//    - Bettname = ""
//    - IP Adresse = 192.168.1.2
//    - Gateway = 192.168.1.2
//    - Subnetzmaske = 255.255.255.0
//    - Wifi SSID = ""
//    - Wifi Passwort = ""
//    - Flags: Wifi An; AP-Modus
//
//  Die Default-Einstellungen können zu jedem Zeitpunkt wieder hergestellt werden, indem die Steuerung vom Stromnetz getrennt wird und dann bei 
//  gedrückt gehaltenem Drehdrück-Steller wieder mit dem Stromnetz verbunden wird. Wenn die Steuerung die Default-Einstellungen wieder hergestellt hat,
//  blinkt das Wandlicht für 2s rot. Dann startet die Steuerung neu und befindet sich im Initialzustand. In diesem Zustand ist der 
//  Wifi Accesspoint-Modus aktiv (angezeigt durch blaues Lauflicht. Näheres zu WLAN-Einstellungen siehe Kapitel WLAN-Einstellungen  
//
//
//  * Implementierungshinweise:
//  ---------------------------
//  Die Hauptsteuerung liegt in statemachine.ino. Diese Statemachine verarbeitet Eingabe-Events und steuert damit die LED-Stripes.
//  Eingabeevents sind:
//    - Drehdrücksteller (siehe rotary.ino - BUTTON_SHORTPRESS, BUTTON_LONGPRESS, BUTTON_DOUBLECLICK, Incremente links, Incremente rechts)
//    - HTTP-Kommandos (siehe http.ino - LED_ON, LED_OFF, FAKETV_ON, FAKETV_OFF, Sw-Update)
//    - Lichtsensor (siehe ldr.ino - Raumlicht an, Raumlicht aus)
//  
//  Der Drehdrücksteller hat eine eigene Statemachine, um die Events BUTTON_SHORTPRESS, BUTTON_LONGPRESS, BUTTON_DOUBLECLICK zu erzeugen.
//
//  zum Programmstart: Zuerst wird das Eeprom ausgelesen (eeprom_Read), um Daten für den Startzustand zu ermitteln. Von besonderer
//  Bedeutung für die Ermittlung des Startzustandes sind die Eeprom VAriablen "validMarker" und "flags". Die Flags bestimmen u.a., ob
//  im WLAN Clientmode oder WLAN-AP-Mode gestartet wird.
//  Der Startzustand wird in Statemachine_Init ermittelt. 
//  
//
//  * Verwendete Bibliothekten:
//  ---------------------------
//  Board Package ESP8266 by ESP Community in Version 2.4.2 - Wemos D1 mini Support (Board: LOLIN Wemos D1 R2 & mini)
//  Bibliothek WS2812FX in Version 1.0.6              - steuert die LED-Stripes
//  Bibliothek Adafruit_NeoPixel in Version 1.1.6     - wird von der Lib WS2812b benötigt
//  Bibliothek Encoder in Version 1.4.1               - wird zum Auslesen des Drehdrückstellers benötigt
//  Bibliothek ESP8266WiFi in Version 1.0             - wird zum Aufbau der WLAN Verbindung benötigt
//  Bibliothek DallasTemperature in Version 3.8.0     - wird zum Auslesen der Temperatursensoren benötigt
//  Bibliothek ESP8266WebServer in Version 1.0        - wird für die Fernsteuerung mittels HTTP-Requests benötigt.
//  Bibliothek ESP8266HTTPUpdateServer in Version 1.0 - (angepasst - Callbacks hinzu) wird für Remote Update über WLAN benötigt
//
//
//  * Manuell entfernte Bibliotheken:
//  --------------------------------
//  Bibliothek MAX31850_OneWire in Version 1.0.1
//
//
//  * Angepasste Bibliotheken:
//  --------------------------
//  ESP8266HTTPUpdateServer: Die Bibliothek wurde angepasst. Es wurden benutzerdefinierte Callbackfunktionen
//  hinzu gefügt, die aufgerufen werden, wenn die Update-Webseite geladen wird und wenn ein Update-Upload beginnt.  
//    - THandlerFunction onWebpage
//    - THandlerFunction onUpload
//  Wenn der benutzerdefinierte Callback onWebpage false zurückgibt, wird kein Software-Update zugelassen.
//  Die angepasste Bibliothek liegt in /home/frank/.arduino15/packages/esp8266/hardware/esp8266/2.3.0/libraries/ESP8266HTTPUpdateServer
//  Sie ist gezippt zusätzlich im Projektordner abgelegt.
//  Encoder: /home/frank/Arduino/libraries/Encoder/encoder.h:
//    - Zeile static void update(Encoder_internal_state_t *arg) {
//    - geändert zu: static void ICACHE_RAM_ATTR update(Encoder_internal_state_t *arg) {
//    - nach #include "pins_arduino.h" #endif
//    - einfügen: #ifndef ICACHE_FLASH #define ICACHE_RAM_ATTR #endif 
//
//  * Map file erstellen
//  --------------------
//  ggf zuerst avr binutils installieren: sudo apt-get install binutils-avr
//  Dann: frank@frank:~$ avr-objdump -t /tmp/arduino_build_238121/wemos.ino.elf  > /tmp/arduino_build_238121/wemos.ino.map
//
//
//  * Crashdaten auswerten
//  ----------------------
//  http://http://192.168.1.44/INFO  --> epc1 enthält die Absturzadresse - bspw. 0x40202c08
//  dann mapfile öffnen und nach 40202 suchen, um die Funktion zu finden, in der der Absturz passiert ist
//  Wenn ab Adresse 40202 viele Funktionen zu finden sind, kann die Suche eingegrenzt werden:
//  - 40202b
//  - 40202a
//  - usw
//
//  Die Adresse der gesuchten Funktion muss kleiner als die Absturzadresse sein. 
//  
//  Zur Kontrolle, ob die gefundene Funktion die richtige ist:
//  Zeile in map file: 40202be4 g     F .irom0.text  00000188 _Z18onHttpGet_Settingsv
//                         ^                              ^
//                         |                              |
//                    Funktionsanfang              Funktionslänge
//  Es wurde die korrekte Funktion gefunden, wenn gilt: (epc1 > Funktionsanfang) && (epc1 < (Funktionsanfang+Funktionslänge)
//-------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------
//
// Library includes
//
//-------------------------------------------------------------------------------------
#include <WS2812FX.h>
#include <ESP8266WiFi.h>
#include "build_defs.h"

//-------------------------------------------------------------------------------------
//
// Defines
//
//-------------------------------------------------------------------------------------
#define PROGRAM_VERSION_STRING      "V0.0.8"

#define EEPROM_VERSION_00           0x10101010

#define EEPROM_CURRENT_VERSION      0x22222222  //wenn sich die Version ändert, muss für Rückwärtskompatibilität gesorgt werden
#define EEPROM_SIZE                 122         //Bytes

#define EEPROM_VERSION_t1           0x11111111
#define EEPROM_SIZE_t1              120         //Bytes

#define EEPROM_VALIDMARKER          0xc4c4c4c4

#define EEPROM_FLAG_WIFI_ON         0x00000001  //bit set: Wifi on, else Wifi off
#define EEPROM_FLAG_WIFI_AP         0x00000002  //bit set: Wifi is in AP mode
#define EEPROM_FLAG_WIFI_DONTUSE    0x00000004  //do not use this bit, formerly used as wifi connect retry bit
#define EEPROM_FLAG_SWAP_1WIRE      0x00000008  //bit set: if Wifi clientmode connect fails, reset to AP mode again

#define WIFI_SSID                   "3-LTE-3FF2C0"
#define WIFI_PASSWORD               "14et3010"
#define WIFI_IPADDRESS              192,168,1,49
#define WIFI_GATEWAY                192,168,1,2
#define WIFI_SUBNETZ                255, 255, 255, 0

#define TASK_TIME_MS                20
#define TIME_SECONDS(x)             ((x) * (1000/TASK_TIME_MS))
#define TIME_MILLISECONDS(x)        ((x) / TASK_TIME_MS)

#define PORT_LESELICHT              D1
#define PORT_WANDLICHT              D2
#define PORT_ONEWIRE                D4
#define PORT_PUSHBUTTON             D5
#define PORT_ENCODER_DIRA           D6
#define PORT_ENCODER_DIRB           D7
#define PORT_POWERSWITCH            D8

#define DOUBLECLICK_TIME            TIME_MILLISECONDS(200) //Doppelklick-Geschwindigkeit (größere Zahl = langsameres Doppelklicken)
#define LONGPRESS_TIME              (DOUBLECLICK_TIME * 3)
#define BUTTON_DEBOUNCE_TIME        TIME_MILLISECONDS(60)

#define WS2812B_WARMWHITE           0xFFC08C
#define DEFAULT_COLORWHEEL_COLOR    0x00FF00
#define DEFAULT_BRIGHTNESS          3  //Stufe 3
#define BRIGHTNESS_RISETIME_ON      400
#define BRIGHTNESS_RISETIME_OFF     400    
#define BRIGHTNESS_RISETIME_ENC     200     //Dreh-Drueck

#define LDR_HELLDUNKELGRENZE        300
#define LED_OFFSETGRENZE            10

#define FUNCTION_TIMEOUT_REMOTEHTTP TIME_SECONDS(120)  //Fernsteuerkommandos müssen innerhalb dieses Intervalls wiederholt werden, sonst wird wieder abgeschaltet

//-------------------------------------------------------------------------------------
//
// globale Typdefinitionen
//
//-------------------------------------------------------------------------------------
typedef enum _tag_eLEDSTATE
{
  RGB_RED,          
  RGB_GREEN,
  RGB_BLUE,
  COLOR_RED_INCR,       //3
  COLOR_RED_DECR,       //4
  COLOR_GREEN_INCR,     //5
  COLOR_GREEN_DECR,     //6
  COLOR_BLUE_INCR,      //7
  COLOR_BLUE_DECR       //8
}eLEDSTATE;

typedef enum _tag_eLEDID
{
  LED_LESELICHT,
  LED_WANDLICHT,
  //LED_AUXLICHT,
  LED_ANZAHL_KANAELE
}eLEDIDENT;

typedef enum _tag_eSYSTEMSTATES
{
  SYS_INIT,
  SYS_SIGNAL2NORMAL,
  SYS_LESELICHT,
  SYS_LESELICHT2,
  SYS_WANDLICHT_FARBAUSWAHL,
  SYS_WANDLICHT_EFFEKTE,
  SYS_UPDATE,
  SYS_OFF,
  SYS_WAITOFF,
  SYS_REMOTE,
  SYS_FAKETV,
  SYS_AUTOMATIC_ON,
  SYS_WAITCFG,
  SYS_RESTART,
}eSYSTEMSTATE;

typedef enum _tag_ePUSHBUTTON_EVENTS
{
  BUTTON_NOEVENT,
  BUTTON_SHORTPRESS,
  BUTTON_LONGPRESS,
  BUTTON_DOUBLECLICK
}eBUTTONEVENT;

typedef enum _tag_ROOMLIGHT_EVENTS
{
  LIGHT_NOEVENT,
  LIGHT_SWITCHON,
  LIGHT_SWITCHOFF
}eROOMLIGHT_EVENT;

typedef enum _tag_ePUSHBUTTON_STATE
{
  BUTTON_IDLE,
  BUTTON_DOWN,
  BUTTON_UP,
  BUTTON_WAITDOUBLECLICK,
  BUTTON_WAITRELEASE
}eBUTTONSTATE;

typedef enum _tag_eHTTPCMD
{
  HTTPCMD_NULL,
  HTTPCMD_FAKETV_AN,
  HTTPCMD_FAKETV_AUS,
  HTTPCMD_LICHT_AN,
  HTTPCMD_LICHT_AUS,
  HTTPCMD_RESTART,
}eHTTPCMD;

typedef enum _tag_eWifiState
{
  WIFI_STATUS_OFF,             //Wifi off
  WIFI_STATUS_STA_CONNECTING,  //Clientmode connecting
  WIFI_STATUS_STA_CONNECTED,   //Clientlode connected to access point
  WIFI_STATUS_AP_ACTIVE,       //accesspoint mode for configuration
}eWIFISTATE;

typedef struct _tagLEDCONTROL
{
  WS2812FX     *pws2812b;
  int           rgbColor[3];
  uint32_t      color;
  uint8_t       brightness;       //0..255 entspr. WS2812FX::setBrightness
  float         brightnessEx;     //wie brightness nur als float
  float         brightnessStep;   //Schrittweite zum Erreichen des Sollwertes in der vorgegebenen Zeit
  uint8_t       brightnessSetVal; //Sollwert - Achtung: das ist kein absoluter Wert sondern der Index des arrays brightness
  uint16_t      brightnessTime;
  uint16_t      muteTime;
  eLEDSTATE     colorState;
  int           colorStep;
  int           effectId;
}LEDCONTROL;


typedef enum _tag_sEepResetMode
{
  EEP_RESET_ALL,
  EEP_RESET_WIFI
}eEEPRESETMODE;

typedef union Eeprom
{
  struct
  {    
    unsigned long version;      //4 Byte  always at offset 0 for any further eeprom layouts
    unsigned long validMarker;  //4 Byte  always at offset 4 for any further eeprom layouts
    unsigned long flags;

    int           ldrSchwelle;  //4 Byte
    int           ldrGradient;  //4 Byte
    char          mdnsName[32]; //32 Byte
    char          ssid[32];     //32 Byte
    char          password[32]; //32 Byte
    uint8_t       ip[4];        //4 Byte;
    uint8_t       gw[4];        //4 Byte;
    uint8_t       subnet[4];    //4 Byte;
  }t1;

  struct
  {    
    unsigned long version;      //4 Byte  always at offset 0 for any further eeprom layouts
    unsigned long validMarker;  //4 Byte  always at offset 4 for any further eeprom layouts
    unsigned long flags;

    int           ldrSchwelle;  //4 Byte
    int           ldrGradient;  //4 Byte
    char          mdnsName[32]; //32 Byte
    char          ssid[32];     //32 Byte
    char          password[32]; //32 Byte
    uint8_t       ip[4];        //4 Byte;
    uint8_t       gw[4];        //4 Byte;
    uint8_t       subnet[4];    //4 Byte;
    uint8_t       ledsLese;
    uint8_t       ledsWand;
    
  }t;
  
  byte b[EEPROM_SIZE];
};


//-------------------------------------------------------------------------------------
//
// globale Objekte
//
//-------------------------------------------------------------------------------------
unsigned long   lastWait; //wenn die Funktion millis überläuft (ca. alle 1193 Stunden), kann die Wartezeit in diesem Zyklus nicht berechnet werden. Dann wird die letzte Wartezeit benutzt. 
LEDCONTROL      ledCtrl[LED_ANZAHL_KANAELE];
Eeprom          eepData;


int wx_effect_array[][3] = 
{ 
  {FX_MODE_STATIC,             500, 0xff},  //static white
  {FX_MODE_STATIC,             500, 0xff},  //static color Farbe, die mit colorwheel eingestellt wurde
  {FX_MODE_LARSON_SCANNER,     500, 0xff},  //ab hier Farbe, die mit colorwheel eingestellt wurde
  {FX_MODE_COMET,              300, 0xff},
  {FX_MODE_RAINBOW_CYCLE,      250, 0xff},
  {FX_MODE_RAINBOW,           3000, 0xff},
  {FX_MODE_COLOR_WIPE_RANDOM, 2000, 0xff},
  {FX_MODE_MULTI_DYNAMIC,      500, 0xff},
  {FX_MODE_BLINK,               90, 0xff},
  {FX_MODE_FIREWORKS_RANDOM,   300, 0xff}
};


//-------------------------------------------------------------------------------------
//
// setup
//
//-------------------------------------------------------------------------------------
void setup() 
{
  Serial.begin(115200);
 
  Serial.println("\n");
  Serial.print("Version: ");
  Serial.print(PROGRAM_VERSION_STRING);
  Serial.print("  Build Number: ");
  Serial.print(TimestampedVersion);
  Serial.print("  Build date: ");
  Serial.println(__DATE__);
     
  lastWait = TASK_TIME_MS - 4; //4ms ist die durchschnittliche Zykluszeit
  eeprom_Read();
  Rotary_Init();
  WS2812B_Init();
  Wifi_Init();
  Statemachine_Init();
  Http_Init();
  Ldr_Init();
  onewire_Init();
  FakeTv_Init();
}


//-------------------------------------------------------------------------------------
//
// loop
//
//-------------------------------------------------------------------------------------
void loop() 
{
    unsigned long ts = millis();

    if ( SYS_OFF != Statemachine_GetState() )
      WS2812B_Service();
      
    onewire_Service();
    Http_Service();
    Ldr_Service();
    Wifi_Service();
    Statemachine_Service();
    
    WaitUntilNextTaskCycle(ts);
}


//-------------------------------------------------------------------------------------
//
// WaitUntilNextTaskCycle
// 
//-------------------------------------------------------------------------------------
void WaitUntilNextTaskCycle(unsigned long start)
{
  unsigned long wait;
  unsigned long runtime;
  unsigned long end = millis();
  
  if ( end >= start )
  {
    runtime = end - start; 

    if ( runtime <= TASK_TIME_MS )
    {
      wait = TASK_TIME_MS - runtime;
      //Serial.print("Wait: ");
      //Serial.println(wait);
      lastWait = wait;
      delay(wait);
    }
    else
    {
      Serial.print("Tasktime exceeded: ");
      Serial.print(runtime);
      Serial.println("ms");
    }
  }
  else
  {
    //Funktion millis() ist uebergelaufen, nutze fuer diesen Zyklus die zuletzt
    //ermittelte Wartezeit
    delay(lastWait);
  }
}


//-------------------------------------------------------------------------------------
//
// begr
// Hilfsfunktion - begrenzt einen Wert auf min bzw max
//-------------------------------------------------------------------------------------
int begr(int val, int min, int max)
{
  int v = val;
  
  if ( v < min )
    v = min;
  
  if ( v > max )
    v = max;

  return v;
}
