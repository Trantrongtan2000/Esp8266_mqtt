/*Khai báo thư viện*/

#include <DHTesp.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>;
#include <ESP8266WiFi.h>
#include<PubSubClient.h>
#include "ThingSpeak.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"  

/*Phần Trang làm*/
/*Khai báo các giá trị cần thiết của channel "cambien" trên thingspeak để lưu trữ dữ liệu lên cloud */

/*Write data to Thingspeak*/
unsigned long myChannelNumber = 1265826; 
const char * myWriteAPIKey = "LGR9ANUDLG6PXRC2";


/*Khai báo giá trị*/

int cambien=12;   //Chan D6
int relay =16;    //Chan D0
int relay2=5;     //Chan D1
int relay3=4;     //Chan D2
int gas=13;       //Chan D7
int bom=15;       //Chan D8
int dat=A0;        //Chan A0
                  //LCD: chan SDA=D3, SCL=D4  
#define DHTpin 14 //chan D5
DHTesp dht;

int lcdColumns = 16;
int lcdRows = 2;

const char* mqtt_server="test.mosquitto.org";
WiFiClient espclient;
WiFiClient espclient2;

int dem=0;
int dem2=0;

const char* sensor_humi_topic = "sensor/nd";      // Topic của Sensor đọc độ ẩm
const char* sensor_temp_topic = "sensor/da";
const char* sensor_light_topic = "sensor/light";
const char* sensor_gas_topic = "sensor/gas";
const char* sensor_dadat_topic = "sensor/dadat";

const char* sensor_topic = "sensor"; 

PubSubClient client(mqtt_server,1883,callback,espclient);

long lastMsg = 0;
char msg[50];
int value = 0;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

/*Các hàm tính năng*/

void configModeCallback (WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void callback(char* topic,byte* payload,unsigned int length1){    
Serial.print("message arrived[");
Serial.print(topic);
Serial.println("]");

for(int i=0;i<length1;i++){
  Serial.print(payload[i]); 
  
}
if(payload[0]==49) dem++;//ASCII VALUE OF '1' IS 49
Serial.println("");
if(payload[0]==50) dem2++; //ASCII VALUE OF '2' IS 50
Serial.println("");
}


void print_sensor(String a, String b){
    lcd.clear();
    lcd.setCursor(0, 0);       
    lcd.print(a);
    lcd.setCursor(0, 1);    
    lcd.print(b);      
}

void convert(int check, const char* topic, String dieukien1, String dieukien2){
    char dk1[dieukien1.length() + 1];
    dieukien1.toCharArray(dk1, dieukien1.length());
    char dk2[dieukien2.length() + 1];
    dieukien2.toCharArray(dk2, dieukien2.length());
    if (check==int(1)) {
    client.publish(topic, dk1);
  } else {
    client.publish(topic, dk2);
  }  
}

void convert3(int check, const char* topic, String dieukien1, String dieukien2){
    char dk1[dieukien1.length() + 1];
    dieukien1.toCharArray(dk1, dieukien1.length());
    char dk2[dieukien2.length() + 1];
    dieukien2.toCharArray(dk2, dieukien2.length());
    if (round(check)>int(50)) {
    client.publish(topic, dk1);
  } else {
    client.publish(topic, dk2);
  }  
}

void reconnect(){
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  while(!client.connected()){
  if(client.connect("ESP8266Client123456789")){
    Serial.println("connected");
    client.publish(sensor_topic, "Reconnect");
    client.subscribe("sensor");
  }
    else{
      Serial.print("failed,rc=");
      Serial.println(client.state());
      delay(500);
    }
  } 
}

/*Hàm setup*/
void setup() {
  dht.setup(DHTpin, DHTesp::DHT11); 
  pinMode(cambien,INPUT);
  pinMode(gas,INPUT);
  pinMode(dat,INPUT);
  pinMode(relay,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(relay3,OUTPUT);
  pinMode(bom,OUTPUT);
  Wire.begin(D3, D4);
  lcd.begin();                                                    
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Da khoi dong");
  delay(1000);
  Serial.begin(115200);
  Serial.print("connecting");

  
  ThingSpeak.begin(espclient2);
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect())
  {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  }
  
  reconnect();

}

void senddata(){
  unsigned long last_time = 0;
  if ((millis()-last_time) > 1000){   
   float h= int(dht.getHumidity());
   float t= int(dht.getTemperature()); 
   static char temperatureTemp[7];
   dtostrf(t, 6, 2, temperatureTemp);
   static char humidityTemp[7];
   dtostrf(h, 6, 2, humidityTemp);
   
  int as = digitalRead(cambien); // Đọc cảm biến ánh sáng
  int khigas = digitalRead(gas); //Đọc cảm biến khí gas 
  int dadat = int(analogRead(dat))*100/1000;
  int cambiendat=0;
  if (dadat>=50){cambiendat=1;}
  else {cambiendat=0;}
  
   if (dem==2){dem=0;}  
   if (dem2==2){dem2=0;}  
   if (dem==as) {  
    digitalWrite(relay,LOW);
  } else {
    digitalWrite(relay,HIGH);
    }
  if (khigas==int(1)) {
    digitalWrite(relay2,HIGH);
  } else {
    digitalWrite(relay2,LOW);
  }  
   if (dem2==cambiendat) {
    digitalWrite(bom,LOW);
  } else {
    digitalWrite(bom,HIGH);
  } 
  convert(as, sensor_light_topic, "Trời tối, relay đang bật ", "Trời sáng, relay đang tắt ");
  convert(khigas, sensor_gas_topic, "Không có khí gas, an toàn ", "Có khí gas. Báo động!!! ");
  convert3(dadat, sensor_dadat_topic, "Đất ẩm, máy bơm không chạy ", "Đất khô, máy bơm đang chạy "); 
  client.publish(sensor_humi_topic, temperatureTemp);
  client.publish(sensor_temp_topic, humidityTemp); 

  int x =  ThingSpeak.setField(1, h); //setField(field, value)
  int y =  ThingSpeak.setField(2, t); //setField(field, value)
  int k =  ThingSpeak.setField(3, dadat); 
  int cbas =  ThingSpeak.setField(4, as);
  int cbgas =  ThingSpeak.setField(5, khigas);
  int z = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
   last_time = millis();
  }
}

/*Hàm loop đặc trưng*/
void loop() {
    if(!client.connected()){
      reconnect();
    }
    
    client.loop();
  int as = digitalRead(cambien); // Đọc cảm biến ánh sáng
  int khigas = digitalRead(gas); //Đọc cảm biến khí gas
  int dadat = int(analogRead(dat))*100/1000;
  float h= dht.getHumidity();
  float t= dht.getTemperature();
    
  senddata();
 
  if (as==int(1)) {    
  print_sensor("Troi toi","Relay dang bat");   
  } else {
  print_sensor("Troi sang","Relay dang tat");  
  } 
  senddata(); /*hoạt động giống như relay*/
  
  if (khigas==int(0)) {   
    print_sensor("Co khi gas","Bao dong");  
  } else {
    print_sensor("Khong co khi gas","An toan"); 
  } 
  senddata();
 
  if (dadat<int(50)) {   
    print_sensor("Dat kho qua","");  
  } else {
    print_sensor("Dat am roi day",""); 
  }
  senddata();
  String nd="Nhiet do: "+ String(int(t))+"'C ";
  String da="Do am: "+String(int(h))+"%";
  print_sensor(nd,da);
  senddata();
}
