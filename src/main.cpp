/*  Code von M3S7t
    Dient der Abfrage von Temperatur-Sensoren DS18B20 für eine Kondensator-Temperatur-Überwachung
    Abgefragt werden die Sensoren der einzelenen Kondensatoren, in Summe 68 Stück.
    Genutzt wurden die Libarys "paulstoffregen/OneWire@^2.3.8" "milesburton/DallasTemperature @ ^3.11.0"
    Als Microcontroller soll ein Arduino Mega 2560 verwendet. 
    (Optional wäre auch ein Uno möglich)
    Bei Zugriff via Seriel-Com-Port >> Flow-Control auf DSR/DTR stellen
    um ein Reset des Programms, zur Ermittlung des auslösenden Sensors, zu verhinden
    
    Aufbau der Kompensationstation Siemens AN2.3MW
    ____________________________________
    | C1.1 | C1.2 | C1.3 | C1.4 | C1.5 |   Stufe 01 (Klein)
    ____________________________________
    ____________________________________
    | C2.6 | C2.7 | C2.8 | C2.9 |          Stufe 02 (Standard)
    | C2.1 | C2.2 | C2.3 | C2.4 | C2.5 |
    ____________________________________

    .
    .
    .
    ____________________________________
    | C8.6 | C8.7 | C8.8 | C8.9 |          Stufe 08 (Standard)
    | C8.1 | C8.2 | C8.3 | C8.4 | C8.5 |
    ____________________________________

*/

#include <OneWire.h>
#include <DallasTemperature.h>

int inputBus[] = {2, 3, 4, 5, 6, 7, 8, 9};                                  // Input Pins der einzelnen Stufen
const uint8_t AnzahlInputStufen = sizeof(inputBus) / sizeof(inputBus[0]);   // autoamtisiserte Ermittlung der Sensoreingänge 
const uint8_t AnzahlSensorenStufeKlein  = 5;                                // Anzahl Sensoren Stufe 01
const uint8_t AnzahlSensorenStufe       = 6;                                // Anzahl Sensoren Stufe 02 - 08
uint8_t diffSensoren = 0;                                                   // Variable zm halten der Anzahl der Sensorabweichung Stufe 1 vs. 2 - 8                                            
const uint8_t relaisOutput = 10;                                        
float temperaturSchwelle = 21;                                              // Temperaturschwelle zur Auslösung
bool schwelleUeberschritten = false;        
uint8_t angesprocheneStufe = 0;       
uint8_t angesprochenerSensor = 0;
float angesprocheneTemp = 0;
bool runCheckTemperatur = true;

String matrixAdressen[AnzahlInputStufen][AnzahlSensorenStufe];              // Matrix für Sensoradressen
float temperatureMatrix[AnzahlInputStufen][AnzahlSensorenStufe];            // Matrix für aktuelle Sensor-Temperaturen
float lastTemperatureMatrix[AnzahlInputStufen][AnzahlSensorenStufe];        // Matrix zur Sensorkontrolle
// int adressenKonvert[8];

OneWire oneWireBus[AnzahlInputStufen];
DallasTemperature sensor[AnzahlInputStufen];

//-------------------------------------------------------------------------------------------------
// Funktionen

void sucheSensorAdresse() {
  Serial.println();
  Serial.println("...Suche nach Temperatur-Sensoren...");
  Serial.println();

  // Durchlauf durch die einzelnen Stufen
  for (uint8_t i = 0; i < AnzahlInputStufen; i++) {
    Serial.print("Adressen der Sensoren Stufe ");
    Serial.print(i + 1);
    Serial.println(":");

  // Überprüfung ob Stufe 1 klein abgefragt wird >> nur 5x Sensoren
  if (i == 0) diffSensoren = AnzahlSensorenStufeKlein;
  else diffSensoren = AnzahlSensorenStufe;

    for (uint8_t j = 0; j < diffSensoren; j++) {
      // Adresse für jeden Sensor abrufen
      DeviceAddress sensorAdresse;
     sensor[i].getAddress(sensorAdresse, j);

      // Ausgabe der Adresse
      Serial.print("Sensor ");
      Serial.print(j + 1);
      Serial.print(" Adresse: ");

        for (int k = 0; k < 8; k++) {
          if (sensorAdresse[k] < 16) Serial.print("0");
          Serial.print(sensorAdresse[k], HEX);
          if (sensorAdresse[k] < 16) matrixAdressen[i][j] += String("0");
          matrixAdressen[i][j] += String(sensorAdresse[k],HEX); 
        }
      Serial.println();
    }
  }
}


void abfrageTemperaturen() {
for (int i = 0; i < AnzahlInputStufen; i++) {
    sensor[i].requestTemperatures();  // Alle Sensoren Temperaturmessung starten

// Überprüfung ob Stufe 1 (Klein) abgefragt wird >> nur 5x Sensoren
if (i == 0) diffSensoren = AnzahlSensorenStufeKlein;
else diffSensoren = AnzahlSensorenStufe;      

    for (int j = 0; j < diffSensoren; j++) {
      // Temperatur für jeden Sensor abfragen und in die Matrix speichern
      temperatureMatrix[i][j] = sensor[i].getTempCByIndex(j);
    }
  }
}

