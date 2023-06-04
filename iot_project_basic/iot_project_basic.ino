#include <UbidotsESPMQTT.h>
#include "DHT.h"
#include <WiFi.h>
#include <IFTTTWebhook.h>

#define dhtpin 19
#define dhttype DHT11
#define u_trigger 16
#define u_echo 17
#define mq_d 12
#define mq_a 14
#define relay 13 

#define token "BBFF-2MHQBnxIVqW5DMUqLh1VzsEhBdNA3b"
#define id "TheCosmos"
#define pass "thedivinefun"

#define ifttt_api "lwQRQ6OGbIvHahQguCplu-t8t55IFYPCdgX10hBvMcR" 
#define ifttt_event "intruder_alert" 

DHT dht(dhtpin, dhttype);
Ubidots client(token);
IFTTTWebhook ifttt_webhook(ifttt_api, ifttt_api);
int op=0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  op=(char)payload[0]-48;
  //if(op==0 || op==1){
    Serial.print("Command: ");
    bool command = *payload - 48;
    Serial.println(command);
    digitalWrite(relay, !command);
  //}
  
  Serial.println();
  Serial.println(op);
}

void setup() {
  Serial.begin(9600);

  Serial.println("Welcome to Home assistant.");

  dht.begin();
  pinMode(u_trigger, OUTPUT);
  pinMode(u_echo, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(mq_a, INPUT);
  
  Serial.print("Connecting to SSID: ");
  Serial.print(id);
  Serial.print(", Password: ");
  Serial.println(pass);
  client.wifiConnection(id, pass);
  Serial.println("Done");

  Serial.println(" Initializing Ubidots Connection...");
  client.ubidotsSetBroker("industrial.api.ubidots.com");
  client.setDebug(true);
  client.begin(callback);
  client.ubidotsSubscribe("home-assistant","relay");
  client.ubidotsSubscribe("home-assistant","option");
  Serial.println("Done");

}

void dht_task1(){
  float h  = dht.readHumidity();    // read humidity
  float t = dht.readTemperature(); // read temperature

  Serial.print("Temp.: ");
  Serial.print(t);     // print the temperature
  Serial.print((char)223); // print ° character
  Serial.println("C");

  Serial.print("Humidity: ");
  Serial.print(h);      // print the humidity
  Serial.println("%");

  if (!client.connected()) {
    client.reconnect();
  }
  client.add("t", t);
  client.add("h", h);
  client.ubidotsPublish("home-assistant");
  //client.loop();
  delay(1000);
}

void ultrasonic_task2(){
  digitalWrite (u_trigger, HIGH);
  delayMicroseconds (10);
  digitalWrite (u_trigger, LOW);
  int time = pulseIn (u_echo, HIGH);
  int distance = (time * 0.034) / 2;
  if (distance <= 20) 
        {
        Serial.print (" Distance= ");              
        Serial.println (distance);        
        Serial.println("Intrusion Detected!");
        ifttt_webhook.trigger();
        delay(500);
        }
  if(!client.connected()){
    client.reconnect();
  }
  client.add("distance",distance);
  client.ubidotsPublish("home-assistant");
  client.loop();
  delay(1000);
}

void mq135_task3(){
  float ppm = analogRead(mq_a) / 4; // PPM
  Serial.print("Air Quality : ");
  Serial.print(ppm);
  Serial.println(" PPM");
  
  // Establising connection with Ubidots
  if (!client.connected()) {
    client.reconnect();
  }

  // Publising data of both variable to Ubidots
  client.add("ppm", ppm);  // Insert your variable Labels and the value to be sent
  client.ubidotsPublish("home-assistant"); // insert your device label here
  client.loop();
  delay(1000);
}

void relay_task4(){
  if (!client.connected()) {
    client.reconnect();
    client.ubidotsSubscribe("home-assistant","relay");
  }
  client.loop();
  delay(1000);
}

void loop() {
  if (!client.connected()) {
    client.reconnect();
    client.ubidotsSubscribe("home-assistant","option");
  }
  client.loop();
  delay(1000);
  
  if (op==6){
    dht_task1();
  }
  else if (op==7){
    ultrasonic_task2();
  }
  else if (op==8){
    mq135_task3();
  }
  else if(op==9){
    relay_task4();
  }
}
