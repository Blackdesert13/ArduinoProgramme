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

#include <Servo.h>
const byte ArdNr = 1;
#define eingSo 24 // Anzahl Eingänge
#define ausgSo 16 // Anzahl Ausgänge
#define sAusgSo 0 // Anzahl ServoAusgänge
#define hsSo 16 //   Anzahl Haltestellen
#define skSo 5 // Anzahl Strecken

#define ZeitAbfrage(Zeit) (millis() > (unsigned long)Zeit)

struct Haltestelle;
typedef void(*myFunctionPointer) (Haltestelle &haltestelle);

void StandardAbfahrtsbedingung(Haltestelle &h);


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

struct Ausgang
{
	unsigned long schaltZeit = 0;
	int einschaltVerzoegerung = 0;
	int ausschaltVerzoegerung = 0;
	byte ausgang = 255;
	bool invertiert = false;
	bool stellung = false;//HIGH-aktiv
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

struct Haltestelle
{
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
	byte gleisAusgang = 255;

	unsigned long gleisSchaltzeit;
	unsigned long abfahrtsZeit = 0;
	unsigned int warteZeit = 10000;
	bool abfahrtsZeitErreicht = false;
	bool abfahren = false;
	myFunctionPointer AbfahrtsBedingung = StandardAbfahrtsbedingung;

	int abfahrtsVerzoegerung = 0;//wenn Zeit zum Stellen von Weichen benötigt wird
	bool abfahrtsVerzoegerungAktiv = false;//ist true wenn die AbfahrtsVerzögerung aktiv ist

	byte weicheAusfahrt = 255;//Array-Nr. des Ausgangs
	bool weichenStellungAusfahrt;
	byte weicheAlternativesZiel = 255;//Array-Nr. des Ausgangs
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
byte outPinStart[6];


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

	if (h.streckeAusfahrt < skSo && sk[h.streckeAusfahrt].belegt)
	{//prüft ob eine ew. Strecke frei ist
		//Serial.write(66);
		//if (h.abfahren) {
		//	Serial.write(h.rkAusfahrt1);
		//}
		//Serial.write(66);
		return;
	}

