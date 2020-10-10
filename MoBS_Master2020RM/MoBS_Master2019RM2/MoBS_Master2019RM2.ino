#include <Wire.h>
struct SlaveRM
{
	bool neu[2];//gibt an ob die Befehle an PC gesendet werden müssen
	byte Befehl0[5];
	byte Befehl1[5];
};
byte RMEingang[10];//zu Aufnehmen des RM-Befehls vom Slave
const int RMArdSo = 6;//Anzahl der zu lesenden RM Arduinos 
SlaveRM SlaveRMAblage[RMArdSo];
const byte NeuZeichnenBefehl[5] = { 1,19,0,0,20 };
const byte AlleAusgaengeSendenBefehl[5] = { 1,49,0,0,50 };
const byte ArdNr = 1;//ArduinoNummer
					 //Rückmeldung
//unsigned long TimeNextRMAbfrage = 0;//
//long TimeIntervallRMAbfrage = 3000;
//Rückmeldung Slave
int SlaveAktivListe = 0;
long RMintervalSlaveAbfrage = 500;//Interval zum abfragen der Rückmelder vom Slave
unsigned long RMZeitSlaveAbfrage = 0;//nächste RM-Aktualisierung
									 //allgemein test
bool RMAlleSenden = true;
byte TestByte = 0;

//USB
int ByteZaehlerUSBEingang = 0;
int USBDatenEingangByteZahler;
byte USBBefehlEingang[5];
//Ausgänge
byte outPinStart[6];
/*
#define PIN_WATCHDOG 13
const unsigned long _Watchdog_ZeitInterval = 1000;
unsigned long _Watchdog_NaechsterWechsel = 0;
byte _zaehler_WatchDog =0;
*/
void setup()
{
	Wire.begin();
	Serial.begin(9600);
	pinMode(18, OUTPUT); digitalWrite(18, HIGH);
	pinMode(19, OUTPUT); digitalWrite(19, HIGH);

	//pinMode(PIN_WATCHDOG, OUTPUT);
	//digitalWrite(PIN_WATCHDOG, _zaehler_WatchDog%2);

	for (int i = 2; i<18; i++) { pinMode(i, OUTPUT); digitalWrite(i, HIGH); }
	for (int i = 22; i<54; i++) { pinMode(i, OUTPUT); digitalWrite(i, HIGH); }
	outPinStart[0] = 2;//festlegen des Output-Pins für Adresse0 LOWByte und Bit0
	outPinStart[1] = 10;//festlegen des Output-Pins für Adresse0 HIGHByte und Bit0
	outPinStart[2] = 22;//festlegen des Output-Pins für Adresse1 LOWByte und Bit0
	outPinStart[3] = 30;//festlegen des Output-Pins für Adresse1 HIGHByte und Bit0
	outPinStart[4] = 38;//festlegen des Output-Pins für Adresse2 LOWByte und Bit0
	outPinStart[5] = 46;//festlegen des Output-Pins für Adresse2 HIGHByte und Bit0
	delay(3);
	RMSlaveDefinition();
	//SlaveAktivAbfrage();

	//Serial.write(AlleAusgaengeSendenBefehl, 5);
}


