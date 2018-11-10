#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <config.h>

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

DHT dht;

void setup(void)
{
    Serial.begin(115200);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(wifiSsid);

    WiFi.begin(wifiSsid, wifiPassword);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    dht.setup(2);

    client.setServer(mqttBrokerAddress, mqttBrokerPort);
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void loop(void)
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    const char *status = dht.getStatusString();

    Serial.print(status);
    Serial.print("\t");
    Serial.print(humidity, 1);
    Serial.print("\t\t");
    Serial.print(temperature, 1);
    Serial.print("\t\t");
    Serial.println(dht.toFahrenheit(temperature), 1);

    if (status == "OK") {

        StaticJsonBuffer<256> jsonBuffer;

        // Parse the root object
        JsonObject &sensorJson = jsonBuffer.createObject();

        // Set the values
        sensorJson["sensor"] = sensorName;
        sensorJson["temp"] = temperature;
        sensorJson["humidity"] = humidity;

        String jsonString;
        sensorJson.printTo(jsonString);

        client.publish(mqttTopic, jsonString.c_str());
    }
    delay(15000);
}