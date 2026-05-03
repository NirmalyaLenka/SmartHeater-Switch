# 🔌 Wiring Guide

> **⚠️ DANGER — MAINS VOLTAGE (220 V AC)**  
> Switch off your MCB / circuit breaker before making any AC connections.  
> If unsure, hire a licensed electrician for the mains wiring portion.

---

## Connection Overview

```
220 V AC Mains
     │
     ├─── NEUTRAL ──────────────────────────────────► Water Heater (Neutral)
     │
     └─── LIVE ──► [25 A Fuse] ──► SSR Terminal 1
                                        │
                                   SSR Terminal 2 ──► Water Heater (Live)

                   SSR Control (+) ──► NodeMCU D1 (GPIO5)
                   SSR Control (−) ──► NodeMCU GND

220 V ──► HLK-PM01 (AC IN) ──► 5V DC out ──► NodeMCU VIN + GND
```

---

## Pin-by-Pin Table

| NodeMCU Pin | Label | Connects To | Notes |
|-------------|-------|-------------|-------|
| D1 (GPIO5)  | RELAY_PIN | SSR Control (+) | 3.3 V HIGH triggers SSR on |
| GND | GND | SSR Control (−) | Shared ground |
| D4 (GPIO2)  | LED_PIN | Onboard LED | Active LOW, used for status blink |
| D2 (GPIO4)  | BUTTON_PIN | Push button → GND | Internal pull-up enabled |
| VIN | 5V in | HLK-PM01 5V out | Powering NodeMCU from mains module |
| GND | GND | HLK-PM01 GND | Common ground |

---

## Step-by-Step AC Wiring (inside the enclosure)

### Step 1 — Bring in mains cable
Run a 3-wire (Live / Neutral / Earth) cable from your distribution box into the enclosure via a cable gland.

### Step 2 — Fuse on Live wire
Connect the Live wire → one end of the 25 A fuse holder → other end to SSR **Output Terminal 1 (Load+)**.

### Step 3 — Neutral pass-through
Connect Neutral directly to one terminal block. Run a second wire from the same terminal block to the Water Heater's Neutral input.

### Step 4 — SSR output to heater
**SSR Output Terminal 2 (Load−)** → Water Heater Live input.

### Step 5 — SSR control from NodeMCU
- SSR **Control (+)** → NodeMCU **D1**  
- SSR **Control (−)** → NodeMCU **GND**

The ESP8266 GPIO outputs 3.3 V which is within the SSR's 3–32 V trigger range. No transistor needed.

### Step 6 — Power the NodeMCU
- HLK-PM01 **L (AC)** → Live wire (after the fuse)  
- HLK-PM01 **N (AC)** → Neutral wire  
- HLK-PM01 **+Vo (5 V)** → NodeMCU **VIN**  
- HLK-PM01 **−Vo (GND)** → NodeMCU **GND**

### Step 7 — Optional physical button
- One leg of push button → NodeMCU **D2**  
- Other leg → NodeMCU **GND**  
(Firmware uses INPUT_PULLUP — no external resistor needed)

---

## SSR Heatsink Sizing

| Load Current | Heatsink Required |
|-------------|-------------------|
| < 8 A | None (SSR stays cool) |
| 8–15 A | Small aluminium plate (50×50 mm) |
| 15–25 A | Medium heatsink (80×80 mm) or mount SSR flush against metal enclosure wall |

The SSR-25 DA dissipates approximately **1.0–1.5 V × load current** as heat.  
At 20 A → ~20–30 W → use a proper heatsink to avoid thermal shutdown.

---

## Enclosure Layout Suggestion

```
┌─────────────────────────────────────────┐
│  [Cable Gland In]   [Cable Gland Out]   │
│                                         │
│  ┌──────────┐    ┌───────────────────┐  │
│  │  NodeMCU │    │    SSR-25 DA      │  │
│  │  ESP8266 │    │  (on heatsink)    │  │
│  └──────────┘    └───────────────────┘  │
│                                         │
│  ┌──────────┐    ┌───────────────────┐  │
│  │ HLK-PM01 │    │  Fuse Holder      │  │
│  │  5V PSU  │    │  Terminal Block   │  │
│  └──────────┘    └───────────────────┘  │
│                        [LED] [Button]   │
└─────────────────────────────────────────┘
```

---

## Checking Your Wiring Before Power-On

- [ ] Fuse is in place
- [ ] SSR output terminals fully insulated (heat-shrink or terminal covers)
- [ ] HLK-PM01 AC input terminals fully covered
- [ ] Enclosure lid can close without pinching any wire
- [ ] NodeMCU is NOT touching the SSR body (SSR shell may be at mains potential internally)
- [ ] Earth/ground wire connected to enclosure if metal
