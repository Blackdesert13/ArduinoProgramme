/*
 Name:		HeinersgruenSlave5.ino
 Author:	Robert Muh
*/

#include <Servo.h>
#include <Wire.h>

#define ARDUINO2

#ifdef  ARDUINO2
const byte ArdNr = 2;
#define ausg_5255 1
#define ausg_5252 0
#define ausg_5254 2
#define ausg_5202 3
#define ausg_5251 6
#define ausg_5201 7 
#endif
#ifdef  ARDUINO3
const byte ArdNr = 3;
#define ausg_5308 0
#define ausg_5301 1
#define ausg_5374 2
#define ausg_5371 3
#define ausg_5369 4
#define ausg_5365 5
#define ausg_5901 6
#define ausg_5362 7
#define ausg_5361 8
#define ausg_5253 9
#endif
#ifdef  ARDUINO4
const byte ArdNr = 4;
#define ausg_5372 0
#define ausg_5370 1
#define ausg_5367 2
#define ausg_5375 3
#define ausg_5303 5
#define ausg_5205 6
#define ausg_5310 7
#define ausg_5302 8
#define ausg_5313 9
#define ausg_5304 10
#endif
#ifdef  ARDUINO5
const byte ArdNr = 5;
#define ausg_5206 0
#define ausg_5203 1
#define ausg_5207 2
#define ausg_5311 3
#define ausg_5204 4
#define ausg_5314 8
#define ausg_5208 9
#define ausg_5312 10
#define ausg_5902 11
#endif
#ifdef  ARDUINO6
const byte ArdNr = 6;
#define ausg_5261 0
#define ausg_5258 1
#define ausg_5364 2
#define ausg_5257 3
#define ausg_5363 4
#define ausg_5366 5
#define ausg_5368 6
#define ausg_5259 7
#define ausg_5260 8
#define ausg_5373 9
#define ausg_5256 10
#endif
#ifdef  ARDUINO7
const byte ArdNr = 7;
#define ausg_5211 2
#define ausg_5306 3
#define ausg_5212 4
#define ausg_5213 5
#define ausg_5209 6
#define ausg_5307 7
#define ausg_5315 8
#define ausg_5210 9
#define ausg_5305 10
#define ausg_5309 11
#endif



#define ServoAbschaltzeit (unsigned long)1000 //millisekunden, Zeit bis zum Abschalten nach dem letzten schalten
#define ServoNachstellenZeitintervall (unsigned long)2500 //in millisekunden, Zeitintervall nach dem der nächste Servo nachgestellt wird
#define ServoNachstellenAbschaltzeit (unsigned long)500 //in millisekunden, Zeit zum Abschalten der Servos beim Nachstellen


const int AnzahlServos = 12;

struct Servo_t {
	int pin = 255;
	Servo servo;
	bool manuell = false;
	int winkelStellung1 = 135; //größerer Winkel
	int winkelStellung0 = 45; // kleinerer Winkel
	unsigned long letzteZeit = 0;
	unsigned long warteZeit = 0;
	int stellung;
	int zielstellung = 45;
};
Servo_t servos[AnzahlServos];
int aktuellerServo = 0;
unsigned long ServoNachstellenLetzteZeit = 0;

int ByteZaehlerUSBEingang = 0;
int USBDatenEingangByteZahler;
byte eb = 0;//Eingangsbyte
byte bzS = 0;//ByteZ�hler �ber Serial
byte bzW = 0;//ByteZ�hler �ber I2C
byte BefehlInputS[5];//Befehl �ber Serial
byte BefehlInputW[5];//Befehl �ber I2C
byte BefehlOutputS[5];//Befehl �ber Serial
byte BefehlOutputW[5];//Befehl �ber I2C
byte USBBefehlEingang[5];

//Ausgänge
byte outPinStart[2];


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	Wire.begin(ArdNr);//(ArdNr);       receiveEvent        
	Wire.onReceive(receiveEvent); // register event
	//Wire.onRequest(requestEvent); //EreignisMethode zur Datenabfrage
	
	outPinStart[0] = 2;//festlegen des Output-Pins für Adresse0 LOWByte und Bit0
	outPinStart[1] = 10;//festlegen des Output-Pins für Adresse0 HIGHByte und Bit0
	Definition();
}

