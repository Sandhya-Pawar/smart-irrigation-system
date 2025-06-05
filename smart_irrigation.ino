#define BLYNK_TEMPLATE_ID "TMPL3nefkNNav"
#define BLYNK_TEMPLATE_NAME "Smart Irrigation"
#define BLYNK_AUTH_TOKEN "p3YlpJgGMLNTg1HocKxE8yb5-VvW8_yJ"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Define DHT11 sensor
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "vivo-1906";
char pass[] = "1234567890";

// Define pins
#define IN3 12
#define IN4 14
#define MOISTURE_PIN 39
#define THRESHOLD_PERCENT 40

bool manualControl = false;
int blynkState = 0;

// Blynk virtual pin V0 for manual pump control
BLYNK_WRITE(V0) {
  blynkState = param.asInt();
  manualControl = true;

  if (blynkState == 1) {

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    Serial.println("Pump ON via Blynk");
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    Serial.println("Pump OFF via Blynk");
  }
}

void setup() {
  Serial.begi
  n(115200);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  analogSetAttenuation(ADC_11db); // full ADC range
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  delay(1000);
}

void loop() {
  Blynk.run();

  // 🌱 Soil Moisture Reading
  int sensorValue = analogRead(MOISTURE_PIN);
  int moisturePercent = map(sensorValue, 0, 4095, 100, 0); // Corrected mapping
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Moisture: ");
  Serial.print(moisturePercent);
  Serial.println(" %");

  // Show DRY/WET status in Serial and Blynk V4
  if (moisturePercent < THRESHOLD_PERCENT) {
    Serial.println("🌱 Soil is DRY");
    Blynk.virtualWrite(V4, "Soil is DRY");
  } else {
    Serial.println("🌿 Soil is WET");
    Blynk.virtualWrite(V4, "Soil is WET");
  }

  // Send moisture % to Gauge in Blynk (V1)
  Blynk.virtualWrite(V1, moisturePercent);

  // 💧 Pump Auto Control (only if manual not used)
  if (!manualControl) {
    if (moisturePercent < THRESHOLD_PERCENT) {
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      Serial.println("Pump ON (Auto - Soil is DRY)");
    } else {
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      Serial.println("Pump OFF (Auto - Soil is WET)");
    }
  }

  // 🌡️ DHT11 Sensor (Temperature and Humidity)
  float h = 0, t = 0;
  int samples = 5;
  int validSamples = 0;

  for (int i = 0; i < samples; i++) {
    float tempReading = dht.readTemperature();
    float humReading = dht.readHumidity();
    if (!isnan(tempReading) && !isnan(humReading)) {
      t += tempReading;
      h += humReading;
      validSamples++;
    }
    delay(200);
  }

  if (validSamples > 0) {
    t /= validSamples;
    h /= validSamples;

    Serial.print("Temp: ");
    Serial.print(t);
    Serial.print(" °C | Humidity: ");
    Serial.print(h);
    Serial.println(" %");

    Blynk.virtualWrite(V2, t); // Temperature
    Blynk.virtualWrite(V3, h); // Humidity
  } else {
    Serial.println("❌ Failed to read from DHT sensor!");
  }

  delay(500); // short delay for responsive updates
}
