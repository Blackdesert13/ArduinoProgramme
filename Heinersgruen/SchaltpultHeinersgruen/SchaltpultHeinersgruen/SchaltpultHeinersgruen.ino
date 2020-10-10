/*
Name: SchaltpultHeinersgruen.ino
Author: Robert Muh
*/

#include <Servo.h>
#include <Wire.h>
const byte ArdNr = 1;

#define ausg_Schieberegler A0

#define ausg_5201 0
#define ausg_5202 1
#define ausg_5203 2
#define ausg_5204 3
#define ausg_5205 4
#define ausg_5207 5
#define ausg_5206 6
#define ausg_5208 7
#define ausg_5209 8
#define ausg_5210 9
#define ausg_5211 10
#define ausg_5212 11
#define ausg_5213 12
#define ausg_5251 13
#define ausg_5252 14
#define ausg_5253 15
#define ausg_5254 16
#define ausg_5255 17
#define ausg_5256 18
#define ausg_5257 19
#define ausg_5258 20
#define ausg_5260 21
#define ausg_5259 22
#define ausg_5261 23
#define ausg_5301 24
#define ausg_5302 25
#define ausg_5303 26
#define ausg_5304 27
#define ausg_5305 28
#define ausg_5306 29
#define ausg_5307 30
#define ausg_5308 31
#define ausg_5309 32
#define ausg_5310 33
#define ausg_5311 34
#define ausg_5312 35
#define ausg_5313 36
#define ausg_5314 37
#define ausg_5315 38
#define servoAusg_5901 0
#define servoAusg_5902 1
#define servoAusg_5361 2
#define servoAusg_5362 3
#define servoAusg_5363 4
#define servoAusg_5364 5
#define servoAusg_5365 6
#define servoAusg_5366 7
#define servoAusg_5367 8
#define servoAusg_5368 9
#define servoAusg_5369 10
#define servoAusg_5370 11
#define servoAusg_5371 12
#define servoAusg_5372 13
#define servoAusg_5373 14
#define servoAusg_5374 15
#define servoAusg_5375 16

#define eing_5201 0
#define eing_5203 1
#define eing_5205_5207 2
#define eing_5209_5208 3
#define eing_5211 4
#define eing_5301 5
#define eing_5303 6
#define eing_5305 7
#define eing_5307 8
#define eing_5309 9
#define eing_5311 10
#define eing_5313 11
#define eing_5315 12
#define eing_5901 A1
#define eing_5251 14
#define eing_5253 15
#define eing_5255 16
#define eing_5257_5258 17
#define eing_5361 18
#define eing_5363 19
#define eing_5365 20
#define eing_5367 21
#define eing_5369 22
#define eing_5371 23
#define eing_5373 24
#define eing_5375 25
#define eing_5202 0
#define eing_5204 1
#define eing_5206 2
#define eing_5210 3
#define eing_5212_5213 4
#define eing_5302 5
#define eing_5304 6
#define eing_5306 7
#define eing_5308 8
#define eing_5310 9
#define eing_5312 10
#define eing_5314 11
#define eing_5902 A2
#define eing_5252 13
#define eing_5254 14
#define eing_5256 15
#define eing_5260_5259 16
#define eing_5362 17
#define eing_5364 18
#define eing_5366 19
#define eing_5368 20
#define eing_5370 21
#define eing_5372 22
#define eing_5374 23
#define eing_5261 24

#define eingGruppe_5201 0
#define eingGruppe_5203 0
#define eingGruppe_5205_5207 0
#define eingGruppe_5209_5208 0
#define eingGruppe_5211 0
#define eingGruppe_5301 0
#define eingGruppe_5303 0
#define eingGruppe_5305 0
#define eingGruppe_5307 0
#define eingGruppe_5309 0
#define eingGruppe_5311 0
#define eingGruppe_5313 0
#define eingGruppe_5315 0
#define eingGruppe_5901 0
#define eingGruppe_5251 0
#define eingGruppe_5253 0
#define eingGruppe_5255 0
#define eingGruppe_5257_5258 0
#define eingGruppe_5361 0
#define eingGruppe_5363 0
#define eingGruppe_5365 0
#define eingGruppe_5367 0
#define eingGruppe_5369 0
#define eingGruppe_5371 0
#define eingGruppe_5373 0
#define eingGruppe_5375 0
#define eingGruppe_5202 1
#define eingGruppe_5204 1
#define eingGruppe_5206 1
#define eingGruppe_5210 1
#define eingGruppe_5212_5213 1
#define eingGruppe_5302 1
#define eingGruppe_5304 1
#define eingGruppe_5306 1
#define eingGruppe_5308 1
#define eingGruppe_5310 1
#define eingGruppe_5312 1
#define eingGruppe_5314 1
#define eingGruppe_5902 1
#define eingGruppe_5252 1
#define eingGruppe_5254 1
#define eingGruppe_5256 1
#define eingGruppe_5260_5259 1
#define eingGruppe_5362 1
#define eingGruppe_5364 1
#define eingGruppe_5366 1
#define eingGruppe_5368 1
#define eingGruppe_5370 1
#define eingGruppe_5372 1
#define eingGruppe_5374 1
#define eingGruppe_5261 1