	if (h.nachbarHS < hsSo && hs[h.nachbarHS].abfahrtsZeitErreicht)
	{// prüft ew. NachbarHaltestellen auf Vorrang
		if (hs[h.nachbarHS].abfahrtsZeit >= h.abfahrtsZeit) {
			return;
		}
	}
	h.abfahren = true;
}

void setup()
{
	Serial.begin(9600);
	//Ausgänge
	outPinStart[0] = 30;
	outPinStart[1] = 38;
	Definition();
	delay(1000);
}

void loop()
{
	USBAnzeige();
	//USBAnzeigeSK();
	AnlagenCheck();
	//  delay(5);
}

void AnlagenCheck()
{
	EingangsCheck();//prüft alle Eingänge
	HaltestellenCheck();
	// USBAnzeigeHS();
	HaltestellenAbfahrt();
	//AusgSchaltVerzoegerungsCheck();
	ResetEingangHLFlanke();
}

void USBAnzeigeSK() {
	Serial.write(99);
	Serial.write(eing[12].stellung);
	Serial.write(sk[0].belegt);
}

void USBAnzeige()
{
	USBEmpfang();
	if (ZeitAbfrage(zeitUSB))
	{
		//USBAusgangsZustand();
		USBEingangsZustand();
		USBHaltestellenZustand();
		zeitUSB = millis() + intervallUSB;
	}
}

void USBEmpfang()
{
	while (Serial.available()>0)
	{
		byte daten = 0;
		daten = Serial.read();
		if (USBEingangsByteZaehler < 5)
		{
			USBEingang[USBEingangsByteZaehler] = daten;
			//     Serial.write(USBBefehlEingang[USBDatenEingangByteZahler]);
		}
		else//EingangsBytes schieben
		{
			//Serial.write(222);
			USBEingang[0] = USBEingang[1];
			USBEingang[1] = USBEingang[2];
			USBEingang[2] = USBEingang[3];
			USBEingang[3] = USBEingang[4];
			USBEingang[4] = daten;//Serial.read();
								  //Serial.write(USBEingang,5);
		}
		//Serial.write(USBEingangsByteZaehler);
		//Serial.write(USBEingang,5);
		USBEingangsByteZaehler++;
		if (USBEingangsByteZaehler>4)
		{
			if (BefehlsPruefung(USBEingang))
			{// BefehlsAusfuerung
				BefehlsAusfuehrung(USBEingang);
				USBEingangsByteZaehler = 0;
				//Serial.write(USBEingang,5);
				// Serial.write(99);
				// Serial.write(USBEingangsByteZaehler);
			}
			else
			{
				// Serial.write(98);
				// Serial.write(USBEingangsByteZaehler);
			}
			// Serial.write(USBEingang,5);
			// Serial.write(USBEingangsByteZaehler);
			//USBEingangsByteZaehler = 0;
		}

	}
}

bool BefehlsPruefung(byte Befehl[5])
{//geprüft  
	byte summe = Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3];
	if (summe == Befehl[4])
	{   //Serial.write(199);
		return true;
	}
	else
	{    //Serial.write(198);
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
	byte befehl[5];
	//bool st ;
	for (int i = 0; i<8; i++) { bitWrite(befehl[2], i, ausg[i].stellung); }
	for (int i = 0; i<8; i++) { bitWrite(befehl[3], i, ausg[i + 8].stellung); }
	befehl[1] = 40;
	BefehlAnPC(befehl);
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
	//for (int i = 0; i<8; i++) { bitWrite(befehl1[3], i, eing[i + 24].stellung); }
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
	{//PermanentOutput 16Bit auf Adresse0
	 //Serial.write(Befehl,5);
	 // ByteToRelais(Befehl[3],outPinStart[0]);
	 // ByteToRelais(Befehl[2],outPinStart[1]);
	 //Serial.write(Befehl,5);
	 //Serial.write(77);
	 //void ByteToAusgang(byte datenByte, int startNr)
		ByteToAusgang(Befehl[2], 0);
		ByteToAusgang(Befehl[3], 8);
		break;
	}case 41:
	{  //PermanentOutput 16Bit auf Adresse1
	   // ByteToRelais(Befehl[3],outPinStart[2]);
	   // ByteToRelais(Befehl[2],outPinStart[3]);
	   // ByteToEingang(Befehl[2], 16);
	   //  ByteToEingang(Befehl[3], 24);
		break;
	}
	case 42:
	{//PermanentOutput 16Bit auf Adresse2
	 // ByteToRelais(Befehl[3],outPinStart[4]);
	 //ByteToRelais(Befehl[2],outPinStart[5]);
		break;
	}
	}
}

