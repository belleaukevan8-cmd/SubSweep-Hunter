# SubSweep Hunter 📡

A simple SubGHz frequency sweeper for the Flipper Zero that scans a frequency range, reads RSSI, detects signals above a configurable threshold, and logs results to the SD card.

---

## Features

- Sweep 300–928 MHz (full CC1101 range)
- Real-time RSSI display per frequency
- Configurable detection threshold
- Signal counter
- Best frequency tracking (highest RSSI)
- Save results to `/ext/subsweep_logs.txt`

---

## Compatibility

| Firmware | Status |
|---|---|
| Official Flipper Zero stable | ✅ Compatible |
| FZOC (Flipper Zero Official Compiler) | ✅ Compatible |
| Unleashed / RogueMaster | ⚠️ Untested (likely works) |
| Xtreme | ⚠️ Untested |

> ⚠️ This app uses **only public, official Flipper Zero APIs**. No direct CC1101 register access. No private internal APIs.

---

## Project Structure

```
applications_user/subsweep_hunter/
├── application.fam        # App manifest
├── subsweep_hunter.h      # Main app struct & entry point declaration
├── subsweep_hunter.c      # App init / run / free + event wiring
├── app_state.h            # AppState data structure
├── app_state.c            # State manipulation functions
├── subghz_sweeper.h       # SubGHz sweep interface
├── subghz_sweeper.c       # SubGHz sweep implementation (public API only)
├── ui_main.h              # Main view declaration
├── ui_main.c              # Main view (sweep status, RSSI, controls)
├── ui_result.h            # Result view declaration
├── ui_result.c            # Result view (stats + save)
├── storage_manager.h      # Storage interface
├── storage_manager.c      # Log file save to SD card
└── README.md              # This file
```

---

## How to Compile

### Using FBT (Flipper Build Tool)

1. Clone the official Flipper Zero firmware:
   ```bash
   git clone https://github.com/flipperdevices/flipperzero-firmware.git
   cd flipperzero-firmware
   ```

2. Copy this folder to `applications_user/subsweep_hunter/`

3. Build:
   ```bash
   ./fbt fap_subsweep_hunter
   ```

4. The `.fap` file will be in `build/f7-firmware-D/.extapps/`

5. Copy to your Flipper SD card:
   ```
   /ext/apps/RF/subsweep_hunter.fap
   ```

### Using ufbt (micro FBT)

```bash
ufbt build
ufbt launch  # deploy and launch directly via USB
```

---

## Usage

### Controls

| Button | Action |
|---|---|
| **OK** | Start / Stop sweep |
| **UP** | Raise RSSI threshold (+5 dBm) |
| **DOWN** | Lower RSSI threshold (-5 dBm) |
| **RIGHT** | View results screen |
| **BACK** | Quit app |

### On the Result screen

| Button | Action |
|---|---|
| **RIGHT** | Save log to SD card |
| **BACK** | Return to main screen |

---

## Log Format

Results are saved to `/ext/subsweep_logs.txt`:

```
# SubSweep Hunter Log
# Timestamp | Frequency | RSSI | Signals
12345 | 433920000 Hz | -72 dBm | 14
```

---

## Default Settings

| Parameter | Value |
|---|---|
| Start frequency | 300 MHz |
| End frequency | 928 MHz |
| Step size | 1 MHz |
| Default threshold | -80 dBm |
| Sweep speed | ~20 steps/second |

---

## Notes

- The sweep wraps back to the start frequency after reaching the end.
- RSSI readings are approximate; the CC1101 has a known ±3 dBm margin.
- Increasing step size = faster sweep but lower frequency resolution.
- Stack size is set to 2048 bytes — do not add heavy allocations.

---

## License

MIT — free to use, modify, and distribute.