#define anzEingangsGruppen 2 // Anzahl Eingänge
const byte anzEingaenge[] = { 26, 25 };
#define anzAusgaenge 39 // Anzahl Ausgänge
#define anzServoAusgaenge 17 // Anzahl ServoAusgänge
#define anzPlatinen 6 //Anzahl Platinen

#define ZeitAbfrage(Zeit) (millis() > (unsigned long)Zeit)
#define BefehlsPruefung(Befehl) Befehl[4] == (byte)(Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3])



struct Eingang //für alle Arten von Schaltern, Reedkontakten, Lichtschranken
{ //L-aktiv
	byte eingang = 255;//über 53 ist beim Mega inaktiv
	bool stellung = false;
	//zum Entprellen, zur Vermeidung von mehrfach Schalten
	byte leseWiederholung = 0;
};
byte leseWiederholung = 1;//so oft muss das Signal am Eingang gleich sein um die Stellung zu ändern
unsigned long debugZeit = 0;


struct Eingangsgruppe {
	Eingang * Eingaenge = NULL;

	byte Ausgang = 255;
};
Eingangsgruppe eing[anzEingangsGruppen];


struct Platine {
	unsigned short Ausgaenge;
	byte Arduino;
	byte Befehl;
	bool senden;
};
Platine platinen[anzPlatinen];

struct Ausgang {
	byte platine;
	byte ausgang = 255;
	bool invertiert = false;
	byte eingangsGruppe;
	byte eingang;
};
Ausgang ausg[anzAusgaenge];

struct ServoAusgang {
	byte ausgang;
	byte platine;
	byte eingangsGruppe;
	byte eingang;

	bool analogerEingang = false;
	byte letzterWinkel = 0;

	int winkelStellung1 = 135; //größerer Winkel
	int winkelStellung0 = 45; // kleinerer Winkel
	int stellung;
};
ServoAusgang servoAusg[anzServoAusgaenge];
byte aktiverServo = 255;

//USB-Definitionen*/
byte USBEingang[5];
byte USBEingangsByteZaehler = 0;
unsigned long zeitUSB = 0;
int intervallUSB = 500;
byte sendeArray[5];
byte letzterWinkel = 0;


void setup() {
	Wire.begin();
	Serial.begin(9600);
	for (int i = 0; i < anzEingangsGruppen; i++) {
		eing[i].Eingaenge = new Eingang[anzEingaenge[i]];
	}
	Definition();
	//delay(1000);
}

void loop() {
	USBAnzeige();
	AnlagenCheck();
	////  delay(5);
	for (int i = 0; i < anzPlatinen; i++) {

		Platine plat = platinen[i];
		if (plat.senden) {
			if (platinen[i].Arduino == ArdNr) {
				//if (platinen[i].Befehl == 41) {
				//	ByteToRelais(lowByte(platinen[i].Ausgaenge), 22);
				//	ByteToRelais(highByte(platinen[i].Ausgaenge), 30);
				//}
				//else if (platinen[i].Befehl == 42) {
				//	//Serial.write(99);
				//	ByteToRelais(lowByte(platinen[i].Ausgaenge), 38);
				//	ByteToRelais(highByte(platinen[i].Ausgaenge), 46);
				//}
				//else if (platinen[i].Befehl == 40) {
				//	ByteToRelais(lowByte(platinen[i].Ausgaenge), 2);
				//	ByteToRelais(highByte(platinen[i].Ausgaenge), 10);
				//}
				continue;
			}
			else {
				byte befehl[5];
				befehl[0] = platinen[i].Arduino;
				befehl[1] = platinen[i].Befehl;
				befehl[2] = lowByte(platinen[i].Ausgaenge);
				befehl[3] = highByte(platinen[i].Ausgaenge);
				BefehlAnSlave(befehl);
			}
		}
	}

	byte winkelProzent = ReadAnalogPin33Prozent(ausg_Schieberegler);
	//Serial.write(winkelProzent);
	if (aktiverServo < anzServoAusgaenge 
		&& servoAusg[aktiverServo].platine < anzPlatinen
	) {
		int diff = letzterWinkel - winkelProzent;
		diff = abs(diff);
		//Serial.write(diff);
		if (diff >= 2 || winkelProzent == 0 || winkelProzent == 255) {
			letzterWinkel = winkelProzent;
			//Serial.write(diff);
			//delay(1000);
			byte befehl[5];
			befehl[0] = platinen[servoAusg[aktiverServo].platine].Arduino;
			befehl[1] = 73;
			befehl[2] = (platinen[servoAusg[aktiverServo].platine].Befehl - 40) * 16 + servoAusg[aktiverServo].ausgang;
			befehl[3] = winkelProzent;
			BefehlAnSlave(befehl);
			//BefehlAnPC(befehl);
		}
	}

	//delay(5);

}

