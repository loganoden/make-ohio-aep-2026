# LineSense Grid Mitigation Demo

Make OHI/O 2026 AEP challenge project for monitoring simulated transmission line conditions with an ESP32 and triggering route mitigation logic through a FastAPI backend.

## Overview

This repository contains three parts:

1. `firmware/`: ESP32 code that reads sensors, drives status actuators, and publishes telemetry over MQTT.
2. `backend/`: FastAPI service that ingests telemetry and calculates safer grid routes using A* search.
3. `frontend/`: React + Vite dashboard scaffold for visualization and controls.

## System Architecture

1. ESP32 reads:
	- NTC thermistor (line temperature proxy)
	- LDR (ambient light proxy)
2. ESP32 computes hazard and controls:
	- Green/yellow/red LEDs
	- Buzzer intensity
	- PWM fan speed (wind simulation)
3. ESP32 publishes telemetry to MQTT topic `grid/telemetry`.
4. Backend accepts telemetry at `POST /api/telemetry` and updates edge weights in a `networkx` grid model.
5. If a line status is `WARNING` or `CRITICAL`, backend returns a mitigation plan with an alternate route from `Node_A` to `Node_D`.

## Repository Layout

```text
.
|- backend/
|  |- main.py            # FastAPI app and API routes
|  |- mitigation.py      # Graph model + A* mitigation logic
|  `- requirements.txt   # Python deps for backend (currently empty)
|- firmware/
|  |- platformio.ini     # Board + framework + libraries
|  |- include/secrets.h  # Credentials file placeholder
|  `- src/
|     |- main.cpp        # MQTT publishing loop
|     |- sensors.*       # Sensor initialization and reads
|     `- actuators.*     # LEDs, buzzer, fan PWM logic
`- frontend/
	|- src/App.tsx        # Current React starter page
	`- package.json       # Frontend scripts and deps
```

## Prerequisites

- Python 3.10+
- Node.js 20+
- PlatformIO CLI or VS Code PlatformIO extension
- ESP32 DevKit board
- MQTT broker reachable by ESP32 and backend network (for end-to-end telemetry)

## Backend Setup

From the repository root:

```powershell
cd backend
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install fastapi uvicorn networkx pydantic
```

Run the API:

```powershell
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

Backend endpoints:

- `GET /` - health check
- `POST /api/telemetry` - ingest line telemetry and optionally return mitigation plan
- `GET /api/grid_status` - current graph state and line weights

Example telemetry payload expected by backend:

```json
{
  "line_id": "Line_1",
  "conductor_temperature": 72.5,
  "ambient_temperature": 33.2,
  "wind_speed_simulation": 0.5,
  "status": "WARNING"
}
```

## Firmware Setup (ESP32)

1. Open `firmware/` with PlatformIO.
2. Update Wi-Fi and MQTT settings in `firmware/src/main.cpp`:
	- `ssid`
	- `password`
	- `mqtt_server`
	- `mqtt_port`
3. Build and upload:

```powershell
cd firmware
pio run
pio run -t upload
pio device monitor
```

Current published JSON shape from firmware:

```json
{
  "temperature_c": 41.7,
  "light_level": 1830,
  "hazard_level": 0.23,
  "wind_pwm": 141
}
```

Note: firmware currently publishes to MQTT only. If you want direct backend ingestion, add an MQTT consumer bridge in the backend or push HTTP telemetry from firmware.

## Frontend Setup

From repository root:

```powershell
cd frontend
npm install
npm run dev
```

Useful scripts:

- `npm run dev` - local development server
- `npm run build` - production build
- `npm run preview` - preview production bundle
- `npm run lint` - run ESLint

The frontend is currently the Vite starter page and can be connected next to `GET /api/grid_status` for a live grid dashboard.

## Suggested Local Demo Flow

1. Start MQTT broker.
2. Start backend (`uvicorn ...`).
3. Flash firmware and open serial monitor.
4. Send backend test payloads to simulate mitigation responses.
5. Run frontend for UI work.

Example manual API test:

```powershell
curl -X POST http://localhost:8000/api/telemetry `
  -H "Content-Type: application/json" `
  -d '{"line_id":"Line_1","conductor_temperature":95,"ambient_temperature":34,"wind_speed_simulation":0.2,"status":"CRITICAL"}'
```

## Team Notes

- `backend/requirements.txt` should be populated before final handoff/deployment.
- `firmware/include/secrets.h` is a placeholder comment; credentials are currently hardcoded in `firmware/src/main.cpp` and should be moved to a git-ignored config before production.

## License

This project is licensed under the terms in `LICENSE`.
