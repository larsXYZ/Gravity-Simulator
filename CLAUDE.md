# Gravity Simulator

## Build

```bash
cd D:/Projects/Gravity-Simulator && cmake --build build --config Debug --target Gravity-Simulator
```

## Testing with UDP Server

### Launch

```bash
cd D:/Projects/Gravity-Simulator/build/bin/Debug && ./Gravity-Simulator.exe --skip-launcher --windowed --udp-server 5555
```

Run as a background task. Wait ~3 seconds before sending commands.

### Send UDP commands (Python)

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

### Take a screenshot

```bash
python D:/Projects/Gravity-Simulator/.claude/screenshot.py
```

Then read the output path with the Read tool to view the image.

### Cleanup

```bash
taskkill //F //IM Gravity-Simulator.exe
```

## UDP Command Reference

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

## Tips

- Always `RESET` before spawning test objects to start clean
- After RESET, IDs continue incrementing (they don't reset to 0)
- `SET fuel_burn 0` prevents stars from consuming fuel during tests
