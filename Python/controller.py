import argparse
import sys
import time
from typing import Dict, Set

import serial
from pynput import keyboard


# Map keyboard keys to Arduino commands
KEY_TO_COMMAND: Dict[str, str] = {
    "w": "W",
    "a": "A",
    "s": "S",
    "d": "D",
}

# Direction priority = most recently pressed key wins
held_keys: Set[str] = set()
press_order = []  # keeps track of recent key presses
last_command = ""
ser = None


def open_serial(port: str, baud: int) -> serial.Serial:
    try:
        connection = serial.Serial(port=port, baudrate=baud, timeout=0.1)
        time.sleep(2.0)
        return connection
    except serial.SerialException as exc:
        print(f"Could not open {port}: {exc}")
        sys.exit(1)


def send_command(command: str) -> None:
    global last_command, ser

    if ser is None:
        return

    if command != last_command:
        ser.write(command.encode("ascii"))
        ser.flush()
        print(f"Sent: {command}")
        last_command = command


def get_active_command() -> str:
    # Most recently pressed held direction wins
    for key in reversed(press_order):
        if key in held_keys:
            return KEY_TO_COMMAND[key]
    return "X"


def normalize_key(key) -> str | None:
    try:
        # Letter keys like w, a, s, d
        if key.char:
            return key.char.lower()
    except AttributeError:
        pass

    # Special keys
    if key == keyboard.Key.space:
        return "space"
    return None


def on_press(key) -> None:
    global press_order

    k = normalize_key(key)
    if k is None:
        return

    # Quit
    if k == "q":
        send_command("X")
        print("Quitting...")
        return False

    # Force stop
    if k == "space":
        held_keys.clear()
        press_order.clear()
        send_command("X")
        return

    # Direction key pressed
    if k in KEY_TO_COMMAND:
        if k not in held_keys:
            held_keys.add(k)
            press_order.append(k)

        send_command(get_active_command())


def on_release(key) -> None:
    k = normalize_key(key)
    if k is None:
        return

    if k in held_keys:
        held_keys.remove(k)

        # Clean old duplicates from press_order
        press_order[:] = [x for x in press_order if x != k]

        # If another direction is still being held, switch to it.
        # Otherwise stop.
        send_command(get_active_command())


def main() -> None:
    global ser

    parser = argparse.ArgumentParser(
        description="Press-and-hold Windows controller for HC-06 RC tank"
    )
    parser.add_argument("port", help="Bluetooth COM port, for example COM7")
    parser.add_argument("--baud", type=int, default=9600, help="Baud rate (default: 9600)")
    args = parser.parse_args()

    ser = open_serial(args.port, args.baud)

    print("Connected.")
    print("Controls:")
    print("  Hold W = forward")
    print("  Hold A = left")
    print("  Hold S = backward")
    print("  Hold D = right")
    print("  Release key = stop (or return to another held direction)")
    print("  SPACE = immediate stop")
    print("  Q = quit")
    print()

    try:
        with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
            listener.join()
    except KeyboardInterrupt:
        print("\nInterrupted by user.")
    finally:
        try:
            send_command("X")
        except Exception:
            pass

        if ser is not None and ser.is_open:
            ser.close()

        print("Serial port closed.")


if __name__ == "__main__":
    main()
