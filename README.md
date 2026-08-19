# MeshCore Carpeater

A fork of [MeshCore](https://github.com/meshcore-dev/MeshCore) that turns a
repeater into a **mobile, in-vehicle node that reports where it parks** — a
*car repeater*. When the vehicle stops moving for a few minutes, it pushes its
live GPS location to *both* networks at once — a **Meshtastic beacon** and a
fresh **MeshCore advert** — so the node shows up where you actually left it,
on both maps.

> The car node lives in [`examples/car_node/`](examples/car_node/) — the
> [**full documentation is there**](examples/car_node/README.md).
> Prebuilt firmware + flashing instructions:
> [**alexdowney.net/carnode**](https://alexdowney.net/carnode/).

## What it does

- **Park-triggered** — while driving it sends no location, only a light
  presence. Once the GPS fix sits still past `park` seconds (default 5 min),
  one unified update fires on both networks with the same fix.
- **Jitter-proof** — the park logic uses the median of the last ~9 fixes and
  a movement-confirm window, so trunk-mounted GPS multipath can't fake a drive
  or misplace the pin.
- **Repeat sleep** — parked past `sleep` hours (default 20), the node stops
  repeating until it drives again; a car parked overnight isn't a useful
  mobile repeater.
- **Quiet zones** — `carnode zone add <name>` stores a place the vehicle parks
  often (up to 4: home, work, …). Parking in one turns repeat off immediately
  and suppresses every broadcast, so that location never goes on the air.
- **Channel block list** — `block #<channel>` tells the repeater not to
  rebroadcast a MeshCore channel (the hash is derived from the name alone), so
  a noisy channel can be kept off your local airtime.
- **Battery on the map** — each burst carries Meshtastic telemetry, so a car
  parked for days shows its charge draining in any Meshtastic client.
- **Self-recovering** — the radio re-initialises itself after a run of
  transmits the chip reports as dead, and `mtbeacon stats` shows the running
  transmit health. nRF52 boards also arm a hardware watchdog (a hung node
  reboots itself) and answer `get pwrmgt.bootreason` with why they last reset.

Everything is runtime-configurable over serial or an admin remote-CLI session
via two verbs: `mtbeacon` (the Meshtastic beacon's RF + appearance, including
region, modem preset, and an explicit frequency slot) and `carnode` (park
behaviour). See the
[command reference](examples/car_node/README.md#runtime-control).

## Supported boards

| Board | Radio | GPS | Env |
| --- | --- | --- | --- |
| Heltec V4 | ESP32-S3 · SX1262 | external NMEA module | `heltec_v4_carnode` |
| Heltec T114 | nRF52840 · SX1262 | external NMEA module | `Heltec_t114_carnode` |
| Seeed T1000-E | nRF52840 · LR1110 | onboard | `t1000e_dualmode_carnode` |

The T1000-E build is [dual-mode](examples/dualmode/): a 5× button press
switches between the car node (repeater mode) and a normal phone-paired
companion node. Dual-mode combos that pair the car node with a
Meshtastic-presence companion — for the T1000-E, Heltec V4, and Heltec
T114 — live in [**MeshCore-dualmode**](https://github.com/tek126/MeshCore-dualmode).

```
pio run -e heltec_v4_carnode
pio run -e Heltec_t114_carnode
pio run -e t1000e_dualmode_carnode
```

Adding a board is a small platformio.ini env on top of its `*_repeater` env —
see [the build docs](examples/car_node/README.md#building).

## Related repos

The car node is built on the [`meshtastic_beacon`](examples/meshtastic_beacon/)
add-on and reuses its on-air-verified Meshtastic interop math unchanged; the
static fixed-interval beacon is replaced with the park-triggered, GPS-driven
one.

- [**MeshCore-mtbeacon**](https://github.com/tek126/MeshCore-mtbeacon) — the
  beacon add-on itself: the fixed-repeater beacon fleet and the companion-side
  Meshtastic presence.
- [**MeshCore-dualmode**](https://github.com/tek126/MeshCore-dualmode) —
  Dual-Node: one firmware that is both this car node and a
  Meshtastic-presence companion, switched by a 5× button press.
- [**MeshCore**](https://github.com/meshcore-dev/MeshCore) — the upstream
  project; everything not listed above is unmodified base firmware.

## Scope / etiquette

One-way presence only: the node appears on Meshtastic as a named node + map
pin (+ optional chat text) but does **not** route or rebroadcast Meshtastic
traffic. A park floods a MeshCore advert mesh-wide and adds airtime to a
shared public Meshtastic channel — keep `park` reasonable, respect your
region's limits, and don't beacon onto channels you don't operate.

## License

MIT, same as upstream MeshCore (see [`license.txt`](license.txt)). MeshCore
Carpeater is an independent project and is not affiliated with or endorsed by
the MeshCore or Meshtastic projects. Meshtastic® is a registered trademark of
Meshtastic LLC.

## Built with generative AI

The car node, the underlying beacon add-on, and this fork's docs were
developed with the assistance of generative AI (Anthropic's Claude), then
reviewed and tested by a human. Read the source, verify behavior, and test
before relying on it.