byte ReadAnalogPin33Prozent(byte analogerPin) {
	long messw = map(analogRead(analogerPin), 0, 692, 0, 255);
	byte winkelProzent = (byte)constrain(messw, 0, 255);
	return winkelProzent;
}

void AnlagenCheck() {
	EingangsCheck();//prüft alle Eingänge
	AusgangsCheck();
}

void USBAnzeige() {
	//USBEmpfang();
	if (ZeitAbfrage(zeitUSB)) {
		USBAusgangsZustand();
		zeitUSB = millis() + intervallUSB;
	}
}

void USBEmpfang() {
	while (Serial.available() > 0) {
		byte daten = 0;
		daten = Serial.read();
		if (USBEingangsByteZaehler < 5) {
			USBEingang[USBEingangsByteZaehler] = daten;
		}
		else {
			USBEingang[0] = USBEingang[1];
			USBEingang[1] = USBEingang[2];
			USBEingang[2] = USBEingang[3];
			USBEingang[3] = USBEingang[4];
			USBEingang[4] = daten;
		}

		USBEingangsByteZaehler++;
		if (USBEingangsByteZaehler > 4) {
			if (BefehlsPruefung(USBEingang)) {// BefehlsAusfuerung
				BefehlsAusfuehrung(USBEingang);
				USBEingangsByteZaehler = 0;
			}
		}

	}
}

void USBAusgangsZustand()
{ //sendet den Zustand der Relais-Ausgänge
	for (int i = 0; i < anzPlatinen; i++) {
		byte befehl[5];
		befehl[0] = platinen[i].Arduino;
		befehl[1] = platinen[i].Befehl;
		befehl[2] = lowByte(platinen[i].Ausgaenge);
		befehl[3] = highByte(platinen[i].Ausgaenge);
		BefehlAnPC(befehl);
	}
}


void BefehlAnPC(byte Befehl[5])
{ //sendet einen Befehl an den PC, Kontrollbyte wird automatisch ergänzt
	Befehl[4] = (byte)(Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3]);
	Serial.write(Befehl, 5);
}

void BefehlAnSlave(byte Befehl[5])
{
	Befehl[4] = (byte)(Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3]);
	//Serial.write(Befehl, 5);
	Wire.beginTransmission(Befehl[0]);
	Wire.write(Befehl, 5);
	Wire.endTransmission();
}

void BefehlsAusfuehrung(byte Befehl[5])
{
	// int a = 0;
	switch (Befehl[1])
	{
	case 1:
	{ //Slave abfrage einfügen
	  // SlaveAktivAbfrage();
	  //Serial.write(98);
		break;
	}
	case 2:
	{
		// SlaveAktivListe = 0;
		break;
	}
	case 9:
	{
		break;
	}
	case 40:
	{ //PermanentOutput 16Bit auf Adresse0
		for (int i = 0; i < anzPlatinen; i++) {
			if (platinen[i].Arduino == Befehl[0] && platinen[i].Befehl == 40) {
				platinen[i].Ausgaenge = ((unsigned short)(Befehl[3] << 8)) + Befehl[2];
				platinen[i].senden = true;
			}
		}
		break;
	} case 41:
	{ //PermanentOutput 16Bit auf Adresse1
		for (int i = 0; i < anzPlatinen; i++) {
			if (platinen[i].Arduino == Befehl[0] && platinen[i].Befehl == 41) {
				platinen[i].Ausgaenge = ((unsigned short)(Befehl[3] << 8)) + Befehl[2];
				platinen[i].senden = true;
			}
		}
		break;
	}
	case 42:
	{ //PermanentOutput 16Bit auf Adresse2
		for (int i = 0; i < anzPlatinen; i++) {
			if (platinen[i].Arduino == Befehl[0] && platinen[i].Befehl == 42) {
				Serial.write(42);
				platinen[i].Ausgaenge = ((unsigned short)(Befehl[3] << 8)) + Befehl[2];
				platinen[i].senden = true;
			}
		}
		break;
	}
	}
}

void ByteToRelais(byte datenByte, int startPin)
{ //von MoBS_Master
	datenByte = ~datenByte;//datenByte muß invertiert werden weil die Relaisplatinen low-aktiv sind!
	for (int i = 0; i < 8; i++) {
		digitalWrite(i + startPin, bitRead(datenByte, i));
		if (i + startPin > 51) {
			//Serial.write(bitRead(datenByte, i));
		}
	}
}