void loop()
{
	//delay(1);
	//Watchdog();
	USBDatenEmpfang();
	//Rueckmeldung();
	//RMvonSlaves();
	RMvonSlavesNeu();
	// SlaveAktivAbfrage();
}
/*
void Watchdog() {
	if (millis() > _Watchdog_NaechsterWechsel) {
		_zaehler_WatchDog++;
		digitalWrite(PIN_WATCHDOG, _zaehler_WatchDog % 2);
		_Watchdog_NaechsterWechsel = _Watchdog_ZeitInterval + millis();

		//Serial.write(NeuZeichnenBefehl, 5);
	}
}
*/
void USBDatenEmpfang()
{
	while (Serial.available()>0)
	{//wenn vom PC Daten gesendet wurden
		if (USBDatenEingangByteZahler<5)
		{
			USBBefehlEingang[USBDatenEingangByteZahler] = Serial.read();
			Serial.write(USBBefehlEingang[USBDatenEingangByteZahler]);
		}
		else {}//{ BefehlSchieben(); BefehlInput[4]=Serial.read();}
			   // Serial.write(bz);    
		USBDatenEingangByteZahler++;

		if (USBDatenEingangByteZahler>4)
		{
			//for(int i=0 ; i<8 ; i++ ){digitalWrite( i+30, bitRead(BefehlInput[3],i) );}
			if (controlBefehl(USBBefehlEingang))//wenn die Prüfsumme stimmt
			{

				if (USBBefehlEingang[0] == 1)
				{
					BefehlsAusfuerung(USBBefehlEingang);
					// Serial.write(12);
				}
				if (USBBefehlEingang[0]>1)
				{//Befehl an EINEN Slave-Arduino
					Wire.beginTransmission(USBBefehlEingang[0]);
					Wire.write(USBBefehlEingang, 5);
					Wire.endTransmission();
				}
				if (USBBefehlEingang[0] == 0)
				{//Befehl an alle Arduinos
				 // es mussen noch die Kontroll-Bytes korrigiert werden
					USBBefehlEingang[0] = 1;
					BefehlsAusfuerung(USBBefehlEingang);
					for (int i = 0; i<16; i++)
					{
						if (bitRead(SlaveAktivListe, i))
						{
							USBBefehlEingang[0] = i + 2;
							Wire.beginTransmission(USBBefehlEingang[0]);
							Wire.write(USBBefehlEingang, 5);
							Wire.endTransmission();
						}
					}
				}
				USBDatenEingangByteZahler = 0;
			}
		}
	}
}