void ByteToRelais(byte datenByte, int startPin)
{ //von MoBS_Master
	datenByte = ~datenByte;//datenByte muß invertiert werden weil die Relaisplatinen low-aktiv sind!
	for (int i = 0; i<8; i++) { digitalWrite(i + startPin, bitRead(datenByte, i)); }
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
			if (stellung != ausg[Nr].stellung)
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

			if (stellung == ausg[Nr].stellung)
			{
				//AusgSchalten(Nr,stellung);
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
		if (hs[i].rkAlternativesZiel < eingSo)
		{
			if (eing[hs[i].rkAlternativesZiel].stellung) { hs[i].alternativesZiel = true; }
		}
		//Ausfahrt, gibt die Haltestelle wieder wieder frei beim passieren der Ausfahrts-Reed-Kontakte
		if ((EingAbfrageHLFlanke(hs[i].rkAusfahrt1)) || (EingAbfrageHLFlanke(hs[i].rkAusfahrt2)))
		{
			bool schalten = true;
			if (hs[i].ausfahrtGleisCheck)
			{
				// byte ga = hs[i].gleisAusgang;
				if (!ausg[hs[i].gleisAusgang].stellung) { schalten = false; }
			}
			if (schalten)
			{
				AusgSchalten(hs[i].gleisAusgang, false);
				hs[i].belegt = false;
				hs[i].blockiert = false;
				hs[i].abfahrtsZeit = 0;
				hs[i].alternativesZiel = false;
			}
		}
		//Einfahrt
		if ((hs[i].rkEinfahrt1 != 255) && eing[hs[i].rkEinfahrt1].stellung)
			//if(EingAbfrageHLFlanke(hs[i].rkEinfahrt1))
		{
			AusgSchalten(hs[i].gleisAusgang, false);
			hs[i].belegt = true;
			hs[i].blockiert = false;
			hs[i].abfahrtsZeit = millis() + hs[i].warteZeit;
			/*if (sAusgSo > hs[i].autoStopServo)
			{
				ServoSchalten(hs[i].autoStopServo, true);
			}*/
		}
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
	for (int i = 0; i < hsSo; i++)
	{
		if (hs[i].abfahren)
		{ //das Abfahren schalten
			if (hs[i].streckeAusfahrt<skSo)
			{
				sk[hs[i].streckeAusfahrt].belegt = true;
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
			//AusfahrtsWeiche
			if (hs[i].weicheAusfahrt < ausgSo)
			{
				AusgSchalten(hs[i].weicheAusfahrt, hs[i].weichenStellungAusfahrt);
			}
			//Ziel blockieren
			byte Ziel = hs[i].zielHS1;
			if (hs[i].rkAlternativesZiel < eingSo)
			{//alternativesZiel
				byte weiche = hs[i].weicheAlternativesZiel;
				if (hs[i].alternativesZiel)
				{
					Ziel = hs[i].zielHS2;
					if (weiche < ausgSo) { AusgSchalten(weiche, true); }
				}
				else { if (weiche < ausgSo) { AusgSchalten(weiche, false); } }
			}
			hs[Ziel].blockiert = true;
			hs[i].abfahren = false;
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
		if (stellung != ausg[nr].stellung)
		{

			//ab hier die Schaltverzögerung		
			if (ausg[nr].schaltZeit == 0)
			{//es wird geprüft ob es Schaltverzögerung erfolgen soll
				if (ausg[nr].stellung) {
					if (ausg[nr].ausschaltVerzoegerung > 0) { ausg[nr].schaltZeit = millis() + ausg[nr].ausschaltVerzoegerung; }
				}
				else {
					if (ausg[nr].einschaltVerzoegerung) { ausg[nr].schaltZeit = millis() + ausg[nr].einschaltVerzoegerung; }
				}
			}
			//wenn die Schaltverzögerung aktiv ist kann hier nicht geschalten werden
			if (ausg[nr].schaltZeit == 0)
			{
				ausg[nr].stellung = stellung;
				if (ausg[Nr].invertiert)
				{
					digitalWrite(ausg[nr].ausgang, ausg[nr].stellung);
				}
				else
				{
					digitalWrite(ausg[nr].ausgang, !ausg[nr].stellung);
				}
			}
		}
	}
}

void AusgSchaltenAlt(byte Nr, bool Stellung)
{
	int nr = Nr;
	bool stellung = Stellung;
	if (nr < ausgSo)
	{// soll nur bei Veränderung geschalten werden 
		if (stellung != ausg[nr].stellung)
		{
			ausg[nr].stellung = stellung;
			//ab hier die Schaltverzögerung		
			//bis hier die Schaltverzögerung		
			if (ausg[Nr].invertiert)
			{
				digitalWrite(ausg[nr].ausgang, ausg[nr].stellung);
			}
			else
			{
				digitalWrite(ausg[nr].ausgang, !ausg[nr].stellung);
			}
			//Serial.write(nr);Serial.write(stellung+100); 
		}
	}
}

void AusgSchaltVerzoegerungsCheck()
{
	for (int i = 0; i < ausgSo; i++)
	{
		if (ausg[i].schaltZeit>0)
		{
			if (ZeitAbfrage(ausg[i].schaltZeit))
			{
				// AusgSchalten( i, !ausg[i].stellung);
				ausg[i].stellung = !ausg[i].stellung;
				digitalWrite(ausg[i].ausgang, ausg[i].stellung);
				ausg[i].schaltZeit = 0;
			}
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
	for (int i = 0; i < eingSo; i++)
	{
		delay(1);
		pinMode(eing[i].eingang, INPUT_PULLUP);
	}

	ausg[0].ausgang = 30; //5122x
	ausg[1].ausgang = 31; //5212;Weiche
	ausg[2].ausgang = 32; //5124
	ausg[3].ausgang = 33; //5123x
	ausg[4].ausgang = 34; //3126
	ausg[5].ausgang = 35; //5125
	ausg[6].ausgang = 36; //5124
	ausg[7].ausgang = 37; //3125
	ausg[8].ausgang = 38; //1151
	ausg[9].ausgang = 39; //3127
						  // ausg[9].einschaltVerzoegerung = 4000;
	ausg[10].ausgang = 40;//3201;Weiche
						  // ausg[10].invertiert = true;
	ausg[11].ausgang = 41;//1152
	ausg[12].ausgang = 42;//5211;Weiche
	ausg[12].invertiert = true;
	ausg[13].ausgang = 43;//3202;Weiche
	ausg[14].ausgang = 44;//frei
	ausg[15].ausgang = 45;//frei
	for (int i = 0; i<eingSo; i++)
	{
		pinMode(ausg[i].ausgang, OUTPUT);
		digitalWrite(ausg[i].ausgang, LOW);
		//digitalWrite(ausgang[i].ausgang,HIGH);
		delay(100);
		digitalWrite(ausg[i].ausgang, HIGH);
		//digitalWrite(ausgang[i].ausgang,LOW);
		delay(10);
	}

	
	hs[HS0].gleisAusgang = 0;
	hs[HS0].rkEinfahrt1 = rk_HS11HS0;
	hs[HS0].rkAusfahrt1 = rk_HS0HS1;
	hs[HS0].zielHS1 = HS1;
	hs[HS0].streckeAusfahrt = StreckeTeil7;
	
	hs[HS1].gleisAusgang = 0;
	hs[HS1].rkEinfahrt1 = rk_HS0HS1;
	hs[HS1].rkEinfahrt2 = rk_HS1vorn;
	hs[HS1].rkAusfahrt1 = rk_HS1HS9;
	hs[HS1].rkAusfahrt2 = rk_HS1vorn;
	hs[HS1].zielHS1 = HS2;
	
	hs[HS2].gleisAusgang = 3; 
	hs[HS2].rkEinfahrt1 = rk_EinHS2;
	hs[HS2].rkAusfahrt1 = rk_AusHS2;
	hs[HS2].zielHS1 = HS3;
	hs[HS2].streckeAusfahrt = StreckeTeil6_7;
	
	hs[HS3].gleisAusgang = 5;
	hs[HS3].rkEinfahrt1 = rk_EinHS3;
	hs[HS3].rkAusfahrt1 = rk_AusHS3; 
	hs[HS3].zielHS1 = HS4; 
	hs[HS3].zielHS2 = HS12;
	hs[HS3].streckeAusfahrt = StreckeTeil4_5_6;

	hs[HS4].gleisAusgang = 9; 
	hs[HS4].rkEinfahrt1 = rk_EinHS4;
	hs[HS4].rkAusfahrt1 = rk_AusHS4; 
	hs[HS4].zielHS1 = HS5; 
	hs[HS4].streckeAusfahrt = StreckeTeil2;
	 
	hs[HS5].gleisAusgang = 11;
	hs[HS5].rkEinfahrt1 = rk_EinHS5;
	hs[HS5].rkAusfahrt1 = rk_AusHS5;
	hs[HS5].zielHS1 = HS6; 
	hs[HS5].streckeAusfahrt = StreckeTeil2;
	
	hs[HS6].gleisAusgang = 8; 
	hs[HS6].rkEinfahrt1 = rk_EinHS6;
	hs[HS6].rkAusfahrt1 = rk_AusHS6;
	hs[HS6].zielHS1 = HS7;
	hs[HS6].streckeAusfahrt = StreckeTeil4_5_6;
					   
	hs[HS7].gleisAusgang = 7;
	hs[HS7].rkEinfahrt1 = rk_EinHS7;
	hs[HS7].rkAusfahrt1 = rk_AusHS7; 
	hs[HS7].zielHS1 = HS8;
	hs[HS7].streckeAusfahrt = StreckeTeil6_7;
	
	hs[HS8].gleisAusgang = 4; 
	hs[HS8].rkEinfahrt1 = rk_EinHS8;
	hs[HS8].rkAusfahrt1 = rk_AusHS8; 
	hs[HS8].zielHS1 = HS1;
	hs[HS8].streckeAusfahrt = StreckeTeil7;
	
	hs[HS9].warteZeit = 500;
	hs[HS9].gleisAusgang = 13;
	hs[HS9].rkEinfahrt1 = rk_HS1HS9;
	hs[HS9].rkAusfahrt1 = rk_HS9HS10; 
	hs[HS9].zielHS1 = HS10;

	hs[HS10].warteZeit = 500;
	hs[HS10].gleisAusgang = 13;
	hs[HS10].rkEinfahrt1 = rk_HS9HS10;
	hs[HS10].rkAusfahrt1 = rk_HS10HS11;
	hs[HS10].zielHS1 = HS11;

	hs[HS11].warteZeit = 500;
	hs[HS11].gleisAusgang = 13;
	hs[HS11].rkEinfahrt1 = rk_HS10HS11;
	hs[HS11].rkAusfahrt1 = rk_HS11HS0;
	hs[HS11].zielHS1 = HS0;

	hs[HS13].gleisAusgang = 7;
	hs[HS13].rkEinfahrt1 = rk_BahnhofHS13;
	hs[HS13].rkAusfahrt1 = rk_BahnhofHS13;
	hs[HS13].zielHS1 = HS12;
	hs[HS13].streckeAusfahrt = StreckeBahnhof;

	hs[HS14].gleisAusgang = 4;
	hs[HS14].rkEinfahrt1 = rk_BahnhofHS14;
	hs[HS14].rkAusfahrt1 = rk_BahnhofHS14;
	hs[HS14].zielHS1 = HS12;
	hs[HS14].streckeAusfahrt = StreckeBahnhof;

	sk[StreckeTeil7].anzahlRkAusfahrt = 2;
	sk[StreckeTeil7].rkAusfahrt = new byte[2]{ rk_EinHS2, rk_HS1HS9 };
	sk[StreckeTeil7].anzahlRkEinfahrt = 2;
	sk[StreckeTeil7].rkEinfahrt = new byte[2]{ rk_AusHS8, rk_HS0HS1 };

	sk[StreckeTeil6_7].anzahlRkAusfahrt = 2;
	sk[StreckeTeil6_7].rkAusfahrt = new byte[2]{ rk_EinHS3, rk_EinHS8 };
	sk[StreckeTeil6_7].anzahlRkEinfahrt = 2;
	sk[StreckeTeil6_7].rkEinfahrt = new byte[2]{ rk_AusHS2, rk_AusHS7 };

	sk[StreckeTeil4_5_6].anzahlRkAusfahrt = 3;
	sk[StreckeTeil4_5_6].rkAusfahrt = new byte[3]{ rk_EinHS7, rk_EinHS4, rk_HS12ZurStadt };
	sk[StreckeTeil4_5_6].anzahlRkEinfahrt = 2;
	sk[StreckeTeil4_5_6].rkEinfahrt = new byte[2]{ rk_AusHS3, rk_AusHS6 };

	sk[StreckeTeil2].anzahlRkAusfahrt = 2;
	sk[StreckeTeil2].rkAusfahrt = new byte[2]{rk_EinHS6, rk_EinHS5};
	sk[StreckeTeil2].anzahlRkEinfahrt = 2;
	sk[StreckeTeil2].rkEinfahrt = new byte[2]{rk_AusHS5, rk_AusHS4};

	sk[StreckeBahnhof].anzahlRkAusfahrt = 3;
	sk[StreckeBahnhof].rkAusfahrt = new byte[3]{ rk_EinHS7, rk_BahnhofHS13, rk_BahnhofHS14 };
	sk[StreckeBahnhof].anzahlRkEinfahrt = 1;
	sk[StreckeBahnhof].rkEinfahrt = new byte[1]{ rk_HS12ZurStadt};
}

