#include <EEPROM.h>
#include <TM1637Display.h>

const uint8_t _fail[] = {
    SEG_A | SEG_E | SEG_F | SEG_G,                  // F
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
    SEG_B | SEG_C,                                  // |
    SEG_D | SEG_E | SEG_F,                          // L
};

const uint8_t _play[] = {
    SEG_C | SEG_E,  // ||
};

const uint8_t _stop[] = {
    SEG_C | SEG_D | SEG_E | SEG_G,  // []
};

const uint8_t _volume[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // U
};

const uint8_t _blank[] = {
    SEG_G,  // -
    SEG_G,  // -
    SEG_G,  // -
    SEG_G,  // -
};

const uint8_t _loading[] = {
    SEG_D | SEG_E | SEG_F,                          // L
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
};

const uint8_t _hi[] = {
    SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // H
    SEG_B | SEG_C,                          // |
};

const uint8_t _clear[] = {
    SEG_C | SEG_E | SEG_G,                          // n
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
    SEG_C | SEG_E | SEG_G,                          // n
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,          // E
};

const uint8_t _card[] = {
    SEG_A | SEG_D | SEG_E | SEG_F,                  // C
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
    SEG_E | SEG_G,                                  // r
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
};

const uint8_t _maxVolume[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // U
    SEG_D | SEG_E | SEG_F,                  // L
};

const uint8_t _sleepTimer[] = {
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,  // S
    SEG_D | SEG_E | SEG_F,                  // L
};

const uint8_t _brightnessLabel[] = {
    SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,  // b
    SEG_E | SEG_G,                          // r
};

const uint8_t _eqLabel[] = {
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,  // E
    SEG_A | SEG_B | SEG_C | SEG_F | SEG_G,  // q
};

// Two-letter names of the equalizer presets, in the order of DY::eq_t
const uint8_t _eqNames[][2] = {
    {SEG_C | SEG_E | SEG_G, SEG_C | SEG_D | SEG_E | SEG_G},                          // no (Normal)
    {SEG_A | SEG_B | SEG_E | SEG_F | SEG_G, SEG_C | SEG_D | SEG_E | SEG_G},          // Po (Pop)
    {SEG_E | SEG_G, SEG_C | SEG_D | SEG_E | SEG_G},                                  // ro (Rock)
    {SEG_B | SEG_C | SEG_D | SEG_E, SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G},  // JA (Jazz)
    {SEG_A | SEG_D | SEG_E | SEG_F, SEG_D | SEG_E | SEG_F},                          // CL (Classic)
};

const uint8_t _off[] = {SEG_G, SEG_G};  // --

const uint8_t _nothing[] = {0, 0};

const uint8_t _end[] = {
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,  // E
    SEG_C | SEG_E | SEG_G,                  // n
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,  // d
    0,
};

class YDisplay {
   public:
    static const uint8_t MAX_BRIGHTNESS = 7;

    YDisplay(byte clkPin, byte dioPin) : _display(clkPin, dioPin) { _display.setBrightness(_brightness); }

    void begin() {
        // A never-written EEPROM reads 0xFF, so fall back to the default brightness
        uint8_t savedBrightness = EEPROM.read(EEPROM_BRIGHTNESS_ADDR);
        if (savedBrightness <= MAX_BRIGHTNESS) _brightness = savedBrightness;
        _display.setBrightness(_brightness);
    }

    uint8_t brightness() { return _brightness; }

    // Used by whatever is shown next, without saving it, so you can see it while choosing
    void previewBrightness(uint8_t brightness) { _display.setBrightness(brightness); }

    void setBrightness(uint8_t brightness) {
        _brightness = brightness;
        _display.setBrightness(_brightness);
        EEPROM.update(EEPROM_BRIGHTNESS_ADDR, _brightness);
        Serial.print(F("Brightness: "));
        Serial.println(_brightness);
    }

    void clear() {
        _display.clear();
        Serial.println(F("Display cleared"));
    }

    void showTrack(uint16_t track, bool isPlaying) {
        _display.setSegments(isPlaying ? _play : _stop, 1);
        _display.showNumberDec(track, false, 3, 1);
        Serial.print(F("Track: "));
        Serial.println(track);
    }

    void showVolume(uint8_t volume) {
        _display.setSegments(_volume, 1);
        _display.showNumberDec(volume, false, 3, 1);
    }

    // Fills the lower vertical bars from left to right, two per digit, so 0 - 8 steps
    void showProgress(uint8_t steps) {
        uint8_t segments[4] = {0, 0, 0, 0};
        for (uint8_t i = 0; i < steps; i++) segments[i / 2] |= i % 2 == 0 ? SEG_E : SEG_C;
        _display.setSegments(segments);
    }

    void showCard() { _display.setSegments(_card); }

    // "UL" (volume limit) and the value, which can be hidden to make it blink while editing
    void showMaxVolume(uint8_t maxVolume, bool isValueVisible) {
        _display.setSegments(_maxVolume, 2, 0);
        if (isValueVisible) {
            _display.showNumberDec(maxVolume, false, 2, 2);
        } else {
            _display.setSegments(_nothing, 2, 2);
        }
    }

    // "SL" (sleep) and the minutes, or "--" when off. The value can be hidden to make it blink.
    void showSleepTimer(uint8_t minutes, bool isValueVisible) {
        _display.setSegments(_sleepTimer, 2, 0);
        if (!isValueVisible) {
            _display.setSegments(_nothing, 2, 2);
        } else if (minutes == 0) {
            _display.setSegments(_off, 2, 2);
        } else {
            _display.showNumberDec(minutes, false, 2, 2);
        }
    }

    // "br" and the brightness on the last digit. The value can be hidden to make it blink.
    void showBrightness(uint8_t brightness, bool isValueVisible) {
        _display.setSegments(_brightnessLabel, 2, 0);
        _display.setSegments(_nothing, 1, 2);
        if (isValueVisible) {
            _display.showNumberDec(brightness, false, 1, 3);
        } else {
            _display.setSegments(_nothing, 1, 3);
        }
    }

    // "Eq" and the name of the preset. The name can be hidden to make it blink.
    void showEq(uint8_t eq, bool isValueVisible) {
        _display.setSegments(_eqLabel, 2, 0);
        _display.setSegments(isValueVisible ? _eqNames[eq] : _nothing, 2, 2);
    }

    void showEnd() { _display.setSegments(_end); }

    void showAlbum(uint16_t album) {
        if (album == 0) {
            _display.setSegments(_clear);
        } else {
            _display.showNumberDec(album, true, 4, 0);
        }
        Serial.print(F("Album: "));
        Serial.println(album);
    }

    void showFail() {
        _display.setSegments(_fail); 
        Serial.println(F("Fail"));
    }

    void showBlank() {
        _display.setSegments(_blank); 
        Serial.println(F("Blank"));
    }

    void showLoading() {
        _display.setSegments(_loading); 
        Serial.println(F("Loading"));
    }

    void showHi() {
        _display.clear();
        _display.setSegments(_hi, 2, 1);
        Serial.println(F("Hi"));
    }

   private:
    TM1637Display _display;
    uint8_t _brightness = 0;
};
