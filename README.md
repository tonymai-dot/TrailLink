# 🧭 TrailLink — GPS/LoRa Tracker for Outdoor Sports

> Locate your group in real time, without a phone, without a network, without a subscription.

[![Version](https://img.shields.io/badge/version-1.3-orange)](https://github.com/your-repo/traillink)
[![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue)](https://www.lilygo.cc/)
[![Radio](https://img.shields.io/badge/radio-LoRa%20868%20MHz-green)](https://www.semtech.com/)
[![License](https://img.shields.io/badge/license-Proprietary-red)](./LICENSE)

---

## 📖 Overview

**TrailLink** is an open-hardware GPS/LoRa tracking device designed for outdoor sports groups. It allows 2 to 10 people to see each other's position in real time using LoRa radio — no cellular network, no satellite subscription required.

Built on the **LilyGo T3 LoRa (ESP32-S3)**, TrailLink is compact, standalone, and fully autonomous thanks to its integrated solar panel and OLED display.

### Supported sports
- 🪂 **Paragliding** (primary use — cross / bivouac / competition)
- 🥾 Hiking / Trail running
- 🎿 Off-piste skiing / Freeride
- 🚵 MTB / Enduro

---

## ✨ Features

### Core (MVP)

| Feature | Description |
|---------|-------------|
| **F1 — Real-time GPS** | Position fix every 1–5 s, transmitted via LoRa 868 MHz. Range: 10–50 km line-of-sight |
| **F2 — Group mesh** | Proprietary lightweight protocol, up to 10 devices per group, configurable group ID |
| **F3 — OLED display** | 4 pages: compass view, group list, my position, system status — 100% standalone |
| **F4 — Manual SOS** | 3-second press triggers continuous buzzer on ALL group devices until acknowledged |

### Advanced

| Feature | Description |
|---------|-------------|
| **F5 — Fall detection** | MPU6050 accelerometer detects hard impact; auto-SOS after 60 s of no movement |
| **F6 — Wi-Fi config** | Captive portal (no app required) for settings + OTA firmware updates |
| **F7 — Track history** | Last 500 GPS positions in RAM + GPX export via Wi-Fi or microSD |
| **F8 — Night mode** | Adjustable OLED brightness, auto-dim at night |

---

## 🔧 Hardware

### Base module

| Component | Spec |
|-----------|------|
| MCU | ESP32-S3 dual-core 240 MHz, 8 MB Flash, 8 MB PSRAM |
| LoRa radio | SX1262 / SX1276 — 868 MHz (EU ISM band) |
| Display | OLED 0.96″ SSD1306 (128×64 px) |
| Wi-Fi | 802.11 b/g/n — AP mode for config |
| Bluetooth | BLE 5.0 — companion app ready |
| USB | USB-C charge & programming |
| Power | 3.7 V Li-Po via JST 1.25 mm |

### Required add-ons

| Component | Ref | Cost | Role |
|-----------|-----|------|------|
| GPS module | Ublox MAX-M10S or NEO-M8N | ~8–20 € | Precise positioning |
| Active buzzer 3.3 V | Piezo 12 mm | ~0.50 € | SOS alert (direct GPIO) |
| Li-Po battery | 3.7 V 2000 mAh slim | ~5–8 € | 6 h+ guaranteed |
| LoRa antenna | 868 MHz SMA flexible | ~2–4 € | Optimal radio range |
| Accelerometer | MPU6050 | ~1.50 € | Fall detection |
| Solar panel | 5 V mono 60×90 mm (~1.5 W) | ~2–4 € | Extended autonomy |
| Solar charger | CN3791 or TP4056 | ~0.80 € | Solar + battery management |

> ⚠️ The T3 LoRa does **not** include GPS or a buzzer. Both are required.

---

## 📡 Radio Specifications

| Parameter | Value |
|-----------|-------|
| Frequency | 868 MHz (ISM Europe — license-free) |
| Spreading Factor | SF12 (max range) |
| Bandwidth | 125 kHz |
| TX Power | 20 dBm (100 mW) |
| Range (paragliding, altitude) | 30–50 km line-of-sight |
| Range (hiking / mountain) | 3–10 km |
| Range (ski / MTB / forest) | 1–5 km |
| Useful bitrate SF12 | ~250 bps |
| Emission cycle | Every 10–30 s (duty cycle < 1%) |

### Packet format (20 bytes)

```
│ Group ID (2B) │ User ID (1B) │ Lat (4B) │ Lon (4B) │ Alt (3B) │ Speed (2B) │ Heading (2B) │ Battery (1B) │ Status (1B) │
```

- Optional AES-128 encryption (shared key per group)
- CSMA collision avoidance
- ACK only for SOS packets

---

## 🏗️ Physical Design

| Spec | Value |
|------|-------|
| Dimensions | 85 × 55 × 25 mm |
| Weight | < 90 g (battery + solar panel included) |
| Material | PETG (UV & moisture resistant) |
| Sealing | IP65 — silicone O-ring between two half-shells |
| Mounting | Integrated harness clip + carabiner loop |
| Buttons | 2 × IP67 waterproof (usable with thick gloves) |
| Color | Bright orange (rescue visibility) |
| Screen | Anti-glare film on OLED window |

---

## 💻 Firmware

**Stack:** C++ / Arduino Framework via PlatformIO

**Key libraries:**
- [`RadioLib`](https://github.com/jgromes/RadioLib) — LoRa radio
- [`TinyGPS++`](https://github.com/mikalhart/TinyGPSPlus) — GPS parsing
- [`U8g2`](https://github.com/olikraus/u8g2) — OLED rendering

**Architecture:** FreeRTOS multi-task (GPS, LoRa, display, buttons run in parallel)

```
firmware/
├── src/
│   ├── main.cpp          # Entry point & FreeRTOS task init
│   ├── gps.cpp / .h      # GPS acquisition (TinyGPS++)
│   ├── lora.cpp / .h     # LoRa TX/RX (RadioLib)
│   ├── display.cpp / .h  # OLED pages (U8g2)
│   ├── sos.cpp / .h      # SOS logic & buzzer
│   ├── fall.cpp / .h     # Fall detection (MPU6050)
│   ├── webui.cpp / .h    # Wi-Fi config portal + OTA
│   └── protocol.h        # Packet structure definition
├── platformio.ini
└── README.md
```

### Building

```bash
# Install PlatformIO
pip install platformio

# Clone the repo
git clone https://github.com/your-username/traillink.git
cd traillink/firmware

# Build
pio run

# Upload (USB-C)
pio run --target upload

# OTA update (Wi-Fi)
# Hold button 5 s → connect to TrailLink-XXXX → open 192.168.4.1
```

---

## ⚡ Power & Autonomy

| Scenario | Duration |
|----------|----------|
| Battery only (2000 mAh) | **6 h minimum** (guaranteed) |
| Battery recommended | **12 h** |
| Full sun (paragliding) | **Unlimited** — solar compensates consumption |
| USB-C full charge | ~2 h |

> 💡 The 60×90 mm solar panel produces ~600–900 mA in direct sunlight. The device consumes ~130–150 mA — solar **more than compensates** during flight.

---

## 💰 Bill of Materials (BOM)

| Component | Cost (100+ units) |
|-----------|-------------------|
| LilyGo T3 LoRa (ESP32-S3) | ~18–22 € |
| GPS module (NEO-M8N) | ~8–12 € |
| Solar panel 5 V mono | ~2–4 € |
| Solar charge controller (CN3791) | ~0.80 € |
| Li-Po battery 2000 mAh | ~4–6 € |
| O-ring + M2 screws | ~0.60 € |
| Active buzzer 3.3 V | ~0.50 € |
| LoRa antenna 868 MHz | ~2–3 € |
| Accelerometer MPU6050 | ~1.50 € |
| PETG enclosure (3D printed) | ~3–5 € |
| Cables + connectors | ~1.50 € |
| Assembly + test + flash | ~5–8 € |
| **TOTAL** | **~50–68 €** |

---

## 🗺️ Roadmap

```
Phase 1 — Functional prototype  [Month 1–3]
  ✅ T3 LoRa + GPS + buzzer + accelerometer wiring
  ✅ Base firmware: GPS → LoRa → OLED display
  ✅ Range tests in real conditions (paragliding, mountain)
  ✅ First 3D-printed enclosure

Phase 2 — Shippable MVP  [Month 4–6]
  ⬜ Full firmware: SOS, fall detection, WebUI, GPX export
  ⬜ Final OLED UI — paragliding pilot field tests
  ⬜ CE self-declaration (ETSI EN 300 220 + EN 301 489)
  ⬜ IP65 enclosure with harness clip + anti-glare film
  ⬜ Launch — 50 pilot units

Phase 3 — Industrial scale  [Month 7–12]
  ⬜ TrailLink Pro: Ublox MAX-M10S + RF amp (50 km guaranteed)
  ⬜ Injection-molded IP65 enclosure (1 000 units)
  ⬜ Optional smartphone companion app
  ⬜ Paragliding school & outdoor club partnerships
  ⬜ Distribution: Parapente Acro, Air Aventure, etc.
```

---

## ⚖️ Regulatory

- **868 MHz ISM band** — license-free in Europe, duty cycle < 1% (auto-enforced by firmware)
- **CE marking** required for EU sale — self-declaration via ETSI EN 300 220 + EN 301 489 (~2 500–5 000 €)
- **EN 18031** cybersecurity compliance required from August 2025
- **WEEE** — ADEME registration required (~50–200 €/year)

---

## 🆚 Comparison

| Product | Price | Limitation vs TrailLink |
|---------|-------|------------------------|
| Garmin inReach Mini 2 | ~350–400 € | Mandatory satellite subscription ~15 €/month |
| Spot Gen4 | ~150 € | Subscription + no group tracking |
| Meshtastic DIY | ~30–50 € | Not a finished product, technical assembly required |
| Live Track (Forclaz) | ~40–80 € | Limited range, no screen, GSM only |
| **TrailLink** | **89–149 €** | ✅ No subscription, LoRa 30–50 km, standalone OLED, fall SOS |

---

## 📄 License

© 2025 TrailLink Project — All rights reserved.  
This project is currently **proprietary**. Licensing terms will be defined before public release.

---

*TrailLink v1.3 — Cahier des Charges (June 2025)*
