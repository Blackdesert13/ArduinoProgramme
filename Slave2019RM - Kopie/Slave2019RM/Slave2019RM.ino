/*
 Name:		Slave2019RM.ino
 Created:	30.07.2019 17:35:08
 Author:	Jürgen
*/
/*Slave befindet sich momentan in Entwicklung
Stand vom 15.6.2014
R�ckmeldung eingebaut noch nicht getestet,
vorl�ufig �ber Serial gesendet
*/
#include <Servo.h>
#include <Wire.h>
const byte ArdNr = 4;//ArduinoNummer
unsigned long timeAkt;//aktuelle Zeit
//unsigned long lastRMupdateTime = 0;//Speichert die Zeit des letzten R�ckmelde-Updates
byte outPinStart[6];
//int intervalImpuls =1000;//Haltezeit f�r ImpulsAusg�nge
//unsigned long timeEnd[3];//Zeitpunkt zum r�cksetzen der 3Impulsadressen
//const int pinstart=2;//der erste outputpin
byte eb = 0;//Eingangsbyte
byte bzS = 0;//ByteZ�hler �ber Serial
byte bzW = 0;//ByteZ�hler �ber I2C
byte BefehlInputS[5];//Befehl �ber Serial
byte BefehlInputW[5];//Befehl �ber I2C
byte BefehlOutputS[5];//Befehl �ber Serial
byte BefehlOutputW[5];//Befehl �ber I2C
					  //byte kontrollbyte;//dient nur zum testen
byte tz = 0;//Testz�hler
			/*boolean RMStatus = true;//ist true wenn RM gesendet werden muss
			byte RM[4];//neue R�ckmeldebyts
			byte RMalt[4];//letzte R�ckmeldebyts
			long RMinterval = 5000;//Interval zum lesen der R�ckmelder
			byte RMWert[32];//Messwerte der Rueckmeldung
			byte RMBefehl0[4];//Befehl der 1.Gruppe von 16 RM's
			byte RMBefehl1[4];//Befehl der 2.Gruppe von 16 RM's*/

byte RMBefehl[10];//2x RM-Befehle
byte RMMessWert[32][10];//Messwerte der Rueckmeldung
byte RMMesswerteAuswertung[32];//der zweitniedrigste Messwert soll zur Auswertung der RM herangezogenwerden
byte RMTiefsteMesswerteAuswertung[32];//der niedrigste Messwert soll bei Auswertung der RM ignoriert werden um Fehlmessungen zu minimieren
//byte RMaktuellerMesswert = 0;
//long RMinterval = 100;//Interval zum lesen der R�ckmelder
byte RMSchwelle = 252;/*Schaltschwelle unter der
					  ein analoger Eingang f�r R�ckmeldung als aktiv gewertet wird*/

unsigned long RMZeitAktualisierungUSB = 0;//n�chste RM-Aktualisierung
int RMUSBInterval = 1000;
byte USBBefehlEingang[5];
int USBDatenEingangByteZahler;

