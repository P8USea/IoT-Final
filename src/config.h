
#define DHTPIN 33
#define DHTTYPE DHT22
#define ENA 25
#define IN1 26
#define IN2 27
#define SPEED 192
#define MOISTURE_PIN 35

//Cau hinh che do ngu?
#define DEEP_SLEEP_TIME 60000000 //10p
#define AWAKE_TIME 180000 //3p
#define SLEEP true

//Cac topic
const char *moisture_topic = "sensor/moisture/data";
const char *humidity_topic = "sensor/humidity";
const char *temperature_topic = "sensor/temperature";
const char *motor_topic = "motor";
const char *warning_topic = "warning";
   
//Nguong cam bien canh bao
unsigned int warning_temp_up = 38;
unsigned int warning_temp_down = 10;
unsigned int warning_hum = 10;
unsigned int warning_moist_up = 30;
unsigned int warning_moist_down = 80; 

int moisture = 0;
int reliability_count = 0; //Bien dem kiem tra do on dinh cua cam bien
int moisture_threshold = 70; //Nguong am de tuoi nuoc
int noise_threshold = 3; //Nguong sai so trong truong hop nhieu tin hieu
int moisture_last = 0; //Gia tri do am dat cu
long last_warning = 0; // Thoi gian warning cu
bool auto_flag = true; //Bien co dung de tuoi nuoc auto/manual

