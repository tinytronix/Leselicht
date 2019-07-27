# Funktions-Beschreibung:
Diese Steuerung ist dazu da, um am Bett zwei LED Stripes Typ WS2812b zu steuern.
- Stripe 1: Leselicht (an, aus, Helligkeit, nur weißes Licht)
- Stripe 2: Wandlicht (an, aus, Helligkeit, Farb-Effekte)

Die Stripes werden mittels Drehdrücksteller gesteuert. Die Steuerung ist in einer 
Statemachine (Variable sysState, siehe Statemachine.ino) zusammengefasst. 
Über WLAN kann die Beleuchtung ferngesteuert werden. Die Fernsteuerung läuft über HTTP-Requests.
Beispiel: http://192.168.1.44/CMD?LED=1 -> schaltet das Licht an
Folgende Funktionen sind fernsteuerbar:
- Licht An, Aus
- Fernsehsimulation http://192.168.1.44/CMD?FAKETV=1

Die entsprechenden http-Kommandos müssen zyklisch gesendet werden, sonst wird der Fernsteuermodus wieder 
verlassen und alle LEDs werden ausgeschaltet.

Es gibt zwei 1wire Sensoren:
- Temperatur Netzteil
- Temperatur Raum
Beide Temperaturen können per http abgefragt werden.

Die Updatefunktion wird aktiviert, indem die Upload-Webseite aufgerufen wird.
Beispiel: http://192.168.1.49/update
    
Wenn ein Lichtsensor (LDR) angeschlossen ist, aktiviert sich die Beleuchtung automatisch für eine bestimmte Zeit (ca. 15s), 
wenn es im Zimmer dunkel wird. 
  
Die WLAN Einstellungen können per Browser vorgenommen werden Die WLAN-Einstellungen können jederzeit geändert werden.
Der Betrieb ohne WLAN ist ebenfalls möglich. Näheres siehe in den Kapiteln "WLAN-Einstellungen" und "Default-Einstellungen"

# WLAN-Einstellungen
Im Initialzustand (Default) wird die Steuerung im Accesspoint-Modus betrieben.
- SSID: ESP8266
- Passwort: ESP8266
- IP: 192.168.1.2

Der  Accesspoint-Modus kann jederzeit aktiviert werden, indem der Drehdrücksteller gehalten wird während 
die Spannungsversorgung eingeschaltet wird.
Dieser Betriebsmodus wird durch ein blaues Lauflicht (Komet) signalisiert. Er kann auf zwei Arten beendet werden:
1. Webseite http://192.168.1.2, dann dort zu den Wifi Einstellungen und SSID, IP, Passwort etc eingeben und OK drücken.
2. Drehdrücksteller kurz drücken. 
Dann startet die Steuerung ebenfalls neu und leuchtet dabei für 1s grün. 
Das WLAN ist nun vollständig deaktiviert, kann aber später erneut aktiviert werden. 

# Eeprom:
Im Eeprom werden Netzwerk-Connectivity Daten gespeichert:
- Bettname
- die IP Adresse 
- Gateway
- Subnetzmaske
- Wifi SSID
- Wifi Passwort

Das Eeprom kann via http Request aktualisiert werden.
Beispiel: http://192.168.1.50/EEP?name=BettOben&ip=192.168.1.50&gw=192.168.1.2&sn=255.255.255.0&ssid=3-LTE-3FF2C0&pw=14et3010

# Default-Einstellungen
Die Default-Einstellungen sind:
- Bettname = ""
- IP Adresse = 192.168.1.2
- Gateway = 192.168.1.2
- Subnetzmaske = 255.255.255.0
- Wifi SSID = ""
- Wifi Passwort = ""
- Flags: Wifi An; AP-Modus

