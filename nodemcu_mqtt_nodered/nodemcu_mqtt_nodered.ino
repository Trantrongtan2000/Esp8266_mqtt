#include <DHTesp.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>;
#include <ESP8266WiFi.h>
#include<PubSubClient.h>

const char* mqttServer="test.mosquitto.org";


String clientId = "ClientESP8266";            // Client ID của mạch
const char* sensor_humi_topic = "sensor/nd";      // Topic của Sensor đọc độ ẩm
const char* sensor_temp_topic = "sensor/da";
const char* sensor_light_topic = "sensor/light";
const char* sensor_gas_topic = "sensor/gas";
const char* sensor_dadat_topic = "sensor/dadat";
const char* sensor_relay_topic = "sensor/relay";
const char* sensor_topic = "sensor"; 

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
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);


void print_sensor(String a, String b){
    lcd.clear();
    lcd.setCursor(0, 0);       
    lcd.print(a);
    lcd.setCursor(0, 1);    
    lcd.print(b);      
}

void setup() {
  Serial.begin(115200);
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
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) 
  {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message read [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish(sensor_topic, "Reconnect");               // Gửi dữ liệu
      client.subscribe(sensor_topic);                          // Theo dõi dữ liệu
    } else {
      Serial.println("failed, rc=");
      Serial.print(client.state());
      // Doi 1s
      delay(1000);
    }
  }
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

void convert2(int check, const char* topic, String dieukien1, String dieukien2){
    char dk1[dieukien1.length() + 1];
    dieukien1.toCharArray(dk1, dieukien1.length());
    char dk2[dieukien2.length() + 1];
    dieukien2.toCharArray(dk2, dieukien2.length());
    if (round(check)>=33) {
    client.publish(topic, dk1);
  } else {
    client.publish(topic, dk2);
  }  
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
  int dadat = digitalRead(dat);
  
  if (as==int(1)) {  
    digitalWrite(relay,HIGH);
  } else {
    digitalWrite(relay,LOW);
  } 
  if (khigas==int(1)) {
    digitalWrite(relay2,HIGH);
  } else {
    digitalWrite(relay2,LOW);
  } 
  if (round(t)>=32) {
    digitalWrite(relay3,HIGH);
  } else {
    digitalWrite(relay3,LOW);
  } 
   if (dadat==int(1)) {
    digitalWrite(bom,LOW);
  } else {
    digitalWrite(bom,HIGH);
  } 
  convert(as, sensor_light_topic, "Trời tối, relay đang bật ", "Trời sáng, relay đang tắt ");
  convert(khigas, sensor_gas_topic, "Không có khí gas, an toàn ", "Có khí gas, Báo động!!! ");
  convert(dadat, sensor_dadat_topic, "Đất ẩm, máy bơm không chạy ", "Đất khô, máy bơm đang chạy ");
  convert2(t, sensor_relay_topic, "Nhiệt độ >= 32, quạt đang bật ", "Nhiệt độ <32, quạt đang tắt ");  
  client.publish(sensor_humi_topic, temperatureTemp);
  client.publish(sensor_temp_topic, humidityTemp); 
   last_time = millis();
}
  }
void loop() {
   if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int as = digitalRead(cambien); // Đọc cảm biến ánh sáng
  int khigas = digitalRead(gas); //Đọc cảm biến khí gas
  int dadat = digitalRead(dat);
  float h= dht.getHumidity();
  float t= dht.getTemperature();  

  
  senddata();
  delay(500);
  if (as==int(1)) {    
  print_sensor("Troi toi","Relay dang bat");   
  } else {
  print_sensor("Troi sang","Relay dang tat");  
  } 
  senddata();
  delay(500);
  if (khigas==int(0)) {   
    print_sensor("Co khi gas","Bao dong");  
  } else {
    print_sensor("Khong co khi gas","An toan"); 
  } 
  senddata();
  delay(500);
  if (dadat==int(0)) {   
    print_sensor("Dat kho qua","");  
  } else {
    print_sensor("Dat am roi day",""); 
  }
  senddata();
  delay(500);
  if (round(t)>=32) {   
  print_sensor("Nhiet do >32","Quat dang bat");    
  } else {
  print_sensor("Nhiet do <32","Quat dang tat"); 
  } 
  senddata();
  delay(500);
  String nd="Nhiet do: "+ String(int(t))+"'C ";
  String da="Do am: "+String(int(h))+"%";
  print_sensor(nd,da);
  senddata();
}