// the loop function runs over and over again until power down or reset
void loop() {
	USBDatenEmpfang();
	ServoBewegung();
	//Rueckmeldung();
	// RMvonSlaves();
	// RMvonSlavesNeu();
	// SlaveAktivAbfrage();
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

void ServoBewegung() {
	for (int i = 0; i < AnzahlServos; i++) {
		if (servos[i].pin == 255) {
			continue;
		}
		unsigned long zeit = millis();
		if (servos[i].stellung != servos[i].zielstellung) {
			//Serial.write(i);
			if (servos[i].warteZeit != 0) {
				if (zeit - servos[i].letzteZeit >= servos[i].warteZeit) {
					servos[i].letzteZeit = zeit;
					if (servos[i].zielstellung > servos[i].stellung) {
						servos[i].stellung++;
					}
					else {
						servos[i].stellung--;
					}
				}
			}
			else {
				servos[i].stellung = servos[i].zielstellung;
				servos[i].letzteZeit = zeit;
			}

			if (!servos[i].servo.attached()) {
				//Serial.print("Attach ");
				//Serial.println(i);
				servos[i].servo.attach(servos[i].pin);
			}
			servos[i].servo.write(servos[i].stellung);
		}
		else {
			if (servos[i].servo.attached()) {
				if (((zeit - servos[i].letzteZeit) >= ServoAbschaltzeit)
					&& ((zeit - ServoNachstellenLetzteZeit) >= ServoNachstellenAbschaltzeit)) {
					servos[i].servo.detach();
					//Serial.print("Detach ");
					//Serial.println(i);
				}
			}
		}
	}
	unsigned long zeit = millis();
	if (zeit - ServoNachstellenLetzteZeit >= ServoNachstellenZeitintervall) {
		ServoNachstellenLetzteZeit = zeit;
		servos[aktuellerServo].servo.attach(servos[aktuellerServo].pin);
		aktuellerServo = (aktuellerServo + 1) % AnzahlServos;
	}

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
				else if (USBBefehlEingang[0] == 0)
				{//Befehl an alle Arduinos
				 // es mussen noch die Kontroll-Bytes korrigiert werden
					USBBefehlEingang[0] = 1;
					BefehlsAusfuehrung(USBBefehlEingang);
				}
				USBDatenEingangByteZahler = 0;
			}
		}
	}
}

boolean controlBefehl(byte Befehl[5]) {
	//kontrolliert die Prüfsumme
	byte cb = Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3];
	return cb == Befehl[4];
}

void BefehlsAusfuehrung(byte Befehl[5])
{
	byte servoPin;
	switch (Befehl[1])
	{
	case 1:
	{//Slave abfrage einfügen
	 //SlaveAktivAbfrage();
	 //Serial.write(98);
		break;
	}
	case 2:
	{
		//SlaveAktivListe = 0;
		break;
	}
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
		ByteToServo(Befehl[2], outPinStart[0]);
		ByteToServo(Befehl[3], outPinStart[1]);
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
	case 70:
		servoPin = (byte)(Befehl[2] & 0b00111111);
		switch (Befehl[2] >> 6) {
		case 0:
			if (Befehl[3] & 0b00000001) {
				servos[servoPin].manuell = true;
			}
			else {
				servos[servoPin].manuell = false;
			}
			break;
		case 1:
			servos[servoPin].warteZeit = Befehl[3];
			break;
		case 2:
			break;
		case 3:
			break;
		default:
			break;
		}
		break;
	case 71:
		servoPin = (byte)(Befehl[2] & 0b00111111);
		if ((Befehl[2] & 0b10000000) != 0) {
			servos[servoPin].winkelStellung1 = Befehl[3];
		}
		else {
			servos[servoPin].winkelStellung0 = Befehl[3];
		}
		break;
	case 72:
		servoPin = (byte)(Befehl[2] & 0b00111111);
		switch (Befehl[2] >> 6) {
		case 0:
			//Servo Winkel manuell einstellen
			ServoManuellSteuern(Befehl[2] & 0b00111111, Befehl[3]);
			break;
		case 1:
			if (Befehl[3] == 0) {
				ServoManuellSteuern(servoPin, servos[servoPin].stellung);
			}
			else if (Befehl[3] > 127) {
				ServoManuellSteuern(servoPin, servos[servoPin].winkelStellung1);
			}
			else {
				ServoManuellSteuern(servoPin, servos[servoPin].winkelStellung0);
			}
			break;
		case 2:
			int winkel = servos[servoPin].stellung;

			winkel += (char)Befehl[3];
			Serial.write(winkel);
			ServoManuellSteuern(servoPin, winkel);
			break;
		}
		break;
	case 73:
		servoPin = (byte)(Befehl[2] & 0b00111111);
		//Servo Winkel manuell einstellen
		if (servoPin < 12) {
			if (servos[servoPin].winkelStellung0 < servos[servoPin].winkelStellung1) {
				ServoManuellSteuern(servoPin,
					(unsigned int)map(Befehl[3], 0, 255, servos[servoPin].winkelStellung0, servos[servoPin].winkelStellung1)
					);
			}
			else{
				ServoManuellSteuern(servoPin,
					(unsigned int)map(Befehl[3], 255, 0, servos[servoPin].winkelStellung1, servos[servoPin].winkelStellung0)
					);
			}
		}
		break;
	}
}

