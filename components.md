# 🔩 Components List & Why Each Was Chosen

> Every component below was selected for **cost-effectiveness, safety at 20–25 A loads, and ease of sourcing** in India.  
> Estimated total build cost: **₹350–₹600** (~$4–$7 USD).

---

## 1. 🧠 NodeMCU ESP8266 (v3 / LoLin)
**Role:** The brain — runs all code, handles WiFi, drives the relay, syncs time.

| Spec | Value |
|------|-------|
| MCU | Tensilica L106 @ 80 / 160 MHz |
| Flash | 4 MB |
| WiFi | 802.11 b/g/n (2.4 GHz) |
| GPIO | 11 usable pins |
| Voltage | 3.3 V logic, 5 V via USB/Vin |
| Cost | ₹130–₹180 |

**Why this over Arduino + WiFi shield?**  
WiFi is *built-in*. An Arduino Uno + ESP-01 combo would cost more, be harder to wire, and consume more power. The ESP8266 runs the entire stack (WiFi, NTP, Alexa cloud, timer logic) on a single cheap chip. It flashes straight from Arduino IDE.

**Why not ESP32?**  
ESP32 would work too, but costs ₹50–₹100 more and has more horsepower than we need. ESP8266 is sufficient for this project.

---

## 2. ⚡ 25 A SSR (Solid-State Relay) — e.g., Fotek SSR-25 DA
**Role:** The actual switch that breaks the AC mains circuit to the water heater.

| Spec | Value |
|------|-------|
| Output rating | 25 A @ 24–380 V AC |
| Control input | 3–32 V DC (3.3 V GPIO triggers it directly) |
| Isolation | Optical (opto-isolated) |
| Switch time | < 10 ms |
| Zero-crossing | Yes (reduces electrical noise) |
| Heatsink needed | Yes for loads > 10 A |
| Cost | ₹120–₹200 |

**Why SSR over a mechanical relay module?**

| Feature | SSR 25 A | Mechanical Relay |
|---------|----------|-----------------|
| Current (safe) | 25 A | 10 A max (module versions) |
| Sparking | None | Yes (contacts arc) |
| Lifespan | 10 million cycles | ~100,000 cycles |
| Noise / EMI | Very low | Clicks, spikes |
| Speed | Silent, fast | Audible click |
| GPIO friendly | Yes (3.3 V trigger) | Yes (via transistor) |

A water heater draws **8–22 A** (1800 W–4800 W at 220 V). Mechanical relay modules rated at 10 A are **unsafe** for this load. The SSR-25 DA handles 25 A continuously — with a heatsink it stays cool.

**Important:** Mount the SSR on an aluminium heatsink (or a metal DIN-rail enclosure wall). At 20 A, it dissipates ~20 W.

---

## 3. 🔌 HLK-PM01 (5 V / 600 mA AC-DC converter)
**Role:** Powers the NodeMCU directly from 220 V AC mains — no wall adapter needed.

| Spec | Value |
|------|-------|
| Input | 90–264 V AC |
| Output | 5 V DC, 600 mA |
| Efficiency | ~80% |
| Size | 34 × 20 × 15 mm |
| Cost | ₹80–₹130 |

**Why this?**  
It fits inside a standard electrical box and converts mains power to 5 V for the NodeMCU without a bulky external adapter. The HLK series is CE/UL certified and well-proven for IoT projects. 600 mA is enough — NodeMCU needs ~80 mA idle, ~200 mA peak (during WiFi tx).

**Alternative:** USB charger + micro-USB cable works during prototyping but is not ideal for a sealed enclosure.

---

## 4. 🔴 Ceramic Fuse — 25 A, 250 V (with inline fuse holder)
**Role:** Overcurrent protection — if anything shorts, the fuse blows instead of wiring.

| Spec | Value |
|------|-------|
| Rating | 25 A, 250 V AC |
| Type | Fast-blow ceramic |
| Cost | ₹15–₹30 |

**Why mandatory?**  
The SSR does not self-protect against short circuits. A 25 A fuse on the Live wire means worst-case the fuse blows, not your wiring or your house MCB.

---

## 5. 🟢 Status LED (3 mm, green + 220 Ω resistor)
**Role:** Visual indicator — ON when heater is running, blinks when auto-off imminent.

NodeMCU has an onboard LED (active LOW on GPIO2 / D4) which we reuse in code. A separate external LED on D5 can be added for enclosure visibility.

**Cost:** ₹2

---

## 6. 🔘 Momentary Push Button (6×6 mm PCB type)
**Role:** Physical override — press to toggle heater ON/OFF without phone/voice.

Useful when WiFi is down. Debounced in firmware (300 ms window).

**Cost:** ₹5–₹10

---

## 7. 🧵 Wiring & Enclosure
| Item | Notes | Cost |
|------|-------|------|
| 2.5 mm² copper wire | For AC mains (Live, Neutral to SSR) | ₹30/m |
| Jumper wires | NodeMCU to SSR, button, LED | ₹20 |
| ABS electrical box (100×68×50 mm) | Encloses everything safely | ₹40–₹80 |
| DIN rail clip (optional) | Mounts SSR inside box | ₹20 |
| Terminal block connectors | Clean AC wiring | ₹20 |

---

## 📦 Full BOM Summary

| # | Component | Qty | Est. Cost (₹) |
|---|-----------|-----|---------------|
| 1 | NodeMCU ESP8266 v3 | 1 | 150 |
| 2 | SSR-25 DA (25 A Solid State Relay) | 1 | 160 |
| 3 | HLK-PM01 (5V AC-DC) | 1 | 100 |
| 4 | 25 A ceramic fuse + holder | 1 | 25 |
| 5 | Status LED + resistor | 1 | 5 |
| 6 | Push button | 1 | 10 |
| 7 | Wiring + enclosure + misc | — | 100 |
| | **TOTAL** | | **~₹550** |

---

## ⚠️ Safety Notes

1. **Always work with mains power OFF** at the MCB before touching wiring.
2. The SSR output terminals carry **220 V AC** — insulate all connections inside the enclosure.
3. Use **2.5 mm²** (minimum) wire for all AC connections carrying the heater load.
4. The HLK-PM01 output (5 V) is safe to touch, but its input side is mains voltage — keep it covered.
5. Ground the metal enclosure if using one.
6. Get a local electrician to do the final panel wiring if you are not confident with mains AC.
