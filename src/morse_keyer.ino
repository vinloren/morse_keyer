/*
 * Copyright (c) 2021, github/vinloren
 * All rights reserved.
 * Create a morse keyer on ESP8266-12e 
 * relying on TCP server and passive buzzer
 * connected on D2 of ESP12e (PIO 4).
 * Sound tone set at at 800Hz.
*/

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <EEPROM.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_SDA 4
#define OLED_SCL 5
Adafruit_SSD1306 display(128, 32, &Wire, -1);

ADC_MODE(ADC_VCC);  // Set ADC for read Vcc

int port = 8888;  //Port number
WiFiServer server(port);

int ssid_psw = 50;
const char* APssid     = "ESP12_AP";
IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

//Server connect to WiFi Network
String Ssid = "yourssid";   
String passwd = "yourpsw";  
bool LANWIFI = false;
bool APMODE = false;
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
String sp1 = "CHARSPACE1\r\n";
String sp2 = "CHARSPACE2\r\n";

#define tonePin 14

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
float voltage;
char inbuf[2048];
char rcvBuf[32] = {0};
int bytecount = 5;
int tbit = 80;  // ms tempo di bit per 15wpm, 60 per 20wpm, 48 per 25wpm 40 per 30wpm
int nota = 800; // tono buzzer 800Hz
int spchr = 1;  // spazi fra dit / dah nei caratteri

void showOled(char* s) {
  display.clearDisplay();
  display.setTextSize(2); 
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(s);
  display.display();  
}

// show sliding data to oled from rcvBuf
void showData() {
  if(bytecount > 20){
    for(int i=0;i<20;i++)
      rcvBuf[i] = rcvBuf[i+1];
    bytecount--;
  }
  rcvBuf[20] = 0;
  showOled(rcvBuf);
}

void showMorseData(char c) {
  rcvBuf[bytecount] = c;
  bytecount++;
  if(bytecount > 20){
    for(int i=0;i<20;i++)
      rcvBuf[i] = rcvBuf[i+1];
    bytecount--;
  }
  rcvBuf[20] = 0;
  showOled(rcvBuf);
}

void getVoltage(){
  voltage = ESP.getVcc();
  delay(500); 
  voltage = voltage/1024.0;  //volt  
  voltage = voltage * 1.127; // correction for esp-12
  voltage = round(100*voltage)/100;  
}

bool connectWiFi() {
  int i = 0;
  char c;
  WiFi.mode(WIFI_STA);
  int firstssid = EEPROM.read(ssid_psw);
  Serial.print("First ssid byte: ");
  Serial.println(firstssid);  
  if(firstssid != '\0') {
    Ssid = "";
    while(EEPROM.read(ssid_psw+i) != '\0') {
      c = EEPROM.read(ssid_psw+i);
      Ssid += String(c);
      //Serial.println(Ssid);
      i++;
    }
    i++;
    passwd = "";
    while(EEPROM.read(ssid_psw+i) != '\0') {
      c = EEPROM.read(ssid_psw+i);
      passwd += String(c);
      //Serial.println(passwd);
      i++;
    }
  }
  Serial.print("Connecting ");
  Serial.println(Ssid);
  Serial.print("Password: ");
  Serial.println(passwd);
  WiFi.begin(Ssid, passwd); 
 
  // Wait for connection  
  Serial.println("Connecting to Wifi");
  i = 0;
  while (WiFi.status() != WL_CONNECTED) {   
    delay(500);
    Serial.print(".");
    delay(500);
    i++;
    if(i > 19)
      break;
  }
  Serial.println("");
  if(i > 19){
    WiFi.mode(WIFI_AP);
    return false;
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(Ssid);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
  return true;
}

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  Serial.println();
  digitalWrite(2, HIGH);  // Turn the LED off 

  EEPROM.begin(512);
  delay(10);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Failed to initialize the dispaly");
    for (;;);
  }
  Serial.println("Display initialized");
  LANWIFI = connectWiFi();
  
  if(LANWIFI) {
    server.begin();
    Serial.print("Open Telnet and connect to IP:");
    Serial.print(WiFi.localIP());
    Serial.print(" on port ");
    Serial.println(port);
    sprintf(rcvBuf,"Host ");
    showOled(rcvBuf);
    delay(2000);
    
    String s = WiFi.localIP().toString()+String("\r");
    findMorse(s,tbit);

    getVoltage();
    char bf[10] = {0};
    sprintf(bf," Vb: %0.2f",voltage);
    for(int i=0;i<9;i++)
      showMorseData(bf[i]);
    Serial.print("Voltage:");
    Serial.println(voltage);
    delay(1000);  
  }
  else {
    Serial.println("No local WiFi router, setting ESP12_AP");
    Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "ESP12_AP" : "Failed!");

    Serial.print("Setting soft-AP ... ");
    Serial.println(WiFi.softAP(APssid ? "ESP12_AP" : "Failed!"));

    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP()); 
    sprintf(rcvBuf,"AP %d.%d.%d.%d",local_IP[0],local_IP[1],local_IP[2],local_IP[3]);
    showOled(rcvBuf);
    while(WiFi.softAPgetStationNum() < 1) {
      delay(2000);
      Serial.println("No station connected to Soft-AP");
    }
    getVoltage();
    char bf[32] = {0};
    sprintf(bf," Vb: %0.2f              ",voltage);
    showOled(bf);
    APMODE = true;
    server.begin();
    delay(2000);
  }
}


