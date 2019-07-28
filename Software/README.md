# Funktions-Beschreibung:
Diese Steuerung ist dazu da, um am Bett zwei LED Stripes Typ WS2812b zu steuern.
- Stripe 1: Leselicht (an, aus, Helligkeit, nur weißes Licht)
- Stripe 2: Wandlicht (an, aus, Helligkeit, Farb-Effekte)

Die Stripes werden mittels Drehdrücksteller gesteuert. Die Steuerung ist in einer 
Statemachine (Variable sysState, siehe Statemachine.ino) zusammengefasst. 
Über WLAN kann die Beleuchtung ferngesteuert werden. Die Fernsteuerung läuft über HTTP-Requests.
Folgende Funktionen sind fernsteuerbar:

- Licht An, Aus (Beispiel: http://192.168.1.44/CMD?LED=1)
- Fernsehsimulation An, Aus (Beispiel http://192.168.1.44/CMD?FAKETV=1)

Die entsprechenden http-Kommandos müssen zyklisch gesendet werden, sonst wird der Fernsteuermodus wieder 
verlassen und alle LEDs werden ausgeschaltet.

Es gibt zwei 1wire Sensoren und einen Lichtsensor:
- Temperatur im Steuergerät nahe Netzteil
- Temperatur Raum
- Helligkeit im Raum

Alle Sensoren können per http abgefragt werden:

- Temperatur im Steuergerät nahe Netzteil: http://192.168.1.44/ADC0
- Temperatur Raum: http://192.168.1.44/ADC1
- Helligkeit im Raum: http://192.168.1.44/ADC2

Wenn ein Lichtsensor (LDR) angeschlossen ist, aktiviert sich die Beleuchtung automatisch für eine bestimmte Zeit (ca. 15s), 
wenn es im Zimmer dunkel wird. 
 
# Setup
Alle Einstellungen können per Web-Browser vorgenommen werden. Folgende Einstellungen sind möglich:
- Name des Access Points (SSID) 
- WLAN-Passwort
- Subnetzmaske
![lt](https://github.com/tinytronix/Leselicht/blob/master/Photos/WifiSettings.jpg)

- Anzahl der LEDs auf Stripe 1
- Anzahl der LEDs auf Stripe 2
- Helligkeitsschwelle Raumlicht und Gradient
- Festlegung, welcher 1wire Sensor auf ADC0/ADC1 ausgegeben wird

![lt](https://github.com/tinytronix/Leselicht/blob/master/Photos/Settings.jpg)
Im Initialzustand (Default) wird die Steuerung im Accesspoint-Modus betrieben.
- SSID: ESP8266
- Passwort: ESP8266
- IP: 192.168.1.2

Alle Einstellungen können jederzeit geändert werden. Der  Accesspoint-Modus kann jederzeit aktiviert werden, indem der Drehdrücksteller gedrückt gehalten wird während die Spannungsversorgung eingeschaltet wird.
Dieser Betriebsmodus wird durch ein blaues Lauflicht (Komet) signalisiert. Er kann auf zwei Arten beendet werden:
1. mit dem AccessPoint ESP8266 verbinden und dann entweder die Webseite http://192.168.1.2 aufrufen, dann dort zu den Wifi Einstellungen und SSID, IP, Passwort etc eingeben und OK drücken.
2. oder: Ohne weitere Einstellungen vorzunehmen einfach den Drehdrücksteller kurz drücken. 
Dann startet die Steuerung ebenfalls neu und leuchtet dabei für 1s grün. 
Das WLAN ist nun vollständig deaktiviert, kann aber später erneut aktiviert werden. 

# Software-Update
Die Updatefunktion wird aktiviert, indem die Upload-Webseite aufgerufen wird.
Beispiel: http://192.168.1.44/update

# Eeprom:
Im Eeprom werden Netzwerk-Connectivity Daten gespeichert:
- Bettname
- die IP Adresse 
- Gateway
- Subnetzmaske
- Wifi SSID
- Wifi Passwort
- Anzahl der LEDs auf Stripe 1
- Anzahl der LEDs auf Stripe 2

Das Eeprom Daten werden über den Web-Browser eingegeben.

# Default-Einstellungen
Die Default-Einstellungen sind:
- Bettname = ""
- IP Adresse = 192.168.1.2
- Gateway = 192.168.1.2
- Subnetzmaske = 255.255.255.0
- Wifi SSID = ""
- Wifi Passwort = ""
- Flags: Wifi An; AP-Modus

Die Default-Einstellungen können zu jedem Zeitpunkt wieder hergestellt werden, indem die Steuerung vom Stromnetz getrennt wird und dann bei gedrückt gehaltenem Drehdrück-Steller wieder mit dem Stromnetz verbunden wird. Wenn die Steuerung die Default-Einstellungen wieder hergestellt hat, blinkt das Wandlicht für 2s rot. Dann startet die Steuerung neu und befindet sich im Initialzustand. In diesem Zustand ist der Wifi Accesspoint-Modus aktiv (angezeigt durch blaues Lauflicht). Näheres zu WLAN-Einstellungen siehe Kapitel Setup.

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
http://192.168.1.44/INFO  --> epc1 enthält die Absturzadresse - bspw. 0x40202c08
dann mapfile öffnen und nach 40202 suchen, um die Funktion zu finden, in der der Absturz passiert ist
Wenn ab Adresse 40202 viele Funktionen zu finden sind, kann die Suche eingegrenzt werden:
- 40202b
- 40202a

...usw. Die Adresse der gesuchten Funktion muss kleiner als die Absturzadresse sein. 
Zur Kontrolle, ob die gefundene Funktion die richtige ist:
Zeile in map file: 40202be4 g     F .irom0.text  00000188 _Z18onHttpGet_Settingsv
<br>
40202be4: Funktionsanfang <br>
00000188: Funktionslänge <br>

Es wurde die korrekte Funktion gefunden, wenn gilt: (epc1 > Funktionsanfang) && (epc1 < (Funktionsanfang+Funktionslänge)
