#include "secrets/wifi.h"
#include "wifi_connect.h"
#include <WiFiClientSecure.h>
#include "ca_cert.h"
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include "secrets/mqtt.h"
#include <PubSubClient.h>

#include <Ticker.h>

#define DHTPIN 33
#define DHTTYPE DHT22
#define ENA 25
#define IN1 26
#define IN2 27
#define SPEED 192
#define MOISTURE_PIN 35
void soil_Moisture_Handler();
void doMotorStuff();
void warning(char* topic, String message);
namespace
{
    const char *ssid = WiFiSecrets::ssid;
    const char *password = WiFiSecrets::pass;
    const char *moisture_topic = "sensor/moisture/data";
    const char *humidity_topic = "sensor/humidity";
    const char *temperature_topic = "sensor/temperature";
    const char *motor_topic = "motor";
    const char *warning_topic = "warning";

    int data = 0;
    unsigned int warning_temp_up = 38;
    unsigned int warning_temp_down = 10;
    unsigned int warning_hum = 70;
    unsigned int warning_moist_up = 30;
    unsigned int warning_moist_down = 80; 

    int reliability_count = 0; //Bien dem kiem tra do on dinh cua cam bien
    int moisture_threshold = 70; //Nguong am de tuoi nuoc
    int noise_threshold = 3; //Nguong sai so trong truong hop nhieu tin hieu
    int sensor_last = 0;
    long last_warning = millis();
    boolean auto_flag = true;
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
    if(String(topic) == "sensor/moisture/data"){
        soil_Moisture_Handler();
    }
    // Thay doi nguong do am tuoi nuoc
    if(String(topic) == "sensor/moisture/change"){
        moisture_threshold = message.toInt();
    }

    //Neu nhan thong tin tuoi nuoc cua nguoi dung
    if (String(topic) == "motor/control" && message == "ON" && auto_flag) {
            doMotorStuff();
            Serial.println("Handy Watering...");

    }
    // Bat/tat tu dong tuoi nuoc
    if(String(topic) == "motor/auto"){
        auto_flag = bool(message);
    }
    if(millis() - last_warning > 60*1000){
        //Canh bao nhiet do
    if(String(topic) == "sensor/temperature" 
    && (message.toFloat() > warning_temp_up || message.toFloat() < warning_temp_down)){
        warning("warning/temp", "Temperature Issue!");
    }
    //Canh bao do am dat
    if(String(topic) == "sensor/moisture" 
    && (message.toFloat() > warning_moist_up || message.toFloat() < warning_moist_down)){
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
    int sensor_now = data;
    //Neu khong bi bien dong do nhieu trong 10 lan dem
    if(reliability_count >= 10){
        //Neu dat du nguong am de tuoi nuoc
        if(data < moisture_threshold){
            doMotorStuff();
            Serial.println("Watering...");
        }
        //Reset bien dem ve 0
        reliability_count = 0;
    }
    else{
        //Neu lan do truoc va lan do sau khong bi chenh lech qua nhieu do nhieu
        if(abs(sensor_last - sensor_now) < noise_threshold){
            //Tang bien dem tin cay
            reliability_count++;
        }
        sensor_last = sensor_now;
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
void soil_Moisture_Reader(){
    int value = analogRead(MOISTURE_PIN);
    data = map(value, 0, 4095, 0, 100); // map gia tri 0-12 bit ->> 0-100%
    mqttClient.publish(moisture_topic, String(data).c_str(), false);

}
void warning(char* topic, String message){
    mqttClient.publish(topic, message.c_str(), false);
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

    soil_Moisture_Publish_Ticker.attach(5, soil_Moisture_Reader);
    DHT22_Publish_Ticker.attach(5, DHT22_Reader);
}

void loop()
{

    if (!mqttClient.connected())
    {
        mqttReconnect();
    }
    mqttClient.loop();
}