/*
 Name:		StrabaWeingarten.ino
 Created:	13.08.2019
 Author:	Robert Muh
*/

#define NachbarHS(el) hs[el.nachbarHS]
#define VorNachbarHS(el) !(NachbarHS(el).abfahrtsZeitErreicht && (NachbarHS.abfahrtsZeit < el.abfahrtsZeit))
#define VorNachbarHS(halteStelle,nachbarHS) !(nachbarHS.abfahrtsZeitErreicht && (nachbarHS.abfahrtsZeit < halteStelle.abfahrtsZeit))
#define HSfrei(halteStelle) !(halteStelle.blockiert || halteStelle.belegt)
#define ZielHS(el) (el.alternativesZiel?el.zielHS2:el.zielHS0) 

#define HS0 0
#define HS1 1
#define HS2 2
#define HS3 3
#define HS4 4
#define HS5 5
#define HS6 6
#define HS7 7
#define HS8 8
#define HS9 9
#define HS10 10
#define HS11 11
#define HS12 12
#define HS13 13
#define HS14 14
#define HS15 15

#define StreckeTeil7 0
#define StreckeTeil6_7 1
#define StreckeTeil4_5_6 2
#define StreckeTeil2 3
#define StreckeBahnhof 4

#define ausg_1152_2152_3152_FSS 1
#define ausg_3156_4156_FSS 0
#define ausg_5156_6156_FSS 3
#define ausg_5152_6151_FSS 2
#define ausg_6154_FSS 5
#define ausg_6162_7151_7152_FSS 4
#define ausg_7161_7164_FSS 7
#define ausg_1154_FSS 6
#define ausg_3154_FSS 9
#define ausg_3155_FSS 8
#define ausg_6160_FSS 11
#define ausg_6161_FSS 10
#define ausg_7154_FSS 13
#define ausg_7158_FSS 12
#define ausg_7172_FSS 15
#define ausg_7171_FSS 14
#define ausg_1151 17
#define ausg_3151 16
#define ausg_3152 19
#define ausg_5154 18
#define ausg_5155 21
#define ausg_6152 20
#define ausg_6155 23
#define ausg_6158 22
#define ausg_7153 25
#define ausg_7157 24
#define ausg_7160 27
#define ausg_7166 26
#define ausg_7167 29
#define ausg_7168 28
#define ausg_7169 31
#define ausg_7163 30
#define ausg_5251_We 33
#define ausg_6252_We 32

#define rk_HS1HS9 23
#define rk_HS9HS10 22
#define rk_HS10HS11 21
#define rk_HS11HS0 20
#define rk_HS0HS1 19
#define rk_HS1vorn 15
#define rk_EinHS2 17
#define rk_AusHS2 14
#define rk_EinHS3 11
#define rk_AusHS3 10
#define rk_EinHS4 4
#define rk_AusHS4 1
#define rk_EinHS5 5
#define rk_AusHS5 0
#define rk_EinHS6 3
#define rk_AusHS6 2
#define rk_EinHS7 13
#define rk_AusHS7 12
#define rk_EinHS8 18
#define rk_AusHS8 16
#define rk_HS12ZumBahnhof 6
#define rk_HS12ZurStadt 9
#define rk_BahnhofHS14 8
#define rk_BahnhofHS13 7
#define rk_Alternative 24
#define rk_Bahnuebergang 25

#include <Servo.h>
#include <Wire.h>
const byte ArdNr = 1;
#define eingSo 26 // Anzahl Eingänge
#define ausgSo 34 // Anzahl Ausgänge
#define sAusgSo 0 // Anzahl ServoAusgänge
#define hsSo 16 //   Anzahl Haltestellen
#define skSo 5 // Anzahl Strecken
#define platSo 3 //Anzahl Platinen

#define ZeitAbfrage(Zeit) (millis() > (unsigned long)Zeit)

struct Haltestelle;
typedef void(*funcPtr_Haltestelle) (Haltestelle &haltestelle);

void StandardAbfahrtsbedingung(Haltestelle &h);
void AbfahrtsbedingungHS8(Haltestelle &h);
void AbfahrtsbedingungHS7(Haltestelle &h);
void HsEinAusfahrtsCheck(Haltestelle &h);
void Hs1EinAusfahrtsCheck(Haltestelle &h);
void Hs3EinAusfahrtsCheck(Haltestelle &h);
void Hs6EinAusfahrtsCheck(Haltestelle &h);


struct Eingang //für alle Arten von Schaltern, Reedkontakten, Lichtschranken 
{//L-aktiv
	bool invertiert = false;
	byte eingang = 255;//über 53 ist beim Mega inaktiv
	unsigned long zeitHLFlanke = 0;
	bool stellung = false;
	bool stellungHLFlanke = false;
	//zum Entprellen, zur Vermeidung von mehrfach Schalten (Straßenbahn Grauenstein)
	byte leseWiederholung = 0;
};
Eingang eing[eingSo];
byte leseWiederholung = 1;//so oft muss das Signal am Eingang gleich sein um die Stellung zu ändern
int halteZeit = 1000;
unsigned long debugZeit = 0;
int zustand = 0;

struct Platine {
	unsigned short Ausgaenge;
	byte Arduino;
	byte Befehl;
	bool senden;
};
Platine platinen[platSo];

struct Ausgang {
	unsigned long schaltZeit = 0;
	int einschaltVerzoegerung = 0;
	int ausschaltVerzoegerung = 0;
	byte platine;
	byte ausgang = 255;
	//byte stellung;
};
Ausgang ausg[ausgSo];

//struct ServoAusgang
//{
//	Servo servo;
//	byte ausgang = 255;
//	int servoStellung = 90;
//	//int zielStellung =  90;
//	int stellungAn = 135;//Stellung = true
//	int stellungAus = 45;//Stellung = false
//	bool stellung = false;
//};
//ServoAusgang sAusg[sAusgSo];

struct Schaltbefehl {
	byte Ausgang;
	bool Zustand;
};

struct Haltestelle {
	bool belegt = false;//Anwesendheit
	bool blockiert = false;
	byte rkEinfahrt1 = 255;// die Array-Nummer des ReedKontaktes im PGM
	byte rkEinfahrt2 = 255;
	byte rkAusfahrt1 = 255;
	byte rkAusfahrt2 = 255;

	byte rkAlternativesZiel = 255;//Array-Nummer des Eingangs(Reedkontakt)
	bool alternativesZiel = false;//false Linie1, true alternative Linie2

	byte nachbarHS = 255;// 255-> deaktiv
	byte zielHS1 = 255;  // 255-> deaktiv
	byte zielHS2 = 255;  // 255-> deaktiv;
	byte streckeAusfahrt = 255; // 255-> deaktiv;

