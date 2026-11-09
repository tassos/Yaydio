#!/usr/bin/env bash
#
# Flashes the Yaydio firmware onto the Arduino Nano, on macOS or Linux.
#
# Usage: ./flash.sh [firmware.hex] [port]
#   firmware.hex  Defaults to firmware.hex next to this script.
#   port          Defaults to the Nano's port, found automatically.
#
# Needs avrdude. On macOS: brew install avrdude

# Written for the bash 3.2 that comes with macOS, so no newer bash features.

set -u

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
FIRMWARE="${1:-$SCRIPT_DIR/firmware.hex}"
PORT="${2:-}"

# Nanos with the new bootloader upload at 115200 baud, with the old bootloader at 57600
BAUD_RATES="115200 57600"

fail() {
    echo
    echo "❌ $1"
    shift
    for line in "$@"; do echo "   $line"; done
    exit 1
}

echo "📻 Yaydio firmware flasher"
echo

# avrdude does the actual flashing
if ! command -v avrdude >/dev/null 2>&1; then
    if [ "$(uname)" = "Darwin" ]; then
        fail "avrdude is not installed." \
            "Install Homebrew from https://brew.sh if you don't have it, then run:" \
            "  brew install avrdude" \
            "and run this script again."
    else
        fail "avrdude is not installed." \
            "Install it with your package manager, e.g.: sudo apt install avrdude"
    fi
fi

if [ ! -f "$FIRMWARE" ]; then
    fail "Firmware file not found: $FIRMWARE" \
        "Put firmware.hex next to this script, or pass its path:" \
        "  ./flash.sh path/to/firmware.hex"
fi
echo "Firmware: $FIRMWARE"

# Find the Nano: its USB chip shows up as a USB serial port
if [ -z "$PORT" ]; then
    shopt -s nullglob
    if [ "$(uname)" = "Darwin" ]; then
        PORTS=(/dev/cu.usbserial* /dev/cu.wchusbserial* /dev/cu.usbmodem*)
    else
        PORTS=(/dev/ttyUSB* /dev/ttyACM*)
    fi
    shopt -u nullglob

    if [ "${#PORTS[@]}" -eq 0 ]; then
        HINTS=(
            "- Is the Nano connected? Its power LED should be on."
            "- Use a USB data cable: many charging cables only carry power."
            "- If the Nano stays dark on a USB-C to USB-C cable, try a USB-A to USB-C"
            "  cable through an adapter or hub: some USB-C Nanos need that."
        )
        if [ "$(uname)" = "Darwin" ]; then
            HINTS+=("- On macOS versions older than 11 (Big Sur), the CH340 USB driver may be missing.")
        fi
        fail "No Arduino Nano found." "${HINTS[@]}"
    elif [ "${#PORTS[@]}" -eq 1 ]; then
        PORT="${PORTS[0]}"
    else
        echo "Several USB serial ports found. Which one is the Nano?"
        echo "(Unplug the Nano and run this script again to see which one disappears.)"
        select CHOICE in "${PORTS[@]}"; do
            if [ -n "$CHOICE" ]; then
                PORT="$CHOICE"
                break
            fi
        done
    fi
fi
echo "Port:     $PORT"
echo

LOG="$(mktemp -t yaydio-flash.XXXXXX)"
trap 'rm -f "$LOG"' EXIT

for BAUD in $BAUD_RATES; do
    echo "Flashing at $BAUD baud... (this takes about 10 seconds)"
    if avrdude -p atmega328p -c arduino -P "$PORT" -b "$BAUD" -D -U "flash:w:$FIRMWARE:i" >"$LOG" 2>&1; then
        echo
        echo "✅ Done! The Yaydio restarts with the new firmware."
        exit 0
    fi

    # Only a Nano that doesn't answer is worth a second try at the other speed
    if ! grep -qiE "not in sync|not responding|programmer is not" "$LOG"; then
        break
    fi
    echo "   No answer at $BAUD baud, the Nano may have the other bootloader."
done

echo
echo "avrdude output:"
tail -n 15 "$LOG" | sed 's/^/   /'
fail "Flashing failed." \
    "- Close anything else using the Nano's port (e.g. a serial monitor) and try again." \
    "- Check that the right port was used: $PORT" \
    "- Unplug the Nano, plug it back in, and try again."