void setup()
{
	// tz=0;
	Serial.begin(9600);
	Wire.begin(ArdNr);//(ArdNr);       receiveEvent        
	Wire.onReceive(receiveEvent); // register event
	Wire.onRequest(requestEvent); //EreignisMethode zur Datenabfrage
	for (int i = 2; i<18; i++) { pinMode(i, OUTPUT); digitalWrite(i, LOW); }
	for (int i = 22; i<54; i++) { pinMode(i, OUTPUT); digitalWrite(i, LOW); }
	delay(2000);
	for (int i = 22; i<54; i++) { pinMode(i, OUTPUT); digitalWrite(i, HIGH); }
	for (int i = 2; i<18; i++) { pinMode(i, OUTPUT); digitalWrite(i, HIGH); }
	outPinStart[0] = 2;//festlegen des Output-Pins f�r Adresse0 und Bit0
	outPinStart[1] = 10;//festlegen des Output-Pins f�r Adresse1 und Bit0
	outPinStart[2] = 22;//festlegen des Output-Pins f�r Adresse2 und Bit0
	outPinStart[3] = 30;//festlegen des Output-Pins f�r Adresse3 und Bit0
	outPinStart[4] = 38;//festlegen des Output-Pins f�r Adresse4 und Bit0
	outPinStart[5] = 46;//festlegen des Output-Pins f�r Adresse5 und Bit0
	pinMode(18, OUTPUT);   pinMode(19, OUTPUT);
	RMBefehl[0] = ArdNr;
	RMBefehl[1] = 10;
	RMBefehl[5] = ArdNr;
	RMBefehl[6] = 11;

	//RMsendenSerial();
	RMBefehl[0]=ArdNr; RMBefehl[5]=ArdNr;
	RMBefehl[1]=10   ; RMBefehl[6]=11   ;
	//RM-Messwerte definieren
	for (int i = 0; i < 32; i++)
	{
		RMMesswerteAuswertung[i] = 255;
		for (int j = 0; j < 10; j++) { RMMessWert[i][j] = 255; }
	}
	//RMsendenSerial();
}
void loop(){
	delay(1);
	USBDatenEmpfang();
	RueckmeldungSlave();
	if (IntervallControl(RMZeitAktualisierungUSB))
	{
		RueckmeldungSlaveUSB();
		RMZeitAktualisierungUSB += RMUSBInterval;
	}
}//ende von loop

