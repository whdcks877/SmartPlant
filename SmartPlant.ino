#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define A0Pin A0 //습도센서
#define PUMP D7 //펌프모터
//Wifi 설정
const char *ssid = "SIOR";
const char *password = "abcd1234";

//mqtt 서버 정보
const char* mqtt_server = "tailor.cloudmqtt.com";
const char* mqtt_topic = "Arduino/Commend";
char* mqtt_message = "Hello world #%ld";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int sensorVal = 0; //습도 센서 값
String val_s;
char val_c[50];

int WateringCommend = 0;
#define USE_SERIAL Serial
void setup() {
    pinMode(PUMP,OUTPUT);
    //USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);
    USE_SERIAL.setDebugOutput(true);
    USE_SERIAL.println();

    for(uint8_t t = 3; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFi.begin(ssid, password);

    Serial.print("Connecting to AP");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    

    client.setServer(mqtt_server, 17298);
    client.setCallback(callback);

    randomSeed(analogRead(1));
}


void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    sensorVal = analogRead(A0Pin);
    delay(1000);
    Serial.print("Analog sensor = ");
    Serial.println(sensorVal);
    val_s = String(sensorVal);
    val_s.toCharArray(val_c,val_s.length()+1);
    client.publish("Arduino/sensorVal",val_c);
  
   if((sensorVal >900) || (WateringCommend==1)) {
      Serial.println("Very Dry!");
      digitalWrite(PUMP,HIGH);
      Serial.println("Pump On for 1 Second!");
      delay(2000);
      digitalWrite(PUMP,LOW);
      WateringCommend = 0;
    } else if(sensorVal <= 900) {
      Serial.println("Very Wet!");
    }
    
    long now = millis();
    long randNumber = random(0, 15000);
    if (now - lastMsg > 15000 + randNumber) {
        lastMsg = now;
        ++value;
        snprintf (msg, 75, mqtt_message, value);
    }
}

//메세지가 왔을때 실행되는 함수
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");


    String msgText = "=> ";
    
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      msgText += (char)payload[i];
    }
    
    Serial.println(msgText);
    if(msgText.compareTo("=> Watering") == 0)
    {
      WateringCommend = 1;
      Serial.println("Wertering Commend arrived");
    }
}


void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("ESP8266Client","wvewfgbg","0BwuLSnOSda4")) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish(mqtt_topic, "hello world");
            // ... and resubscribe
            client.subscribe(mqtt_topic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}
