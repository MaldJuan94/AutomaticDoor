# AutomaticDoor

An IoT system that lets you open your building's door remotely from your phone. It works as an **interceptor for a 4-wire intercom system**, where 2 of the wires control the door lock (signal + ground). A PWA web app communicates with an ESP32 microcontroller through Firebase Realtime Database to trigger a relay that opens the door.

## How It Works

```
┌──────────────┐         ┌──────────────────┐         ┌──────────────┐
│   PWA App    │  HTTPS  │  Firebase RTDB   │  Stream  │    ESP32     │
│  (Browser)   │────────>│  /command/       │────────>│  + Relay     │
│              │   PUT   │  opendoor        │         │              │
└──────────────┘         └──────────────────┘         └──────┬───────┘
                                                             │
                                                             │ GPIO 15
                                                             ▼
                                                      ┌──────────────┐
                                                      │  Door Lock   │
                                                      │  (Intercom)  │
                                                      └──────────────┘
```

1. The user taps **"Open the door"** on the PWA.
2. The PWA authenticates with Firebase and sends a command (`PUT`) to Firebase Realtime Database at `/command/opendoor`.
3. The ESP32 listens to Firebase via a persistent stream. When it detects a change, it activates the relay on GPIO 15 for ~1 second.
4. The relay closes the circuit between the two intercom wires (signal and ground), unlocking the door.

## Hardware

| Component | Description |
|---|---|
| **ESP32** (WiFi + Bluetooth) | Microcontroller that connects to Firebase and controls the relay |
| **10A Relay module** | Switches the intercom door-open circuit |
| **Mini voltage regulator** (110V AC → 5V DC) | Powers the ESP32 from the building's electrical supply |
| **4-wire intercom** | Building intercom where 2 wires control the door lock |

### Wiring

```
  ┌─────────────────────────────────────────────────────────┐
  │                    POWER SUPPLY                         │
  │                                                         │
  │  110V AC ──► [Voltage Regulator 110V→5V] ──┬── VIN ESP32│ 
  │                                            └── GND ESP32│
  └─────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────┐
  │                  ESP32 ──► RELAY                        │
  │                                                         │
  │  ESP32 D15  ──► Relay IN2                               │
  │  ESP32 3V3  ──► Relay VCC                               │
  │  ESP32 GND  ──► Relay GND                               │
  └─────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────┐
  │               RELAY ──► INTERCOM                        │
  │                                                         │
  │  Relay NO   ──► Intercom wire (signal)                  │
  │  Relay COM  ──► Intercom wire (ground)                  │
  └─────────────────────────────────────────────────────────┘
```

> **Power**: The mini voltage regulator converts 110V AC to 5V DC and feeds the ESP32 through the **VIN** and **GND** pins.
>
> **Relay**: Powered at **3.3V** from the ESP32's 3V3 pin. The control signal comes from **GPIO 15 (D15)** via the relay's **IN2** port.
>
> **Door**: The relay is configured as **normally open (NO)**: when the ESP32 drives GPIO 15 LOW, the relay closes the circuit between the two intercom wires (signal + ground), unlocking the door.

## Software

### ESP32 Firmware (`esp32.ino`)

- **WiFi**: connects to a stored network (credentials saved in NVS flash via `Preferences`).
- **Bluetooth**: exposes a serial interface (`DOOR_BT`) to configure WiFi credentials from a phone. Send `SSID,password` via Bluetooth to update.
- **Firebase**: authenticates with email/password and listens on `/command` for changes via a persistent stream.
- **Door control**: parses the command `"time,timestamp"` and activates the relay for `time` milliseconds.
- **Watchdog**: automatically restarts every 6 hours to maintain stability.
- **Auto-reconnect**: handles WiFi drops, Firebase disconnections, and stream timeouts with exponential backoff.
- **FCM notifications**: sends a push notification when WiFi reconnects successfully.

### PWA Web App

- **Vanilla HTML/CSS/JS** — no build tools or frameworks needed.
- **Service Worker** with offline caching support.
- **Installable** on mobile devices as a home-screen app (fullscreen mode).
- **First-run setup**: prompts for Firebase email, password, and API key (stored in `localStorage`).
- **Authentication**: signs in via Firebase Auth REST API.
- **Door command**: sends `"1000,{timestamp}"` to Firebase RTDB, triggering the ESP32.
- **Haptic feedback**: vibrates the device on successful door open.

## Project Structure

```
AutomaticDoor/
├── esp32.ino            # ESP32 Arduino firmware
├── index.html           # Main PWA page
├── offline.html         # Offline fallback
├── private.html         # Private page placeholder
├── pwa-manifest.json    # PWA manifest
├── pwa-sw.js            # Service worker
├── css/
│   └── style.css        # Styles
├── js/
│   └── main.js          # PWA logic (auth, door control)
├── img/                 # Icons (iOS, Android, Windows)
└── LICENSE              # MIT License
```

## Setup

### Prerequisites

- [Arduino IDE](https://www.arduino.cc/en/software) with ESP32 board support
- A [Firebase](https://firebase.google.com/) project with:
  - **Realtime Database** enabled
  - **Authentication** (email/password) enabled
  - A registered user account
  - A **service account** with a private key
- A web server or hosting service (e.g. Firebase Hosting, GitHub Pages) to serve the PWA

### 1. Firebase Configuration

1. Create a Firebase project.
2. Enable **Realtime Database** and set up security rules to allow authenticated writes to `/command`.
3. Enable **Email/Password authentication** and create a user.
4. Generate a **service account private key** from Project Settings > Service Accounts.

### 2. ESP32 Firmware

1. Open `esp32.ino` in Arduino IDE.
2. Install the required libraries:
   - `FirebaseESP32` by mobizt
   - `Preferences` (built-in)
   - `BluetoothSerial` (built-in)
3. Update the following constants with your Firebase project details:
   ```cpp
   #define API_KEY "your-firebase-api-key"
   #define DATABASE_URL "your-project.firebaseio.com"
   #define USER_EMAIL "your-email@example.com"
   #define USER_PASSWORD "your-password"
   #define FIREBASE_PROJECT_ID "your-project-id"
   #define FIREBASE_CLIENT_EMAIL "your-service-account@your-project.iam.gserviceaccount.com"
   // Update PRIVATE_KEY with your service account's private key
   ```
4. Select **ESP32 Dev Module** as the board and upload.

### 3. WiFi Configuration via Bluetooth

After flashing, the ESP32 starts with default WiFi credentials. To update:

1. Pair your phone with the Bluetooth device **`DOOR_BT`**.
2. Send your WiFi credentials in the format: `SSID,password`
3. The ESP32 will save the credentials to flash and reconnect.

The onboard LED blinks to indicate status:
- **2 blinks**: WiFi connected successfully
- **3 blinks**: WiFi connection failed
- **4 blinks**: Restarting (watchdog)

### 4. PWA Deployment

1. Host the project files on any static web server (the PWA requires HTTPS for service worker registration).
2. Open the URL on your phone's browser.
3. On first visit, enter your Firebase **email**, **password**, and **API key** in the popup.
4. Tap **"Open the door"** to unlock.
5. Add the app to your home screen for quick access.

## Security Considerations

> **Important**: The current code contains hardcoded credentials and keys. Before deploying to production:

- Replace all hardcoded Firebase credentials in `esp32.ino` with your own.
- Never commit real credentials to a public repository. Use environment variables or a config file excluded via `.gitignore`.
- Set restrictive Firebase Realtime Database security rules to only allow authenticated users.
- Consider implementing token refresh in the PWA for long-lived sessions.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