void RueckmeldungSlaveUSB() 
{
	//RMMesswerteAuswerten();
	Serial.write(RMMesswerteAuswertung, 32);
	Serial.write(RMSchwelle);

	Serial.write(RMBefehl[8]);//senden der DatenBytes
	Serial.write(RMBefehl[7]);
	Serial.write(RMBefehl[3]);
	Serial.write(RMBefehl[2]);

	Serial.write(RMSchwelle);

}
void RueckmeldungSlave()// vormals Rueckmeldung2
{ //aktuelle Version 20.10.2019
	int RMNr = 0;
	byte bitNr, byteNr;
	for (int w = 0; w<10; w++)//es werden alle Eing�nge 10x ausgemessen
	{
		for (int h = 0; h<2; h++)//beide Adressen der 16er RM-Gruppen
		{//Schleife f�r beide Adressen
			digitalWrite(18, HIGH);
			digitalWrite(19, HIGH);
			digitalWrite(18 + h, LOW);
			for (int i = 0; i<16; i++)//16 Analog-Eing�nge werden eingelesen
			{//Schleife f�r eine Adresse
				bitNr = i % 8;
				byteNr = (i / 8) + 2 + (5 * h);
				RMNr = (h * 16) + i;
				RMMessWert[RMNr][w] = analogRead(i) / 4;//Messwert wird auf durch Division durch 4 auf Byte-gr�sse reduziert
				RMMesswerteAuswertung[RMNr] = 255;
				for (int a = 0; a < 10; a++) {//auswerten der letzten 10 Messwerte eines RM
					if (RMMessWert[RMNr][a] < RMMesswerteAuswertung[RMNr]) 
					{ 
						RMMesswerteAuswertung[RMNr] = RMMessWert[RMNr][a]; 
					}
				}
				if(RMMesswerteAuswertung[RMNr]<RMSchwelle){ bitWrite(RMBefehl[byteNr], bitNr, 1); }
				else{ bitWrite(RMBefehl[byteNr], bitNr, 0); }
			}
		}
	}
	digitalWrite(18, HIGH);//R�ckmelder werden ausgeschalten
	digitalWrite(19, HIGH);//R�ckmelder werden ausgeschalten
}
void RueckmeldungSlaveNeu()// 
{ //neue Version, es soll der tiefste wert ausgeschlossen werden
	int RMNr = 0;
	byte bitNr, byteNr;
	for (int w = 0; w<10; w++)//es werden alle Eing�nge 10x ausgemessen
	{
		for (int h = 0; h<2; h++)//beide Adressen der 16er RM-Gruppen
		{//Schleife f�r beide Adressen
			digitalWrite(18, HIGH);
			digitalWrite(19, HIGH);
			digitalWrite(18 + h, LOW);
			for (int i = 0; i<16; i++)//16 Analog-Eing�nge werden eingelesen
			{//Schleife f�r eine Adresse
				bitNr = i % 8;
				byteNr = (i / 8) + 2 + (5 * h);
				RMNr = (h * 16) + i;
				RMMessWert[RMNr][w] = analogRead(i) / 4;//Messwert wird auf durch Division durch 4 auf Byte-gr�sse reduziert
				RMMesswerteAuswertung[RMNr] = 255;//es soll nur der zweittiefste Wert verwendet werden
				for (int a = 0; a < 10; a++) {//auswerten der letzten 10 Messwerte eines RM
					if (RMMessWert[RMNr][a] < RMTiefsteMesswerteAuswertung[RMNr]) { //RMMesswerteAuswertung[RMNr])					
						RMMesswerteAuswertung[RMNr] = RMTiefsteMesswerteAuswertung[RMNr];
						RMTiefsteMesswerteAuswertung[RMNr] = RMMessWert[RMNr][a];
						//RMMesswerteAuswertung[RMNr] = RMMessWert[RMNr][a];
					}
					else{
						if (RMMessWert[RMNr][a] < RMMesswerteAuswertung[RMNr]) {
							RMMesswerteAuswertung[RMNr] = RMMessWert[RMNr][a];
						}
					}
				}
				if (RMMesswerteAuswertung[RMNr]<RMSchwelle) { bitWrite(RMBefehl[byteNr], bitNr, 1); }
				else { bitWrite(RMBefehl[byteNr], bitNr, 0); }
			}
		}
	}
	digitalWrite(18, HIGH);//R�ckmelder werden ausgeschalten
	digitalWrite(19, HIGH);//R�ckmelder werden ausgeschalten
}
//void RMMesswerteAuswerten()//entfällt
//{//Bits in den SendeByts setzen
//	byte tiefstwert = 255;
//	for (int i = 0; i<8; i++)
//	{ //umsetzen der Messwerte in RM-Bytes
//		if (RMMesswerteAuswertung[i]<RMSchwelle)
//		{
//			bitWrite(RMBefehl[2], i, 1);
//		}
//		else
//		{
//			bitWrite(RMBefehl[2], i, 0);
//		}
//	}
//	tiefstwert = 255;
//	for (int i = 0; i<8; i++)
//	{ //umsetzen der Messwerte in RM-Bytes
//		if (RMMesswerteAuswertung[i + 8]<RMSchwelle)
//		{
//			bitWrite(RMBefehl[3], i, 1);
//		}
//		else { bitWrite(RMBefehl[3], i, 0); }
//	}
//	for (int i = 0; i<8; i++)
//	{ //umsetzen der Messwerte in RM-Bytes
//		if (RMMesswerteAuswertung[i + 16]<RMSchwelle)
//		{
//			bitWrite(RMBefehl[7], i, 1);
//		}
//		else { bitWrite(RMBefehl[7], i, 0); }
//	}
//	for (int i = 0; i<8; i++)
//	{ //umsetzen der Messwerte in RM-Bytes
//		if (RMMesswerteAuswertung[i + 24]<RMSchwelle)
//		{
//			bitWrite(RMBefehl[8], i, 1);
//		}
//		else { bitWrite(RMBefehl[8], i, 0); }
//	}
//}

inline void RMBefehlAktuell()
{ //berechnet die Kontrollbytes f�r die R�ckmeldung
	//RMBefehl[0] = ArdNr; RMBefehl[5] = ArdNr;
	//RMBefehl[1] = 10; RMBefehl[6] = 11;
	RMBefehl[4] = RMBefehl[0] + RMBefehl[1] + RMBefehl[2] + RMBefehl[3];
	RMBefehl[9] = RMBefehl[5] + RMBefehl[6] + RMBefehl[7] + RMBefehl[8];
}

