#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// WIFI
const char* ssid = "OPPO A57";
const char* password = "fadli5524";

// PIN
#define DHTPIN      27
#define DHTTYPE     DHT22
#define RELAY_PIN   16

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

// DATA
float temperature = 0;
float humidity = 0;

bool relayState = false;

unsigned long lastSensor = 0;

const unsigned long SENSOR_INTERVAL = 2000;

// READ SENSOR
void readSensor()
{
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t))
        temperature = t;

    if (!isnan(h))
        humidity = h;

    Serial.printf(
        "Temp : %.1f C | Hum : %.1f %% | Relay : %s\n",
        temperature,
        humidity,
        relayState ? "ON" : "OFF"
    );
}

// SENSOR API
void sensorAPI()
{
    String json = "{";

    json += "\"temperature\":";
    json += String(temperature,1);

    json += ",\"humidity\":";
    json += String(humidity,1);

    json += ",\"relay\":";
    json += relayState ? "true":"false";

    json += ",\"status\":\"";
    json += relayState ? "AC ON":"AC OFF";
    json += "\"}";

    server.sendHeader("Access-Control-Allow-Origin","*");
    server.send(200,"application/json",json);
}

// TEMP
void tempAPI()
{
    server.sendHeader("Access-Control-Allow-Origin","*");
    server.send(200,"text/plain",String(temperature,1));
}

// HUM
void humAPI()
{
    server.sendHeader("Access-Control-Allow-Origin","*");
    server.send(200,"text/plain",String(humidity,1));
}

// RELAY ON
void relayON()
{
    relayState = true;

    digitalWrite(RELAY_PIN,HIGH);

    server.sendHeader("Access-Control-Allow-Origin","*");
    server.send(200,"application/json",
    "{\"relay\":true,\"status\":\"AC ON\"}");

    Serial.println("Relay ON");
}

// RELAY OFF
void relayOFF()
{
    relayState = false;

    digitalWrite(RELAY_PIN,LOW);

    server.sendHeader("Access-Control-Allow-Origin","*");
    server.send(200,"application/json",
    "{\"relay\":false,\"status\":\"AC OFF\"}");

    Serial.println("Relay OFF");
}

// ROOT
void root()
{
    server.send(200,"text/plain","ESP32 ONLINE");
}

// WIFI
void connectWiFi()
{
    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid,password);

    Serial.print("Connecting");

    while(WiFi.status()!=WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    Serial.print("IP : ");
    Serial.println(WiFi.localIP());
}

// SETUP
void setup()
{
    Serial.begin(115200);

    pinMode(RELAY_PIN,OUTPUT);

    digitalWrite(RELAY_PIN,LOW);

    dht.begin();

    connectWiFi();

    readSensor();

    server.on("/",root);

    server.on("/sensor",sensorAPI);

    server.on("/dht-temp",tempAPI);

    server.on("/dht-humidity",humAPI);

    server.on("/relay/on",relayON);

    server.on("/relay/off",relayOFF);

    server.begin();

    Serial.println("HTTP Server Ready");
}

// LOOP
void loop()
{
    server.handleClient();

    if(WiFi.status()!=WL_CONNECTED)
    {
        WiFi.reconnect();
    }

    if(millis()-lastSensor>=SENSOR_INTERVAL)
    {
        lastSensor=millis();

        readSensor();
    }
}
