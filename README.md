# 💧 SmartHeater Switch

> A **low-cost, Alexa/Google-enabled smart switch** for water heaters (and any 20–25 A appliance) built on the ESP8266 NodeMCU.  
> Total cost: **₹350–₹600** (~$5 USD). No subscription fees. No cloud lock-in.

---

##  Features

| Feature | Details |
|---------|---------|
|  **Voice Control** | "Alexa, turn on water heater" via SinricPro (free) |
|  **Auto-Off Timer** | Heater shuts off automatically after 15 minutes |
|  **Daily Schedule** | Set a time (e.g. 08:00) — heater turns on every day automatically |
|  **Physical Button** | Toggle without phone or internet |
|  **Status LED** | Solid ON = running, blink = standby |
|  **NTP Time Sync** | Accurate scheduling — no RTC chip needed |
|  **Persistent Schedule** | EEPROM saves your schedule across power cuts |
|  **20–25 A Capable** | Uses a Solid-State Relay — safe for any Indian geyser |
|  **Offline fallback** | Physical button works even if WiFi is down |

---

##  Hardware Required

See [`components.md`](components.md) for detailed component reasoning and sourcing tips.

| Component | Purpose | Approx. Cost |
|-----------|---------|--------------|
| NodeMCU ESP8266 v3 | Brain + WiFi | ₹150 |
| SSR-25 DA (25 A Solid State Relay) | Mains switching | ₹160 |
| HLK-PM01 (5 V AC-DC) | Powers NodeMCU from mains | ₹100 |
| 25 A ceramic fuse + holder | Overcurrent protection | ₹25 |
| Push button + LED | Physical controls | ₹15 |
| Enclosure + wiring + misc | Safe packaging | ₹100 |

<img width="310" height="297" alt="image" src="https://github.com/user-attachments/assets/16bac0bb-1cca-41f7-8eef-6013ddf714d9" /> SSR-25 DA
<img width="500" height="376" alt="image" src="https://github.com/user-attachments/assets/d9bc0507-a120-436b-997a-724a4e3d89f2" /> HLK-PM01
<img width="172" height="270" alt="image" src="https://github.com/user-attachments/assets/1f1dfc07-1ed1-49c1-9eb5-7f8985a3616f" /> 25 A ceramic fuse

---

##  Other Use Cases (Beyond Water Heaters)

This is a general-purpose **high-current smart switch**. Any appliance up to **25 A / 5500 W** at 220 V can be controlled:

| Use Case | Why It's Useful |
|----------|----------------|
|  **Wet Grinder / Mixer** | Schedule morning grinding, auto-off for safety |
|  **Water Pump (1–2 HP)** | Fill overhead tank at off-peak hours (lower electricity tariff) |
|  **Room Heater / Oil Radiator** | Voice-on before you wake up, auto-off |
|  **Desert Air Cooler** | Turn on before reaching home ("Alexa, start the cooler") |
|  **High-Wattage Outdoor Lighting** | Schedule dusk-to-dawn without a dedicated timer unit |
|  **Aquarium Heater + Pump combo** | Automate maintenance windows |
|  **Solar Water Tank Top-Up Heater** | Top-up only on cloudy days via voice |
|  **Small Workshop Machinery** | Scheduled pre-heat of CNC spindle or vulcanizer |
|  **Instant Geyser (backup)** | Voice-on 5 min before shower — auto-off protects element |
|  **Grow Light (400 W HPS / LED)** | Precise photoperiod scheduling for indoor plants |

---

##  Repository Structure

```
SmartHeater-Switch/
├── README.md              ← You are here
├── components.md          ← Why each component was chosen
├── .gitignore
├── LICENSE
├── src/
│   ├── SmartHeater.ino    ← Main firmware (Arduino)
│   ├── config.h           ← Your credentials (NOT committed)
│   └── config.example.h  ← Template — safe to commit
├── wiring/
│   └── wiring_notes.md   ← Step-by-step wiring + safety
└── demo/
    └── index.html         ← Browser demo of the UI/logic
```

---

##  Getting Started

### 1. Clone the repo
```bash
git clone https://github.com/YOUR_USERNAME/SmartHeater-Switch.git
cd SmartHeater-Switch
```

### 2. Set up Arduino IDE
1. Install [Arduino IDE 2.x](https://www.arduino.cc/en/software)
2. Add ESP8266 board manager URL:  
   `File → Preferences → Additional Board URLs:`  
   `https://arduino.esp8266.com/stable/package_esp8266com_index.json`
3. `Tools → Board → Boards Manager` → search **esp8266** → install

### 3. Install libraries
Open `Tools → Manage Libraries` and install:

| Library | Version tested |
|---------|---------------|
| SinricPro | ≥ 2.10 |
| NTPClient | ≥ 3.2 |
| ArduinoJson | ≥ 6.21 |

### 4. Create your SinricPro account
1. Go to [https://sinric.pro](https://sinric.pro) and sign up (free)
2. Click **Add Device** → choose type **Switch**
3. Name it "Water Heater"
4. Copy your **App Key**, **App Secret**, and **Device ID**

### 5. Configure credentials
```bash
cp src/config.example.h src/config.h
# Edit src/config.h with your WiFi SSID, password, and SinricPro keys
```

### 6. Flash
1. Connect NodeMCU via USB
2. `Tools → Board → NodeMCU 1.0 (ESP-12E Module)`
3. `Tools → Port → COMx` (or `/dev/ttyUSB0` on Linux)
4. Click **Upload**
5. Open Serial Monitor at 115200 baud to see boot log

### 7. Link to Alexa
1. Open the Alexa app → `More → Skills & Games` → search **SinricPro**
2. Enable the skill and sign in with your SinricPro account
3. Discover devices
4. Say: **"Alexa, turn on water heater"** 🎉

### 8. Set a daily schedule (optional)
In the Alexa app:  
`More → Routines → (+) → Add trigger → Schedule → 08:00 AM`  
→ Action: `Smart Home → Water Heater → Turn On`

The heater will turn on at 8 AM every day and auto-off after 15 minutes.

---

##  Customisation

### Change the auto-off time
In `config.h` (or directly in `SmartHeater.ino`):
```cpp
#define AUTO_OFF_MINUTES   20   // change to any value in minutes
```

### Change timezone
```cpp
#define NTP_UTC_OFFSET_SEC   19800L   // IST (India)
// UK:  0L
// PST: -28800L
// EST: -18000L
```

---

##  Safety Checklist

- [ ] Use **2.5 mm² copper wire** for all AC connections
- [ ] SSR-25 DA is **mounted on a heatsink** (mandatory above 10 A)
- [ ] All 220 V terminals are insulated / heat-shrunk inside enclosure
- [ ] A **25 A ceramic fuse** is in series with the Live wire
- [ ] Enclosure lid screwed shut before energising
- [ ] MCB / main circuit breaker is OFF during assembly

---

##  License

MIT — free to use, modify, and share. See [`LICENSE`](LICENSE).

---

##  Acknowledgements

- [SinricPro](https://sinric.pro) — free Alexa/Google smart home backend
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [NTPClient](https://github.com/arduino-libraries/NTPClient)
