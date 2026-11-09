# Flashing the Yaydio! firmware

This guide puts the latest firmware on a **Yaydio!** (version 1, with an Arduino Nano) from a Mac. It works the same way on Linux.

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
- A **USB-C data cable** to connect the Arduino Nano to the Mac. Some cables only charge and don't carry data, so if the Nano isn't found, try another cable.
- The two files from the [latest release](../../releases/latest): `firmware.hex` and `flash.sh`.


## 🪜 Steps

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

### 3. Download the firmware

Download `firmware.hex` and `flash.sh` from the [latest release](../../releases/latest). Keep them together in your **Downloads** folder.

### 4. Connect the Yaydio!

Connect the Arduino Nano inside the **Yaydio!** to your Mac with the USB-C cable. A small light on the Nano turns on.

### 5. Flash it

In the Terminal, run:

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


## 🛟 If something goes wrong

The script explains what went wrong. The most common problems:

| Message | What to do |
| ------- | ---------- |
| `avrdude is not installed` | Do steps 1 and 2 again, then close and reopen the Terminal. |
| `Firmware file not found` | Check that `firmware.hex` is in the same folder as `flash.sh`, and that you ran `cd ~/Downloads` first. |
| `No Arduino Nano found` | Check the cable (try another one: it must carry data). If the Nano's light stays off with a USB-C to USB-C cable, use a USB-A to USB-C cable with a USB-C adapter or hub. On macOS older than 11 (Big Sur), install the CH340 USB driver. |
| `Several USB serial ports found` | Pick the Nano from the list. Unplug it and run the script again to see which one disappears. |
| `Flashing failed` | Close anything else that might use the Nano (for example the Arduino IDE), unplug and replug the Nano, and try again. |

You can't break the **Yaydio!** by flashing: if it fails, just try again.


## 🛠️ For developers

The firmware is a [PlatformIO](https://platformio.org) project:

```bash
pio run              # build, the firmware ends up in .pio/build/nanoatmega328/firmware.hex
pio run -t upload    # build and flash
```

`platformio.ini` is set up for the new Nano bootloader (`board = nanoatmega328new`). For a Nano with the old bootloader, use `board = nanoatmega328` instead.

PlatformIO needs Python 3.10 to 3.13. With a newer Python, run it through [uv](https://docs.astral.sh/uv/): `uvx --python 3.13 --from platformio pio run`.