	//wird gebraucht wenn zwei HS den gleichen Kontakt für die Ausfahrt benutzen
	//es wird dann geprüft ob der Gleisausgang eingeschalten ist
	//so wird sicher gestellt das das richtige Gleis als frei geschalten wird
	bool ausfahrtGleisCheck = false;
	byte autoStopServo = 255;

	unsigned long gleisSchaltzeit;
	unsigned long abfahrtsZeit = 0;
	unsigned int warteZeit = 10000;
	bool abfahrtsZeitErreicht = false;
	bool abfahren = false;
	funcPtr_Haltestelle AbfahrtsBedingung = StandardAbfahrtsbedingung;
	funcPtr_Haltestelle EinAusfahrtsCheck = HsEinAusfahrtsCheck;

	int abfahrtsVerzoegerung = 0;//wenn Zeit zum Stellen von Weichen benötigt wird
	bool abfahrtsVerzoegerungAktiv = false;//ist true wenn die AbfahrtsVerzögerung aktiv ist
	
	byte gleisAusgang = 255;
	byte anzSchaltbefehle = 0;
	Schaltbefehl * Schaltbefehle = NULL;
	byte anzSchaltbefehleAlternativesZiel = 0;
	Schaltbefehl * SchaltbefehleAlternativesZiel = NULL;
};
byte hsAnzeige = 255;

struct Strecke
{
	bool belegt = false;//Anwesendheit
	byte anzahlRkEinfahrt = 0;
	byte * rkEinfahrt = NULL;// die Array-Nummer der ReedKontakte im PGM
	byte anzahlRkAusfahrt = 0;
	byte * rkAusfahrt = NULL;// die Array-Nummer der ReedKontakte im PGM
};
Strecke sk[skSo];
Haltestelle hs[hsSo];


/*const int baSo = 2;//Blink-Ausgang
Ausgang ba[baSo];  //Blink-Ausgang
const int saSo = 2;//Signal-Ausgang
Ausgang sa[saSo];  //Signal-Ausgang
Ausgang carAusgang;//LOW-Aktiv*/
//USB-Definitionen*/
byte USBEingang[5];
byte USBEingangsByteZaehler = 0;
unsigned long zeitUSB = 0;
int intervallUSB = 500;
byte sendeArray[5];
//Ausgänge

void Hs1EinAusfahrtsCheck(Haltestelle &h) {
	if (h.belegt && h.abfahrtsZeit < millis()) {
		//Ausfahrt, gibt die Haltestelle wieder wieder frei beim passieren der Ausfahrts-Reed-Kontakte
		if ((EingAbfrageHLFlanke(h.rkAusfahrt1)) || (EingAbfrageHLFlanke(h.rkAusfahrt2))) {
			bool schalten = true;
			if (h.ausfahrtGleisCheck) {
				Ausgang a = ausg[h.gleisAusgang];
				if (!(bitRead(platinen[a.platine].Ausgaenge,a.ausgang))) {
					schalten = false;
				}
			}
			if (schalten) {
				AusgSchalten(h.gleisAusgang, false);
				h.belegt = false;
				h.blockiert = false;
				h.abfahrtsZeit = 0;
				h.alternativesZiel = false;
			}
		}
	}
	else {
		//Einfahrt
		if (eing[rk_HS0HS1].stellung) {
			AusgSchalten(h.gleisAusgang, false);
			h.belegt = true;
			h.blockiert = false;
			h.abfahrtsZeit = millis() + h.warteZeit;
		}
		if (eing[rk_HS1vorn].stellung) {
			AusgSchalten(h.gleisAusgang, false);
			h.belegt = true;
			h.blockiert = false;
			h.abfahrtsZeit = millis() + h.warteZeit;
			h.alternativesZiel = true;
		}
	}
}

void Hs6EinAusfahrtsCheck(Haltestelle &h) {
	//Ausfahrt, gibt die Haltestelle wieder wieder frei beim passieren der Ausfahrts-Reed-Kontakte
	if ((EingAbfrageHLFlanke(h.rkAusfahrt1)) || (EingAbfrageHLFlanke(h.rkAusfahrt2))) {
		bool schalten = true;
		if (h.ausfahrtGleisCheck) {
			Ausgang a = ausg[h.gleisAusgang];
			if (!(bitRead(platinen[a.platine].Ausgaenge, a.ausgang))) {
				schalten = false;
			}
		}
		if (schalten) {
			Serial.write(61);
			AusgSchalten(h.gleisAusgang, false);
			h.belegt = false;
			h.blockiert = false;
			h.abfahrtsZeit = 0;
			h.alternativesZiel = false;
		}
	}

	//Einfahrt
	if (!h.belegt && (h.rkEinfahrt1 != 255) && eing[h.rkEinfahrt1].stellung) {
		if (!h.belegt) {
			Serial.write(62);
		}
		AusgSchalten(h.gleisAusgang, false);
		h.belegt = true;
		h.blockiert = false;
		h.abfahrtsZeit = millis() + h.warteZeit;
		/*if (sAusgSo > hs[i].autoStopServo) {
		ServoSchalten(hs[i].autoStopServo, true);
		}*/
	}
}

void Hs3EinAusfahrtsCheck(Haltestelle &h) {
	//Ausfahrt, gibt die Haltestelle wieder wieder frei beim passieren der Ausfahrts-Reed-Kontakte
	if ((EingAbfrageHLFlanke(h.rkAusfahrt1)) || (EingAbfrageHLFlanke(h.rkAusfahrt2))) {
		bool schalten = true;
		if (h.ausfahrtGleisCheck) {
			Ausgang a = ausg[h.gleisAusgang];
			if (!(bitRead(platinen[a.platine].Ausgaenge, a.ausgang))) {
				schalten = false;
			}
		}
		if (schalten) {
			Serial.write(61);
			AusgSchalten(h.gleisAusgang, false);
			h.belegt = false;
			h.blockiert = false;
			h.abfahrtsZeit = 0;
			h.alternativesZiel = false;
		}
	}

	//Einfahrt
	if (!hs[HS8].blockiert && (h.rkEinfahrt1 != 255) && eing[h.rkEinfahrt1].stellung) {
		if (!h.belegt) {
			Serial.write(62);
		}
		AusgSchalten(h.gleisAusgang, false);
		h.belegt = true;
		h.blockiert = false;
		h.abfahrtsZeit = millis() + h.warteZeit;
		/*if (sAusgSo > hs[i].autoStopServo) {
		ServoSchalten(hs[i].autoStopServo, true);
		}*/
	}
}

