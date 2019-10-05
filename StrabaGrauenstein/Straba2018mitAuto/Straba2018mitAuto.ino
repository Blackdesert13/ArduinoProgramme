// ist Arbeitsstand a, ohne Excel
// HeikoEckardt6b -> leseWiederholung
#include <Servo.h>
const byte ArdNr = 1;
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
byte leseWiederholung = 10;//so oft muss das Signal am Eingang gleich sein um die Stellung zu ändern
int halteZeit = 1000;
const int eingSo = 18;
Eingang eing[eingSo];
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
const int ausgSo = 16;
Ausgang ausg[ausgSo];

struct ServoAusgang
{
	Servo servo;
	byte ausgang = 255;
	int servoStellung = 90;
	//int zielStellung =  90;
	int stellungAn = 135;//Stellung = true
	int stellungAus = 45;//Stellung = false
	bool stellung = false;
};
const int sAusgSo = 2;
ServoAusgang sAusg[sAusgSo];

struct Haltestelle
{
	byte autoStopServo = 255;
	bool belegt = false;//Anwesendheit
	bool blockiert = false;
	byte rkEinfahrt1 = 255;// die Array-Nummer des ReedKontaktes im PGM
	byte rkEinfahrt2 = 255;
	byte rkAusfahrt1 = 255;
	byte rkAusfahrt2 = 255;

	//wird gebraucht wenn zwei HS den gleichen Kontakt für die Ausfahrt benutzen
	//es wird dann geprüft ob der Gleisausgang eingeschalten ist
	//so wird sicher gestellt das das richtige Gleis als frei geschalten wird
	bool ausfahrtGleisCheck = false;

	byte gleisAusgang = 255;
	unsigned long abfahrtsZeit = 0;
	unsigned int warteZeit = 10000;
	//bool gleisServo = false;
	//byte autoStopp = 57;
	//bool autoStoppServo = false;
	bool abfahren = false;
	unsigned long gleisSchaltzeit;
	int abfahrtsVerzoegerung = 0;//wenn Zeit zum Stellen von Weichen benötigt wird
	bool abfahrtsVerzoegerungAktiv = false;//ist true wenn die AbfahrtsVerzögerung aktiv ist
	byte rkAlternativesZiel = 255;//Array-Nummer des Eingangs(Reedkontakt)
	bool alternativesZiel = false;//false Linie1, true alternative Linie2
	byte nachbarHS = 255;// 255-> deaktiv
	byte zielHS1 = 255;  // 255-> deaktiv
	byte zielHS2 = 255;  // 255-> deaktiv;
	byte streckeAusfahrt = 255; // 255-> deaktiv;
								//byte strecke2Ausfahrt = 255;//eine ew. zusätzlich zu sperrende Strecke, 255-> aus
	byte weicheAusfahrt = 255;//Array-Nr. des Ausgangs
	bool weichenStellungAusfahrt;
	byte weicheAlternativesZiel = 255;//Array-Nr. des Ausgangs
};
const int hsSo = 10;
Haltestelle hs[hsSo];
byte hsAnzeige = 255;

