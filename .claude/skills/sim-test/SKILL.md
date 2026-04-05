---
name: sim-test
description: Launch the Gravity Simulator with UDP server, send commands, and take screenshots for visual verification
argument-hint: [command sequence description]
---

# Sim Test Skill

Launch the Gravity Simulator with the UDP command server, execute commands, and capture screenshots for evaluation.

## Workflow

### 1. Build (if needed)

```bash
cd D:/Projects/Gravity-Simulator && cmake --build build --config Debug --target Gravity-Simulator 2>&1 | tail -5
```

### 2. Launch the simulator in the background

```bash
cd D:/Projects/Gravity-Simulator/build/bin/Debug && ./Gravity-Simulator.exe --skip-launcher --windowed --udp-server 5555 2>&1
```

Run this as a **background task**. Wait ~3 seconds before sending commands.

### 3. Send UDP commands

Use Python to send commands to the simulator:

```python
import socket

def cmd(c, host="127.0.0.1", port=5555, timeout=2.0):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(timeout)
    s.sendto(c.encode(), (host, port))
    try:
        r = s.recvfrom(4096)[0].decode()
    except socket.timeout:
        r = "TIMEOUT"
    s.close()
    return r
```

### 4. Take a screenshot

Use the existing screenshot script:

```bash
python D:/Projects/Gravity-Simulator/.claude/screenshot.py
```

Then read the output path with the Read tool to view the image.

### 5. Cleanup

Kill the simulator when done:

```bash
taskkill //F //IM Gravity-Simulator.exe
```

## Available UDP Commands

| Command | Syntax | Response |
|---------|--------|----------|
| PING | `PING` | `PONG` |
| ADD | `ADD <x> <y> <xv> <yv> <mass> [TYPE]` | `OK <id>` |
| REMOVE | `REMOVE <id>` | `OK` / `ERR not found` |
| PAUSE | `PAUSE` | `OK` |
| UNPAUSE | `UNPAUSE` | `OK` |
| TOGGLE_PAUSE | `TOGGLE_PAUSE` | `OK paused` / `OK running` |
| TIMESTEP | `TIMESTEP <value>` | `OK` |
| RESET | `RESET` | `OK` |
| KEY | `KEY <key_name>` | `OK` |
| GET_OBJECT | `GET_OBJECT <id>` | `<id> <x> <y> <xv> <yv> <mass> <radius> <type>` |
| LIST | `LIST` | `<id1> <id2> ...` |
| COUNT | `COUNT` | `<N>` |
| STATE | `STATE` | `<paused\|running> <timestep> <count> <iteration>` |
| SET | `SET <key> <value>` | `OK` |
| GET | `GET <key>` | `<value>` |

### ADD type override

Types: `ROCKY`, `TERRESTRIAL`, `GASGIANT`, `BROWNDWARF`, `STAR`, `WHITEDWARF`, `NEUTRONSTAR`, `BLACKHOLE`

Example: `ADD 0 0 0 0 100 WHITEDWARF`

### SET/GET keys

`gravity`, `heat`, `bloom`, `gui`, `autobound`, `fuel_burn`, `render_life`, `timestep`, `paused`

Boolean settings use `0`/`1`. Example: `SET gravity 0`, `GET bloom`

### KEY names

`P` (pause), `R` (reset), `N`, `O`, `A`, `D`, `C`, `G`, `S`, `Q`, `I`, `F`, `B`, `COMMA`, `PERIOD`, `F1` (toggle GUI)

## Notes

- Always `RESET` before spawning test objects to start clean
- After RESET, IDs continue incrementing (they don't reset to 0)
- `SET fuel_burn 0` prevents stars from consuming fuel during tests
- The simulator must be built and running before sending commands
- Screenshot script saves to `.claude/screenshots/` with timestamps