void SlaveAktivAbfrage()//neu
{
	SlaveAktivListe = 0;
	byte rm[10];
	for (int i = 0; i<16; i++)
	{ //Serial.write(i);
		int j = 0;
		bitWrite(SlaveAktivListe, i, 0);
		Wire.requestFrom(i + 2, 10);//fordert von Arduino Nr.2-17 10Bytes an  
		while (Wire.available())
		{//bitWrite(SlaveAktivListe,i,1);
			rm[j] = Wire.read();
			//Serial.write(rm[j]);
			j++;
			if (j == 10) {
				//rm[9] += 1;
				if (RMPruefen(rm, i + 2))//prüft ob die Daten stimmig sind
				{
					bitWrite(SlaveAktivListe, i, 1);
				}
			}
			//else{ bitWrite(SlaveAktivListe,i,0); }
		}
	}
	byte sba[5];//SendeByteArray
	sba[0] = ArdNr;
	sba[1] = 1;
	sba[2] = highByte(SlaveAktivListe);
	sba[3] = lowByte(SlaveAktivListe);
	befehlAnPC(sba);
}
boolean RMPruefen(byte RM[10], byte Nr)//neu
{//liefert TRUE wenn die Rückmeldung passt
	if (RM[0] != Nr) { return false; }
	if (RM[5] != Nr) { return false; }
	//if(RM[4] != RM[0] + RM[1] + RM[2] + RM[3] ){ return false; }
	//if(RM[9] != RM[5] + RM[6] + RM[7] + RM[8] ){ return false; }
	return true;
}
void RMvonSlavesNeu()
{//zum test
	if (IntervallControl(RMZeitSlaveAbfrage))//wenn der nächste Abfrage-Zeitpunkt erreicht ist
	{
		//Serial.write(99);//zum test
		RMZeitSlaveAbfrage = RMZeitSlaveAbfrage + RMintervalSlaveAbfrage;//nächsten Abfrage-Zeitpunkt festlegen
		bool senden = false;//gibt an ob überhaubt an PC gesendet werden muss
		for (int i = 0; i < RMArdSo; i++)
		{
			//Serial.write(RMArdSo);
			//Serial.write(SlaveRMAblage[i].Befehl0[0]);
			Wire.requestFrom((int)SlaveRMAblage[i].Befehl0[0], 10);//fordert von Slave-Arduino 10Bytes an

			int bz = 0;
			while (Wire.available()) {    // slave may send less than requested
				RMEingang[bz] = Wire.read(); // receive a byte as character
				if (RMAlleSenden) {
					Serial.write(RMEingang[bz]);
					senden = true;
				}
				bz++;

			}
			if ((byte)(RMEingang[0] + RMEingang[1] + RMEingang[2] + RMEingang[3]) == RMEingang[4])//KontrollByte prüfen
			{
				if (RMEingang[2] != SlaveRMAblage[i].Befehl0[2])// bei Veränderung
				{
					//Serial.write(60);
					SlaveRMAblage[i].Befehl0[2] = RMEingang[2];
					SlaveRMAblage[i].neu[0] = true;
					senden = true;
				}
				if (RMEingang[3] != SlaveRMAblage[i].Befehl0[3])// bei Veränderung
				{
					//Serial.write(61);
					SlaveRMAblage[i].Befehl0[3] = RMEingang[3];
					SlaveRMAblage[i].neu[0] = true;
					senden = true;
				}
			}
			if ((byte)(RMEingang[5] + RMEingang[6] + RMEingang[7] + RMEingang[8]) == RMEingang[9])//KontrollByte prüfen
			{
				//Serial.write(76);				
				if (RMEingang[7] != SlaveRMAblage[i].Befehl1[2])// bei Veränderung
				{
					//Serial.write(62);
					SlaveRMAblage[i].Befehl1[2] = RMEingang[7];
					SlaveRMAblage[i].neu[1] = true;
					senden = true;
				}
				if (RMEingang[8] != SlaveRMAblage[i].Befehl1[3])// bei Veränderung
				{
					//Serial.write(63);
					SlaveRMAblage[i].Befehl1[3] = RMEingang[8];
					SlaveRMAblage[i].neu[1] = true;
					senden = true;
				}
			}
			//	Serial.write(NeuZeichnenBefehl, 5);
		}

		//Serial.write(RMEingang, 10);
		if (senden)//ausblenden zum test
		{
			//Serial.write(60);
			for (int i = 0; i < RMArdSo; i++) {
				if (SlaveRMAblage[i].neu[0])
				{
					//Serial.write(61);
					SlaveRMAblage[i].Befehl0[4] = SlaveRMAblage[i].Befehl0[0] + SlaveRMAblage[i].Befehl0[1]
						+ SlaveRMAblage[i].Befehl0[2] + SlaveRMAblage[i].Befehl0[3];
					Serial.write(SlaveRMAblage[i].Befehl0, 5); 
					SlaveRMAblage[i].neu[0] = false;
				}
				if (SlaveRMAblage[i].neu[1])
				{ 
					//Serial.write(62);
					SlaveRMAblage[i].Befehl1[4] = SlaveRMAblage[i].Befehl1[0] + SlaveRMAblage[i].Befehl1[1]
						+ SlaveRMAblage[i].Befehl1[2] + SlaveRMAblage[i].Befehl1[3];
					Serial.write(SlaveRMAblage[i].Befehl1, 5);
					SlaveRMAblage[i].neu[1] = false;
					//Serial.write(62);
				}
			}

			Serial.write(NeuZeichnenBefehl, 5);
			//Serial.write(RMArdSo);
			//Serial.write(SlaveRMAblage[2].Befehl0[0]);
		}

		if (RMAlleSenden) {
			Serial.write(NeuZeichnenBefehl, 5);
			RMAlleSenden = false;
		}
		
	}
}

void RMvonSlaves()
{//getestet
	if (IntervallControl(RMZeitSlaveAbfrage))//wenn der nächste Abfrage-Zeitpunkt erreicht ist
	{//Serial.write(99);
		Wire.requestFrom(6, 10);//fordert von Arduino Nr.6 10Bytes an
		while (Wire.available()) {    // slave may send less than requested
			byte b = Wire.read(); // receive a byte as character
			Serial.write(b);    //Schickt die ausgelesenen Bytes unmittelbar an PC(USB)
		}
		//Wire.requestFrom(5, 10);//fordert von Arduino Nr.5 10Bytes an
		//while (Wire.available()) {    // slave may send less than requested
		//	byte b = Wire.read(); // receive a byte as character
		//	Serial.write(b);    //Schickt die ausgelesenen Bytes unmittelbar an PC(USB)
		//}
		//Wire.requestFrom(3, 10);//fordert von Arduino Nr.3 10Bytes an
		//while (Wire.available()) {    // slave may send less than requested
		//	byte b = Wire.read(); // receive a byte as character
		//	Serial.write(b);    //Schickt die ausgelesenen Bytes unmittelbar an PC(USB)
		//}
		RMZeitSlaveAbfrage = RMZeitSlaveAbfrage + RMintervalSlaveAbfrage;//nächsten Abfrage-Zeitpunkt festlegen
	}
}

