#define BLYNK_TEMPLATE_ID "TMPL3v_HLEWl1"
#define BLYNK_TEMPLATE_NAME "Event Driven Safety Protection Kernel"
#define BLYNK_AUTH_TOKEN "wXTAirsDiEZ77vTocUOK1XXqKNRPXwKo"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define POT_PIN      34
#define BUTTON_PIN   27
#define BUZZER_PIN   26
#define GREEN_LED    18
#define RED_LED      19

LiquidCrystal_I2C lcd(0x27, 16, 2);

enum SystemState {
  NORMAL,
  UNDERVOLTAGE_FAULT,
  OVERVOLTAGE_FAULT,
  SENSOR_FAULT,
  FLUCTUATION_FAULT
};

SystemState currentState = NORMAL;

unsigned long lastBuzzerTime = 0;
unsigned long lastLCDTime = 0;
unsigned long faultStartTime = 0;

bool buzzerState = false;
float previousVoltage = 0;

// Thresholds
const float UNDER_VOLTAGE = 1.0;
const float OVER_VOLTAGE  = 2.8;

const float RECOVERY_LOW  = 1.2;
const float RECOVERY_HIGH = 2.6;

const float FLUCT_LIMIT   = 0.5;
const unsigned long RELAY_LOCK_TIME = 5000;

float readVoltage() {
  int adc = analogRead(POT_PIN);
  return (adc / 4095.0) * 3.3;
}

void updateBlynk(float voltage, String state, String fault,
                 String relayStatus, String buzzerStatus,
                 String recoveryStatus) {

  Blynk.virtualWrite(V0, voltage);
  Blynk.virtualWrite(V1, state);
  Blynk.virtualWrite(V2, fault);
  Blynk.virtualWrite(V3, relayStatus);
  Blynk.virtualWrite(V4, buzzerStatus);
  Blynk.virtualWrite(V5, recoveryStatus);
}

void setup() {

  Serial.begin(115200);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Safety Kernel");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
}

void loop() {

  Blynk.run();

  float voltage = readVoltage();

  // Sensor anomaly
  if (digitalRead(BUTTON_PIN) == LOW) {
    currentState = SENSOR_FAULT;
    faultStartTime = millis();
  }

  // Undervoltage
  else if (voltage < UNDER_VOLTAGE) {
    currentState = UNDERVOLTAGE_FAULT;
    faultStartTime = millis();
  }

  // Overvoltage
  else if (voltage > OVER_VOLTAGE) {
    currentState = OVERVOLTAGE_FAULT;
    faultStartTime = millis();
  }

  // Rapid fluctuation
  else if (abs(voltage - previousVoltage) > FLUCT_LIMIT) {
    currentState = FLUCTUATION_FAULT;
    faultStartTime = millis();
  }

  // Recovery logic
  else {

    if (millis() - faultStartTime > RELAY_LOCK_TIME) {

      if (voltage > RECOVERY_LOW &&
          voltage < RECOVERY_HIGH &&
          digitalRead(BUTTON_PIN) == HIGH) {

        currentState = NORMAL;
      }
    }
  }

  previousVoltage = voltage;

  updateOutputs(voltage);
}

void updateOutputs(float voltage) {

  switch (currentState) {

    case NORMAL:

      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      noTone(BUZZER_PIN);

      updateBlynk(
        voltage,
        "NORMAL",
        "NONE",
        "CONNECTED",
        "OFF",
        "STABLE"
      );

      if (millis() - lastLCDTime > 500) {

        lastLCDTime = millis();

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SYSTEM OK");

        lcd.setCursor(0, 1);
        lcd.print("V:");
        lcd.print(voltage, 2);
      }

      break;

    case UNDERVOLTAGE_FAULT:

      updateBlynk(
        voltage,
        "FAULT",
        "LOW VOLTAGE",
        "DISCONNECTED",
        "ON",
        "RECOVERING"
      );

      faultMode("LOW VOLTAGE");
      break;

    case OVERVOLTAGE_FAULT:

      updateBlynk(
        voltage,
        "FAULT",
        "OVERVOLTAGE",
        "DISCONNECTED",
        "ON",
        "RECOVERING"
      );

      faultMode("OVER VOLT");
      break;

    case SENSOR_FAULT:

      updateBlynk(
        voltage,
        "FAULT",
        "SENSOR ERROR",
        "DISCONNECTED",
        "ON",
        "RECOVERING"
      );

      faultMode("SENSOR ERROR");
      break;

    case FLUCTUATION_FAULT:

      updateBlynk(
        voltage,
        "FAULT",
        "UNSTABLE CELL",
        "DISCONNECTED",
        "ON",
        "RECOVERING"
      );

      faultMode("UNSTABLE CELL");
      break;
  }
}

void faultMode(String message) {

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);

  if (millis() - lastBuzzerTime > 500) {

    lastBuzzerTime = millis();

    buzzerState = !buzzerState;

    if (buzzerState)
      tone(BUZZER_PIN, 1000);
    else
      noTone(BUZZER_PIN);
  }

  if (millis() - lastLCDTime > 500) {

    lastLCDTime = millis();

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("WARNING");

    lcd.setCursor(0, 1);
    lcd.print(message);
  }
}