void requestEvent()
{//EreignisMethode zur Datenabfrage
	RMBefehl[4] = RMBefehl[0] + RMBefehl[1] + RMBefehl[2] + RMBefehl[3];
	RMBefehl[9] = RMBefehl[5] + RMBefehl[6] + RMBefehl[7] + RMBefehl[8];
	Wire.write(RMBefehl, 10);
	//Serial.write(RMBefehl, 10);
}

void receiveEvent(int howMany)
{//EreignisMethode zum Daten empfangen vom Master-Arduino
 //Serial.write(111);
	while (0 < Wire.available())
	{
		eb = Wire.read();
		BefehlInputW[bzW] = eb;
		//Serial.write(bz);
		// Serial.write(eb);
		bzW++;
		if (bzW>4)
		{
			if (BefehlInputW[0] == ArdNr)
			{
				bzW = 0;
				BefehlsAusfuehrung(BefehlInputW);
			}
		}
	}
}
//void RMmessWertSendenSerial(){ for(int i=0;i<32 ;i++) {Serial.write( RMWert[i]); }}
void RMsendenI2C()
{//RM senden �ber I2C
 //noch nicht getestet
 /*byte WireBefehle[10];
 tz++;                     // as expected by master
 RMBefehl[0] = ArdNr;
 RMBefehl[1] = 10;
 RMBefehl[2] = tz;//RM[0];
 RMBefehl[3] = tz+1;//RM[1];
 RMBefehl[4] = controlByteBerechnen( RMBefehl0 );
 // Wire.write(RMBefehl0,5);

 RMBefehl[5] = ArdNr;
 RMBefehl[6] = 11;
 RMBefehl[7] =tz+2;// RM[2];
 RMBefehl[8] =tz+3;// RM[3];
 RMBefehl[9] = controlByteBerechnen( RMBefehl1 );
 // Wire.write(RMBefehl,10);
 RMBefehlAktuell();//aktualisiert die Kontrollbyts
 Serial.write(RMBefehl,10);
 WireBefehle[0]=ArdNr        ;WireBefehle[5]=ArdNr;
 WireBefehle[1]= 10          ;WireBefehle[6]= 11  ;
 WireBefehle[2]= RMBefehl0[2];WireBefehle[7]= RMBefehl1[2];
 WireBefehle[3]= RMBefehl0[3];WireBefehle[8]= RMBefehl1[3];
 WireBefehle[4]= RMBefehl0[4];WireBefehle[9]= RMBefehl1[4];
 Wire.write(WireBefehle,10);*/
}

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
			if (controlBefehl(USBBefehlEingang))//wenn die Pr�fsumme stimmt
			{

				if (USBBefehlEingang[0] == ArdNr)
				{
					BefehlsAusfuehrung(USBBefehlEingang);
					// Serial.write(12);
				}
				if (USBBefehlEingang[0]>1)
				{//Befehl an EINEN Slave-Arduino
				 /*Wire.beginTransmission(USBBefehlEingang[0]);
				 Wire.write(USBBefehlEingang, 5);
				 Wire.endTransmission();*/
				}
				if (USBBefehlEingang[0] == 0)
				{//Befehl an alle Arduinos
				 // es mussen noch die Kontroll-Bytes korrigiert werden
					USBBefehlEingang[0] = 1;
					BefehlsAusfuehrung(USBBefehlEingang);
					/*for (int i = 0; i<16; i++)
					{
					if (bitRead(SlaveAktivListe, i))
					{
					USBBefehlEingang[0] = i + 2;
					Wire.beginTransmission(USBBefehlEingang[0]);
					Wire.write(USBBefehlEingang, 5);
					Wire.endTransmission();
					}
					}*/
				}
				USBDatenEingangByteZahler = 0;
			}
		}
	}
}