void serialPrint() {
  Serial.println();
  Serial.print(" ---Die Temperaturschwelle zur Ausloesung liegt bei: ");
  Serial.print(temperaturSchwelle);
  Serial.println(" °C--- ");
  Serial.println();
  Serial.print("Die aktuell gemessenen Temperaturen sind:");

  for (int i = 0; i < AnzahlInputStufen; i++) {
    Serial.println();
    Serial.print("Stufe ");
    Serial.print(i + 1);
    Serial.print(" >> ");

  // Überprüfung ob Stufe 1 (Klein) abgefragt wird >> nur 5x Sensoren
  if (i == 0) diffSensoren = AnzahlSensorenStufeKlein;
  else diffSensoren = AnzahlSensorenStufe;

    for (int j = 0; j < diffSensoren; j++) {
      Serial.print("Sensor ");
      Serial.print(j + 1);
      Serial.print(": ");
      Serial.print(temperatureMatrix[i][j]);
      Serial.print("°C | ");
    }
  }
  if (schwelleUeberschritten) {
    Serial.println();
    Serial.println();
    Serial.print("!!! An Stufe [");
    Serial.print(angesprocheneStufe);
    Serial.print("] hat Sensor [");
    Serial.print(angesprochenerSensor);
    Serial.print("] mit der Adresse : ");
    Serial.print(matrixAdressen[angesprocheneStufe - 1][angesprochenerSensor - 1]);
    Serial.print(" bei ");
    Serial.print(angesprocheneTemp);
    Serial.print("°C angesprochen !!!");
    Serial.println();
  }
}

void checkTemperatur() {
  // Durchlauf durch die einzelnen Stufen
  for (uint8_t i = 0; i < AnzahlInputStufen; i++) {

    // Überprüfung ob Stufe 1 klein abgefragt wird >> nur 5x Sensoren
    if (i == 0) diffSensoren = AnzahlSensorenStufeKlein;
    else diffSensoren = AnzahlSensorenStufe;

    for (uint8_t j = 0; j < diffSensoren; j++) {
      if ( temperatureMatrix[i][j] > temperaturSchwelle ) {
        schwelleUeberschritten = true;
        if (runCheckTemperatur)
        angesprocheneStufe = i + 1;
        angesprochenerSensor = j + 1;
        angesprocheneTemp = temperatureMatrix[i][j];
        runCheckTemperatur = false;                     // Abschalten der checkTemperatur-Funktion um den Auslösewerte zu behalten.
      }
    }
  }
}

void checkAusloesungRelais() {

      if ( schwelleUeberschritten ) {
        digitalWrite(relaisOutput, HIGH);
      }
      else {
        digitalWrite(relaisOutput, LOW);    
      }

  // - Stufe und Sensor speichern und ausgeben
  // - Auslöse-LED

}

/*
void ueberprüfungSensoren() {

  - Anzahl Sensoren je Stufe = 5x / 9x
  - Adressen vergleichen >> jede Adresse !=
  - Temperaturen vergleichen >> in zweites Array (current / last) kopieren
  - Abgleich alle 10min ob Temperatur-Änderung je Sensor erfolgt
    >> Wenn 1x oder mehrer Sensoren den gleichen Wert aufweisen, Variable (uint32_t nutzen) erhöhen
    >> Warnmeldung ab gewisser Schwelle auslösen / Stufe und Sensor speichern und ausgeben. Warn-LED?

}
*/

//-------------------------------------------------------------------------------------------------
// Ende Funktionen

// setup wuird nur beim Programm-Start ausgeführt
void setup() {
  if (runCheckTemperatur) pinMode(relaisOutput, OUTPUT);
  Serial.begin(9600);

  // Jeden OneWire-Bus und die zugehörigen DallasTemperature-Sensor initialisieren
  for (int i = 0; i < AnzahlInputStufen; i++) {
    oneWireBus[i] = OneWire(inputBus[i]);
    sensor[i] = DallasTemperature(&oneWireBus[i]);
    sensor[i].begin();
  }
  
  // Adressenbestimmung der Sensoren
  sucheSensorAdresse();
}

// Programm in Schleife
void loop() {

  // Temperaturen abfragen und in die Matrix speichern
  abfrageTemperaturen();

  // Abgleich der gemessen Temperaturen mit festgelegter Temperaturschwelle wenn noch nicht ausgelöst
  if ( runCheckTemperatur ) {
  checkTemperatur();
  }
  // Check wenn Schwelle überschritten Ansteuerung Relaisausgang
  checkAusloesungRelais();

  // Temperaturen ausgeben
  serialPrint();
  
  delay(3000); // Pause von x Sekunden (von 1 Sekunde nach oben angepasst werden)
}