#include <Arduino.h>
#include <TM1637Display.h>

#include "button.h"
#include "config.h"
#include "display.h"
#include "mp3_player.h"
#include "nfc.h"
#include "rotary_encoder.h"

YButton cardBtn(CARD_BTN_PIN);

YButton leftBtn(LEFT_BTN_PIN);
YButton rightBtn(RIGHT_BTN_PIN);

YRotaryEncoder leftRotaryEncoder(LEFT_CLK_PIN, LEFT_DT_PIN);
YRotaryEncoder rightRotaryEncoder(RIGHT_CLK_PIN, RIGHT_DT_PIN);

YDisplay display(DISP_CLK_PIN, DISP_DIO_PIN);

YNFC nfc(NFC_IRQ_PIN, NFC_RST_PIN);

YMP3Player mp3Player(MP3_RX_PIN, MP3_TX_PIN);

enum YMODE {
    NO_CARD_MODE,
    READ_CARD_MODE,
    WRITE_CARD_MODE,
    PLAYER_MODE,
    SET_ALBUM_MODE,
    NO_ALBUM_MODE,
    FAIL_MODE,
    PARENT_MODE,
};

YMODE mode = NO_CARD_MODE;
bool hasModeChanged = true;
bool isNewModeComing = false;

// Holding both knobs for PARENT_HOLD_MS opens the parent menu
enum YPARENT_HOLD { HOLD_IDLE, HOLD_ACTIVE, HOLD_WAIT_RELEASE };
const unsigned long PARENT_HOLD_MS = 5000;
const uint8_t PARENT_HOLD_STEPS = 8;

YPARENT_HOLD parentHold = HOLD_IDLE;
unsigned long parentHoldStartedAt = 0;
uint8_t parentHoldProgress = 0;

enum YPARENT_MENU_ITEM { CARD_ITEM, MAX_VOLUME_ITEM, END_ITEM, MENU_ITEM_COUNT };

uint8_t parentMenuItem = CARD_ITEM;
bool isWaitingForCard = false;

bool isEditingMaxVolume = false;
uint8_t editedMaxVolume = 0;
unsigned long maxVolumeChangedAt = 0;

bool isPreviewPending = false;
unsigned long albumChangedAt = 0;

unsigned long lastUpdate = 0;
unsigned long currentMillis = 0;

uint16_t selectedAlbum = 1;

void updateTrackScreen() {
    display.showTrack(mp3Player.currentTrack(), mp3Player.isPlaying());
    lastUpdate = currentMillis;
}

// Shown until the next track screen update, about a second later
void updateVolumeScreen() {
    display.showVolume(mp3Player.volume());
    lastUpdate = currentMillis;
}

// Preview the album once the knob has rested, so scrolling past albums doesn't start each one
void selectAlbum(uint16_t album) {
    selectedAlbum = album;
    display.showAlbum(selectedAlbum);
    albumChangedAt = currentMillis;

    if (selectedAlbum == 0) {
        mp3Player.stop();
        isPreviewPending = false;
    } else {
        isPreviewPending = true;
    }
}

void changeMode(YMODE newMode) {
    mode = newMode;
    isNewModeComing = true;
}

void showParentMenuItem() {
    switch (parentMenuItem) {
        case CARD_ITEM:
            display.showCard();
            break;
        case MAX_VOLUME_ITEM:
            display.showMaxVolume(mp3Player.maxVolume(), true);
            break;
        case END_ITEM:
            display.showEnd();
            break;
    }
}

// The value blinks while editing, but stays on for a moment after each turn so it's easy to read
// Only redraws when something changed, as updating the display on every loop slows down the knobs
void showEditedMaxVolume(bool isForced) {
    static bool wasValueVisible = false;
    static uint8_t shownMaxVolume = 0;

    bool isValueVisible = currentMillis - maxVolumeChangedAt < 800 || (currentMillis / 400) % 2 == 0;
    if (!isForced && isValueVisible == wasValueVisible && editedMaxVolume == shownMaxVolume) return;

    display.showMaxVolume(editedMaxVolume, isValueVisible);
    wasValueVisible = isValueVisible;
    shownMaxVolume = editedMaxVolume;
}

// Restores the screen of the modes the parent hold can start from, after a cancelled hold
void redrawScreen() {
    switch (mode) {
        case NO_CARD_MODE:
            display.showHi();
            break;
        case PLAYER_MODE:
            updateTrackScreen();
            break;
        case NO_ALBUM_MODE:
            display.showBlank();
            break;
        default:
            break;
    }
}

