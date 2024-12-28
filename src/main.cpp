#include "secrets/wifi.h"
#include "wifi_connect.h"
#include <WiFiClientSecure.h>
#include "ca_cert.h"
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include "secrets/mqtt.h"
#include <PubSubClient.h>
#include "config.h"
#include <Ticker.h>

void soil_Moisture_Handler();
void doMotorStuff();
void warning(char* topic, String message);
namespace
{
    const char *ssid = WiFiSecrets::ssid;
    const char *password = WiFiSecrets::pass;
  
}

WiFiClientSecure tlsClient;
PubSubClient mqttClient(tlsClient);

Ticker mqttPulishTicker;
Ticker DHT22_Publish_Ticker;
Ticker soil_Moisture_Publish_Ticker;

DHT dht(DHTPIN, DHTTYPE);

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.printf("Message arrived from topic [%s]: %s\n", topic, message.c_str());

    //Xu ly thong tin cua cam bien
    if(String(topic) == "sensor/moisture/data" && auto_flag){
        soil_Moisture_Handler();
    }
    // Thay doi nguong do am tuoi nuoc
    if(String(topic) == "sensor/moisture/change"){
        moisture_threshold = message.toInt();
    }

    //Neu nhan thong tin tuoi nuoc cua nguoi dung
    if (String(topic) == "motor/control" && message == "IRRIGATE") {
            doMotorStuff();
            Serial.println("Handy Watering...");

    }
    // Bat/tat tu dong tuoi nuoc
    if(String(topic) == "motor/auto"){
        auto_flag = bool(message);
    }
    if(millis() - last_warning > 5*60*1000){
        //Canh bao nhiet do
    if(String(topic) == "sensor/temperature" 
    && (message.toFloat() > warning_temp_up || message.toFloat() < warning_temp_down)){
        warning("warning/temp", "Temperature Issue!");
    }
    //Canh bao do am dat
    if(String(topic) == "sensor/moisture/data" 
    && (message.toFloat() < warning_moist_up || message.toFloat() > warning_moist_down)){
        warning("warning/moisture","Moisture Issue!");
    }
    // Canh bao do am khong khi
     if(String(topic) == "sensor/humidity" && message.toFloat() < warning_hum){
        warning("warning/humidity", "Air Humidity Issue!");
    }
    last_warning = millis();
    }

}

void mqttReconnect()
{
    while (!mqttClient.connected())
    {
        Serial.println("Attempting MQTT connection...");
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        if (mqttClient.connect(client_id.c_str(), MQTT::username, MQTT::password))
        {
            Serial.print(client_id);
            Serial.println(" connected");
            mqttClient.subscribe("sensor/#");
            mqttClient.subscribe("motor/#");

        }
        else
        {
            Serial.print("MTTT connect failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 1 seconds");
            delay(1000);
        }
    }
}


//MCU dua ra quyet dinh dua tren cam bien
void soil_Moisture_Handler(){
    int moisture_now = moisture;
    //Neu khong bi bien dong do nhieu trong 10 lan dem
    if(reliability_count >= 10){
        //Neu dat du nguong am de tuoi nuoc
        if(moisture < moisture_threshold){
            doMotorStuff();
            Serial.println("Watering...");
        }
        //Reset bien dem ve 0
        reliability_count = 0;
    }
    else{
        //Neu lan do truoc va lan do sau khong bi chenh lech qua nhieu do nhieu
        if(abs(moisture_last - moisture_now) < noise_threshold){
            //Tang bien dem tin cay
            reliability_count++;
        }
        moisture_last = moisture_now;
    }
    
}

//Ham dieu khien dong co voi xung pwm o muc 75%
void doMotorStuff(){
    mqttClient.publish(motor_topic, "Watering...", false);
    
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, SPEED);

    delay(3000);

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);

    delay(1000);
}
//Ham doc/gui gia tri nhiet do - do am khong khi
void DHT22_Reader(){
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    mqttClient.publish(temperature_topic, String(temperature).c_str(), false);
    mqttClient.publish(humidity_topic, String(humidity).c_str(), false);

}
//Ham doc/gui gia tri nhiet do - do am dat
void soil_Moisture_Reader(){
    int value = analogRead(MOISTURE_PIN);
    moisture = map(value, 0, 4095, 0, 100); // map gia tri 0-12 bit ->> 0-100%
    mqttClient.publish(moisture_topic, String(moisture).c_str(), false);

}
//Ham gui canh bao
void warning(char* topic, String message){
    mqttClient.publish(topic, message.c_str(), false);
}
//Ham ngu? cua ESP
void goToSleep(bool Sleep){
    if(Sleep)
    {
        delay(AWAKE_TIME);
        esp_deep_sleep(DEEP_SLEEP_TIME);
    }
}
void setup()
{
    Serial.begin(115200);
    dht.begin();
    delay(10);
    setup_wifi(ssid, password);
    tlsClient.setCACert(ca_cert);

    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    mqttClient.setCallback(mqttCallback);
    mqttClient.setServer(MQTT::broker, MQTT::port);

    soil_Moisture_Publish_Ticker.attach(10, soil_Moisture_Reader);
    DHT22_Publish_Ticker.attach(10, DHT22_Reader);
}

void loop()
{

    if (!mqttClient.connected())
    {
        mqttReconnect();
    }
    mqttClient.loop();

    goToSleep(SLEEP);
}