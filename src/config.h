
// Card contact switch
#define CARD_BTN_PIN A3

// Display
#define DISP_CLK_PIN 10
#define DISP_DIO_PIN 11

// Knobs: "left" and "right" follow the README wiring. On my build the knobs are mounted the other
// way around, so physically the volume knob is on the right and the track knob on the left.
// - Left knob (D2-D4): volume, changes values and albums in the parent menu (press to save)
// - Right knob (D5-D7): tracks, moves through the parent menu (press to select, or to cancel a change)

// Left button
#define LEFT_BTN_PIN 2

// Right button
#define RIGHT_BTN_PIN 5

// Left rotary encoder
#define LEFT_CLK_PIN 4
#define LEFT_DT_PIN 3

// Right rotary encoder
#define RIGHT_CLK_PIN 7
#define RIGHT_DT_PIN 6

// PN532 (NFC)
#define NFC_IRQ_PIN 8
#define NFC_RST_PIN 9

// DY-SV5W (MP3 Player)
#define MP3_RX_PIN 12
#define MP3_TX_PIN 13

// EEPROM addresses for settings that survive a restart
#define EEPROM_VOLUME_ADDR 0
#define EEPROM_MAX_VOLUME_ADDR 1
#define EEPROM_SLEEP_MINUTES_ADDR 2
#define EEPROM_BRIGHTNESS_ADDR 3

// Length of a sleep timer minute. Build with -DSLEEP_MINUTE_MS=1000UL to test it in seconds.
#ifndef SLEEP_MINUTE_MS
#define SLEEP_MINUTE_MS 60000UL
#endif