void loop() 
{
  WiFiClient client = server.available();
  
  if (client) {
    if(client.connected())
    {
      Serial.println("Client Connected");
      for(int i=0;i<sizeof(rcvBuf);i++)
        rcvBuf[i] = '\0';
      bytecount = 0;
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
      sprintf(buf,"Velocità morse %s nota at %dHz Livello batteria: %.2f\r\n",speed,nota,voltage);
      client.write(buf);
      if(APMODE) {
        sprintf(buf,"Per configurare ssid/psw WiFi inviare ^^nomessid password da Transmit window.");
        delay(200);
        client.write(buf);
      }
    }
    
    while(client.connected()){      
      String newSSID = "";
      String newPSW = "";
      
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
        if(s.charAt(0) == '^' && s.charAt(1) == '^') {
          Serial.println("Setting ssid / psw.");
          s += String("\r\n");
          j = 2;
          while(s.charAt(j) != ' ') {
            newSSID += String(s.charAt(j++)); 
            if(j > 20)
              break;
          }
          if(j < 20) {
            j++;
            while(s.charAt(j) != '\r') {
              newPSW += String(s.charAt(j++));
              if(j > 30)
                break;
            }
            for(j=0;j<sizeof(newSSID);j++) {
              EEPROM.write(ssid_psw+j,newSSID.charAt(j));
              delay(100);
              EEPROM.commit();
            }
            Serial.print("newSSID: ");
            Serial.println(newSSID);
            EEPROM.write(ssid_psw+j,'\0');
            j++;
            delay(100);
            EEPROM.commit();
            for(i=0;i<sizeof(newPSW);i++) {
              EEPROM.write(ssid_psw+j+i,newPSW.charAt(i));
              delay(100);
              EEPROM.commit(); 
            }
            i++;
            Serial.print("newPSW: ");
            Serial.println(newPSW);
            EEPROM.write(ssid_psw+j+i,'\0');
            delay(100);
            EEPROM.commit(); 
            client.write("ACK\r\n");
          }
        }
        else {
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
          else if(sp1.compareTo(s) == 0) {
            spchr = 1;  
            Serial.println("Singolo spazio dit/dah");   
          }
          else if(sp2.compareTo(s) == 0) {
            spchr = 2;  
            Serial.println("Doppio spazio dit/dah");   
          }
          else {
            Serial.println("Vado a trasmettere in morse..");
            findMorse(s, tbit);
          }
          client.write("Ack\r\n");
          Serial.println("Ack");
          getVoltage();
          char bf[5] = {0};
          sprintf(bf,"%0.2f",voltage);
          Serial.print("Vbatt: ");
          Serial.println(bf);
          client.write(bf);
        }
        datin = false;
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
    tone(tonePin,nota);     // suono su io14
    digitalWrite(2, LOW);
    delay(c);
    noTone(tonePin);        // cessa suono su io14
    digitalWrite(2, HIGH);
    delay(tbit*spchr);  // ritardo spazio fra punti e linee
  }
  delay(2*tbit*spchr);  // ritardo spazio fra caratteri
}


void findMorse (String s, int tbit) {
  int i = 0;
  int j;
  while(s.charAt(i) != '\r') {
      if(s.charAt(i) == ' ') {
         showMorseData(' ');
         delay(4*tbit*spchr);    // se blank spazia ritardo fra parole
      }
      else {
         for(j=0;j<sizeof(alfanum);j++) {
           if(s.charAt(i) == alfanum[j])
             break;
         }
         if(j < sizeof(alfanum)) {
           playmorse(codes[j],tbit);
           showMorseData(s.charAt(i));
         }
         else {
           showMorseData(' ');
           delay(4*tbit*spchr); // carattere non previsto, trattalo come spazio
         }
     }
     i++;
  }
}