#include <OneWire.h>
#include <DallasTemperature.h>

int inputPins[] = {2, 3, 4, 5, 6, 7, 8, 9};    // Input Pins der einzelnen ANZAHL_Stufen
const uint8_t anzahlInputPins = sizeof(inputPins) / sizeof(inputPins[0]);
const uint8_t AnzahlSensorenStufe       = 6;              // Anzahl Sensoren Testaufbau
const uint8_t AnzahlSensorenStufeKlein  = 5;    // Anzahl Sensoren Stufe 1
uint8_t differenzStufen = 0;

String matrixAdressen[anzahlInputPins][AnzahlSensorenStufe];
float temperatureMatrix[anzahlInputPins][AnzahlSensorenStufe];
int adressenKonvert[8];

OneWire oneWireBus[anzahlInputPins];
DallasTemperature sensor[anzahlInputPins];

//-------------------------------------------------------------------------------------------------
// Funktionen

void sucheSensorAdresse() {
  Serial.println();
  Serial.println("...Suche nach Temperatur-Sensoren...");
  Serial.println();

  // Durchlauf durch die einzelnen Stufen
  for (uint8_t i = 0; i < anzahlInputPins; i++) {
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

  - 

}
*/


void abfrageTemperaturen() {
for (int i = 0; i < anzahlInputPins; i++) {
    sensor[i].requestTemperatures();  // Alle Sensoren Temperaturmessung starten

// Überprüfung ob Stufe 1 klein abgefragt wird >> nur 5x Sensoren
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

  for (int i = 0; i < anzahlInputPins; i++) {
    Serial.print("Stufe ");
    Serial.print(i + 1);
    Serial.println(":");

// Überprüfung ob Stufe 1 klein abgefragt wird >> nur 5x Sensoren
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
  for (int i = 0; i < anzahlInputPins; i++) {
    oneWireBus[i] = OneWire(inputPins[i]);
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
  // Beispiel: Temperaturen ausgeben
  serielprintTemperatur();
  
  delay(5000); // Pause von 1 Sekunde (oder passen Sie die Verzögerung an, wie es für Ihre Anwendung geeignet ist)

}