void ByteToAusgang(byte datenByte, int startNr)
{ // für Straba
	byte start = startNr;
	bool stellung = false;
	byte Nr = startNr;
	byte daten = datenByte;
	//Serial.write(Nr);
	//Serial.write(daten);
	for (int i = 0; i < 8; i++)
	{
		Nr = start + i;
		if (Nr < anzAusgaenge)
		{ //Serial.write(Nr);
			stellung = bitRead(daten, i);
			//delay(10);
			//Serial.write(stellung);
			Ausgang a = ausg[Nr];
			bool ausgStellung = (bitRead(platinen[a.platine].Ausgaenge, a.ausgang)) != 0;
			if (stellung != ausgStellung)
			{
				AusgSchalten(Nr, stellung);
			}
			//  if (ausg[Nr].stellung) { Serial.write(Nr + 200); }
			//  else{ Serial.write(Nr + 100); }
		}
	}
}


void AusgSchalten(byte Nr, bool Stellung)
{
	int nr = Nr;
	bool stellung = Stellung;
	if (nr < anzAusgaenge) {
		Ausgang a = ausg[Nr];
		bool ausgStellung = (bitRead(platinen[a.platine].Ausgaenge, a.ausgang)) != 0;
		if (stellung != ausgStellung) {
			bitWrite(platinen[a.platine].Ausgaenge, a.ausgang, stellung);
			platinen[a.platine].senden = true;
		}
	}
}

void ServoAusgSchalten(byte Nr, bool Stellung)
{
	int nr = Nr;
	bool stellung = Stellung;
	if (!stellung && aktiverServo == Nr) {
		return;
	}
	if (nr < anzServoAusgaenge) {
		if (stellung) {
			aktiverServo = nr;
		}
		ServoAusgang a = servoAusg[Nr];
		bool ausgStellung = (bitRead(platinen[a.platine].Ausgaenge, a.ausgang)) != 0;
		if (stellung != ausgStellung) {
			bitWrite(platinen[a.platine].Ausgaenge, a.ausgang, stellung);
			platinen[a.platine].senden = true;
		}
	}
}


void AusgangsCheck() {
	for (int i = 0; i < anzAusgaenge; i++) {
		bool zustand = 
			eing[ausg[i].eingangsGruppe].Eingaenge[ausg[i].eingang].stellung;
		AusgSchalten(i, ausg[i].invertiert ? !zustand : zustand);
		//Serial.write(zustand);
	}
	//Serial.write(98);
	//delay(1000);

	for (int i = 0; i < anzServoAusgaenge; i++) {
		if (servoAusg[i].analogerEingang) {
			byte winkelProzent = ReadAnalogPin33Prozent(servoAusg[i].eingang);
			//Serial.write(winkelProzent);
			if (servoAusg[i].platine < anzPlatinen) {
				int diff = servoAusg[i].letzterWinkel - winkelProzent;
				diff = abs(diff);
				//Serial.write(diff);
				if (diff >= 2 || winkelProzent == 0 || winkelProzent == 255) {
					letzterWinkel = winkelProzent;
					//Serial.write(diff);
					//delay(1000);
					byte befehl[5];
					befehl[0] = platinen[servoAusg[i].platine].Arduino;
					befehl[1] = 73;
					befehl[2] = (platinen[servoAusg[i].platine].Befehl - 40) * 16 + servoAusg[i].ausgang;
					befehl[3] = winkelProzent;
					//BefehlAnSlave(befehl);
					//BefehlAnPC(befehl);
				}
			}
		}
		else {
			bool zustand =
				eing[servoAusg[i].eingangsGruppe].Eingaenge[servoAusg[i].eingang].stellung;
			ServoAusgSchalten(i, zustand);
			//Serial.write(zustand);
		}
	}
	//Serial.write(98);
	//delay(1000);
}


void EingangsCheck()//prüft alle Eingänge
{ //LOW-aktiv
	bool zustand;
	for (int i = 0; i<anzEingangsGruppen; i++) {
		AlleEingangsGruppenAusschalten();
		if (eing[i].Ausgang < 255) {
			pinMode(eing[i].Ausgang, OUTPUT);
			digitalWrite(eing[i].Ausgang, LOW);
		}
		for (int j = 0; j < anzEingaenge[i]; j++) {
			if (eing[i].Eingaenge[j].eingang < 255) {
				zustand = digitalRead(eing[i].Eingaenge[j].eingang);
				//Serial.write(zustand);
				//der Eingang ist LOW-aktiv, die Stellung ist HIGH-aktiv
				if (zustand == eing[i].Eingaenge[j].stellung) {
					eing[i].Eingaenge[j].leseWiederholung++;
				}
				else {
					eing[i].Eingaenge[j].leseWiederholung = 0;
				}
				if (leseWiederholung <= eing[i].Eingaenge[j].leseWiederholung)
				{
					eing[i].Eingaenge[j].stellung = !zustand;
				}
				//Serial.write(eing[i].Eingaenge[j].stellung);
				continue;

			}
		}
		//Serial.write(99);
		//delay(1000);
	}
	AlleEingangsGruppenAusschalten();
}

