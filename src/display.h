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

const uint8_t _nothing[] = {0, 0};

const uint8_t _end[] = {
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,  // E
    SEG_C | SEG_E | SEG_G,                  // n
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,  // d
    0,
};

class YDisplay {
   public:
    YDisplay(byte clkPin, byte dioPin) : _display(clkPin, dioPin) { _display.setBrightness(0); }

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
};
