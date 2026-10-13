![Yaydio!](images/yaydio.gif)

# Yaydio! 📻
**No more scratched CDs!**

**Yaydio!** is a portable music player designed for kids, ditching the hassle of CDs in favor of easy-to-use keycards.


## ✨ Features
- **Keycard playback:** Insert a keycard to instantly play an album.
- **Simple controls:** Play/pause, next/previous track, volume up/down, repeat and shuffle.
- **At a glance:** The display shows the play mode, the track number and whether it's playing or paused.
- **Parent menu:** Assign albums to keycards and set a max volume, a sleep timer, the display brightness and the equalizer, hidden from little fingers.
- **Audio options:** Built-in speaker or connect headphones via the 3.5mm jack.
- **High capacity:**
    - Up to 999 songs per album
    - Up to 9999 albums
    - Up to 65535 songs in total
    - Supports micro SD cards up to 32GB.
- **MP3 compatibility:** Plays all your favorite MP3 files.
- **Affordable & accessible:** Works with cost-effective MF S50 RFID keycards.


## 🕹️ Usage

The left knob is the volume knob and the right knob is the track knob, as wired in [Wiring](#-wiring).

### Play / pause
Inserting a keycard into the **Yaydio!** will automatically start the playback.

Press the left knob to pause or resume the playback.

Remove the keycard to stop the playback.

During the playback, the display shows the [play mode](#repeat--shuffle) and the track number, e.g. `A :12`. The colon is on while playing and blinks while paused.


### Volume control
Turn the left knob to increase or decrease the volume. The display shows the volume (`U 15`) while you turn it.

The volume is remembered when the **Yaydio!** is switched off, and never goes above the [max volume](#parent-menu).


### Next / previous track
Turn the right knob to skip to the next or previous track.

Turning back once goes to the start of the track. Turn back again within a second to go to the previous track.

While [shuffling](#repeat--shuffle), skipping plays a random track of the album.


### Repeat / shuffle
Press the right knob to switch between the play modes:

| Display | Before the track number | Play mode                                |
| ------- | ----------------------- | ---------------------------------------- |
| `ALL`   | `A`                     | Play the album and start over at the end |
| `rEP1`  | `r`                     | Repeat the current song                  |
| `SUFF`  | `S`                     | Shuffle the album                        |

The play mode is shown for a moment after pressing, and stays on the display as the letter before the track number. Every keycard starts in `ALL`.


### Sleep timer
When a [sleep timer](#parent-menu) is set, the playback fades out and pauses once the time is up, counting from when the keycard was inserted. Press the left knob to resume the playback and start a new timer.


### Setup
1. Format the SD card to FAT16 or FAT32 and name it `YAYDIO`.
2. Copy the `README.txt` file onto the SD card.
3. Windows only: Download [DriveSort](https://www.anerty.net/software/file/DriveSort/?lang=en) and copy `DriveSort.exe` onto the SD card.
4. macOS only: Download [FatDriveSorter](https://fat-drive-sorter.netlify.app) and copy `FatDriveSorter.app` onto the SD card.


### Add albums to the SD card
1.  Create a new folder named `0001`, where "1" represents the next album number.
2.  Copy the album's songs into the folder you created.
3.  Rename the first song of the album to `001.mp3`.
4.  Repeat steps 1-3 for each album.
5.  Windows only: Use `DriveSort` to sort the files alphabetically in the SD card's table.
    -   Sort by `long name` and `ascending`.
    -   Remember to press the save button.
    -   You may need to sort every new folder individually and press save after each one.
6.  macOS only: Use `FatDriveSorter` to sort the files alphabetically in the SD card's table.
    -   Next to `Order:` select `Directories first`.
    -   Tick the box next to `Case-sensitive:`.


### Parent menu
Hold both knobs for 5 seconds, with or without a keycard. The bottom bars of the display fill up while you hold them. Let go once they are full to open the parent menu.

In the parent menu:
- Turn the right knob to go through the settings, and press it to change one.
- While changing a setting, turn the left knob to change it and press the left knob to save it, or press the right knob to cancel.
- Select `End` to leave the parent menu. If a keycard is inserted, it starts playing.

| Display | Setting                                                                                                   |
| ------- | --------------------------------------------------------------------------------------------------------- |
| `CArd`  | Assign an album to a keycard, see below                                                                    |
| `UL17`  | Max volume (1 - 30). After saving, it plays for 3 seconds at the max volume so you can hear how loud it is |
| `SL--`  | Sleep timer: off (`--`), 15, 30, 45, 60 or 90 minutes                                                      |
| `br 0`  | Display brightness (0 - 7), shown on the display while you choose it                                       |
| `Eqno`  | Equalizer: Normal (`no`), Pop (`Po`), Rock (`ro`), Jazz (`JA`) or Classic (`CL`), playing while you choose it |
| `End`   | Leave the parent menu                                                                                      |

All settings are remembered when the **Yaydio!** is switched off.


### Assign an album to a keycard

1. Open the [parent menu](#parent-menu) and select `CArd`.
2. Insert the keycard, if it isn't inserted yet. The display shows `----` until it is.
3. Turn the left knob to select the album number. The album starts playing when you stop turning, so you can hear which one it is. `nOnE` clears the keycard.
4. Press the left knob to write the album number to the keycard, or the right knob to cancel.


## 🔨 Make your own

### 🧱 Parts
For a list of parts see [this issue](https://github.com/JakesMD/Yaydio/issues/1).

### 🧵 Wiring

![Wiring Diagram](images/wiring_diagram.png)

| Left Rotary Encoder | Arduino Nano |
| ------------------- | ------------ |
| SW                  | D2           |
| SW GND              | GND          |
| DT                  | D3           |
| CLK                 | D4           |
| GND                 | GND          |

| Right Rotary Encoder | Arduino Nano |
| -------------------- | ------------ |
| SW                   | D5           |
| SW GND               | GND          |
| DT                   | D6           |
| CLK                  | D7           |
| GND                  | GND          |

| 4-Digit 7-Segment 0.36 Inch TM1637 | Arduino Nano |
| ---------------------------------- | ------------ |
| CLK                                | D10          |
| DIO                                | D11          |
| GND                                | GND          |
| 5V                                 | 5V           |

| Card Switch | Arduino Nano |
| ----------- | ------------ |
| NO          | A3           |
| COM         | GND          |

| PN532 NFC Module V3 | Arduino Nano |
| ------------------- | ------------ |
| GND                 | GND          |
| VCC                 | VIN (NOT 5V) |
| SDA                 | A4           |
| SCL                 | A5           |
| RSTPDN              | D9           |
| Switches: ON, OFF (I2C mode)       |

Note: In the images below you'll see that I also connected the IRQ pin to the Arduino. This is not required.

| DY-SV5W Voice Module  | Arduino Nano |
| --------------------- | ------------ |
| -                     | GND          |
| +                     | 5V           |
| TXD                   | D12          |
| RXD                   | D13          |
| Switches: OFF, OFF, ON (UART mode)   |

| DY-SV5W Voice Module  | Speaker      |
| --------------------- | ------------ |
| Speaker +             | +            |
| Speaker -             | -            |


### 🏗️ Assembly

[yaydio.step](yaydio.step) contains the models of the 3D-printed parts.

**Printer:** Bambu Lab A1 with AMS

**Nozzle:** 0.4mm

**Preset:** `0.20mm Standard @BBL A1` with supports enabled

**Filaments:**
- Bambu Lab PLA Matte Mandarin Orange (11300)
- Bambu Lab PLA Matte Marine Blue (11600)
- Bambu Lab PLA Matte Dark Blue (11602)

![Inside view 1](images/inside_view_1.jpg)
![Inside view 2](images/inside_view_2.jpg)