struct Strecke
{
	bool belegt = false;//Anwesendheit
						//bool blockiert = false;
	byte rkEinfahrt1 = 255;// die Array-Nummer des ReedKontaktes im PGM
	byte rkEinfahrt2 = 255;
	byte rkAusfahrt1 = 255;
	byte rkAusfahrt2 = 255;
};
const int skSo = 1;
Strecke sk[skSo];


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
	AusgSchaltVerzoegerungsCheck();
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
		USBAusgangsZustand();
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
	befehl0[1] = 41;
	BefehlAnPC(befehl0);
	byte befehl1[5];
	for (int i = 0; i<8; i++) { bitWrite(befehl1[2], i, eing[i + 16].stellung); }
	for (int i = 0; i<8; i++) { bitWrite(befehl1[3], i, eing[i + 24].stellung); }
	befehl1[1] = 42;
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
	// Serial.write(97);
	// Serial.write(skSo)
	for (int i = 0; i < skSo; i++)
	{
		// Serial.write(98);
		if (sk[i].rkAusfahrt1 < eingSo)
		{
			if (EingAbfrageHLFlanke(sk[i].rkAusfahrt1))
				// if(eing[sk[i].rkAusfahrt1].stellung)
			{
				sk[i].belegt = false;
			}
		}
		if (sk[i].rkAusfahrt2 < eingSo)
		{
			if (EingAbfrageHLFlanke(sk[i].rkAusfahrt2))
				//if (eing[sk[i].rkAusfahrt2].stellung)
			{
				sk[i].belegt = false;
			}
		}
		if (sk[i].rkEinfahrt1 < eingSo)
		{
			if (eing[sk[i].rkEinfahrt1].stellung)
				//if (eing[sk[i].rkAusfahrt2].stellung)
			{
				sk[i].belegt = true;
				
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
		if (eing[hs[i].rkEinfahrt1].stellung)
			//if(EingAbfrageHLFlanke(hs[i].rkEinfahrt1))
		{
			AusgSchalten(hs[i].gleisAusgang, false);
			hs[i].belegt = true;
			hs[i].blockiert = false;
			hs[i].abfahrtsZeit = millis() + hs[i].warteZeit;
			if (sAusgSo > hs[i].autoStopServo)
			{
				ServoSchalten(hs[i].autoStopServo, true);
			}
		}
		//Abfahren möglich?
		if ((hs[i].abfahrtsZeit > 0) && (!hs[i].abfahrtsVerzoegerungAktiv))
		{ //wenn die Abfartszeit läuft
			hs[i].abfahren = ZeitAbfrage(hs[i].abfahrtsZeit);
			{

				if (hs[i].abfahren) //wenn die AbfahrtsZeit erreicht ist
				{
					byte zHS = 255;// lokale Variable für Ziel-Haltestelle
					if (hs[i].alternativesZiel) { zHS = hs[i].zielHS2; }
					else { zHS = hs[i].zielHS1; }
					if (zHS<hsSo)
					{//prüft ob  die Ziel-Haltestelle belegt oder blockiert ist
						if (hs[zHS].belegt) { hs[i].abfahren = false; }
						if (hs[zHS].blockiert) { hs[i].abfahren = false; }
					}
				}
				if (hs[i].abfahren)
				{//prüft ob eine ew. Strecke frei ist
					if (hs[i].streckeAusfahrt < skSo)
					{
						if (sk[hs[i].streckeAusfahrt].belegt) { hs[i].abfahren = false; }
					}
				}
			}
		}
	}
	if (hs[4].belegt && hs[4].alternativesZiel
		&& hs[9].belegt && !hs[9].belegt && !hs[9].blockiert
	) {
		hs[4].abfahrtsZeit = millis();
	}
	HaltestellenAbfahrt4_7_8();
}

void HaltestellenAbfahrt4_7_8()
{
	if (hs[7].belegt) {
		if (hs[4].alternativesZiel && hs[4].belegt) {
			if (hs[4].abfahrtsZeit < hs[7].abfahrtsZeit) {
				hs[4].abfahren = true;
				hs[7].abfahren = false;
			}
			else {
				hs[4].abfahren = false;
				hs[7].abfahren = true;
			}
		}
	}
	if(hs[8].belegt) {
		if (!hs[4].alternativesZiel && hs[4].belegt) {
			if (hs[4].abfahrtsZeit < hs[8].abfahrtsZeit) {
				hs[4].abfahren = true;
				hs[8].abfahren = false;
			}
			else {
				hs[4].abfahren = false;
				hs[8].abfahren = true;
			}
		}
	}
}

void HaltestellenAbfahrt()
{// prüft ew. NachbarHaltestellen auf Vorrang
	for (int i = 0; i < hsSo; i++)
	{
		if (hs[i].nachbarHS < hsSo)
		{
			if (hs[hs[i].nachbarHS].abfahren)
			{
				if (hs[hs[i].nachbarHS].abfahrtsZeit < hs[i].abfahrtsZeit)
				{
					hs[i].abfahren = false;
				}
			}
		}
	}

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
				if(sAusgSo > hs[i].autoStopServo)
				{
					ServoSchalten(hs[i].autoStopServo, false);
				}
				//ServoSchalten//sAusg
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

void ServoSchalten(byte ServoNr, bool Stellung)
{
	byte Nr = ServoNr;
	if (Nr< sAusgSo)
	{
		if (sAusg[Nr].servo.attached())
		{
			sAusg[Nr].stellung = Stellung;
			if (Stellung) { sAusg[Nr].servoStellung = sAusg[Nr].stellungAn; }
			else { sAusg[Nr].servoStellung = sAusg[Nr].stellungAus; }
			sAusg[Nr].servo.write(sAusg[Nr].servoStellung);
		}
	}
}

bool ZeitAbfrage(unsigned long Zeit)
{ //ca.0,004ms //liefert true wenn die SystemZeit größer ist als die "Zeit"

	if (millis() > Zeit) { return true; }
	else { return false; }
}

void Definition()
{
	sAusg[0].ausgang = 22;//3801
	sAusg[0].servo.attach(sAusg[0].ausgang);
	sAusg[0].servo.write(sAusg[0].stellungAus);


	eing[0].eingang = 2;  //3501
	eing[1].eingang = 3;  //3502
	eing[2].eingang = 4;  //3503
	eing[3].eingang = 5;  //3504
	eing[4].eingang = 6;  //3505  //
	eing[5].eingang = 7;  //3506  //
	eing[6].eingang = 8;  //3507  //
	eing[7].eingang = 9;  //3508
	eing[8].eingang = 10; //3509
	eing[9].eingang = 11; //3510  //
	eing[10].eingang = 14;//1521
	eing[11].eingang = 12;//1522
	eing[12].eingang = 15;//5523
	eing[13].eingang = 16;//5524
	eing[14].eingang = 17;//5525
	eing[15].eingang = 18;//5526
	eing[16].eingang = 19;//5527
	eing[17].eingang = 20;//5528
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
	//HSweg
	hs[0].warteZeit = 5000;
	hs[0].gleisAusgang = 6;//ARD36;5124
	hs[0].rkEinfahrt1 = 17;//ARD20;5528
	hs[0].rkAusfahrt1 = 12;//ARD15;5523
	hs[0].zielHS2 = 1;//Linie1
	hs[0].zielHS1 = 2;//Linie2
	hs[0].rkAlternativesZiel = 9;//ARD11;3510
	hs[0].weicheAlternativesZiel = 12;//ARD42;5211
	hs[0].streckeAusfahrt = 0;
	//Linie1
	hs[1].warteZeit = 10000;
	hs[1].gleisAusgang = 0;//ARD30;5122
	hs[1].rkEinfahrt1 = 14;//ARD17;5525
	hs[1].rkAusfahrt1 = 15;//ARD18;5526
	hs[1].zielHS1 = 3;//her
	hs[1].nachbarHS = 2; //Linie2
	hs[1].ausfahrtGleisCheck = true;
	hs[1].weicheAusfahrt = 1;//ARD31;5212
	hs[1].weichenStellungAusfahrt = true;
	//Linie2
	hs[2].warteZeit = 60000;
	hs[2].gleisAusgang = 3; //ARD33;5123
	hs[2].rkEinfahrt1 = 13;//ARD16;5524
	hs[2].rkAusfahrt1 = 15;//ARD18;5526
	hs[2].zielHS1 = 3;//her
	hs[2].nachbarHS = 1; //Linie1
	hs[2].ausfahrtGleisCheck = true;
	hs[2].weicheAusfahrt = 1;//ARD31;5212
	hs[2].weichenStellungAusfahrt = false;
	//hin
	hs[3].warteZeit = 10000;
	hs[3].gleisAusgang = 5;//ARD35;5125
	hs[3].rkEinfahrt1 = 16;//ARD19;5527
	hs[3].rkAusfahrt1 = 8; //ARD10;3509
	hs[3].zielHS1 = 4; //hoch
					   //hoch
	hs[4].gleisAusgang = 9; //????ARD39;3127
	hs[4].rkEinfahrt1 = 1;//ARD3;3502
	hs[4].rkAusfahrt1 = 6; //ARD8;3507
	hs[4].rkAusfahrt2 = 3; //ARD5;3504
	hs[4].zielHS1 = 5; //vorDepot
	hs[4].zielHS2 = 9; //wende
	hs[4].rkAlternativesZiel = 7;//ARD9;3508
	hs[4].weicheAlternativesZiel = 10;//ARD40;3201
									  //vorDepot
	hs[5].warteZeit = 2;
	hs[5].gleisAusgang = 11;//ARD41;1152
	hs[5].rkEinfahrt1 = 10;//ARD12;1521
	hs[5].rkAusfahrt1 = 11; //ARD14;1522
	hs[5].zielHS1 = 6; //Depot
					   //Depot
	hs[6].warteZeit = 20000;
	hs[6].gleisAusgang = 8; //ARD38;1151
	hs[6].rkEinfahrt1 = 11;//ARD14;1522
	hs[6].rkAusfahrt1 = 4; //ARD6;3505  
	hs[6].zielHS1 = 7; //runter
					   //runter
	hs[7].gleisAusgang = 7; //ARD37;3125
	hs[7].rkEinfahrt1 = 5;//ARD7;3506
	hs[7].rkAusfahrt1 = 0; //ARD2;3501
	hs[7].zielHS1 = 0; //weg
	hs[7].nachbarHS = 8; //nachWende
	hs[7].ausfahrtGleisCheck = true;
	hs[7].weicheAusfahrt = 10;//ARD40;3201
	hs[7].weichenStellungAusfahrt = false;
	hs[7].autoStopServo = 0;//3801
	//nachWende
	hs[8].warteZeit =6000;
	hs[8].gleisAusgang = 4; //ARD34;3126
	hs[8].rkEinfahrt1 = 2;//ARD4;3502
	hs[8].rkAusfahrt1 = 0; //ARD2;3501
	hs[8].zielHS1 = 0; //weg
	hs[8].nachbarHS = 7; //runter
	hs[8].ausfahrtGleisCheck = true;
	hs[8].weicheAusfahrt = 10;//ARD40;3201
	hs[8].weichenStellungAusfahrt = true;
	//wende
	hs[9].warteZeit = 20000;
	hs[9].gleisAusgang = 13; //ARD;
	hs[9].rkEinfahrt1 = 3;//ARD5;3504
	hs[9].rkAusfahrt1 = 2; //ARD4;3503
	hs[9].zielHS1 = 8; //--???--
					   //Strecke nach HS0//Strecke4_7_8
	sk[0].rkAusfahrt1 = 14; //ard17;5525
	sk[0].rkAusfahrt2 = 13;//ARD16;5524
	sk[0].rkEinfahrt1 = 12;//ARD15;  5523
}