void ByteToRelais(byte datenByte, int startPin) {
	datenByte = ~datenByte;//datenByte muß invertiert werden weil die Relaisplatinen low-aktiv sind!
	for (int i = 0; i < 8; i++) {
		digitalWrite(i + startPin, bitRead(datenByte, i));
	}
}

void ByteToServo(byte datenByte, int startPin) {
	startPin -= 2;
	int j = 0;
	for (int i = 0; i < 8; i++) {
		j = startPin + i;
		if (servos[j].manuell)
			continue; //Beendet aktuellen Schleifendurchlauf, Sprung in nächste Iteration
		if (bitRead(datenByte, i)) {
			servos[j].zielstellung = servos[j].winkelStellung1;
		}
		else {
			servos[j].zielstellung = servos[j].winkelStellung0;
		}
	}
}

// pin = Platinen-Nr.*16 + Bit-Nr.(1.Byte 0-7, 2.Byte 8-15)
void ServoManuellSteuern(byte pin, int winkel) {
	if (pin < 12) {
		if (servos[pin].manuell) {
			if (servos[pin].winkelStellung0 <= servos[pin].winkelStellung1) {
				servos[pin].zielstellung = constrain(winkel, servos[pin].winkelStellung0, servos[pin].winkelStellung1);
			}
			else {
				servos[pin].zielstellung = constrain(winkel, servos[pin].winkelStellung1, servos[pin].winkelStellung0);
			}
		}
	}
}