void HsEinAusfahrtsCheck(Haltestelle &h) {
	//Ausfahrt, gibt die Haltestelle wieder wieder frei beim passieren der Ausfahrts-Reed-Kontakte
	if ((EingAbfrageHLFlanke(h.rkAusfahrt1)) || (EingAbfrageHLFlanke(h.rkAusfahrt2))) {
		bool schalten = true;
		if (h.ausfahrtGleisCheck) {
			Ausgang a = ausg[h.gleisAusgang];
			if (!(bitRead(platinen[a.platine].Ausgaenge, a.ausgang))) {
				schalten = false; 
			}
		}
		if (schalten) {
			Serial.write(61);
			AusgSchalten(h.gleisAusgang, false);
			h.belegt = false;
			h.blockiert = false;
			h.abfahrtsZeit = 0;
			h.alternativesZiel = false;
		}
	}
	
	//Einfahrt
	if ((h.rkEinfahrt1 != 255) && eing[h.rkEinfahrt1].stellung) {
		if (!h.belegt) {
			Serial.write(62);
		}
		AusgSchalten(h.gleisAusgang, false);
		h.belegt = true;
		h.blockiert = false;
		h.abfahrtsZeit = millis() + h.warteZeit;
		/*if (sAusgSo > hs[i].autoStopServo) {
			ServoSchalten(hs[i].autoStopServo, true);
		}*/
	}
}

void StandardAbfahrtsbedingung(Haltestelle &h) {
	h.abfahren = false;
	byte zHS = h.alternativesZiel ? h.zielHS2 : h.zielHS1;// lokale Variable für Ziel-Haltestelle
	if (zHS >= hsSo) {
		return;
	}

	//prüft ob  die Ziel-Haltestelle belegt oder blockiert ist
	if (hs[zHS].belegt || hs[zHS].blockiert) {
		return;
	}
	
	//prüft ob eine ew. Strecke frei ist
	if (h.streckeAusfahrt < skSo && sk[h.streckeAusfahrt].belegt) {
		return;
	}

	// prüft ew. NachbarHaltestellen auf Vorrang
	if (h.nachbarHS < hsSo && hs[h.nachbarHS].abfahrtsZeitErreicht) {
		if (hs[h.nachbarHS].abfahrtsZeit >= h.abfahrtsZeit) {
			return;
		}
	}
	Serial.write(60);
	Serial.write(h.gleisAusgang);
	h.abfahren = true;
}

void AbfahrtsbedingungHS7(Haltestelle &h) {
	h.abfahren = false;
	if (eing[rk_Bahnuebergang].stellung) {
		return;
	}

	byte zHS = h.alternativesZiel ? h.zielHS2 : h.zielHS1;// lokale Variable für Ziel-Haltestelle
	if (zHS >= hsSo) {
		return;
	}

	//prüft ob  die Ziel-Haltestelle belegt oder blockiert ist
	if (hs[zHS].belegt || hs[zHS].blockiert) {
		return;
	}

	//prüft ob eine ew. Strecke frei ist
	if (h.streckeAusfahrt < skSo && sk[h.streckeAusfahrt].belegt) {
		return;
	}

	// prüft ew. NachbarHaltestellen auf Vorrang
	if (h.nachbarHS < hsSo && hs[h.nachbarHS].abfahrtsZeitErreicht) {
		if (hs[h.nachbarHS].abfahrtsZeit >= h.abfahrtsZeit) {
			return;
		}
	}
	Serial.write(60);
	Serial.write(h.gleisAusgang);
	h.abfahren = true;
}

void AbfahrtsbedingungHS8(Haltestelle &h) {
	StandardAbfahrtsbedingung(h);
	if (h.abfahren) {
		if (hs[HS9].belegt || hs[HS9].blockiert) {
			h.abfahren = false;
			h.abfahrtsZeit = hs[HS0].abfahrtsZeit + 1;
		}
	}
}

void setup() {
	Wire.begin();
	Serial.begin(9600);
	Definition();
	delay(1000);
}

