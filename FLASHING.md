# Flashing the Yaydio! firmware

This guide puts the latest firmware on a **Yaydio!** (version 1, with an Arduino Nano), from a [Mac](#-macos), a [Windows PC](#-windows) or [Linux](#-linux).

You don't need to know how to program. It takes about 10 minutes the first time, and a minute for later updates.


## ✨ What's new

- A **parent menu**, opened by holding both knobs for 5 seconds. It's where you assign albums to keycards, and set a max volume, a sleep timer, the display brightness and the equalizer.
- **Repeat and shuffle**: press the track knob to switch between them.
- The display shows the **volume** while you turn it, and the **play mode** and **play/pause** next to the track number.
- The volume and all settings are **remembered** when the **Yaydio!** is switched off.

Your keycards keep working: they don't need to be written again. See the **Usage** section of the [README](README.md) for how to use everything.

> [!NOTE]
> Assigning an album to a keycard has moved into the parent menu. Pressing both knobs once no longer does it. See [Assign an album to a keycard](README.md#assign-an-album-to-a-keycard).


## 🧰 What you need

- The **Yaydio!**. It can stay switched off.
- A **USB data cable** for the Arduino Nano's USB port. Some cables only charge and don't carry data, so if the Nano isn't found, try another cable.
- `firmware.hex` from the [latest release](../../releases/latest). On macOS and Linux, also `flash.sh` from the same page.


## 🍎 macOS

### 1. Install Homebrew (once)

[Homebrew](https://brew.sh) installs the tool that talks to the Arduino. Skip this step if you already have it.

1. Open the **Terminal** app (press <kbd>⌘</kbd> + <kbd>Space</kbd>, type `Terminal` and press <kbd>Enter</kbd>).
2. Copy the install command from [brew.sh](https://brew.sh), paste it into the Terminal, and press <kbd>Enter</kbd>. It asks for your Mac password: nothing appears while you type it, which is normal.
3. When it's done, it shows **Next steps** with a few commands to run. Run them, then close and reopen the Terminal.

### 2. Install avrdude (once)

In the Terminal, run:

```bash
brew install avrdude
```

### 3. Flash it

1. Download `firmware.hex` and `flash.sh` from the [latest release](../../releases/latest). Keep them together in your **Downloads** folder.
2. Connect the Arduino Nano inside the **Yaydio!** to your Mac. A small light on the Nano turns on.
3. In the Terminal, run:

   ```bash
   cd ~/Downloads
   bash flash.sh
   ```

After about 10 seconds, it shows:

```
✅ Done! The Yaydio restarts with the new firmware.
```

That's it! Unplug the cable and enjoy.

> [!TIP]
> If the message mentions "the other bootloader", that's fine: some Nanos need a different speed, and the script tries both.


## 🪟 Windows

### 1. Download avrdude (once)

1. Go to the [latest avrdude release](https://github.com/avrdudes/avrdude/releases/latest) and download the file ending in **`windows-x64.zip`**, e.g. `avrdude-v8.3-windows-x64.zip`. On a PC with an ARM processor (e.g. Snapdragon), take the one ending in `windows-arm64.zip`.
2. In your **Downloads** folder, right-click the zip file, choose **Extract All…**, and extract it to a folder called `avrdude`. It contains `avrdude.exe` and `avrdude.conf`.

### 2. Flash it

1. Download `firmware.hex` from the [latest release](../../releases/latest) and move it into the `avrdude` folder.
2. Connect the Arduino Nano inside the **Yaydio!** to your PC. A small light on the Nano turns on.
3. Find the Nano's port: right-click the **Start** button, choose **Device Manager**, and open **Ports (COM & LPT)**. The Nano shows up as e.g. **USB-SERIAL CH340 (COM3)**. Note the `COM` number.
4. Right-click the **Start** button and choose **Terminal**. Run these commands, with your `COM` number instead of `COM3`:

   ```powershell
   cd $HOME\Downloads\avrdude
   .\avrdude.exe -p atmega328p -c arduino -P COM3 -b 115200 -D -U flash:w:firmware.hex:i
   ```

After about 10 seconds, it ends with `avrdude done.  Thank you.`, and the **Yaydio!** restarts with the new firmware.

> [!TIP]
> If it shows `not in sync` or `programmer is not responding`, the Nano has the old bootloader. Run the same command with `-b 57600` instead of `-b 115200`.


## 🐧 Linux

### 1. Install avrdude (once)

Install it with your distribution's package manager:

| Distribution | Command |
| ------------ | ------- |
| Ubuntu, Debian, Linux Mint | `sudo apt install avrdude` |
| Fedora | `sudo dnf install avrdude` |
| Arch, Manjaro | `sudo pacman -S avrdude` |

### 2. Allow access to the Nano (once)

Only members of a specific group may use USB serial ports. Add yourself to it, then **log out and back in**:

| Distribution | Command |
| ------------ | ------- |
| Ubuntu, Debian, Linux Mint, Fedora | `sudo usermod -aG dialout $USER` |
| Arch, Manjaro | `sudo usermod -aG uucp $USER` |

> [!IMPORTANT]
> On **Ubuntu** and its derivatives, the `brltty` package (for Braille displays) takes over the USB chip of most Nanos, so the Nano disappears right after you plug it in. Unless you use a Braille display, remove it: `sudo apt remove brltty`

### 3. Flash it

1. Download `firmware.hex` and `flash.sh` from the [latest release](../../releases/latest). Keep them together in your **Downloads** folder.
2. Connect the Arduino Nano inside the **Yaydio!** to your computer.
3. In a terminal, run:

   ```bash
   cd ~/Downloads
   bash flash.sh
   ```

After about 10 seconds, it shows `✅ Done!`, and the **Yaydio!** restarts with the new firmware.


## 🛟 If something goes wrong

On macOS and Linux, the script explains what went wrong. The most common problems:

| Problem | What to do |
| ------- | ---------- |
| `avrdude is not installed` | Install avrdude as described above, then close and reopen the terminal. |
| `Firmware file not found` | Check that `firmware.hex` is in the same folder as `flash.sh` (or `avrdude.exe` on Windows), and that you ran the `cd` command first. |
| `No Arduino Nano found`, or no `COM` port in the Device Manager | Check the cable (try another one: it must carry data). If the Nano's light stays off with a USB-C to USB-C cable, use a USB-A to USB-C cable with a USB-C adapter or hub. |
| The Nano has no driver | Windows 10 and 11, and macOS 11 (Big Sur) and later, usually install the driver for the Nano's CH340 USB chip by themselves. If not, install the CH340 driver from the chip maker, [WCH](https://www.wch-ic.com/downloads/CH341SER_EXE.html). |
| `Several USB serial ports found` | Pick the Nano from the list. Unplug it and run the script again to see which one disappears. |
| `No permission to use /dev/ttyUSB0` (Linux) | Do [step 2 of the Linux instructions](#2-allow-access-to-the-nano-once), and log out and back in. |
| Windows blocks `avrdude.exe` | Right-click the zip file, choose **Properties**, tick **Unblock**, click **OK**, and extract it again. |
| `Flashing failed`, or `can't open device` on Windows | Close anything else that might use the Nano (for example the Arduino IDE), unplug and replug the Nano, and try again. On Windows, check the `COM` number. |

You can't break the **Yaydio!** by flashing: if it fails, just try again.


## 🛠️ For developers

The firmware is a [PlatformIO](https://platformio.org) project:

```bash
pio run              # build, the firmware ends up in .pio/build/nanoatmega328/firmware.hex
pio run -t upload    # build and flash
```

`platformio.ini` is set up for the new Nano bootloader (`board = nanoatmega328new`). For a Nano with the old bootloader, use `board = nanoatmega328` instead.

PlatformIO needs Python 3.10 to 3.13. With a newer Python, run it through [uv](https://docs.astral.sh/uv/): `uvx --python 3.13 --from platformio pio run`.
