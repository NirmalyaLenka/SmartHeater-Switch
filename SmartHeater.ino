/*
 * ============================================================
 *  SmartHeater Switch — ESP8266 + SinricPro
 *  Author  : Your Name
 *  Version : 1.0.0
 *  License : MIT
 * ============================================================
 *
 *  What this does:
 *  ✔ Turns the relay ON/OFF via Alexa / Google Assistant
 *  ✔ Auto-shuts off after AUTO_OFF_MINUTES (default 15 min)
 *  ✔ Supports a daily scheduled ON time (e.g. 08:00)
 *  ✔ Syncs real-time clock over NTP so schedules survive reboots
 *  ✔ Blinks onboard LED to show current state at a glance
 *  ✔ Handles up to 25 A load via SSR (Solid-State Relay)
 *
 *  Hardware wiring  →  see /wiring/wiring_notes.md
 *  Credentials      →  edit config.h before flashing
 * ============================================================
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "config.h"

// ── Pin definitions ──────────────────────────────────────────
#define RELAY_PIN     D1   // GPIO5  →  SSR control input
#define LED_PIN       D4   // GPIO2  →  onboard LED (active LOW on NodeMCU)
#define BUTTON_PIN    D2   // GPIO4  →  optional physical override button

// ── Timing constants ─────────────────────────────────────────
#define AUTO_OFF_MINUTES    15          // heater stays on this long (minutes)
#define AUTO_OFF_MS         (AUTO_OFF_MINUTES * 60UL * 1000UL)
#define NTP_UPDATE_INTERVAL 60000UL    // re-sync NTP every 60 s
#define SCHEDULE_CHECK_MS   30000UL    // check schedule every 30 s

// ── EEPROM addresses (persist schedule across reboots) ───────
#define EEPROM_SIZE         8
#define ADDR_SCHED_HOUR     0   // 1 byte
#define ADDR_SCHED_MIN      1   // 1 byte
#define ADDR_SCHED_EN       2   // 1 byte  (0xFF = enabled)

// ── NTP setup (IST = UTC+5:30 = 19800 s offset) ─────────────
WiFiUDP   ntpUDP;
NTPClient ntpClient(ntpUDP, "pool.ntp.org", NTP_UTC_OFFSET_SEC, NTP_UPDATE_INTERVAL);

// ── State variables ──────────────────────────────────────────
bool     relayState        = false;
unsigned long relayOnTime  = 0;         // millis() when relay turned ON
bool     autoOffArmed      = false;

uint8_t  schedHour         = 8;         // default: 08:00
uint8_t  schedMin          = 0;
bool     schedEnabled      = false;

unsigned long lastScheduleCheck = 0;
bool     schedFiredToday   = false;     // prevent re-trigger in same minute
int      lastDay           = -1;        // reset schedFiredToday at midnight

// ── Forward declarations ─────────────────────────────────────
void  setRelay(bool on, const char* reason = "");
bool  onPowerState(const String& deviceId, bool& state);
void  loadScheduleFromEEPROM();
void  saveScheduleToEEPROM();
void  checkSchedule();
void  checkAutoOff();
void  handleButton();
void  blinkLED(int times, int ms = 100);
void  printStatus();


// ════════════════════════════════════════════════════════════
//  SETUP
// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\n=== SmartHeater Switch booting ==="));

  // ── GPIO init ────────────────────────────────────────────
  pinMode(RELAY_PIN,  OUTPUT);
  pinMode(LED_PIN,    OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(RELAY_PIN, LOW);   // relay OFF at boot (safety first!)
  digitalWrite(LED_PIN,   HIGH);  // LED off (active LOW)

  // ── EEPROM ───────────────────────────────────────────────
  EEPROM.begin(EEPROM_SIZE);
  loadScheduleFromEEPROM();

  // ── WiFi ─────────────────────────────────────────────────
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    blinkLED(3, 200);
  } else {
    Serial.println(F("\nWiFi FAILED — running offline (schedule still works if NTP was cached)"));
    blinkLED(6, 80);
  }

  // ── NTP ──────────────────────────────────────────────────
  ntpClient.begin();
  ntpClient.update();
  Serial.printf("Current time: %s\n", ntpClient.getFormattedTime().c_str());

  // ── SinricPro (Alexa / Google) ───────────────────────────
  SinricProSwitch& mySwitch = SinricPro[SINRICPRO_SWITCH_ID];
  mySwitch.onPowerState(onPowerState);

  SinricPro.onConnected([]() {
    Serial.println(F("SinricPro connected — voice control active"));
    blinkLED(2, 300);
  });
  SinricPro.onDisconnected([]() {
    Serial.println(F("SinricPro disconnected"));
  });

  SinricPro.begin(SINRICPRO_APP_KEY, SINRICPRO_APP_SECRET);

  // ── Done ─────────────────────────────────────────────────
  Serial.println(F("Boot complete.\n"));
  printStatus();
}


// ════════════════════════════════════════════════════════════
//  LOOP
// ════════════════════════════════════════════════════════════
void loop() {
  SinricPro.handle();
  ntpClient.update();

  checkAutoOff();
  checkSchedule();
  handleButton();

  // Blink LED based on state
  static unsigned long lastBlink = 0;
  static bool ledToggle = false;
  unsigned long now = millis();

  if (relayState) {
    // Heater ON: slow heartbeat blink (every 1 s)
    if (now - lastBlink > 1000) {
      ledToggle = !ledToggle;
      digitalWrite(LED_PIN, ledToggle ? LOW : HIGH);
      lastBlink = now;
    }
  } else {
    // Heater OFF: LED solid off
    digitalWrite(LED_PIN, HIGH);
  }
}


// ════════════════════════════════════════════════════════════
//  RELAY CONTROL — single point of truth
// ════════════════════════════════════════════════════════════
void setRelay(bool on, const char* reason) {
  if (on == relayState) return;   // no change needed

  relayState = on;
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);

  if (on) {
    relayOnTime   = millis();
    autoOffArmed  = true;
    Serial.printf("[RELAY ON]  Reason: %s  |  Auto-off in %d min\n", reason, AUTO_OFF_MINUTES);
  } else {
    autoOffArmed  = false;
    Serial.printf("[RELAY OFF] Reason: %s\n", reason);
  }

  // Push state back to SinricPro so Alexa stays in sync
  SinricProSwitch& mySwitch = SinricPro[SINRICPRO_SWITCH_ID];
  mySwitch.sendPowerStateEvent(on);
}


// ════════════════════════════════════════════════════════════
//  SINRICPRO CALLBACK  (Alexa / Google voice command)
// ════════════════════════════════════════════════════════════
bool onPowerState(const String& deviceId, bool& state) {
  Serial.printf("Voice command → %s\n", state ? "ON" : "OFF");
  setRelay(state, "voice");
  return true;   // returning true confirms the action to the cloud
}


// ════════════════════════════════════════════════════════════
//  AUTO-OFF TIMER CHECK
// ════════════════════════════════════════════════════════════
void checkAutoOff() {
  if (!relayState || !autoOffArmed) return;

  if (millis() - relayOnTime >= AUTO_OFF_MS) {
    Serial.printf("Auto-off triggered after %d minutes\n", AUTO_OFF_MINUTES);
    setRelay(false, "auto-off timer");
    blinkLED(4, 150);
  }
}


// ════════════════════════════════════════════════════════════
//  SCHEDULED ON-TIME CHECK
// ════════════════════════════════════════════════════════════
void checkSchedule() {
  if (!schedEnabled) return;
  unsigned long now = millis();
  if (now - lastScheduleCheck < SCHEDULE_CHECK_MS) return;
  lastScheduleCheck = now;

  ntpClient.update();

  int currentHour  = ntpClient.getHours();
  int currentMin   = ntpClient.getMinutes();
  int currentDay   = ntpClient.getDay();   // 0=Sun … 6=Sat

  // Reset daily-fire flag at midnight
  if (currentDay != lastDay) {
    schedFiredToday = false;
    lastDay = currentDay;
  }

  // Fire schedule: match hour+minute, don't fire twice in same session
  if (!schedFiredToday &&
      currentHour == schedHour &&
      currentMin  == schedMin  &&
      !relayState) {

    Serial.printf("Schedule triggered at %02d:%02d\n", schedHour, schedMin);
    setRelay(true, "schedule");
    schedFiredToday = true;
  }
}


// ════════════════════════════════════════════════════════════
//  PHYSICAL BUTTON HANDLER  (debounced)
// ════════════════════════════════════════════════════════════
void handleButton() {
  static unsigned long lastPress  = 0;
  static bool          lastState  = HIGH;

  bool current = digitalRead(BUTTON_PIN);

  if (current == LOW && lastState == HIGH && millis() - lastPress > 300) {
    // Button pressed → toggle relay
    lastPress = millis();
    setRelay(!relayState, "physical button");
  }
  lastState = current;
}


// ════════════════════════════════════════════════════════════
//  EEPROM HELPERS  (persist schedule across power cycles)
// ════════════════════════════════════════════════════════════
void loadScheduleFromEEPROM() {
  schedHour    = EEPROM.read(ADDR_SCHED_HOUR);
  schedMin     = EEPROM.read(ADDR_SCHED_MIN);
  schedEnabled = (EEPROM.read(ADDR_SCHED_EN) == 0xFF);

  // Sanity check
  if (schedHour > 23) schedHour = 8;
  if (schedMin  > 59) schedMin  = 0;

  Serial.printf("Loaded schedule: %02d:%02d, enabled=%d\n",
                schedHour, schedMin, schedEnabled);
}

void saveScheduleToEEPROM() {
  EEPROM.write(ADDR_SCHED_HOUR, schedHour);
  EEPROM.write(ADDR_SCHED_MIN,  schedMin);
  EEPROM.write(ADDR_SCHED_EN,   schedEnabled ? 0xFF : 0x00);
  EEPROM.commit();
  Serial.printf("Saved schedule: %02d:%02d, enabled=%d\n",
                schedHour, schedMin, schedEnabled);
}


// ════════════════════════════════════════════════════════════
//  UTILITY
// ════════════════════════════════════════════════════════════
void blinkLED(int times, int ms) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LOW);   // on
    delay(ms);
    digitalWrite(LED_PIN, HIGH);  // off
    delay(ms);
  }
}

void printStatus() {
  Serial.println(F("------- Status -------"));
  Serial.printf("Relay      : %s\n",  relayState ? "ON" : "OFF");
  Serial.printf("Schedule   : %s @ %02d:%02d\n",
                schedEnabled ? "ON" : "OFF", schedHour, schedMin);
  Serial.printf("WiFi       : %s\n",
                WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString().c_str() : "Offline");
  Serial.printf("NTP Time   : %s\n",  ntpClient.getFormattedTime().c_str());
  Serial.println(F("----------------------"));
}
