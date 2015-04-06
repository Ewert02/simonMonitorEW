#include <stdlib.h>
#include <SPI.h>
#include <Ethernet.h>
#include <StopWatch.h>
#include <LiquidCrystal.h>
#include <Event.h>
#include <Timer.h>

#define ZOLD 40
#define PIROS 42
#define SARGA 26

#define BUTTONPIN 20
#define LED 8

String UUID="dl101";

///Ethernet settings start
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };    /// MAC addr. of arduino

IPAddress ip(192, 168, 111, 107);                      /// IP addr. of arduino
IPAddress dnServer(192, 168, 100, 4);                 /// DNS server addr.
IPAddress gateway(192, 168, 100, 253);  		/// Gateway addr.
IPAddress subnet(255, 255, 0, 0);                     /// Subnet

IPAddress remote(10, 20, 0, 48);      ///remote logging server address

EthernetClient client;                /// new ethernet conn. handler

/// Ethernet settings end
float vakumMinutes=0.1; ///<-----set vacuum starting minutes (vM*60*1000 ms)

//int BUTTONPIN=20;      // interrupt 3
//int led = 8;           // the pin that the LED is attached to
int poti=A0;          // Potmeter pin

unsigned long timerEnd, timerStart, timeP;	/// StopWatch timecounters

int state;

LiquidCrystal lcd(6, 7, 5, 4, 3, 2);
StopWatch MySW(StopWatch::MILLIS);

Timer t;    /// Event (vakum) timer

float GetPotiallas(){  /// Get the current voltage by the potmeter
  float potiallasa;
  potiallasa=analogRead(poti)/4;
    if(potiallasa>254){
      potiallasa=254;  
    }else if(potiallasa<6){
      potiallasa=6;
    }
   potiallasa=(potiallasa/255)*5;
   return potiallasa;
}

void setup() {
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);		/// Start ethernet connection
  t.every(vakumMinutes*60*1000,vakum);			/// Trigger vaccuum every x (vakumMinutes) minute
  
  pinMode(BUTTONPIN, INPUT);
  pinMode(poti,INPUT);
  pinMode(LED,OUTPUT);
  pinMode(SARGA,OUTPUT);
 
  attachInterrupt(3,timeCounter,CHANGE);  /// Vakum jel figyeles

  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("PLC test");
  lcd.setCursor(5,1);
  
  
  //potiallas=GetPotiallas();
  lcd.print(GetPotiallas());
  lcd.print(" V");

  Serial.begin(9600);
  //char buffera[25];
}

void logData(float vala, int timePa){
  if (client.connect(remote, 80)) {		/// Connect to datalogging server on HTTP 80
      Serial.println("connected");
      //val=dtostrf(potiallas, 3,2,buffera);	/// Cast float (potiallas) into string (val) to we can send it over GET HTTP
      client.println("GET /monitorEW/log.php?gep="+String(UUID)+"&val="+vala+"&time="+String(timePa)+" HTTP/1.1");	/// Open log.php via GET val -> resistor val | time ->running time 
      client.println("Host: 10.20.0.48");		/// Host of the server
      client.println("Connection: close");
      client.println();
      client.stop();				/// Stop client to be able to start another one
  }else{
      Serial.println("Conn NOK");
  }
  
  lcd.clear();
  lcd.setCursor(5,0);
  
  lcd.print(timePa);
  lcd.print("ms");
  
  lcd.setCursor(5,1);  
    
  lcd.print(vala);
  lcd.print(" V");

  Serial.print(vala);
  Serial.print("V | ");
  Serial.println(timePa);
}

void changeState(){
  if(state==1){
    digitalWrite(PIROS,HIGH);
    digitalWrite(ZOLD,LOW);
  }else if(state==0){
    digitalWrite(PIROS,LOW);
    digitalWrite(ZOLD,HIGH);
  }
}

void loop() {
  analogWrite(LED,(analogRead(poti))/4);
  t.update();
  changeState();
}

void timeCounter(){
    if(MySW.isRunning()==0&&digitalRead(BUTTONPIN)==HIGH){
      lcd.clear();
      MySW.start();
      state=1;
    }else if(MySW.isRunning()==1&&digitalRead(BUTTONPIN)==LOW){
       timeP=MySW.elapsed();
      logData(GetPotiallas(), timeP);
      MySW.stop();
      MySW.reset();
      MySW.stop();
      state=0;
    }
}

void vakum(){
  digitalWrite(SARGA,HIGH);
  delay(1000);
  digitalWrite(SARGA,LOW);
  
}
/*
void error(){
  if(digitalRead(vakumPin)==HIGH){
    digitalWrite(zold,LOW);
    digitalWrite(piros,HIGH);
  }else if(digitalRead(vakumPin)==LOW){
    digitalWrite(zold,HIGH);
    digitalWrite(piros,LOW);
  }
}

void ok(){
  digitalWrite(zold,HIGH);
  digitalWrite(piros,LOW);
}*/
