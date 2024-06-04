#define tmp1 A0
#define tmp2 A1
#define tmp3 A2
#define tmp4 A3
#define Vin A4
#define MVin A5
float VinRef = 5.873;
char Name[3] = "02";

String cmdString;
String datenString;
String adresse = ""; //Adresse des Controllers
String par1 = ""; //zusäzliche Parameter
String par2 = ""; //zusäzliche Parameter
int int_tmp = 0;
float MVinRef = 11.270;

bool WhMessung = false;

int sensorValue1 = 0;
int sensorValue2 = 0;
int sensorValue3 = 0;
int sensorValue4 = 0;
int sensorVin = 0;
float voltage = 0.0;
float aref = 1.085;
int Mittelwerte = 10;
float WiderstandLast = 0.24;
unsigned long zeitstempel;
int MosfetPin = 9;
bool WhmessungIn = true;
float ZeitS = 0.0;
float Vmin = 2.450;

void setup() {
  analogReference(INTERNAL);
  pinMode(MosfetPin, OUTPUT);
  digitalWrite(MosfetPin, LOW);
  delay(100);
  pinMode(tmp1, INPUT);
  pinMode(tmp2, INPUT);
  pinMode(tmp3, INPUT);
  pinMode(tmp4, INPUT);
  pinMode(Vin, INPUT);
  pinMode(MVin, INPUT);
  Serial.begin(9600);
  for (int i = 0; i < Mittelwerte; i++) {
    analogRead(tmp1);
    analogRead(tmp2);
    analogRead(tmp3);
    analogRead(tmp4);
    analogRead(Vin);
    analogRead(MVin);
  }
  cmdString.reserve(200);
  Serial.setTimeout(50);
}
//Strommessung
bool Whmessen() {
  if (WhmessungIn) { //führt die Aktion nur einmal aus
    /* Serial.println("");
      Serial.println("");
      Serial.println("Watt Stunden Messung beginnt");
      Serial.println("");
      Serial.println("Ist die Uptime kurz vor 50 Tagen kann es zu einer Fehlrechung kommen aufgrund einen Speicherüberlaufs");
      Serial.println("In desen Fall das System einfach rebooten");
      Serial.print("Die VIN Spannung ist: ");
      Serial.print(getVin(), 3);
      Serial.println(" V");*/
    if (getVin() <= Vmin) { // Messung kann nicht durchgeführt werden. Spannung zu gering
      return false;
    }
    setMosfetOn();
    WhmessungIn = false;
  } else {
    zeitstempel = millis();
    ZeitS = zeitstempel;
    Serial.print(Name);
    Serial.print(",");
    Serial.print(ZeitS, 3);
    Serial.print(",");
    Serial.print(getalast(), 3);
    Serial.print(",");
    Serial.println(getVin(), 3);
    if (getVin() <= Vmin) {
      setMosfetOff();
      WhmessungIn = true;
      WhMessung = false;
      Serial.println("");
      Serial.println("Messung beendet");
    }
  }
  return true;
}
////////////

void setMosfetOn() {
  digitalWrite(MosfetPin, HIGH);
}

void setMosfetOff() {
  digitalWrite(MosfetPin, LOW);
}

float getalast() {
  return (getVin() - getMvin()) / WiderstandLast; //Ermittlung des Spannungsabfalls am Lastwiderstand
}

float getVin() {
  return analogRead(Vin) * (aref / 1023.0) * VinRef;//Ermittlung der Gesamtstammung
}
float getMvin() {
  return analogRead(MVin) * (aref / 1023.0) * MVinRef; //Ermittlung des Spannungsabfalls am Mosfet
}


void OutTmp(int i, int _sensorValue) {
  _sensorValue = _sensorValue / Mittelwerte;
  voltage = _sensorValue * (aref / 1023.0);
  Serial.print(String(Name)+",Tmp"+String(i)+" Spannungswert "+String(_sensorValue, 3) +" V");
  Serial.flush();
  
}
void loop() {
  if (WhMessung) {
    Whmessen();
  }
}

void serialEvent() {
  datenString = Serial.readString();
  cmdString = datenString;
  int_tmp = cmdString.indexOf(",");
  adresse = cmdString.substring(0, int_tmp); // Speichern des Adresse
  cmdString = cmdString.substring(int_tmp + 1);
  int_tmp = cmdString.indexOf(",");
  par1 = cmdString.substring(0, int_tmp); // Speichern der zusäzlichen Parameter
  par2 = cmdString.substring(int_tmp + 1);
  if (adresse == Name) {
    if (par1 == "on") {
      par1 = "";
      Serial.print(String(Name)+",MosfetPin on");
      Serial.flush();
      setMosfetOn();
    }
    if (par1 == "off") {
      par1 = "";
      Serial.print(String(Name)+",MosfetPin off");
      Serial.flush();
      setMosfetOff();
    }
    if (par1 == "tmp") {
      par1 = "";
      //Temperaturmessung
      sensorValue1 = 0;
      sensorValue2 = 0;
      sensorValue3 = 0;
      sensorValue4 = 0;
      for (int i = 0; i < Mittelwerte; i++) {
        sensorValue1 = sensorValue1 + analogRead(tmp1);
        sensorValue2 = sensorValue2 + analogRead(tmp2);
        sensorValue3 = sensorValue3 + analogRead(tmp3);
        sensorValue4 = sensorValue4 + analogRead(tmp4);
      }
      OutTmp(1, sensorValue1);
      OutTmp(2, sensorValue2);
      OutTmp(3, sensorValue3);
      OutTmp(4, sensorValue4);
    }
    if (par1 == "vin") {
      par1 = "";
      Serial.print(String(Name)+",Spannung Vin "+String(getVin(), 3) +" V");
      Serial.flush();
    }
    if (par1 == "mvin") {
      par1 = "";
      Serial.print(String(Name)+",Spannung Mosfet Vin "+String(getMvin(), 3) +" V");
      Serial.flush();
    }
    if (par1 == "alast") {
      par1 = "";
      Serial.print(String(Name)+",Strom am Lastwiderstand "+String(getalast(), 3) +" A");
      Serial.flush();
    }
    if (par1 == "WhMessung") {
      par1 = "";
      WhMessung = true;
    }
    if (par1 == "STOP") {
      par1 = "";
      WhMessung = false;
      WhmessungIn = true;
      setMosfetOff();
    }
  } else {
      Serial.print(datenString);
      Serial.flush();
  }
}
