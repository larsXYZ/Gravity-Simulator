import subprocess
import sys
from datetime import datetime
from pathlib import Path

def install(package):
    subprocess.check_call([sys.executable, "-m", "pip", "install", package, "-q"])

def main():
    out_dir = Path(__file__).parent / "screenshots"
    out_dir.mkdir(exist_ok=True)

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filepath = out_dir / f"screenshot_{timestamp}.png"

    try:
        import pygetwindow as gw
    except ImportError:
        install("PyGetWindow")
        import pygetwindow as gw

    try:
        from PIL import ImageGrab
    except ImportError:
        install("Pillow")
        from PIL import ImageGrab

    windows = gw.getWindowsWithTitle("Gravity Simulator")
    if not windows:
        print("Gravity Simulator window not found, taking full screenshot")
        img = ImageGrab.grab()
    else:
        win = windows[0]
        bbox = (win.left, win.top, win.right, win.bottom)
        img = ImageGrab.grab(bbox=bbox)

    img.save(filepath)
    print(filepath)

if __name__ == "__main__":
    main()
