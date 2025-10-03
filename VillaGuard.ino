#include <Wire.h>
#include "DFRobot_SHT20.h"
#include <HardwareSerial.h>

/* ------------------- Modules ------------------- */
DFRobot_SHT20 sht20;
HardwareSerial sim800(2);            // RX16 TX17

/* ------------------- PINs ------------------- */
#define SHT_SDA    21
#define SHT_SCL    22
#define RELAY_PIN  18   // Main Relay
#define SIREN_PIN  23   // Siren Input
#define ARM_PIN    19   // ARM/DISARM Pin
#define SPK_PIN     5   // Siren Pin(Mute)

/* Owner Phone to call in case of emergency */
const char OWNER_PHONE[] = "Your Phone Number";

/* ---------------- System Status ---------------- */
bool isArmed      = false;
bool called       = false;   // Called?
bool buttonEnable = true;    // System Enabled?
bool speakerOn    = false;

/* ───────────── Button Debounce ───────────── */
bool armPrev = false;
unsigned long armStamp = 0;
const unsigned DEBOUNCE_MS = 100;

/* Sim Buffer */
String simBuf;

/* ---------- Functions ---------- */
void simPrint(const String &cmd) {
  sim800.println(cmd);
  Serial.println("→ " + cmd);
}

void sendSMS(const String &num, const String &txt) {
  sim800.print("AT+CMGS=\"");
  sim800.print(num);
  sim800.println("\"");
  delay(300);                // Wait for '>'
  sim800.print(txt);
  sim800.write(26);          // CTRL-Z
  Serial.println("[SMS → " + num + "]");
  unsigned long t0 = millis();
  while (millis() - t0 < 2000) if (sim800.available()) sim800.read();
}

/* ---------- SIM input process ---------- */
void readSim() {
  while (sim800.available()) simBuf += char(sim800.read());

  int nl;  // new line
  while ((nl = simBuf.indexOf('\n')) >= 0) {
    String line = simBuf.substring(0, nl);
    simBuf.remove(0, nl + 1);
    line.trim();
    if (!line.length()) continue;

    Serial.println(line);

    /* received sms */
    if (line.startsWith("+CMT:")) {
      while (simBuf.indexOf('\n') == -1) if (sim800.available()) simBuf += char(sim800.read());
      int nl2 = simBuf.indexOf('\n');
      String body = simBuf.substring(0, nl2);
      simBuf.remove(0, nl2 + 1);
      body.trim();

      /* sender number */
      int q1 = line.indexOf('"') + 1;
      int q2 = line.indexOf('"', q1);
      String sender = line.substring(q1, q2);
      body.toUpperCase();        

      /* ───────── Commands ───────── */
      if (body == "STATUS") {
        float t = sht20.readTemperature();
        float h = sht20.readHumidity();
        bool siren = digitalRead(SIREN_PIN);
        bool armNow = digitalRead(ARM_PIN);

        String msg = "VillaGuard\n"
                     "Status: " + String(isArmed ? "ARMED" : "DISARMED") +
                     "\nSpeaker: " + String(speakerOn ? "ON" : "OFF") +
                     "\nAlarm: "   + String(siren ? "RED" : "GREEN") +
                     "\nRemote: " + String(buttonEnable ? "Enable" : "Disable") +
                     "\nRemoteNow: " + String(armNow ? "ON" : "OFF") +
                     "\nTemp: " + String(t, 1) + " C\nHum: " + String(h, 1) + " %";
        sendSMS(sender, msg);
      }
      else if (body == "ARM") {
        isArmed = true;
        digitalWrite(RELAY_PIN, LOW);
        sendSMS(sender, "ARMED");
      }
      else if (body == "DISARM") {
        isArmed = false;
        digitalWrite(RELAY_PIN, HIGH);
        if (called) { simPrint("ATH"); called = false; }
        sendSMS(sender, "DISARMED");
      }
      else if (body == "RMTON") {
        buttonEnable = true;
        sendSMS(sender, "Remote ENABLED");
      }
      else if (body == "RMTOFF") {
        buttonEnable = false;
        sendSMS(sender, "Remote DISABLED");
      }
      else if (body == "SPKON") {
        speakerOn = true;
        digitalWrite(SPK_PIN, LOW);
        sendSMS(sender, "Speaker ON");
      }
      else if (body == "SPKOFF") {
        speakerOn = false;
        digitalWrite(SPK_PIN, HIGH);
        sendSMS(sender, "Speaker OFF");
      }
    }
  }
}

/* ======================= setup ======================= */
void setup() {
  Serial.begin(115200);
  sim800.begin(9600, SERIAL_8N1, 16, 17);

  Wire.begin(SHT_SDA, SHT_SCL);
  sht20.initSHT20();
  delay(100);
  sht20.checkSHT20();

  pinMode(RELAY_PIN, OUTPUT); digitalWrite(RELAY_PIN, HIGH);   // DISARM
  pinMode(SPK_PIN,  OUTPUT); digitalWrite(SPK_PIN,  HIGH);     // Siren ON
  pinMode(SIREN_PIN, INPUT);
  pinMode(ARM_PIN,   INPUT);

  /* راه‌اندازی یک‌بارهٔ SIM */
  simPrint("AT");
  delay(300);
  simPrint("ATE0");               
  delay(300);
  simPrint("AT+CMGF=1");           
  delay(300);
  simPrint("AT+CPMS=\"SM\"");      
  delay(300);
  simPrint("AT+CNMI=2,2,0,0,0");  
  delay(300);

  Serial.println("System Ready.");
}

/* ======================= loop ======================== */
void loop() {

  readSim();

  /* ARM/DISARM Button */
  if (buttonEnable) {
    bool armNow = digitalRead(ARM_PIN);
    if (armNow != armPrev && millis() - armStamp >= DEBOUNCE_MS) {
      armPrev = armNow;
      armStamp = millis();
      if (armNow) {
        isArmed = true;
        digitalWrite(RELAY_PIN, LOW);
        sendSMS(OWNER_PHONE, "ARMED (Remote)");
      } else {
        isArmed = false;
        digitalWrite(RELAY_PIN, HIGH);
        if (called) { simPrint("ATH"); called = false; }
        sendSMS(OWNER_PHONE, "DISARMED (Remote)");
      }
    }
  }

  /* Calling the owner in case of Emergency */
  bool siren = digitalRead(SIREN_PIN);
  if (siren && isArmed && !called) {
    simPrint(String("ATD") + OWNER_PHONE + ";");
    called = true;
  } else if (!siren && called) {
    simPrint("ATH");
    called = false;
  }

  /* Test for Call and SMS Function of SIM800 */
  if (Serial.available()) {
    switch (Serial.read()) {
      case 's': sendSMS(OWNER_PHONE, "Test SMS"); break;
      case 'd': simPrint(String("ATD") + OWNER_PHONE + ";");    break;
    }
  }
}