void loop() {
	USBAnzeige();
	//USBAnzeigeSK();
	AnlagenCheck();
	//  delay(5);
	for (int i = 0; i < platSo; i++) {
		Platine plat = platinen[i];
		if (plat.senden) {
			if (platinen[i].Arduino == ArdNr) {
				if (platinen[i].Befehl == 41) {
					ByteToRelais(lowByte(platinen[i].Ausgaenge), 22);
					ByteToRelais(highByte(platinen[i].Ausgaenge), 30);
				}
				else if (platinen[i].Befehl == 42) {
					Serial.write(99);

					ByteToRelais(lowByte(platinen[i].Ausgaenge), 38);
					ByteToRelais(highByte(platinen[i].Ausgaenge), 46);
				}
				else if (platinen[i].Befehl == 40) {
					ByteToRelais(lowByte(platinen[i].Ausgaenge), 2);
					ByteToRelais(highByte(platinen[i].Ausgaenge), 10);
				}
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
}

void AnlagenCheck() {
	EingangsCheck();//prüft alle Eingänge
	HaltestellenCheck();
	// USBAnzeigeHS();
	HaltestellenAbfahrt();
	ResetEingangHLFlanke();
}


void USBAnzeige() {
	USBEmpfang();
	if (ZeitAbfrage(zeitUSB)) {
		USBAusgangsZustand();
		USBEingangsZustand();
		USBHaltestellenZustand();
		zeitUSB = millis() + intervallUSB;
	}
}

void USBEmpfang() {
	while (Serial.available()>0) {
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
		if (USBEingangsByteZaehler>4) {
			if (BefehlsPruefung(USBEingang)) {// BefehlsAusfuerung
				BefehlsAusfuehrung(USBEingang);
				USBEingangsByteZaehler = 0;
			}
		}

	}
}

bool BefehlsPruefung(byte Befehl[5]) {
	byte summe = Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3];
	if (summe == Befehl[4]) {
		return true;
	}
	else {
		return false;
	}
}

void USBAnzeigeHS()
{//USB-Anzeige in AnlagenCheck()
 //zur Ansicht mit dem Formular Serial-ganz-neu
	if (Serial.available()) { hsAnzeige = Serial.read(); }
	if (ZeitAbfrage(zeitUSB))
	{
		for (int i = 0; i<hsSo; i++)
		{/*Anzeige alternativesZiel
		 if(hs[i].zielHS2 < hsSo){ Serial.write(hs[i].alternativesZiel);}
		 else{ Serial.write(255); }*/

		 //Anzeige GleisAusgang
		 /*byte glAusg = hs[i].gleisAusgang;
		 byte sbGleis = digitalRead(ausg[glAusg].ausgang);
		 Serial.write(sbGleis);*/

		 //Anzeige AusfahrtGleisCheck
			Serial.write(hs[i].ausfahrtGleisCheck);
		}
		for (int i = 0; i<hsSo; i++) { Serial.write(hs[i].blockiert); }
		for (int i = 0; i<hsSo; i++) { Serial.write(hs[i].belegt); }
		//sb-> SendeByte
		byte sbBelegt = 0;
		byte sbBlockiert = 0;
		byte sbLZ = 255;//Wartezeit
		byte sbAZ = 0;//AbfahrtsZeit
		byte sbAbfahren = 0;//
		byte sbAlternativesZiel = 255;
		byte sbGleisAusgang = 0;
		if (hsAnzeige < hsSo)
		{
			sbAZ = hs[hsAnzeige].abfahrtsZeit / 1000;
			sbLZ = millis() / 1000;
			sbBelegt = hs[hsAnzeige].belegt;
			sbBlockiert = hs[hsAnzeige].blockiert;
			sbAlternativesZiel = hs[hsAnzeige].alternativesZiel;
			sbGleisAusgang = digitalRead(ausg[hs[hsAnzeige].gleisAusgang].ausgang);
		}
		//for(int i = 0; i<hsSo;i++)  { Serial.write(hs[i].belegt); }
		//for(int i = 0; i<hsSo;i++)  { Serial.write(hs[i].blockiert); }
		if (hsAnzeige < hsSo)
		{
			Serial.write(98);
			Serial.write(sbBelegt);//belegt
			Serial.write(sbBlockiert);//belegt
			Serial.write(sbAlternativesZiel);
			Serial.write(sbLZ);//Systemzeit
			Serial.write(sbAZ);//Abfahrtszeit
			Serial.write(hs[hsAnzeige].abfahren);
			Serial.write(sbGleisAusgang);
			Serial.write(hsAnzeige);
		}
		Serial.write(99);

		zeitUSB = zeitUSB + intervallUSB;
	}
}

void USBAusgangsZustand()
{//sendet den Zustand der Relais-Ausgänge
	//byte befehl[5];
	//for (int i = 0; i<8; i++) { bitWrite(befehl[2], i, ausg[i].stellung); }
	//for (int i = 0; i<8; i++) { bitWrite(befehl[3], i, ausg[i + 8].stellung); }
	//befehl[1] = 41;
	//BefehlAnPC(befehl);

	//befehl[2] = 0;
	//befehl[3] = 0;
	//for (int i = 0; i < 8; i++) { bitWrite(befehl[2], i, ausg[i + 16].stellung); }
	//for (int i = 0; i < 8; i++) { bitWrite(befehl[3], i, ausg[i + 24].stellung); }
	//befehl[1] = 42;
	//BefehlAnPC(befehl);

	//befehl[2] = 0;
	//befehl[3] = 0;
	//for (int i = 0; i < 2; i++) { bitWrite(befehl[2], i, ausg[i + 32].stellung); }
	////for (int i = 0; i < 8; i++) { bitWrite(befehl[3], i, ausg[i + 24].stellung); }
	//befehl[1] = 40;
	//BefehlAnPC(befehl
	for (int i = 0; i < platSo; i++) {
		byte befehl[5];
		befehl[0] = platinen[i].Arduino;
		befehl[1] = platinen[i].Befehl;
		befehl[2] = lowByte(platinen[i].Ausgaenge);
		befehl[3] = highByte(platinen[i].Ausgaenge);
		befehl[4] = (byte)(befehl[0] + befehl[1] + befehl[2] + befehl[3]);
		Serial.write(befehl, 5);
	}
}

void USBEingangsZustand()
{
	byte befehl0[5];
	for (int i = 0; i<8; i++) { bitWrite(befehl0[2], i, eing[i].stellung); }
	for (int i = 0; i<8; i++) { bitWrite(befehl0[3], i, eing[i + 8].stellung); }
	befehl0[1] = 10;
	BefehlAnPC(befehl0);
	byte befehl1[5];
	for (int i = 0; i<8; i++) { bitWrite(befehl1[2], i, eing[i + 16].stellung); }
	befehl1[3] = 0;
	for (int i = 0; i<2; i++) { bitWrite(befehl1[3], i, eing[i + 24].stellung); }
	befehl1[1] = 11;
	BefehlAnPC(befehl1);
}

void USBHaltestellenZustand()
{
	byte befehl[5];
	unsigned int Zeit = millis() / 1000;
	befehl[2] = lowByte(Zeit);
	befehl[3] = highByte(Zeit);
	befehl[1] = 99;
	BefehlAnPC(befehl);
	//unsigned long zeit = millis();
	//if ((zeit - debugZeit) > 3000) {
	//	if (zustand == 3) {
	//		zustand = 0;
	//	}
	//	else {
	//		zustand++;
	//	}
	//}

	bool zeitBit = false;
	for (int i = 0; i<hsSo; i++)
	{
		befehl[2] = 0;
		befehl[3] = 0;
		//switch (zustand) {
		//case 0:
		//	bitWrite(befehl[2], 0, true);
		//	//bitWrite(befehl[2], 1, hs[i].blockiert);
		//	//bitWrite(befehl[2], 2, hs[i].alternativesZiel);
		//	break;
		//case 1:
		//	//bitWrite(befehl[2], 0, hs[i].belegt);
		//	bitWrite(befehl[2], 1, true);
		//	//bitWrite(befehl[2], 2, hs[i].alternativesZiel);
		//	break;
		//case 2:
		//	bitWrite(befehl[2], 0, true);
		//	//bitWrite(befehl[2], 1, hs[i].blockiert);
		//	bitWrite(befehl[2], 2, true);
		//	break;
		//case 3:
		//	//bitWrite(befehl[2], 0, hs[i].belegt);
		//	bitWrite(befehl[2], 1, true);
		//	bitWrite(befehl[2], 2, true);
		//	break;
		//}
		bitWrite(befehl[2], 0, hs[i].belegt);
		bitWrite(befehl[2], 1, hs[i].blockiert);
		bitWrite(befehl[2], 2, hs[i].alternativesZiel);
		unsigned int abfahrtsZeit = hs[i].abfahrtsZeit / 1000;
		zeitBit = bitRead(abfahrtsZeit, 8);
		if (zeitBit) { bitWrite(befehl[2], 3, true); }
		zeitBit = bitRead(abfahrtsZeit, 9);
		if (zeitBit) { bitWrite(befehl[2], 4, true); }
		zeitBit = bitRead(abfahrtsZeit, 10);
		if (zeitBit) { bitWrite(befehl[2], 5, true); }
		zeitBit = bitRead(abfahrtsZeit, 11);
		if (zeitBit) { bitWrite(befehl[2], 6, true); }
		zeitBit = bitRead(abfahrtsZeit, 12);
		if (zeitBit) { bitWrite(befehl[2], 7, true); }
		befehl[3] = lowByte(abfahrtsZeit);
		befehl[1] = 100 + i;
		BefehlAnPC(befehl);
	}
	//Strecke
	for (int i = 0; i < skSo; i++)
	{
		befehl[2] = 0;
		befehl[3] = 0;
		if (sk[i].belegt) { befehl[2] = 1; }
		befehl[1] = 120 + i;
		BefehlAnPC(befehl);
	}
}

void BefehlAnPC(byte Befehl[5])
{ //sendet einen Befehl an den PC, Kontrollbyte und Arduino-Nummer werden automatisch ergänzt    
	Befehl[0] = ArdNr;
	Befehl[4] = Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3];
	Serial.write(Befehl, 5);
}

void BefehlAnSlave(byte Befehl[5])
{ //sendet einen Befehl an den PC, Kontrollbyte und Arduino-Nummer werden automatisch ergänzt    
	Befehl[0] = ArdNr;
	Befehl[4] = Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3];
	//Wire.write(Befehl, 5);
}

