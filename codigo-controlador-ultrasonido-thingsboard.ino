#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define WIFI_AP "Redmi 10A"
#define WIFI_PASSWORD "Sopaipillas con mostaza"
#define TOKEN "3jx0xujx981da4ez0q2i"

char thingsboardServer[] = "iot.ceisufro.cl";
int PORT = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

#define TRIGGER_PIN D1  // Pin del trigger del sensor de ultrasonido
#define ECHO_PIN D2     // Pin del echo del sensor de ultrasonido

long duration;
double distance;

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  InitWiFi(); // Conectar a la red WiFi

  // Inicializar la conexión a ThingsBoard
  client.setServer(thingsboardServer, PORT);
  lastSend = 0;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  if (millis() - lastSend > 1000) {
    getAndSendDistance();
    lastSend = millis();
  }

  client.loop();
}

void getAndSendDistance() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;  // Calcular la distancia en cm


  Serial.println(distance);

  String payload = "{";
  payload += "\"distance\":";
  payload += distance;
  payload += "}";

  char attributes[100];
  payload.toCharArray(attributes, 100);
  client.publish("v1/devices/me/telemetry", attributes);
  Serial.println(attributes);
}

void InitWiFi() {
  Serial.println("Conectando a la red Wi-Fi...");
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Conectado a la red Wi-Fi");
}

void reconnect() {
  // Volver a conectarse a ThingsBoard
  while (!client.connected()) {
    status = WiFi.status();
    if (status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Conectado a la red Wi-Fi");
    }

    Serial.print("Conectando a ThingsBoard...");
    
    // Intentar la conexión a ThingsBoard (clientId, username, password)
    if (client.connect("Sensor_proximidad", TOKEN, NULL)) {
      Serial.println("[DONE]");
    } else {
      Serial.print("[FALLÓ] [ rc = ");
      Serial.print(client.state());
      Serial.println(" : reintentando en 5 segundos]");
      // Esperar 5 segundos antes de intentar nuevamente
      delay(5000);
    }
  }
}