Die Default-Einstellungen können zu jedem Zeitpunkt wieder hergestellt werden, indem die Steuerung vom Stromnetz getrennt wird und dann bei gedrückt gehaltenem Drehdrück-Steller wieder mit dem Stromnetz verbunden wird. Wenn die Steuerung die Default-Einstellungen wieder hergestellt hat,blinkt das Wandlicht für 2s rot. Dann startet die Steuerung neu und befindet sich im Initialzustand. In diesem Zustand ist der Wifi Accesspoint-Modus aktiv (angezeigt durch blaues Lauflicht. Näheres zu WLAN-Einstellungen siehe Kapitel WLAN-Einstellungen  

# Implementierungshinweise:
Die Hauptsteuerung liegt in statemachine.ino. Diese Statemachine verarbeitet Eingabe-Events und steuert damit die LED-Stripes.
Eingabeevents sind:
- Drehdrücksteller (siehe rotary.ino - BUTTON_SHORTPRESS, BUTTON_LONGPRESS, BUTTON_DOUBLECLICK, Incremente links, Incremente rechts)
- HTTP-Kommandos (siehe http.ino - LED_ON, LED_OFF, FAKETV_ON, FAKETV_OFF, Sw-Update)
- Lichtsensor (siehe ldr.ino - Raumlicht an, Raumlicht aus)

Der Drehdrücksteller hat eine eigene Statemachine, um die Events BUTTON_SHORTPRESS, BUTTON_LONGPRESS, BUTTON_DOUBLECLICK zu erzeugen.

# zum Programmstart:
Zuerst wird das Eeprom ausgelesen (eeprom_Read), um Daten für den Startzustand zu ermitteln. Von besonderer
Bedeutung für die Ermittlung des Startzustandes sind die Eeprom VAriablen "validMarker" und "flags". Die Flags bestimmen u.a., ob
im WLAN Clientmode oder WLAN-AP-Mode gestartet wird. Der Startzustand wird in Statemachine_Init ermittelt. 

# Verwendete Bibliothekten:
- Board Package ESP8266 by ESP Community in Version 2.4.2 - Wemos D1 mini Support (Board: LOLIN Wemos D1 R2 & mini)
- Bibliothek WS2812FX in Version 1.0.6              - steuert die LED-Stripes
- Bibliothek Adafruit_NeoPixel in Version 1.1.6     - wird von der Lib WS2812b benötigt
- Bibliothek Encoder in Version 1.4.1               - wird zum Auslesen des Drehdrückstellers benötigt
- Bibliothek ESP8266WiFi in Version 1.0             - wird zum Aufbau der WLAN Verbindung benötigt
- Bibliothek DallasTemperature in Version 3.8.0     - wird zum Auslesen der Temperatursensoren benötigt
- Bibliothek ESP8266WebServer in Version 1.0        - wird für die Fernsteuerung mittels HTTP-Requests benötigt.
- Bibliothek ESP8266HTTPUpdateServer in Version 1.0 - (angepasst - Callbacks hinzu) wird für Remote Update über WLAN benötigt

# Manuell entfernte Bibliotheken:
Bibliothek MAX31850_OneWire in Version 1.0.1

# Angepasste Bibliotheken:
ESP8266HTTPUpdateServer: Die Bibliothek wurde angepasst. Es wurden benutzerdefinierte Callbackfunktionen
hinzu gefügt, die aufgerufen werden, wenn die Update-Webseite geladen wird und wenn ein Update-Upload beginnt.  
- THandlerFunction onWebpage
- THandlerFunction onUpload
Wenn der benutzerdefinierte Callback onWebpage false zurückgibt, wird kein Software-Update zugelassen.
Die angepasste Bibliothek liegt in /home/[username]/.arduino15/packages/esp8266/hardware/esp8266/2.3.0/libraries/ESP8266HTTPUpdateServer

Sie ist gezippt zusätzlich im Projektordner abgelegt.
Encoder: /home/[username]/Arduino/libraries/Encoder/encoder.h:
- Zeile static void update(Encoder_internal_state_t *arg) {
- geändert zu: static void ICACHE_RAM_ATTR update(Encoder_internal_state_t *arg) {
- nach #include "pins_arduino.h" #endif
- einfügen: #ifndef ICACHE_FLASH #define ICACHE_RAM_ATTR #endif 

# Map file erstellen
ggf zuerst avr binutils installieren: sudo apt-get install binutils-avr
Dann: frank@frank:~$ avr-objdump -t /tmp/arduino_build_238121/wemos.ino.elf  > /tmp/arduino_build_238121/wemos.ino.map

# Crashdaten auswerten
http://http://192.168.1.44/INFO  --> epc1 enthält die Absturzadresse - bspw. 0x40202c08
dann mapfile öffnen und nach 40202 suchen, um die Funktion zu finden, in der der Absturz passiert ist
Wenn ab Adresse 40202 viele Funktionen zu finden sind, kann die Suche eingegrenzt werden:
- 40202b
- 40202a
- usw
Die Adresse der gesuchten Funktion muss kleiner als die Absturzadresse sein. 
Zur Kontrolle, ob die gefundene Funktion die richtige ist:
Zeile in map file: 40202be4 g     F .irom0.text  00000188 _Z18onHttpGet_Settingsv
                        ^                              ^
                        |                              |
                   Funktionsanfang              Funktionslänge
Es wurde die korrekte Funktion gefunden, wenn gilt: (epc1 > Funktionsanfang) && (epc1 < (Funktionsanfang+Funktionslänge)