void RMsendenSerial()
{	
	for (int i = 0; i < 32; i++)
	{
		byte MW = 255;
		for (int h = 0; h < 10; h++)
		{//ermitteln des kleinsten Messwerts
			if (RMMessWert[i][h] < MW) { MW = RMMessWert[i][h]; }	    
	    }
		Serial.write(MW);
	}
	Serial.write(RMSchwelle);	
	Serial.write(RMBefehl[8]);
	Serial.write(RMBefehl[7]);
	Serial.write(RMBefehl[3]);
	Serial.write(RMBefehl[2]);
	Serial.write(RMSchwelle);
}

void ByteToRelais(byte datenByte, int startPin)
{
	datenByte = ~datenByte;
	for (int i = 0; i<8; i++) { digitalWrite(i + startPin, bitRead(datenByte, i)); }
}

void IntToRelais(int datenInt, int startPin)
{
	for (int i = 0; i<16; i++) { digitalWrite(i + startPin, bitRead(datenInt, i)); }
}

void BefehlsAusfuehrung(byte Befehl[5])
{
	//Serial.write(255);
	// Serial.write(BefehlInput[1]);
	int a = 0;
	switch (Befehl[1])
	{
	case 3:
	{
		//intervalImpuls = Befehl[2] * 100;
	}
	case 14:
	{
		RMSchwelle = Befehl[3];
		//RMinterval = 100 * Befehl[2];
	}
	case 20://permanentOutput
	{
		ByteToRelais(Befehl[3], outPinStart[Befehl[2]]);
		//BefehlZurueckSenden();
		//Serial.write(255);
		//Serial.write(BefehlInput[1]);
		//a = 30 + (8*BefehlInput[2]);
		// for(int i=0 ; i<8 ; i++ ){digitalWrite( i+pinstart, bitRead(BefehlInput[3],i) );}
		break;
	}
	case 30:
	{
		/*    //a = 6 + (8*BefehlInput[2]);
		// for(int i=0 ; i<8 ; i++ ){digitalWrite( i+pinstart, bitRead(BefehlInput[3],i) );}
		timeEnd[Befehl[2]] = intervalImpuls + millis();
		a = 3 + BefehlInput[2];
		digitalWrite(a,LOW);*/
		break;
	}
	case 40:
	{//PermanentOutput 16Bit auf Adresse0
		ByteToRelais(Befehl[2], outPinStart[0]);
		ByteToRelais(Befehl[3], outPinStart[1]);
		break;
	}
	case 41:
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
//void resetImpuls()//
//{  unsigned long time= millis();//Zeitnahme
//  for(int i=0 ; i<3;i++)
//  {
//    if(time > timeEnd[i]){ digitalWrite(3+i,HIGH);  } 
//  }
//}
//byte controlByteBerechnen(byte Befehl[5]){return  Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3];}
byte controlByteBerechnen(byte Bf[5]) //berechnet das Kontroll-Byte
{
	return  Bf[0] + Bf[1] + Bf[2] + Bf[3];
}

boolean controlBefehl(byte Befehl[5])
{  //kontrolliert die Pr�fsumme
	byte cb = 0;//ControlByte
	cb = Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3];
	if (cb == Befehl[4]) { return true; }
	else { return false; }
}

boolean IntervallControl(unsigned long Zeit)
{//getestet
 //pr�ft ob die "Zeit" erreicht wurde
 //liefert true wenn die aktuelle Zeit 
 //gr�sser ist als die Variable "Zeit"
	if (millis()> Zeit) { return true; }
	else { return false; }
}


void zeitMessung(unsigned long startZeit)
{//berechnet die vergangene Zeit seit der "Startzeit" und sendet das Ergebnis �ber USB
	int zeitDiff = millis() - startZeit;
	BefehlOutputS[0] = ArdNr;//Arduino Nr.
	BefehlOutputS[1] = 50;//Zeitmessung
	BefehlOutputS[2] = highByte(zeitDiff);
	BefehlOutputS[3] = lowByte(zeitDiff);
	BefehlOutputS[4] = controlByteBerechnen(BefehlOutputS);
	Serial.write(BefehlOutputS, 5);
}