void AlleEingangsGruppenAusschalten() {
	for (int i = 0; i < anzEingangsGruppen; i++) {
		if (eing[i].Ausgang < 255) {
			pinMode(eing[i].Ausgang, INPUT);
		}
	}
}

void Definition()
{
	eing[0].Ausgang = 18;
	eing[0].Eingaenge[0].eingang = 22;
	eing[0].Eingaenge[1].eingang = 23;
	eing[0].Eingaenge[2].eingang = 24;
	eing[0].Eingaenge[3].eingang = 25;
	eing[0].Eingaenge[4].eingang = 26;
	eing[0].Eingaenge[5].eingang = 27;
	eing[0].Eingaenge[6].eingang = 28;
	eing[0].Eingaenge[7].eingang = 29;
	eing[0].Eingaenge[8].eingang = 30;
	eing[0].Eingaenge[9].eingang = 31;
	eing[0].Eingaenge[10].eingang = 32;
	eing[0].Eingaenge[11].eingang = 33;
	eing[0].Eingaenge[12].eingang = 34;
	eing[0].Eingaenge[13].eingang = 35;
	eing[0].Eingaenge[14].eingang = 36;
	eing[0].Eingaenge[15].eingang = 37;
	eing[0].Eingaenge[16].eingang = 38;
	eing[0].Eingaenge[17].eingang = 39;
	eing[0].Eingaenge[18].eingang = 40;
	eing[0].Eingaenge[19].eingang = 41;
	eing[0].Eingaenge[20].eingang = 42;
	eing[0].Eingaenge[21].eingang = 43;
	eing[0].Eingaenge[22].eingang = 44;
	eing[0].Eingaenge[23].eingang = 45;
	eing[0].Eingaenge[24].eingang = 46;
	eing[0].Eingaenge[25].eingang = 47;
	eing[1].Ausgang = 19;
	eing[1].Eingaenge[0].eingang = 22;
	eing[1].Eingaenge[1].eingang = 23;
	eing[1].Eingaenge[2].eingang = 24;
	eing[1].Eingaenge[3].eingang = 25;
	eing[1].Eingaenge[4].eingang = 26;
	eing[1].Eingaenge[5].eingang = 27;
	eing[1].Eingaenge[6].eingang = 28;
	eing[1].Eingaenge[7].eingang = 29;
	eing[1].Eingaenge[8].eingang = 30;
	eing[1].Eingaenge[9].eingang = 31;
	eing[1].Eingaenge[10].eingang = 32;
	eing[1].Eingaenge[11].eingang = 33;
	eing[1].Eingaenge[12].eingang = 35;
	eing[1].Eingaenge[13].eingang = 36;
	eing[1].Eingaenge[14].eingang = 37;
	eing[1].Eingaenge[15].eingang = 38;
	eing[1].Eingaenge[16].eingang = 39;
	eing[1].Eingaenge[17].eingang = 40;
	eing[1].Eingaenge[18].eingang = 41;
	eing[1].Eingaenge[19].eingang = 42;
	eing[1].Eingaenge[20].eingang = 43;
	eing[1].Eingaenge[21].eingang = 44;
	eing[1].Eingaenge[22].eingang = 45;
	eing[1].Eingaenge[23].eingang = 46;
	eing[1].Eingaenge[24].eingang = 47;

	for (int i = 0; i < anzEingangsGruppen; i++) {
		for (int j = 0; j < anzEingaenge[i]; j++) {
			//delay(1);
			pinMode(eing[i].Eingaenge[j].eingang, INPUT_PULLUP);
		}
	}
	AlleEingangsGruppenAusschalten();

	platinen[0].Arduino = 2;
	platinen[0].Befehl = 40;
	platinen[0].senden = true;
	platinen[1].Arduino = 3;
	platinen[1].Befehl = 40;
	platinen[1].senden = true;
	platinen[2].Arduino = 4;
	platinen[2].Befehl = 40;
	platinen[2].senden = true;
	platinen[3].Arduino = 5;
	platinen[3].Befehl = 40;
	platinen[3].senden = true;
	platinen[4].Arduino = 6;
	platinen[4].Befehl = 40;
	platinen[4].senden = true;
	platinen[5].Arduino = 7;
	platinen[5].Befehl = 40;
	platinen[5].senden = true;

	servoAusg[servoAusg_5901].platine = 1;
	servoAusg[servoAusg_5901].ausgang = 6;
	//servoAusg[servoAusg_5901].eingangsGruppe = eingGruppe_5901;
	servoAusg[servoAusg_5901].analogerEingang = true;
	servoAusg[servoAusg_5901].eingang = eing_5901;
	servoAusg[servoAusg_5902].platine = 3;
	servoAusg[servoAusg_5902].ausgang = 11;
	//servoAusg[servoAusg_5902].eingangsGruppe = eingGruppe_5902;
	servoAusg[servoAusg_5902].analogerEingang = true;
	servoAusg[servoAusg_5902].eingang = eing_5902;
	servoAusg[servoAusg_5361].platine = 1;
	servoAusg[servoAusg_5361].ausgang = 8;
	servoAusg[servoAusg_5361].eingangsGruppe = eingGruppe_5361;
	servoAusg[servoAusg_5361].eingang = eing_5361;
	servoAusg[servoAusg_5362].platine = 1;
	servoAusg[servoAusg_5362].ausgang = 7;
	servoAusg[servoAusg_5362].eingangsGruppe = eingGruppe_5362;
	servoAusg[servoAusg_5362].eingang = eing_5362;
	servoAusg[servoAusg_5363].platine = 4;
	servoAusg[servoAusg_5363].ausgang = 4;
	servoAusg[servoAusg_5363].eingangsGruppe = eingGruppe_5363;
	servoAusg[servoAusg_5363].eingang = eing_5363;
	servoAusg[servoAusg_5364].platine = 4;
	servoAusg[servoAusg_5364].ausgang = 2;
	servoAusg[servoAusg_5364].eingangsGruppe = eingGruppe_5364;
	servoAusg[servoAusg_5364].eingang = eing_5364;
	servoAusg[servoAusg_5365].platine = 1;
	servoAusg[servoAusg_5365].ausgang = 5;
	servoAusg[servoAusg_5365].eingangsGruppe = eingGruppe_5365;
	servoAusg[servoAusg_5365].eingang = eing_5365;
	servoAusg[servoAusg_5366].platine = 4;
	servoAusg[servoAusg_5366].ausgang = 5;
	servoAusg[servoAusg_5366].eingangsGruppe = eingGruppe_5366;
	servoAusg[servoAusg_5366].eingang = eing_5366;
	servoAusg[servoAusg_5367].platine = 2;
	servoAusg[servoAusg_5367].ausgang = 2;
	servoAusg[servoAusg_5367].eingangsGruppe = eingGruppe_5367;
	servoAusg[servoAusg_5367].eingang = eing_5367;
	servoAusg[servoAusg_5368].platine = 4;
	servoAusg[servoAusg_5368].ausgang = 6;
	servoAusg[servoAusg_5368].eingangsGruppe = eingGruppe_5368;
	servoAusg[servoAusg_5368].eingang = eing_5368;
	servoAusg[servoAusg_5369].platine = 1;
	servoAusg[servoAusg_5369].ausgang = 4;
	servoAusg[servoAusg_5369].eingangsGruppe = eingGruppe_5369;
	servoAusg[servoAusg_5369].eingang = eing_5369;
	servoAusg[servoAusg_5370].platine = 2;
	servoAusg[servoAusg_5370].ausgang = 1;
	servoAusg[servoAusg_5370].eingangsGruppe = eingGruppe_5370;
	servoAusg[servoAusg_5370].eingang = eing_5370;
	servoAusg[servoAusg_5371].platine = 1;
	servoAusg[servoAusg_5371].ausgang = 3;
	servoAusg[servoAusg_5371].eingangsGruppe = eingGruppe_5371;
	servoAusg[servoAusg_5371].eingang = eing_5371;
	servoAusg[servoAusg_5372].platine = 2;
	servoAusg[servoAusg_5372].ausgang = 0;
	servoAusg[servoAusg_5372].eingangsGruppe = eingGruppe_5372;
	servoAusg[servoAusg_5372].eingang = eing_5372;
	servoAusg[servoAusg_5373].platine = 4;
	servoAusg[servoAusg_5373].ausgang = 9;
	servoAusg[servoAusg_5373].eingangsGruppe = eingGruppe_5373;
	servoAusg[servoAusg_5373].eingang = eing_5373;
	servoAusg[servoAusg_5374].platine = 1;
	servoAusg[servoAusg_5374].ausgang = 2;
	servoAusg[servoAusg_5374].eingangsGruppe = eingGruppe_5374;
	servoAusg[servoAusg_5374].eingang = eing_5374;
	servoAusg[servoAusg_5375].platine = 2;
	servoAusg[servoAusg_5375].ausgang = 3;
	servoAusg[servoAusg_5375].eingangsGruppe = eingGruppe_5375;
	servoAusg[servoAusg_5375].eingang = eing_5375;

	ausg[ausg_5201].platine = 0;
	ausg[ausg_5201].ausgang = 7;
	ausg[ausg_5201].eingangsGruppe = eingGruppe_5201;
	ausg[ausg_5201].eingang = eing_5201;
	ausg[ausg_5202].platine = 0;
	ausg[ausg_5202].ausgang = 3;
	ausg[ausg_5202].eingangsGruppe = eingGruppe_5202;
	ausg[ausg_5202].eingang = eing_5202;
	ausg[ausg_5203].platine = 3;
	ausg[ausg_5203].ausgang = 1;
	ausg[ausg_5203].eingangsGruppe = eingGruppe_5203;
	ausg[ausg_5203].eingang = eing_5203;
	ausg[ausg_5204].platine = 3;
	ausg[ausg_5204].ausgang = 4;
	ausg[ausg_5204].eingangsGruppe = eingGruppe_5204;
	ausg[ausg_5204].eingang = eing_5204;
	ausg[ausg_5205].platine = 2;
	ausg[ausg_5205].ausgang = 6;
	ausg[ausg_5205].eingangsGruppe = eingGruppe_5205_5207;
	ausg[ausg_5205].eingang = eing_5205_5207;
	ausg[ausg_5206].platine = 3;
	ausg[ausg_5206].ausgang = 0;
	ausg[ausg_5206].eingangsGruppe = eingGruppe_5206;
	ausg[ausg_5206].eingang = eing_5206;
	ausg[ausg_5207].platine = 3;
	ausg[ausg_5207].ausgang = 2;
	ausg[ausg_5207].eingangsGruppe = eingGruppe_5205_5207;
	ausg[ausg_5207].eingang = eing_5205_5207;
	ausg[ausg_5208].platine = 3;
	ausg[ausg_5208].ausgang = 9;
	ausg[ausg_5208].eingangsGruppe = eingGruppe_5209_5208;
	ausg[ausg_5208].eingang = eing_5209_5208;
	ausg[ausg_5209].platine = 5;
	ausg[ausg_5209].ausgang = 6;
	ausg[ausg_5209].eingangsGruppe = eingGruppe_5209_5208;
	ausg[ausg_5209].eingang = eing_5209_5208;
	ausg[ausg_5210].platine = 5;
	ausg[ausg_5210].ausgang = 9;
	ausg[ausg_5210].eingangsGruppe = eingGruppe_5210;
	ausg[ausg_5210].eingang = eing_5210;
	ausg[ausg_5211].platine = 5;
	ausg[ausg_5211].ausgang = 2;
	ausg[ausg_5211].eingangsGruppe = eingGruppe_5211;
	ausg[ausg_5211].eingang = eing_5211;
	ausg[ausg_5212].platine = 5;
	ausg[ausg_5212].ausgang = 4;
	ausg[ausg_5212].eingangsGruppe = eingGruppe_5212_5213;
	ausg[ausg_5212].eingang = eing_5212_5213;
	ausg[ausg_5213].platine = 5;
	ausg[ausg_5213].ausgang = 5;
	ausg[ausg_5213].eingangsGruppe = eingGruppe_5212_5213;
	ausg[ausg_5213].eingang = eing_5212_5213;
	ausg[ausg_5251].platine = 0;
	ausg[ausg_5251].ausgang = 6;
	ausg[ausg_5251].eingangsGruppe = eingGruppe_5251;
	ausg[ausg_5251].eingang = eing_5251;
	ausg[ausg_5252].platine = 0;
	ausg[ausg_5252].ausgang = 0;
	ausg[ausg_5252].eingangsGruppe = eingGruppe_5252;
	ausg[ausg_5252].eingang = eing_5252;
	ausg[ausg_5253].platine = 1;
	ausg[ausg_5253].ausgang = 9;
	ausg[ausg_5253].eingangsGruppe = eingGruppe_5253;
	ausg[ausg_5253].eingang = eing_5253;
	ausg[ausg_5254].platine = 0;
	ausg[ausg_5254].ausgang = 2;
	ausg[ausg_5254].eingangsGruppe = eingGruppe_5254;
	ausg[ausg_5254].eingang = eing_5254;
	ausg[ausg_5255].platine = 0;
	ausg[ausg_5255].ausgang = 1;
	ausg[ausg_5255].eingangsGruppe = eingGruppe_5255;
	ausg[ausg_5255].eingang = eing_5255;
	ausg[ausg_5256].platine = 4;
	ausg[ausg_5256].ausgang = 10;
	ausg[ausg_5256].eingangsGruppe = eingGruppe_5256;
	ausg[ausg_5256].eingang = eing_5256;
	ausg[ausg_5257].platine = 4;
	ausg[ausg_5257].ausgang = 3;
	ausg[ausg_5257].eingangsGruppe = eingGruppe_5257_5258;
	ausg[ausg_5257].eingang = eing_5257_5258;
	ausg[ausg_5258].platine = 4;
	ausg[ausg_5258].ausgang = 1;
	ausg[ausg_5258].eingangsGruppe = eingGruppe_5257_5258;
	ausg[ausg_5258].eingang = eing_5257_5258;
	ausg[ausg_5259].platine = 4;
	ausg[ausg_5259].ausgang = 7;
	ausg[ausg_5259].eingangsGruppe = eingGruppe_5260_5259;
	ausg[ausg_5259].eingang = eing_5260_5259;
	ausg[ausg_5260].platine = 4;
	ausg[ausg_5260].ausgang = 8;
	ausg[ausg_5260].eingangsGruppe = eingGruppe_5260_5259;
	ausg[ausg_5260].eingang = eing_5260_5259;
	ausg[ausg_5261].platine = 4;
	ausg[ausg_5261].ausgang = 0;
	ausg[ausg_5261].eingangsGruppe = eingGruppe_5261;
	ausg[ausg_5261].eingang = eing_5261;
	ausg[ausg_5301].platine = 1;
	ausg[ausg_5301].ausgang = 1;
	ausg[ausg_5301].eingangsGruppe = eingGruppe_5301;
	ausg[ausg_5301].eingang = eing_5301;
	ausg[ausg_5302].platine = 2;
	ausg[ausg_5302].ausgang = 8;
	ausg[ausg_5302].eingangsGruppe = eingGruppe_5302;
	ausg[ausg_5302].eingang = eing_5302;
	ausg[ausg_5303].platine = 2;
	ausg[ausg_5303].ausgang = 5;
	ausg[ausg_5303].eingangsGruppe = eingGruppe_5303;
	ausg[ausg_5303].eingang = eing_5303;
	ausg[ausg_5304].platine = 2;
	ausg[ausg_5304].ausgang = 10;
	ausg[ausg_5304].eingangsGruppe = eingGruppe_5304;
	ausg[ausg_5304].eingang = eing_5304;
	ausg[ausg_5305].platine = 5;
	ausg[ausg_5305].ausgang = 10;
	ausg[ausg_5305].eingangsGruppe = eingGruppe_5305;
	ausg[ausg_5305].eingang = eing_5305;
	ausg[ausg_5306].platine = 5;
	ausg[ausg_5306].ausgang = 3;
	ausg[ausg_5306].eingangsGruppe = eingGruppe_5306;
	ausg[ausg_5306].eingang = eing_5306;
	ausg[ausg_5307].platine = 5;
	ausg[ausg_5307].ausgang = 7;
	ausg[ausg_5307].eingangsGruppe = eingGruppe_5307;
	ausg[ausg_5307].eingang = eing_5307;
	ausg[ausg_5308].platine = 1;
	ausg[ausg_5308].ausgang = 0;
	ausg[ausg_5308].eingangsGruppe = eingGruppe_5308;
	ausg[ausg_5308].eingang = eing_5308;
	ausg[ausg_5309].platine = 5;
	ausg[ausg_5309].ausgang = 11;
	ausg[ausg_5309].eingangsGruppe = eingGruppe_5309;
	ausg[ausg_5309].eingang = eing_5309;
	ausg[ausg_5310].platine = 2;
	ausg[ausg_5310].ausgang = 7;
	ausg[ausg_5310].eingangsGruppe = eingGruppe_5310;
	ausg[ausg_5310].eingang = eing_5310;
	ausg[ausg_5311].platine = 3;
	ausg[ausg_5311].ausgang = 3;
	ausg[ausg_5311].eingangsGruppe = eingGruppe_5311;
	ausg[ausg_5311].eingang = eing_5311;
	ausg[ausg_5312].platine = 3;
	ausg[ausg_5312].ausgang = 10;
	ausg[ausg_5312].eingangsGruppe = eingGruppe_5312;
	ausg[ausg_5312].eingang = eing_5312;
	ausg[ausg_5313].platine = 2;
	ausg[ausg_5313].ausgang = 9;
	ausg[ausg_5313].eingangsGruppe = eingGruppe_5313;
	ausg[ausg_5313].eingang = eing_5313;
	ausg[ausg_5314].platine = 3;
	ausg[ausg_5314].ausgang = 8;
	ausg[ausg_5314].eingangsGruppe = eingGruppe_5314;
	ausg[ausg_5314].eingang = eing_5314;
	ausg[ausg_5315].platine = 5;
	ausg[ausg_5315].ausgang = 8;
	ausg[ausg_5315].eingangsGruppe = eingGruppe_5315;
	ausg[ausg_5315].eingang = eing_5315;
}