void BefehlsAusfuerung(byte Befehl[5])
{
	// int a = 0;
	switch (Befehl[1])
	{
	//case 1:
	//{//Slave abfrage einfügen
	//	SlaveAktivAbfrage();
	//	//Serial.write(98);
	//	break;
	//}
	//case 2:
	//{
	//	SlaveAktivListe = 0;
	//	break;
	//}
	case 9:
	{
		/* Wire.requestFrom(3, 10);    // request 6 bytes from slave device #2

		while(Wire.available())    // slave may send less than requested
		{
		byte b = Wire.read(); // receive a byte as character
		Serial.write(b);         // print the character
		}*/

		break;
	}
	case 19: 
	{
		RMAlleSenden = true;
		break;
	}
	/*case 3:
	{
	intervalImpuls = BefehlInput[2] * 100;
	break;
	}

	case 20://permanentOutput
	{
	//BefehlZurueckSenden();
	//a = 30 + (8*BefehlInput[2]);
	//for(int i=0 ; i<8 ; i++ ){digitalWrite( i+a, bitRead(BefehlInput[3],i) );}
	ByteToRelais(BefehlInput[3],outPinStart[BefehlInput[2]]);
	break;
	}
	case 30:
	{
	a = 6 + (8*BefehlInput[2]);
	for(int i=0 ; i<8 ; i++ ){digitalWrite( i+a, bitRead(BefehlInput[3],i) );}
	timeEnd[BefehlInput[2]] = intervalImpuls + millis();
	a = 3 + BefehlInput[2];
	digitalWrite(a,LOW);
	break;
	}*/
	case 40:
	{//PermanentOutput 16Bit auf Adresse0
		ByteToRelais(Befehl[2], outPinStart[0]);
		ByteToRelais(Befehl[3], outPinStart[1]);
		break;
	}case 41:
	{//PermanentOutput 16Bit auf Adresse1
		ByteToRelais(Befehl[2], outPinStart[2]);
		ByteToRelais(Befehl[3], outPinStart[3]);
		break;
	}
	case 42:
	{//PermanentOutput 16Bit auf Adresse2
		ByteToRelais(Befehl[2], outPinStart[4]);
		ByteToRelais(Befehl[3], outPinStart[5]);
		break;
	}
	}
}

void ByteToRelais(byte datenByte, int startPin)
{
	datenByte = ~datenByte;//datenByte muß invertiert werden weil die Relaisplatinen low-aktiv sind!
	for (int i = 0; i<8; i++) { digitalWrite(i + startPin, bitRead(datenByte, i)); }
}
void befehlAnPC(byte Befehl[5])
{ //sendet einen Befehl an den PC, Kontrollbyte und Arduino-Nummer werden automatisch ergänzt
	Befehl[0] = ArdNr;
	Befehl[4] = Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3];
	Serial.write(Befehl, 5);
}
boolean controlBefehl(byte Befehl[5])
{  //kontrolliert die Prüfsumme
	byte cb = Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3];
	if (cb == Befehl[4]) { return true; }
	else { return false; }
}

boolean IntervallControl(unsigned long Zeit)
{
	if (millis()> Zeit) { return true; }
	else { return false; }
}
void RMSlaveDefinition()
{
	SlaveRMAblage[0].Befehl0[0] = 2;
	SlaveRMAblage[1].Befehl0[0] = 6;
	SlaveRMAblage[2].Befehl0[0] = 3;
	SlaveRMAblage[3].Befehl0[0] = 4;
	SlaveRMAblage[4].Befehl0[0] = 5;
	SlaveRMAblage[5].Befehl0[0] = 12;
	for (int i = 0; i < RMArdSo; i++) {
		SlaveRMAblage[i].Befehl1[0] = SlaveRMAblage[i].Befehl0[0];
		SlaveRMAblage[i].Befehl0[1] = 10;
		SlaveRMAblage[i].Befehl1[1] = 11;
	}
}