void Definition() {
#ifdef  ARDUINO2
	servos[ausg_5255].winkelStellung0 = 135;
	servos[ausg_5255].winkelStellung1 = 45;
	servos[ausg_5255].zielstellung = servos[ausg_5255].winkelStellung0;
	servos[ausg_5252].winkelStellung0 = 135;
	servos[ausg_5252].winkelStellung1 = 45;
	servos[ausg_5252].zielstellung = servos[ausg_5252].winkelStellung0;
	servos[ausg_5254].winkelStellung0 = 135;
	servos[ausg_5254].winkelStellung1 = 45;
	servos[ausg_5254].zielstellung = servos[ausg_5254].winkelStellung0;
	servos[ausg_5202].winkelStellung0 = 135;
	servos[ausg_5202].winkelStellung1 = 45;
	servos[ausg_5202].zielstellung = servos[ausg_5202].winkelStellung0;
	servos[ausg_5251].winkelStellung0 = 135;
	servos[ausg_5251].winkelStellung1 = 45;
	servos[ausg_5251].zielstellung = servos[ausg_5251].winkelStellung0;
	servos[ausg_5201].winkelStellung0 = 135;	
	servos[ausg_5201].winkelStellung1 = 45;
	servos[ausg_5201].zielstellung = servos[ausg_5201].winkelStellung0;
#endif //ARDUINO2
#ifdef  ARDUINO3
	servos[ausg_5308].winkelStellung0 = 90;
	servos[ausg_5308].winkelStellung1 = 150;
	servos[ausg_5308].zielstellung = servos[ausg_5308].winkelStellung0;
	servos[ausg_5301].winkelStellung0 = 90;
	servos[ausg_5301].winkelStellung1 = 135;
	servos[ausg_5301].zielstellung = servos[ausg_5301].winkelStellung0;
	servos[ausg_5374].manuell = true;
	servos[ausg_5374].winkelStellung0 = 135;
	servos[ausg_5374].winkelStellung1 = 90;
	servos[ausg_5374].zielstellung = servos[ausg_5374].winkelStellung1;
	servos[ausg_5371].manuell = true;
	servos[ausg_5371].winkelStellung0 = 145;
	servos[ausg_5371].winkelStellung1 = 80;
	servos[ausg_5371].zielstellung = servos[ausg_5371].winkelStellung1;
	servos[ausg_5369].manuell = true;
	servos[ausg_5369].winkelStellung0 = 135;
	servos[ausg_5369].winkelStellung1 = 60;
	servos[ausg_5369].zielstellung = servos[ausg_5369].winkelStellung1;
	servos[ausg_5365].manuell = true;
	servos[ausg_5365].winkelStellung0 = 135;
	servos[ausg_5365].winkelStellung1 = 75;
	servos[ausg_5365].zielstellung = servos[ausg_5365].winkelStellung1;
	servos[ausg_5901].manuell = true;
	servos[ausg_5901].winkelStellung0 = 90;
	servos[ausg_5901].winkelStellung1 = 135;
	servos[ausg_5901].zielstellung = servos[ausg_5901].winkelStellung1;
	servos[ausg_5362].manuell = true;
	servos[ausg_5362].winkelStellung0 = 135;
	servos[ausg_5362].winkelStellung1 = 85;
	servos[ausg_5362].zielstellung = servos[ausg_5362].winkelStellung1;
	servos[ausg_5361].manuell = true;
	servos[ausg_5361].winkelStellung0 = 135;
	servos[ausg_5361].winkelStellung1 = 45;
	servos[ausg_5361].zielstellung = servos[ausg_5361].winkelStellung1;
	servos[ausg_5253].winkelStellung0 = 135;
	servos[ausg_5253].winkelStellung1 = 45;
	servos[ausg_5253].zielstellung = servos[ausg_5253].winkelStellung0;
#endif //ARDUINO3
#ifdef  ARDUINO4
	servos[ausg_5372].manuell = true;
	servos[ausg_5372].winkelStellung0 = 150;
	servos[ausg_5372].winkelStellung1 = 45;
	servos[ausg_5372].zielstellung = servos[ausg_5372].winkelStellung1;
	servos[ausg_5370].manuell = true;
	servos[ausg_5370].winkelStellung0 = 160;
	servos[ausg_5370].winkelStellung1 = 75;
	servos[ausg_5370].zielstellung = servos[ausg_5370].winkelStellung1;
	servos[ausg_5367].manuell = true;
	servos[ausg_5367].winkelStellung0 = 135;
	servos[ausg_5367].winkelStellung1 = 45;
	servos[ausg_5367].zielstellung = servos[ausg_5367].winkelStellung1;
	servos[ausg_5375].manuell = true;
	servos[ausg_5375].winkelStellung0 = 135;
	servos[ausg_5375].winkelStellung1 = 45;
	servos[ausg_5375].zielstellung = servos[ausg_5375].winkelStellung1;
	servos[ausg_5303].winkelStellung0 = 45;
	servos[ausg_5303].winkelStellung1 = 135;
	servos[ausg_5303].zielstellung = servos[ausg_5303].winkelStellung0;
	servos[ausg_5205].winkelStellung0 = 135;
	servos[ausg_5205].winkelStellung1 = 45;
	servos[ausg_5205].zielstellung = servos[ausg_5205].winkelStellung0;
	servos[ausg_5310].winkelStellung0 = 45;
	servos[ausg_5310].winkelStellung1 = 135;
	servos[ausg_5310].zielstellung = servos[ausg_5310].winkelStellung0;
	servos[ausg_5302].winkelStellung0 = 70;
	servos[ausg_5302].winkelStellung1 = 135;
	servos[ausg_5302].zielstellung = servos[ausg_5302].winkelStellung0;
	servos[ausg_5313].winkelStellung0 = 60;
	servos[ausg_5313].winkelStellung1 = 150;
	servos[ausg_5313].zielstellung = servos[ausg_5313].winkelStellung0;
	servos[ausg_5304].winkelStellung0 = 45;
	servos[ausg_5304].winkelStellung1 = 135;
	servos[ausg_5304].zielstellung = servos[ausg_5304].winkelStellung0;
#endif //ARDUINO4
#ifdef  ARDUINO5
	servos[ausg_5902].manuell = true;
	servos[ausg_5902].winkelStellung0 = 45;
	servos[ausg_5902].winkelStellung1 = 135;
	servos[ausg_5902].zielstellung = servos[ausg_5902].winkelStellung1;
	servos[ausg_5206].winkelStellung0 = 125;
	servos[ausg_5206].winkelStellung1 = 70;
	servos[ausg_5206].zielstellung = servos[ausg_5206].winkelStellung0;
	servos[ausg_5203].winkelStellung0 = 55;
	servos[ausg_5203].winkelStellung1 = 135;
	servos[ausg_5203].zielstellung = servos[ausg_5203].winkelStellung0;
	servos[ausg_5207].winkelStellung0 = 140;
	servos[ausg_5207].winkelStellung1 = 45;
	servos[ausg_5207].zielstellung = servos[ausg_5207].winkelStellung0;
	servos[ausg_5311].winkelStellung0 = 45;
	servos[ausg_5311].winkelStellung1 = 135;
	servos[ausg_5311].zielstellung = servos[ausg_5311].winkelStellung0;
	servos[ausg_5204].winkelStellung0 = 55;
	servos[ausg_5204].winkelStellung1 = 145;
	servos[ausg_5204].zielstellung = servos[ausg_5204].winkelStellung0;
	servos[ausg_5314].winkelStellung0 = 45;
	servos[ausg_5314].winkelStellung1 = 135;
	servos[ausg_5314].zielstellung = servos[ausg_5314].winkelStellung0;
	servos[ausg_5208].winkelStellung0 = 135;
	servos[ausg_5208].winkelStellung1 = 45;
	servos[ausg_5208].zielstellung = servos[ausg_5208].winkelStellung0;
	servos[ausg_5312].winkelStellung0 = 45;
	servos[ausg_5312].winkelStellung1 = 135;
	servos[ausg_5312].zielstellung = servos[ausg_5312].winkelStellung0;
#endif //ARDUINO5
#ifdef  ARDUINO6
	servos[ausg_5261].winkelStellung0 = 45;
	servos[ausg_5261].winkelStellung1 = 135;
	servos[ausg_5261].zielstellung = servos[ausg_5261].winkelStellung0;
	servos[ausg_5258].winkelStellung0 = 45;
	servos[ausg_5258].winkelStellung1 = 135;
	servos[ausg_5258].zielstellung = servos[ausg_5258].winkelStellung0;
	servos[ausg_5364].manuell = true;
	servos[ausg_5364].winkelStellung0 = 125;
	servos[ausg_5364].winkelStellung1 = 45;
	servos[ausg_5364].zielstellung = servos[ausg_5364].winkelStellung1;
	servos[ausg_5257].winkelStellung0 = 45;
	servos[ausg_5257].winkelStellung1 = 145;
	servos[ausg_5257].zielstellung = servos[ausg_5257].winkelStellung0;
	servos[ausg_5363].manuell = true;
	servos[ausg_5363].winkelStellung0 = 145;
	servos[ausg_5363].winkelStellung1 = 60;
	servos[ausg_5363].zielstellung = servos[ausg_5363].winkelStellung1;
	servos[ausg_5366].manuell = true;
	servos[ausg_5366].winkelStellung0 = 145;
	servos[ausg_5366].winkelStellung1 = 60;
	servos[ausg_5366].zielstellung = servos[ausg_5366].winkelStellung1;
	servos[ausg_5368].manuell = true;
	servos[ausg_5368].winkelStellung0 = 150;
	servos[ausg_5368].winkelStellung1 = 60;
	servos[ausg_5368].zielstellung = servos[ausg_5368].winkelStellung1;
	servos[ausg_5259].winkelStellung0 = 135;
	servos[ausg_5259].winkelStellung1 = 45;
	servos[ausg_5259].zielstellung = servos[ausg_5259].winkelStellung0;
	servos[ausg_5260].winkelStellung0 = 45;
	servos[ausg_5260].winkelStellung1 = 135;
	servos[ausg_5260].zielstellung = servos[ausg_5260].winkelStellung0;
	servos[ausg_5373].manuell = true;
	servos[ausg_5373].winkelStellung0 = 150;
	servos[ausg_5373].winkelStellung1 = 60;
	servos[ausg_5373].zielstellung = servos[ausg_5373].winkelStellung1;
	servos[ausg_5256].winkelStellung0 = 40;
	servos[ausg_5256].winkelStellung1 = 170;
	servos[ausg_5256].zielstellung = servos[ausg_5256].winkelStellung0;
#endif //ARDUINO6
#ifdef  ARDUINO7
	servos[ausg_5211].winkelStellung0 = 45;
	servos[ausg_5211].winkelStellung1 = 135;
	servos[ausg_5211].zielstellung = servos[ausg_5211].winkelStellung0;
	servos[ausg_5306].winkelStellung0 = 45;
	servos[ausg_5306].winkelStellung1 = 135;
	servos[ausg_5306].zielstellung = servos[ausg_5306].winkelStellung0;
	servos[ausg_5212].winkelStellung0 = 35;
	servos[ausg_5212].winkelStellung1 = 135;
	servos[ausg_5212].zielstellung = servos[ausg_5212].winkelStellung0;
	servos[ausg_5213].winkelStellung0 = 45;
	servos[ausg_5213].winkelStellung1 = 135;
	servos[ausg_5213].zielstellung = servos[ausg_5213].winkelStellung0;
	servos[ausg_5209].winkelStellung0 = 135;
	servos[ausg_5209].winkelStellung1 = 45;
	servos[ausg_5209].zielstellung = servos[ausg_5209].winkelStellung0;
	servos[ausg_5307].winkelStellung0 = 45;
	servos[ausg_5307].winkelStellung1 = 135;
	servos[ausg_5307].zielstellung = servos[ausg_5307].winkelStellung0;
	servos[ausg_5315].winkelStellung0 = 45;
	servos[ausg_5315].winkelStellung1 = 135;
	servos[ausg_5315].zielstellung = servos[ausg_5315].winkelStellung0;
	servos[ausg_5210].winkelStellung0 = 135;
	servos[ausg_5210].winkelStellung1 = 45;
	servos[ausg_5210].zielstellung = servos[ausg_5210].winkelStellung0;
	servos[ausg_5305].winkelStellung0 = 45;
	servos[ausg_5305].winkelStellung1 = 135;
	servos[ausg_5305].zielstellung = servos[ausg_5305].winkelStellung0;
	servos[ausg_5309].winkelStellung0 = 65;
	servos[ausg_5309].winkelStellung1 = 155;
	servos[ausg_5309].zielstellung = servos[ausg_5309].winkelStellung0;
#endif //ARDUINO7

	for (int i = 0; i < AnzahlServos; i++) {
		servos[i].pin = (i + 2);
		if (servos[i].pin == 255) {
			continue;
		}
		//servos[i].zielstellung = servos[i].winkelStellung0;
		servos[i].stellung = servos[i].zielstellung;
		servos[i].servo.write(servos[i].stellung);

		//servos[i].servo.attach(servos[i].pin, 1000, 2000);
		servos[i].servo.attach(servos[i].pin);
		delay(500);
		servos[0].servo.detach();
		//ByteToServo(0, 2);
		//ByteToServo(0, 10);
		//delay(500);
	}

}