// Shows a progress bar while both knobs are held and opens the parent menu when it is full.
// Returns true while the hold is in progress, so the current mode ignores the knobs.
bool handleParentHold() {
    bool areBothBtnsPressed = leftBtn.isPressed() && rightBtn.isPressed();

    switch (parentHold) {
        case HOLD_IDLE:
            if (!areBothBtnsPressed) return false;
            if (mode != NO_CARD_MODE && mode != PLAYER_MODE && mode != NO_ALBUM_MODE) return false;

            parentHold = HOLD_ACTIVE;
            parentHoldStartedAt = currentMillis;
            parentHoldProgress = 0;
            display.showProgress(0);
            return true;

        case HOLD_ACTIVE: {
            if (!areBothBtnsPressed) {
                Serial.println(F("Parent hold cancelled"));
                parentHold = HOLD_WAIT_RELEASE;
                redrawScreen();
                return true;
            }

            uint8_t progress = (currentMillis - parentHoldStartedAt) * PARENT_HOLD_STEPS / PARENT_HOLD_MS;
            if (progress >= PARENT_HOLD_STEPS) {
                Serial.println(F("Opening parent menu"));
                display.showProgress(PARENT_HOLD_STEPS);
                changeMode(PARENT_MODE);
                parentHold = HOLD_WAIT_RELEASE;
            } else if (progress != parentHoldProgress) {
                parentHoldProgress = progress;
                display.showProgress(progress);
            }
            return true;
        }

        case HOLD_WAIT_RELEASE:
            // Swallow the knob releases, so they don't also pause the playback or select something
            if (leftBtn.isReleased() && rightBtn.isReleased()) parentHold = HOLD_IDLE;
            return true;
    }
    return false;
}

bool tryInitializePeripherals(int attempts = 5) {
    for (int i = 0; i < attempts; ++i) {
        if (nfc.begin() && mp3Player.begin()) {
            return true;
        }
        Serial.println(F("delay 500ms"));
        delay(500);
    }
    return false;
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("Hi!"));

    if (!tryInitializePeripherals()) {
        Serial.println(F("Failed to initialize peripherals!"));
        display.showFail();
        while (true);
    }
    Serial.println(F("Peripherals initialized successfully!"));
}

