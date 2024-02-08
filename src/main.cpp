/*  Code von M3S7t
    zur Abfrage von Temperatur-Sensoren DS18B20 für eine Kondensator-Temperatur-Überwachung
    Abgefragt werden die Sensoren der einzelenen Kondensatoren, in Summe 68 Stück.
    Genutzt wurden die Libarys "paulstoffregen/OneWire@^2.3.8" "milesburton/DallasTemperature @ ^3.11.0"
    Als Microcontroller soll ein Arduino Mega 2560 verwendet. 
    (Optional wäre auch ein Uno möglich)
    
    Aufbau Kompensation
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
const uint8_t AnzahlInputBus = sizeof(inputBus) / sizeof(inputBus[0]);
const uint8_t AnzahlSensorenStufeKlein  = 5;                                // Anzahl Sensoren Stufe 01
const uint8_t AnzahlSensorenStufe       = 6;                                // Anzahl Sensoren Stufe 02 - 08
uint8_t differenzStufen = 0;

String matrixAdressen[AnzahlInputBus][AnzahlSensorenStufe];
float temperatureMatrix[AnzahlInputBus][AnzahlSensorenStufe];
float lastTemperatureMatrix[AnzahlInputBus][AnzahlSensorenStufe];
int adressenKonvert[8];

OneWire oneWireBus[AnzahlInputBus];
DallasTemperature sensor[AnzahlInputBus];

//-------------------------------------------------------------------------------------------------
// Funktionen

void sucheSensorAdresse() {
  Serial.println();
  Serial.println("...Suche nach Temperatur-Sensoren...");
  Serial.println();

  // Durchlauf durch die einzelnen Stufen
  for (uint8_t i = 0; i < AnzahlInputBus; i++) {
    Serial.print("Adressen der Sensoren Stufe ");
    Serial.print(i + 1);
    Serial.println(":");

  // Überprüfung ob Stufe 1 klein abgefragt wird >> nur 5x Sensoren
  if (i == 0) differenzStufen = AnzahlSensorenStufeKlein;
  else differenzStufen = AnzahlSensorenStufe;    
;

    for (uint8_t j = 0; j < differenzStufen; j++) {
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

/*
void ausloesungRelais() {

  - Output Relais definieren 
  - Stufe und Sensor speichern und ausgeben
  - Auslöse-LED

}
*/


void abfrageTemperaturen() {
for (int i = 0; i < AnzahlInputBus; i++) {
    sensor[i].requestTemperatures();  // Alle Sensoren Temperaturmessung starten

// Überprüfung ob Stufe 1 (Klein) abgefragt wird >> nur 5x Sensoren
if (i == 0) differenzStufen = AnzahlSensorenStufeKlein;
else differenzStufen = AnzahlSensorenStufe;      

    for (int j = 0; j < differenzStufen; j++) {
      // Temperatur für jeden Sensor abfragen und in die Matrix speichern
      temperatureMatrix[i][j] = sensor[i].getTempCByIndex(j);
    }
  }
}

void serielprintTemperatur() {
  Serial.println();
  Serial.println("Gemessene Temperaturen:");
  Serial.println();

  for (int i = 0; i < AnzahlInputBus; i++) {
    Serial.print("Stufe ");
    Serial.print(i + 1);
    Serial.println(":");

  // Überprüfung ob Stufe 1 (Klein) abgefragt wird >> nur 5x Sensoren
  if (i == 0) differenzStufen = AnzahlSensorenStufeKlein;
  else differenzStufen = AnzahlSensorenStufe;

    for (int j = 0; j < differenzStufen; j++) {
      Serial.print("Sensor ");
      Serial.print(j + 1);
      Serial.print(": ");
      Serial.print(temperatureMatrix[i][j]);
      Serial.println(" °C");
    }
  }
}

//-------------------------------------------------------------------------------------------------


void setup() {
  Serial.begin(9600);

  // Jeden OneWire-Bus und die zugehörigen DallasTemperature-Sensor initialisieren
  for (int i = 0; i < AnzahlInputBus; i++) {
    oneWireBus[i] = OneWire(inputBus[i]);
    sensor[i] = DallasTemperature(&oneWireBus[i]);
    sensor[i].begin();
  }

  // start der Funktion zur Adressenbestimmung
  sucheSensorAdresse();

/*  Test-Abfrage
    Serial.println();
    Serial.println(matrixAdressen[0][0]);
    Serial.println(matrixAdressen[1][1]);
    Serial.println(matrixAdressen[7][5]);
*/
}

void loop() {

  // Temperaturen abfragen und in die Matrix speichern
  abfrageTemperaturen();
  // Temperaturen ausgeben
  serielprintTemperatur();
  
  delay(5000); // Pause von 1 Sekunde (oder passen Sie die Verzögerung an, wie es für Ihre Anwendung geeignet ist)

}