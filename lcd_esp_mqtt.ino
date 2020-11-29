#include <DHTesp.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>;
#include <ESP8266WiFi.h>


int cambien=12;   //Chan D6
int relay =16;    //Chan D0
int relay2=5;     //Chan D1
int relay3=4;     //Chan D2
int gas=13;       //Chan D7
int bom=15;       //Chan D8
int dat=3;        //Chan D9
                  //LCD: chan SDA=D3, SCL=D4  
#define DHTpin 14 //chan D5
DHTesp dht;

int lcdColumns = 16;
int lcdRows = 2;
const char *ssid     = "TRONG TAN";     
const char *password = "trongtan2000"; 

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

void load(){
  delay(dht.getMinimumSamplingPeriod());
  int as = digitalRead(cambien); // Đọc cảm biến ánh sáng
  int khigas = digitalRead(gas); //Đọc cảm biến khí gas
  float h= dht.getHumidity();
  float t= dht.getTemperature();  
  String nd="Nhiet do: "+ String(round(t))+"'C ";
  String da="Do am: "+String(round(h))+"%";
  int dadat = digitalRead(dat);
 if (as==int(1)) {  
    digitalWrite(relay,HIGH);
  } else {
    digitalWrite(relay,LOW);
  } 
  if (khigas<int(100)) {
    digitalWrite(relay2,HIGH);
  } else {
    digitalWrite(relay2,LOW);
  } 
  if (round(t)>=32) {;
    digitalWrite(relay3,HIGH);
  } else {
    digitalWrite(relay3,LOW);
  } 
   if (dadat==int(0)) {
    digitalWrite(bom,HIGH);
  } else {
    digitalWrite(bom,LOW);
  }   
}

void print_sensor(String a, String b){
    lcd.setCursor(0, 0);       
    lcd.print(a);
    load();
    lcd.setCursor(0, 1);    
    lcd.print(b);     
    delay(1000);
    lcd.clear();
}

void setup() {
  dht.setup(DHTpin, DHTesp::DHT11); 
  pinMode(cambien,INPUT);
  pinMode(gas,INPUT);
  pinMode(dat,INPUT);
  pinMode(relay,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(relay3,OUTPUT);
  pinMode(bom,OUTPUT);
  lcd.begin(0,2);                                                     
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Da khoi dong");
  delay(1000);
  lcd.clear();
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) 
  {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println( "Got Wifi" );  
}

void loop() {
  delay(dht.getMinimumSamplingPeriod());
  int as = digitalRead(cambien); // Đọc cảm biến ánh sáng
  int khigas = digitalRead(gas); //Đọc cảm biến khí gas
  int dadat = digitalRead(dat);
  float h= dht.getHumidity();
  float t= dht.getTemperature();  
  String nd="Nhiet do: "+ String(round(t))+"'C ";
  String da="Do am: "+String(round(h))+"%";

  load();
  if (as==int(1)) {    
  print_sensor("Troi toi","Relay dang bat");   
  } else {
  print_sensor("Troi sang","Relay dang tat");  
  } 
  
  if (khigas==int(1)) {   
    print_sensor("Co khi gas","Bao dong");  
  } else {
    print_sensor("Khong co khi gas","An toan"); 
  } 

  if (dadat==int(0)) {   
    print_sensor("Dat kho qua","");  
  } else {
    print_sensor("Dat am roi day",""); 
  }

  if (round(t)>=32) {   
  print_sensor("Nhiet do >32","Quat dang bat");    
  } else {
  print_sensor("Nhiet do <32","Quat dang tat"); 
  } 
  print_sensor(nd,da);
}