void loop() {
    cardBtn.loop();
    leftBtn.loop();
    rightBtn.loop();
    leftRotaryEncoder.loop();
    rightRotaryEncoder.loop();
    mp3Player.loop();

    currentMillis = millis();

    if (handleParentHold()) {
        // Keep a mode change for when the modes run again, so its setup isn't skipped
        if (isNewModeComing) {
            hasModeChanged = true;
            isNewModeComing = false;
        }
        return;
    }

    switch (mode) {
        case NO_CARD_MODE:
            if (hasModeChanged) {
                mp3Player.stop();
                display.showHi();
            }
            if (cardBtn.isPressed()) {
                changeMode(READ_CARD_MODE);
                Serial.println(F("Card button pressed, switching to READ_CARD_MODE"));
            }

            break;

        case READ_CARD_MODE:
            if (hasModeChanged) {
                display.showLoading();
                mp3Player.stop();
            }
            switch (nfc.read()) {
                case NO_CARD:
                    changeMode(NO_CARD_MODE);
                    Serial.println(F("No card detected, switching to NO_CARD_MODE"));
                    break;
                case SUCCESS:
                    selectedAlbum = nfc.album();
                    changeMode(selectedAlbum == 0 ? NO_ALBUM_MODE : PLAYER_MODE);
                    Serial.println(F("Card read successfully, switching to appropriate mode"));
                    break;
                case FAIL:
                    changeMode(FAIL_MODE);
                    Serial.println(F("Failed to read card, switching to FAIL_MODE"));
                    break;
            }
            break;

        case WRITE_CARD_MODE:
            if (hasModeChanged) display.showLoading();
            switch (nfc.setAlbum(selectedAlbum)) {
                case NO_CARD:
                    changeMode(NO_CARD_MODE);
                    break;
                case SUCCESS:
                    if (selectedAlbum == 0) {
                        changeMode(NO_ALBUM_MODE);
                    } else {
                        changeMode(PLAYER_MODE);
                    }
                    break;
                case FAIL:
                    changeMode(FAIL_MODE);
                    break;
            }
            break;

        case PLAYER_MODE:
            if (hasModeChanged) {
                mp3Player.playAlbum(selectedAlbum);
                updateTrackScreen();
            }

            if (cardBtn.isReleased()) {
                changeMode(NO_CARD_MODE);
                break;
            }

            if (leftBtn.justReleased() || rightBtn.justReleased()) {
                mp3Player.togglePlayback();
                updateTrackScreen();
            }

            if (leftRotaryEncoder.justTurnedCW()) {
                mp3Player.increaseVolume();
                updateVolumeScreen();
            } else if (leftRotaryEncoder.justTurnedCCW()) {
                mp3Player.decreaseVolume();
                updateVolumeScreen();
            }

            if (rightRotaryEncoder.justTurnedCW()) {
                mp3Player.next();
                updateTrackScreen();
            } else if (rightRotaryEncoder.justTurnedCCW()) {
                mp3Player.previous();
                updateTrackScreen();
            }

            if (currentMillis - lastUpdate > 1000) updateTrackScreen();

            break;

        case SET_ALBUM_MODE:
            if (hasModeChanged) selectAlbum(selectedAlbum == 0 ? 1 : selectedAlbum);

            if (cardBtn.isReleased()) {
                changeMode(NO_CARD_MODE);
                break;
            }

            if (rightBtn.justReleased()) {
                changeMode(WRITE_CARD_MODE);
            } else if (selectedAlbum < 9999 && rightRotaryEncoder.justTurnedCW()) {
                selectAlbum(selectedAlbum + 1);
            } else if (selectedAlbum > 0 && rightRotaryEncoder.justTurnedCCW()) {
                selectAlbum(selectedAlbum - 1);
            }

            if (isPreviewPending && currentMillis - albumChangedAt > 700) {
                mp3Player.previewAlbum(selectedAlbum);
                isPreviewPending = false;
            }

            break;

        case NO_ALBUM_MODE:
            if (hasModeChanged) {
                mp3Player.stop();
                display.showBlank();
            }
            if (cardBtn.isReleased()) {
                changeMode(NO_CARD_MODE);
                break;
            }
            if (rightBtn.justReleased()) {
                changeMode(READ_CARD_MODE);
                break;
            }
            break;

        case PARENT_MODE:
            if (hasModeChanged) {
                mp3Player.stop();
                parentMenuItem = CARD_ITEM;
                isWaitingForCard = false;
                isEditingMaxVolume = false;
                showParentMenuItem();
            }

            // Edited with the volume knob: turn to change, press the same knob to save
            if (isEditingMaxVolume) {
                if (leftBtn.justReleased()) {
                    mp3Player.setMaxVolume(editedMaxVolume);
                    mp3Player.soundCheck(selectedAlbum == 0 ? 1 : selectedAlbum);
                    isEditingMaxVolume = false;
                    showParentMenuItem();
                    break;
                }
                if (rightBtn.justReleased()) {
                    // Cancel without saving
                    isEditingMaxVolume = false;
                    showParentMenuItem();
                    break;
                }

                if (editedMaxVolume < YMP3Player::VOLUME_LIMIT && leftRotaryEncoder.justTurnedCW()) {
                    editedMaxVolume++;
                    maxVolumeChangedAt = currentMillis;
                } else if (editedMaxVolume > 1 && leftRotaryEncoder.justTurnedCCW()) {
                    editedMaxVolume--;
                    maxVolumeChangedAt = currentMillis;
                }
                showEditedMaxVolume(false);
                break;
            }

            if (isWaitingForCard) {
                if (cardBtn.isPressed()) {
                    changeMode(SET_ALBUM_MODE);
                } else if (rightBtn.justReleased()) {
                    isWaitingForCard = false;
                    showParentMenuItem();
                }
                break;
            }

            if (rightRotaryEncoder.justTurnedCW()) {
                parentMenuItem = (parentMenuItem + 1) % MENU_ITEM_COUNT;
                showParentMenuItem();
            } else if (rightRotaryEncoder.justTurnedCCW()) {
                parentMenuItem = (parentMenuItem + MENU_ITEM_COUNT - 1) % MENU_ITEM_COUNT;
                showParentMenuItem();
            }

            if (rightBtn.justReleased()) {
                if (parentMenuItem == END_ITEM) {
                    // Plays the inserted card again, if there is one
                    changeMode(NO_CARD_MODE);
                } else if (parentMenuItem == MAX_VOLUME_ITEM) {
                    isEditingMaxVolume = true;
                    editedMaxVolume = mp3Player.maxVolume();
                    maxVolumeChangedAt = currentMillis;
                    showEditedMaxVolume(true);
                } else if (cardBtn.isPressed()) {
                    changeMode(SET_ALBUM_MODE);
                } else {
                    isWaitingForCard = true;
                    display.showBlank();
                }
            }
            break;

        case FAIL_MODE:
            if (hasModeChanged) {
                lastUpdate = currentMillis;
                display.showFail();
                mp3Player.stop();
            } else if (currentMillis - lastUpdate > 2000) {
                changeMode(NO_CARD_MODE);
            }
            break;
    }

    if (isNewModeComing) {
        hasModeChanged = true;
        isNewModeComing = false;
    } else {
        hasModeChanged = false;
    }
}