void BefehlsAusfuehrung(byte Befehl[5])
{
	// int a = 0;
	switch (Befehl[1])
	{
	case 1:
	{//Slave abfrage einfügen
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
	{   //PermanentOutput 16Bit auf Adresse0
		for (int i = 0; i < platSo; i++) {
			if (platinen[i].Arduino == Befehl[0] && platinen[i].Befehl == 40) {
				platinen[i].Ausgaenge = ((unsigned short)(Befehl[3] << 8)) + Befehl[2];
				platinen[i].senden = true;
			}
		}
		break;
	}case 41:
	{  //PermanentOutput 16Bit auf Adresse1
		for (int i = 0; i < platSo; i++) {
			if (platinen[i].Arduino == Befehl[0] && platinen[i].Befehl == 41) {
				platinen[i].Ausgaenge = ((unsigned short)(Befehl[3] << 8)) + Befehl[2];
				platinen[i].senden = true;
			}
		}
		break;
	}
	case 42:
	{//PermanentOutput 16Bit auf Adresse2
		for (int i = 0; i < platSo; i++) {
			if (platinen[i].Arduino == Befehl[0] && platinen[i].Befehl == 42) {
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
	for (int i = 0; i<8; i++) { 
		digitalWrite(i + startPin, bitRead(datenByte, i)); 
		if (i + startPin > 51) {
			Serial.write(bitRead(datenByte, i));
		}
	}
}

void ByteToAusgang(byte datenByte, int startNr)
{// für Straba
	byte start = startNr;
	bool stellung = false;
	byte Nr = startNr;
	byte daten = datenByte;
	//Serial.write(Nr);
	//Serial.write(daten);
	for (int i = 0; i<8; i++)
	{
		Nr = start + i;
		if (Nr < ausgSo)
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
			//	  if (ausg[Nr].stellung) { Serial.write(Nr + 200); }
			//	  else{ Serial.write(Nr + 100); }
		}
	}
}

void ByteToEingang(byte datenByte, int startNr)
{
	byte start = startNr;
	bool stellung = false;
	byte Nr = startNr;
	for (int i = 0; i<8; i++)
	{
		if (Nr < eingSo)
		{
			stellung = bitRead(datenByte, i);
			Nr = start + i;

			Ausgang a = ausg[Nr];
			bool ausgStellung = (bitRead(platinen[a.platine].Ausgaenge, a.ausgang)) != 0;
			if (stellung != ausgStellung)
			{
				//AusgSchalten(Nr, stellung);
			}
		}
	}
}

void HaltestellenCheck()
{
	//Strecken, prüft die RK der Strecken

	for (int i = 0; i < skSo; i++) {
		for (int j = 0; j < sk[i].anzahlRkAusfahrt; j++) {
			if (EingAbfrageHLFlanke(sk[i].rkAusfahrt[j])) {
				Serial.write(97);
				sk[i].belegt = false;
				break;
			}
		}
		for (int j = 0; j < sk[i].anzahlRkEinfahrt; j++) {
			if (EingAbfrageHLFlanke(sk[i].rkEinfahrt[j])) {
				Serial.write(98);
				sk[i].belegt = true;
				break;
			}
		}
	}
	for (int i = 0; i < hsSo; i++)
	{ //alternative Linie
		if (hs[i].rkAlternativesZiel < eingSo){
			if (eing[hs[i].rkAlternativesZiel].stellung) { hs[i].alternativesZiel = true; }
		}
		hs[i].EinAusfahrtsCheck(hs[i]);
	}

	for (int i = 0; i < hsSo; i++)
	{
		//Abfahren möglich?
		if ((hs[i].abfahrtsZeit > 0) && (!hs[i].abfahrtsVerzoegerungAktiv))
		{ //wenn die Abfartszeit läuft
			if (!hs[i].abfahrtsZeitErreicht) {
				hs[i].abfahrtsZeitErreicht = ZeitAbfrage(hs[i].abfahrtsZeit);
			}
		}
	}

	for (int i = 0; i < hsSo; i++) {
		if (hs[i].abfahrtsZeitErreicht) {
			hs[i].AbfahrtsBedingung(hs[i]);
		}
	}
}

void HaltestellenAbfahrt()
{
	if (eing[rk_Bahnuebergang].stellung && !hs[HS8].blockiert) {
		AusgSchalten(ausg_7153, false);
	}
	else {
		AusgSchalten(ausg_7153, true);
	}

	for (int i = 0; i < hsSo; i++)
	{
		if (hs[i].abfahren)
		{ //das Abfahren schalten
			if (hs[i].streckeAusfahrt<skSo)
			{
				sk[hs[i].streckeAusfahrt].belegt = true;
			}

			if (hs[i].alternativesZiel) {
				for (int j = 0; j < hs[i].anzSchaltbefehleAlternativesZiel; j++) {
					AusgSchalten(
						hs[i].SchaltbefehleAlternativesZiel[j].Ausgang, 
						hs[i].SchaltbefehleAlternativesZiel[j].Zustand
					);
				}
			}
			else {
				for (int j = 0; j < hs[i].anzSchaltbefehle; j++) {
					AusgSchalten(
						hs[i].Schaltbefehle[j].Ausgang,
						hs[i].Schaltbefehle[j].Zustand
					);
				}
			}

			if (hs[i].abfahrtsVerzoegerung > 0)
			{
				hs[i].abfahrtsZeit = millis() + hs[i].abfahrtsVerzoegerung;
			}
			else {
				AusgSchalten(hs[i].gleisAusgang, true);
				//if (sAusgSo > hs[i].autoStopServo)
				//{
				//	ServoSchalten(hs[i].autoStopServo, false);
				//}
				////ServoSchalten//sAusg
			}
			//Ziel blockieren
			byte Ziel = hs[i].zielHS1;
			if (hs[i].alternativesZiel) {
				Ziel = hs[i].zielHS2;
			}

			Serial.write(63);
			hs[Ziel].blockiert = true;
			hs[i].abfahren = false;
			hs[i].abfahrtsZeitErreicht = false;
			hs[i].abfahrtsZeit = 0;
			if (hs[i].streckeAusfahrt < skSo) { sk[hs[i].streckeAusfahrt].belegt = true; }
		}
	}
}

void AusgSchalten(byte Nr, bool Stellung)
{
	int nr = Nr;
	bool stellung = Stellung;
	if (nr < ausgSo)
	{// soll nur bei Veränderung geschalten werden 
		Ausgang a = ausg[Nr];
		bool ausgStellung = (bitRead(platinen[a.platine].Ausgaenge, a.ausgang)) != 0;
		if (stellung != ausgStellung)
		{
			//ab hier die Schaltverzögerung		
			//if (ausg[nr].schaltZeit == 0)
			//{//es wird geprüft ob es Schaltverzögerung erfolgen soll
			//	if (ausgStellung) {
			//		if (ausg[nr].ausschaltVerzoegerung > 0) { ausg[nr].schaltZeit = millis() + ausg[nr].ausschaltVerzoegerung; }
			//	}
			//	else {
			//		if (ausg[nr].einschaltVerzoegerung) { ausg[nr].schaltZeit = millis() + ausg[nr].einschaltVerzoegerung; }
			//	}
			//}
			//wenn die Schaltverzögerung aktiv ist kann hier nicht geschalten werden
			//if (ausg[nr].schaltZeit == 0)
			//{
				//ausg[nr].stellung = stellung;
			bitWrite(platinen[a.platine].Ausgaenge, a.ausgang, stellung);
			platinen[a.platine].senden = true;
			//}
		}
	}
}


bool EingAbfrageHLFlanke(byte Nr)
{
	if (Nr < eingSo) { return eing[Nr].stellungHLFlanke; }
	else { return false; }
}

void EingSchalten(byte Nr, bool Stellung)
{
	if (Nr < eingSo)
	{
		if (true)
		{
			eing[Nr].zeitHLFlanke = millis() + halteZeit;
		}
		if (eing[Nr].stellung != Stellung)
		{
			if (Stellung)
			{
				eing[Nr].stellung = true;
				eing[Nr].zeitHLFlanke = millis() + halteZeit;
			}
			else
			{
				eing[Nr].stellungHLFlanke = true;
				eing[Nr].stellung = false;

			}
			eing[Nr].leseWiederholung = 0;
		}
	}
}

void EingangsCheckNeu()
{
	bool se = false;
	for (int i = 0; i < eingSo; i++)
	{
		if (eing[i].eingang < 255)//wenn aktiv
		{
			se = digitalRead(eing[i].eingang);
			if (se == eing[i].stellung) { eing[i].leseWiederholung++; }
			//der Eingang ist LOW-aktiv, die Stellung ist HIGH-aktiv
			else
			{
				eing[i].leseWiederholung = 0;
				if (eing[i].stellung) { eing[i].zeitHLFlanke = millis() + halteZeit; }
				if (leseWiederholung <= eing[i].leseWiederholung) { EingSchalten(i, !se); }
			}

		}
	}
}

void EingangsCheck()//prüft alle Eingänge
					//void EingangsCheckNeu()//prüft alle Eingänge
{//LOW-aktiv
	bool se = false;// 
	for (int i = 0; i < eingSo; i++)
	{
		if (eing[i].eingang < 255)//wenn aktiv
		{
			se = digitalRead(eing[i].eingang);//Eing.
			if (se == eing[i].stellung) { eing[i].leseWiederholung++; }
			//der Eingang ist LOW-aktiv, die Stellung ist HIGH-aktiv
			else { eing[i].leseWiederholung = 0; }
			if (leseWiederholung <= eing[i].leseWiederholung)
			{//wenn leseWiederholung erreicht ist wird die Stellung gewechselt
				eing[i].leseWiederholung = 0;
				if (se)//stellung auf false
				{
					if (ZeitAbfrage(eing[i].zeitHLFlanke))
					{
						/* eing[i].stellung = false;
						eing[i].stellungHLFlanke = true;*/
						EingSchalten(i, false);
					}
				}
				else//stellung auf true
				{
					EingSchalten(i, true);
					/*eing[i].stellung = true;
					eing[i].zeitHLFlanke = millis() + halteZeit;*/
				}
			}
		}
	}
}

void ResetEingangHLFlanke()
{
	for (int i = 0; i < eingSo; i++) { eing[i].stellungHLFlanke = false; }
}

//void ServoSchalten(byte ServoNr, bool Stellung)
//{
//	byte Nr = ServoNr;
//	if (Nr< sAusgSo)
//	{
//		if (sAusg[Nr].servo.attached())
//		{
//			sAusg[Nr].stellung = Stellung;
//			if (Stellung) { sAusg[Nr].servoStellung = sAusg[Nr].stellungAn; }
//			else { sAusg[Nr].servoStellung = sAusg[Nr].stellungAus; }
//			sAusg[Nr].servo.write(sAusg[Nr].servoStellung);
//		}
//	}
//}


void Definition()
{
	eing[0].eingang = A0;
	eing[1].eingang = A1;
	eing[2].eingang = A2;
	eing[3].eingang = A3;
	eing[4].eingang = A4;
	eing[5].eingang = A5;
	eing[6].eingang = A6;
	eing[7].eingang = A7;
	eing[8].eingang = A8;
	eing[9].eingang = A9;
	eing[10].eingang = A10;
	eing[11].eingang = A11;
	eing[12].eingang = A12;
	eing[13].eingang = A13;
	eing[14].eingang = A14;
	eing[15].eingang = A15;
	eing[16].eingang = 2;
	eing[17].eingang = 3;
	eing[18].eingang = 4;
	eing[19].eingang = 5;
	eing[20].eingang = 6;
	eing[21].eingang = 7;
	eing[22].eingang = 8;
	eing[23].eingang = 9;
	eing[24].eingang = 10;
	eing[25].eingang = 11;
	for (int i = 0; i < eingSo; i++)
	{
		delay(1);
		pinMode(eing[i].eingang, INPUT_PULLUP);
	}

	pinMode(21, INPUT_PULLUP);
	pinMode(20, INPUT_PULLUP);

	platinen[0].Arduino = ArdNr;
	platinen[0].Befehl = 41;
	platinen[0].senden = true;
	platinen[1].Arduino = ArdNr;
	platinen[1].Befehl = 42;
	platinen[1].senden = true;
	platinen[2].Arduino = 2;
	platinen[2].Befehl = 41;
	platinen[2].senden = true;

	for (int i = 0; i < 32; i++){
		ausg[i].ausgang = i%16;
		if (i < 16) {
			ausg[i].platine = 0;
		}
		else {
			ausg[i].platine = 1;
		}
		
	}
	ausg[32].ausgang = 0;
	ausg[32].platine = 2;
	ausg[33].ausgang = 1;
	ausg[33].platine = 2;

	for (int i = 0; i<32; i++)
	{
		digitalWrite(22 + i, HIGH);
		pinMode(22 + i, OUTPUT);
		//digitalWrite(22 + i, LOW);
		////digitalWrite(ausgang[i].ausgang,HIGH);
		//delay(100);
		//digitalWrite(22 + i, HIGH);
		////digitalWrite(ausgang[i].ausgang,LOW);
		//delay(10);
	}

	hs[HS0].rkEinfahrt1 = rk_HS11HS0;
	hs[HS0].rkAusfahrt1 = rk_HS0HS1;
	hs[HS0].zielHS1 = HS1;
	hs[HS0].nachbarHS = HS8;
	hs[HS0].streckeAusfahrt = StreckeTeil7;
	hs[HS0].gleisAusgang = ausg_7169;
	hs[HS0].anzSchaltbefehle = 3;
	hs[HS0].Schaltbefehle = new Schaltbefehl[3]{ 
		{ ausg_7172_FSS,false },//Herzstück ausfahrt HS0
		{ ausg_7161_7164_FSS,false },//Fss Strecke über HS1
		{ ausg_7158_FSS,false } //Herzstück einfahrt HS2
	};

	hs[HS1].rkEinfahrt1 = rk_HS0HS1;
	hs[HS1].rkEinfahrt2 = rk_HS1vorn;
	hs[HS1].rkAusfahrt1 = rk_HS1HS9;
	hs[HS1].rkAusfahrt2 = rk_HS1vorn;
	hs[HS1].zielHS1 = HS2;
	hs[HS1].zielHS2 = HS9;
	hs[HS1].EinAusfahrtsCheck = Hs1EinAusfahrtsCheck;
	hs[HS1].gleisAusgang = ausg_7163;
	hs[HS1].anzSchaltbefehle = 2;
	hs[HS1].Schaltbefehle = new Schaltbefehl[2]{
		{ ausg_7161_7164_FSS,false },//Fss Strecke über HS1
		{ ausg_7158_FSS,false } //Herzstück einfahrt HS2
	};
	hs[HS1].anzSchaltbefehleAlternativesZiel = 2;
	hs[HS1].SchaltbefehleAlternativesZiel = new Schaltbefehl[2]{
		{ ausg_7161_7164_FSS,true },//Fss Strecke über HS1
		{ ausg_7172_FSS,true },//Herzstück einfahrt HS9
	};
	
	hs[HS2].rkEinfahrt1 = rk_EinHS2;
	hs[HS2].rkAusfahrt1 = rk_AusHS2;
	hs[HS2].zielHS1 = HS3;
	hs[HS2].nachbarHS = HS7;
	hs[HS2].streckeAusfahrt = StreckeTeil6_7;
	hs[HS2].gleisAusgang = ausg_7160;
	hs[HS2].anzSchaltbefehle = 3;
	hs[HS2].Schaltbefehle = new Schaltbefehl[3]{
		{ ausg_7154_FSS,false },//Herzstück ausfahrt HS2
		{ ausg_6162_7151_7152_FSS,false },//Fss Strecke Teil7-Teil6
		{ ausg_6161_FSS,false } //Herzstück einfahrt HS3
	};
	
	hs[HS3].rkEinfahrt1 = rk_EinHS3;
	hs[HS3].rkAusfahrt1 = rk_AusHS3; 
	hs[HS3].zielHS1 = HS4; 
	hs[HS3].zielHS2 = HS12;
	hs[HS3].nachbarHS = HS6;
	hs[HS3].streckeAusfahrt = StreckeTeil4_5_6;
	hs[HS3].EinAusfahrtsCheck = Hs3EinAusfahrtsCheck;
	hs[HS3].gleisAusgang = ausg_6155;
	hs[HS3].anzSchaltbefehle = 6;
	hs[HS3].Schaltbefehle = new Schaltbefehl[6]{
		{ ausg_6160_FSS,false }, //Herzstück ausfahrt HS3
		{ ausg_6154_FSS,false }, //FSS bis Weiche
		{ ausg_6252_We,false }, //aWeiche
		{ ausg_5156_6156_FSS,false },//Fss Strecke Teil 5-6
		{ ausg_3156_4156_FSS,false }, //Fss Strecke Teil 3-4-6
		{ ausg_3155_FSS,false } //Herzstück einfahrt HS4
	};
	hs[HS3].anzSchaltbefehleAlternativesZiel = 4;
	hs[HS3].SchaltbefehleAlternativesZiel = new Schaltbefehl[4]{
		{ ausg_6160_FSS,false }, //Herzstück ausfahrt HS3
		{ ausg_6154_FSS,false }, //FSS bis Weiche
		{ ausg_6252_We,false }, //Weiche
		{ ausg_5152_6151_FSS,false }//Fss nach Weiche
	};

	hs[HS4].rkEinfahrt1 = rk_EinHS4;
	hs[HS4].rkAusfahrt1 = rk_AusHS4; 
	hs[HS4].zielHS1 = HS5;
	hs[HS4].streckeAusfahrt = StreckeTeil2;
	hs[HS4].gleisAusgang = ausg_3151; 
	hs[HS4].anzSchaltbefehle = 3;
	hs[HS4].Schaltbefehle = new Schaltbefehl[3]{
		{ ausg_3154_FSS,false }, //Herzstück ausfahrt HS4
		{ ausg_1152_2152_3152_FSS,false }, //FSS Teil 2
		{ ausg_1154_FSS,false } //Schleuse Dorf, innen
	};

	hs[HS5].rkEinfahrt1 = rk_EinHS5;
	hs[HS5].rkAusfahrt1 = rk_AusHS5;
	hs[HS5].zielHS1 = HS6; 
	hs[HS5].streckeAusfahrt = StreckeTeil2;
	hs[HS5].gleisAusgang = ausg_1151;
	hs[HS5].anzSchaltbefehle = 3;
	hs[HS5].Schaltbefehle = new Schaltbefehl[3]{
		{ ausg_1154_FSS,true }, //Schleuse Dorf, innen
		{ ausg_1152_2152_3152_FSS,true }, //FSS Teil 2
		{ ausg_3154_FSS,true } //Herzstück einfahrt HS6
	};
	
	hs[HS6].rkEinfahrt1 = rk_EinHS6;
	hs[HS6].rkAusfahrt1 = rk_AusHS6;
	hs[HS6].zielHS1 = HS7;
	hs[HS6].nachbarHS = HS3;
	hs[HS6].streckeAusfahrt = StreckeTeil4_5_6;
	hs[HS6].gleisAusgang = ausg_3152;
	hs[HS6].EinAusfahrtsCheck = Hs6EinAusfahrtsCheck;
	hs[HS6].anzSchaltbefehle = 6;
	hs[HS6].Schaltbefehle = new Schaltbefehl[6]{
		{ ausg_3155_FSS,true }, //Herzstück ausfahrt HS6
		{ ausg_5156_6156_FSS,true },//Fss Strecke Teil 5-6
		{ ausg_3156_4156_FSS,true }, //Fss Strecke Teil 3-4-6
		{ ausg_6252_We,true }, //Weiche
		{ ausg_6154_FSS,true }, //FSS bis Weiche
		{ ausg_6160_FSS,true } //Herzstück einfahrt HS7
	};
	
	hs[HS7].rkEinfahrt1 = rk_EinHS7;
	hs[HS7].rkAusfahrt1 = rk_AusHS7; 
	hs[HS7].zielHS1 = HS8;
	hs[HS7].nachbarHS = HS2;
	hs[HS7].streckeAusfahrt = StreckeTeil6_7;
	hs[HS7].AbfahrtsBedingung = AbfahrtsbedingungHS7;
	hs[HS7].gleisAusgang = ausg_6158;
	hs[HS7].anzSchaltbefehle = 3;
	hs[HS7].Schaltbefehle = new Schaltbefehl[3]{
		{ ausg_6161_FSS,true }, //Herzstück ausfahrt HS7
		{ ausg_6162_7151_7152_FSS,true },//Fss Strecke Teil7-Teil6
		{ ausg_7154_FSS,true }//Herzstück einfahrt HS8
	};
	
	hs[HS8].rkEinfahrt1 = rk_EinHS8;
	hs[HS8].rkAusfahrt1 = rk_AusHS8; 
	hs[HS8].zielHS1 = HS1;
	hs[HS8].streckeAusfahrt = StreckeTeil7;
	hs[HS8].nachbarHS = HS0;
	hs[HS8].AbfahrtsBedingung = AbfahrtsbedingungHS8;
	hs[HS8].gleisAusgang = ausg_7157;
	hs[HS8].anzSchaltbefehle = 3;
	hs[HS8].Schaltbefehle = new Schaltbefehl[3]{
		{ ausg_7158_FSS,true }, //Herzstück ausfahrt HS8
		{ ausg_7161_7164_FSS,true },//Fss Strecke über HS1
		{ ausg_7172_FSS,true },//Herzstück einfahrt HS9
	};
	
	hs[HS9].warteZeit = 500;
	hs[HS9].rkEinfahrt1 = rk_HS1HS9;
	hs[HS9].rkAusfahrt1 = rk_HS9HS10; 
	hs[HS9].zielHS1 = HS10;
	hs[HS9].gleisAusgang = ausg_7166;

	hs[HS10].warteZeit = 500;
	hs[HS10].rkEinfahrt1 = rk_HS9HS10;
	hs[HS10].rkAusfahrt1 = rk_HS10HS11;
	hs[HS10].zielHS1 = HS11;
	hs[HS10].gleisAusgang = ausg_7167;

	hs[HS11].warteZeit = 500;
	hs[HS11].rkEinfahrt1 = rk_HS10HS11;
	hs[HS11].rkAusfahrt1 = rk_HS11HS0;
	hs[HS11].zielHS1 = HS0;
	hs[HS11].gleisAusgang = ausg_7168;

	hs[HS12].rkEinfahrt1 = rk_HS12ZumBahnhof;
	hs[HS12].rkEinfahrt2 = rk_HS12ZurStadt;
	hs[HS12].rkAusfahrt1 = rk_HS12ZumBahnhof;
	hs[HS12].rkAusfahrt2 = rk_HS12ZurStadt;
	hs[HS12].zielHS1 = HS13;
	hs[HS12].zielHS2 = HS7;
	hs[HS12].gleisAusgang = ausg_6152;

	hs[HS13].rkEinfahrt1 = rk_BahnhofHS13;
	hs[HS13].rkAusfahrt1 = rk_BahnhofHS13;
	hs[HS13].zielHS1 = HS12;
	hs[HS13].streckeAusfahrt = StreckeBahnhof;
	hs[HS13].gleisAusgang = ausg_5154;

	hs[HS14].rkEinfahrt1 = rk_BahnhofHS14;
	hs[HS14].rkAusfahrt1 = rk_BahnhofHS14;
	hs[HS14].zielHS1 = HS12;
	hs[HS14].streckeAusfahrt = StreckeBahnhof;
	hs[HS14].gleisAusgang = ausg_5155;

	sk[StreckeTeil7].anzahlRkAusfahrt = 2;
	sk[StreckeTeil7].rkAusfahrt = new byte[2]{ rk_EinHS2, rk_HS1HS9 };
	sk[StreckeTeil7].anzahlRkEinfahrt = 0;
	sk[StreckeTeil7].rkEinfahrt = new byte[2]{ rk_AusHS8, rk_HS0HS1 };

	sk[StreckeTeil6_7].anzahlRkAusfahrt = 2;
	sk[StreckeTeil6_7].rkAusfahrt = new byte[2]{ rk_EinHS3, rk_EinHS8 };
	sk[StreckeTeil6_7].anzahlRkEinfahrt = 0;
	sk[StreckeTeil6_7].rkEinfahrt = new byte[2]{ rk_AusHS2, rk_AusHS7 };

	sk[StreckeTeil4_5_6].anzahlRkAusfahrt = 3;
	sk[StreckeTeil4_5_6].rkAusfahrt = new byte[3]{ rk_EinHS7, rk_EinHS4, rk_HS12ZurStadt };
	sk[StreckeTeil4_5_6].anzahlRkEinfahrt = 0;
	sk[StreckeTeil4_5_6].rkEinfahrt = new byte[2]{ rk_AusHS3, rk_AusHS6 };

	sk[StreckeTeil2].anzahlRkAusfahrt = 2;
	sk[StreckeTeil2].rkAusfahrt = new byte[2]{rk_EinHS6, rk_EinHS5};
	sk[StreckeTeil2].anzahlRkEinfahrt = 0;
	sk[StreckeTeil2].rkEinfahrt = new byte[2]{rk_AusHS5, rk_AusHS4};

	sk[StreckeBahnhof].anzahlRkAusfahrt = 3;
	sk[StreckeBahnhof].rkAusfahrt = new byte[3]{ rk_EinHS7, rk_BahnhofHS13, rk_BahnhofHS14 };
	sk[StreckeBahnhof].anzahlRkEinfahrt = 1;
	sk[StreckeBahnhof].rkEinfahrt = new byte[1]{ rk_HS12ZurStadt};
}

