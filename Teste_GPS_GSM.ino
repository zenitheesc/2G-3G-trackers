#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

// Connect the GPS Power pin to 5V
// Connect the GPS Ground pin to ground
// Connect the GPS TX (transmit) pin to Digital 5
// Connect the GPS RX (receive) pin to Digital 4

// you can change the pin numbers to match your wiring:
SoftwareSerial mySerial(4, 5); // RX, TX  (GPS)
SoftwareSerial serialGSM(6, 7); // RX, TX   (SIM800L)
Adafruit_GPS GPS(&mySerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  true

String telefoneSMS = "+5514997272290";

void enviaSMS(String telefone);
void configuraGSM();

void setup()
{
  serialGSM.begin(9600);
  configuraGSM();
  delay(5000);
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);


  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time

  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
}

uint32_t timer = millis();
void loop()                     // run over and over again
{
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  //if ((c) && (GPSECHO))
   // Serial.write(c);

  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false

    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  if (millis() - timer > 300000) {  //Tempo entre cada envio 5 minutos
    timer = millis(); // reset the timer

    serialGSM.print("AT+CFUN=1\n");   //Inicia modo de funcionamento completo
    delay(10000);  //tempo máximo é de 10s

    serialGSM.print("AT+CMGS=\"" + telefoneSMS + "\"\n");
    delay(1000);

    serialGSM.print("Time: ");
    if (GPS.hour < 10) { serialGSM.print('0'); }
    serialGSM.print((GPS.hour - 3), DEC); serialGSM.print(':');
    if (GPS.minute < 10) { serialGSM.print('0'); }
    serialGSM.print(GPS.minute, DEC); serialGSM.print(':');
    if (GPS.seconds < 10) { serialGSM.print('0'); }
    serialGSM.print(GPS.seconds, DEC); 
    
    serialGSM.print("\nData: ");
    serialGSM.print(GPS.day, DEC); serialGSM.print('/');
    serialGSM.print(GPS.month, DEC); serialGSM.print("/20");
    serialGSM.println(GPS.year, DEC);
    serialGSM.print("Fix: "); serialGSM.print((int)GPS.fix);
    serialGSM.print(" quality: "); serialGSM.println((int)GPS.fixquality);
    if (GPS.fix) {
      serialGSM.print("Latitude: ");
      serialGSM.print(GPS.latitude, 4); serialGSM.println(GPS.lat);
      serialGSM.print("Longitude: ");
      serialGSM.print(GPS.longitude, 4); serialGSM.println(GPS.lon);

      serialGSM.print("Altitude: "); serialGSM.println(GPS.altitude);
      serialGSM.print("Velocidade (km/h): "); serialGSM.println(1.852*(GPS.speed));
      serialGSM.print("Satélites: "); serialGSM.println((int)GPS.satellites);
      serialGSM.print("Antenna status: "); serialGSM.println((int)GPS.antenna);
    }
    //serialGSM.print((char)27);  //esc, não envia mensagem
    serialGSM.print((char)26);  //ctrl-Z, envia mensagem

    delay(30000); //Delay para enviar mensagem, max de 60s
    serialGSM.print("AT+CFUN=0\n"); //Inicia mode de minimo funcionamento
  }
}


void configuraGSM() {   //Função para configurar módulo GSM
   serialGSM.print("AT+IPR=9600\n");
   serialGSM.print("AT+CMGF=1\n");
   serialGSM.print("AT+CSCS=\"GSM\"\n");
   serialGSM.print("AT+CNMI=2,2,0,0,0\n"); 
   serialGSM.print("AT+COLP=1\n");
   serialGSM.print("AT+CMEE=1\n");
}
