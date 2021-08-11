/*
 * Copyright (c) 2021, github/vinloren
 * All rights reserved.
 * Create a morse keyer on ESP8266-12e 
 * relying on TCP server and passive buzzer
 * connected on D2 of ESP12e (PIO 4).
 * Sound tone set at at 800Hz.
*/

#include <ESP8266WiFi.h>

int port = 8888;  //Port number
WiFiServer server(port);

//Server connect to WiFi Network
const char *ssid = "TIM-30772510";  //Enter your wifi SSID
const char *password = "Vin41s65!";  //Enter your wifi Password

bool datin = false;
String wpm5  = "5WPM\r\n";
String wpm10 = "10WPM\r\n";
String wpm15 = "15WPM\r\n";
String wpm20 = "20WPM\r\n";
String wpm25 = "25WPM\r\n";
String wpm30 = "30WPM\r\n";
String t400 = "400\r\n";
String t500 = "500\r\n";
String t600 = "600\r\n";
String t700 = "700\r\n";
String t800 = "800\r\n";
String t900 = "900\r\n";

char alfanum[] = {'A','B','C','D','E','F','G','H','I','J','K','L',
                  'M','N','O','P','Q','R','S','T','U','V','W','X',
                  'Y','Z','1','2','3','4','5','6','7','8','9','0',
                  '.',',','/','=','?',':',';','-','+','*'};
// codici A-Z 1-0 .,/=?:;-+* nei codici 1 = dit , 3 = dah
String codes[] = {String("13"),String("3111"),String("3131"),String("311"),String("1"),
                  String("1131"),String("331"),String("1111"),String("11"),String("1333"),
                  String("313"),String("1311"),String("33"),String("31"),String("333"),
                  String("1331"),String("3313"),String("131"),String("111"),String("3"),
                  String("113"),String("1113"),String("133"),String("3113"),String("3133"),
                  String("3311"),String("13333"),String("11333"),String("11133"),
                  String("11113"),String("11111"),String("31111"),String("33111"),
                  String("33311"),String("33331"),String("33333"),String("131313"),
                  String("331133"),String("31131"),String("31113"),String("113311"),
                  String("333111"),String("313131"),String("311113"),String("13131"),
                  String("131131")};
char inbuf[2048];
int tbit = 80;  // ms tempo di bit per 15wpm, 60 per 20wpm, 48 per 25wpm 40 per 30wpm
int nota = 800; // tono buzzer 800Hz
void setup() 
{
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Connect to wifi
 
  // Wait for connection  
  Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {   
    delay(500);
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
  server.begin();
  Serial.print("Open Telnet and connect to IP:");
  Serial.print(WiFi.localIP());
  Serial.print(" on port ");
  Serial.println(port);
  digitalWrite(2, HIGH);  // Turn the LED off 

  String s = WiFi.localIP().toString()+String("\r");
  findMorse(s,tbit);
  delay(1000);   
}


void loop() 
{
  WiFiClient client = server.available();
  
  if (client) {
    if(client.connected())
    {
      Serial.println("Client Connected");
      char* speed = "";
      switch (tbit) {
        case 240:
          speed = "5wpm";
          break;
        case 120:
          speed = "10wpm";
          break;
        case 80:
          speed = "15wpm";
          break;
        case 60:
          speed = "20wpm";
          break;
        case 48: 
          speed = "25wpm";
          break;
        case 40:
          speed = "30wpm";
          break;
      }
      char buf[80];
      sprintf(buf,"Velocità morse %s nota at %dHz.",speed,nota);
      client.write(buf);
    }
    
    while(client.connected()){      
      int i,j;
      for(i=0;i<2048;i++)
        inbuf[i] = '\0';
      i = 0;
      while(client.available()>0){
        // read data from the connected client
        inbuf[i++] = client.read();
        datin = true;
      }
      if(datin) {
        String s = String(inbuf);
        s.toUpperCase();
        Serial.println(s);
        if(wpm5.compareTo(s) == 0) {
          tbit = 240;
          Serial.println("Set speed 5wpm");  
        }
        else if(wpm10.compareTo(s) == 0) {
          tbit = 120;
          Serial.println("Set speed 10wpm");  
        }
        else if(wpm15.compareTo(s) == 0) {
          tbit = 80;
          Serial.println("Set speed 15wpm");  
        }
        else if(wpm20.compareTo(s) == 0) {
          tbit = 60;
          Serial.println("Set speed 20wpm");  
        }
        else if(wpm25.compareTo(s) == 0) {
          tbit = 48;
          Serial.println("Set speed 25wpm");  
        }
        else if(wpm30.compareTo(s) == 0) {
          tbit = 40;
          Serial.println("Set speed 30wpm");  
        }
        else if(t400.compareTo(s) == 0) {
          nota = 400;
          Serial.println("Set tone at 400Hz");  
        }
        else if(t500.compareTo(s) == 0) {
          nota = 500;
          Serial.println("Set tone at 500Hz");  
        }
        else if(t600.compareTo(s) == 0) {
          nota = 600;
          Serial.println("Set tone at 600Hz");  
        }
        else if(t700.compareTo(s) == 0) {
          nota = 700;
          Serial.println("Set tone at 700Hz");  
        }
        else if(t800.compareTo(s) == 0) {
          nota = 800;
          Serial.println("Set tone at 800Hz");  
        }
        else if(t900.compareTo(s) == 0) {
          nota = 900;
          Serial.println("Set tone at 900Hz");  
        }
        else {
          Serial.println("Vado a trasmettere in morse..");
          findMorse(s, tbit);
        }
        client.write("Ack\r\n");
        datin = false;
        Serial.println("Ack");
      }
    }
    client.stop();
    Serial.println("Client disconnected");    
  }
}


void playmorse(String x, int tbit) {
  int c;
  for(int i=0;i<x.length();i++) {
    c = (int)(x.charAt(i) - 48)*tbit; 
    tone(4,nota);      // suono su io4
    digitalWrite(2, LOW);
    delay(c);
    noTone(4);        // cessa suono su io4
    digitalWrite(2, HIGH);
    delay(tbit);  // ritardo spazio fra punti e linee
  }
  delay(2*tbit); // ritardo spazio fra caratteri
}


void findMorse (String s, int tbit) {
  int i = 0;
  int j;
  while(s.charAt(i) != '\r') {
      if(s.charAt(i) == ' ') {
         delay(4*tbit);    // se blank spazia ritardo fra parole
      }
      else {
         for(j=0;j<sizeof(alfanum);j++) {
           if(s.charAt(i) == alfanum[j])
             break;
         }
         if(j < sizeof(alfanum)) {
           playmorse(codes[j],tbit);
         }
         else {
           delay(4*tbit); // carattere non previsto, trattalo come spazio
         }
     }
     i++;
  }
}