#include <Arduino.h>
#include <DYPlayerArduino.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

class YMP3Player {
   public:
    // The loudest volume the DY-SV5W supports
    static const uint8_t VOLUME_LIMIT = 30;

    // Its equalizer presets, in the order of DY::eq_t: Normal, Pop, Rock, Jazz, Classic
    static const uint8_t EQ_COUNT = 5;

    enum YPlayMode { ALBUM_PLAY_MODE, REPEAT_ONE_PLAY_MODE, SHUFFLE_PLAY_MODE, PLAY_MODE_COUNT };

    YMP3Player(byte rxPin, byte txPin) : _serial(rxPin, txPin), _player(&_serial) {}

    bool begin() {
        // A never-written EEPROM reads 0xFF, so fall back to the defaults
        uint8_t savedMaxVolume = EEPROM.read(EEPROM_MAX_VOLUME_ADDR);
        if (savedMaxVolume >= 1 && savedMaxVolume <= VOLUME_LIMIT) _maxVolume = savedMaxVolume;

        uint8_t savedVolume = EEPROM.read(EEPROM_VOLUME_ADDR);
        if (savedVolume <= VOLUME_LIMIT) _volume = savedVolume;
        if (_volume > _maxVolume) _volume = _maxVolume;

        uint8_t savedSleepMinutes = EEPROM.read(EEPROM_SLEEP_MINUTES_ADDR);
        if (savedSleepMinutes <= MAX_SLEEP_MINUTES) _sleepMinutes = savedSleepMinutes;

        uint8_t savedEq = EEPROM.read(EEPROM_EQ_ADDR);
        if (savedEq < EQ_COUNT) _eq = savedEq;

        Serial.print(F("Starting volume: "));
        Serial.print(_volume);
        Serial.print(F(", max volume: "));
        Serial.print(_maxVolume);
        Serial.print(F(", sleep timer: "));
        Serial.print(_sleepMinutes);
        Serial.print(F(", equalizer: "));
        Serial.println(_eq);

        _player.begin();
        _player.setVolume(_volume);
        _player.setEq((DY::eq_t)_eq);
        _player.setCycleMode(DY::play_mode_t::RepeatDir);
        _player.stop();

        return !(_player.checkPlayState() == DY::play_state_t::Fail);
    }

    void playAlbum(uint16_t album) {
        _endSoundCheck();
        _playFirstTrack(album);
        delay(1000);
        _firstTrackInAlbum = _player.getFirstInDir();
        _startSleepTimer();
        Serial.print(F("Playing album: "));
        Serial.println(album);
    }

    // Like playAlbum, but without the delay needed for track numbers, so the knobs stay responsive
    void previewAlbum(uint16_t album) {
        _endSoundCheck();
        _playFirstTrack(album);
        Serial.print(F("Previewing album: "));
        Serial.println(album);
    }

    // Plays a few seconds at the max volume, so you can hear how loud it is
    void soundCheck(uint16_t album) {
        _endSoundCheck();
        _player.setVolume(_maxVolume);
        _playFirstTrack(album);
        _isSoundCheckActive = true;
        _soundCheckStartedAt = millis();
        Serial.print(F("Sound check at volume: "));
        Serial.println(_maxVolume);
    }

    void togglePlayback() {
        _isPlaying = !_isPlaying;

        if (_isPlaying) {
            _player.play();
            // Resuming after the sleep timer put the player to sleep gives it a new timer
            if (_sleepState == SLEEP_DONE) _startSleepTimer();
            Serial.println(F("Resuming playback"));
        } else {
            _player.pause();
            Serial.println(F("Pausing playback"));
        }
    }

    void loop() {
        // Save the volume once the knob has rested, rather than on every step, to spare the EEPROM
        if (_isVolumeUnsaved && millis() - _volumeChangedAt > 2000) {
            EEPROM.update(EEPROM_VOLUME_ADDR, _volume);
            _isVolumeUnsaved = false;
            Serial.print(F("Saved volume: "));
            Serial.println(_volume);
        }

        if (_isSoundCheckActive && millis() - _soundCheckStartedAt > 3000) stop();

        _updateSleepTimer();
    }

    // Set the volume ourselves instead of using volumeIncrease/Decrease, so we always know its value
    void increaseVolume() {
        if (_volume < _maxVolume) _setVolume(_volume + 1);
    }

    void decreaseVolume() {
        if (_volume > 0) _setVolume(_volume - 1);
    }

    uint8_t volume() { return _volume; }

    uint8_t maxVolume() { return _maxVolume; }

    // Saved straight away, as it only changes from the parent menu
    void setMaxVolume(uint8_t maxVolume) {
        _maxVolume = maxVolume;
        EEPROM.update(EEPROM_MAX_VOLUME_ADDR, _maxVolume);
        Serial.print(F("Max volume: "));
        Serial.println(_maxVolume);

        if (_volume > _maxVolume) _setVolume(_maxVolume);
    }

    YPlayMode playMode() { return _playMode; }

    // Album (repeat the album), repeat the current track, shuffle the album, and back to album
    YPlayMode cyclePlayMode() {
        _setPlayMode((YPlayMode)((_playMode + 1) % PLAY_MODE_COUNT));
        return _playMode;
    }

    uint8_t eq() { return _eq; }

    // Applies the equalizer without saving it, so you can hear it while choosing
    void previewEq(uint8_t eq) { _player.setEq((DY::eq_t)eq); }

    // Saved straight away, as it only changes from the parent menu
    void setEq(uint8_t eq) {
        _eq = eq;
        _player.setEq((DY::eq_t)_eq);
        EEPROM.update(EEPROM_EQ_ADDR, _eq);
        Serial.print(F("Equalizer: "));
        Serial.println(_eq);
    }

    uint8_t sleepMinutes() { return _sleepMinutes; }

    // 0 turns the sleep timer off. Saved straight away, as it only changes from the parent menu.
    void setSleepMinutes(uint8_t sleepMinutes) {
        _sleepMinutes = sleepMinutes;
        EEPROM.update(EEPROM_SLEEP_MINUTES_ADDR, _sleepMinutes);
        Serial.print(F("Sleep timer: "));
        Serial.println(_sleepMinutes);
    }

    void next() {
        _player.next();
        _isPlaying = true;
        Serial.println(F("Playing next track"));
    }

    void previous() {
        _player.previous();
        _isPlaying = true;
        Serial.println(F("Playing previous track"));
    }

    void stop() {
        Serial.println(F("Stopping playback"));
        _player.stop();
        _isPlaying = false;
        _endSoundCheck();
        _cancelSleepTimer();
    }

    bool isPlaying() { return _isPlaying; }

    uint16_t currentTrack() { return _player.getPlayingSound() - _firstTrackInAlbum + 1; }

      private:
    static const uint8_t MAX_SLEEP_MINUTES = 120;
    static const unsigned long SLEEP_FADE_MS = 30000;

    enum YSleepState { SLEEP_OFF, SLEEP_RUNNING, SLEEP_DONE };

    SoftwareSerial _serial;
    DY::Player _player;

    void _setPlayMode(YPlayMode playMode) {
        _playMode = playMode;
        switch (_playMode) {
            case ALBUM_PLAY_MODE:
                _player.setCycleMode(DY::play_mode_t::RepeatDir);
                break;
            case REPEAT_ONE_PLAY_MODE:
                _player.setCycleMode(DY::play_mode_t::RepeatOne);
                break;
            default:
                _player.setCycleMode(DY::play_mode_t::RandomDir);
                break;
        }
        Serial.print(F("Play mode: "));
        Serial.println(_playMode);
    }

    void _playFirstTrack(uint16_t album) {
        // Every album starts in album mode, so a repeat or shuffle doesn't stick to the next card
        if (_playMode != ALBUM_PLAY_MODE) _setPlayMode(ALBUM_PLAY_MODE);

        // "/0001/001.mp3" is 13 characters plus the terminating zero
        char path[14];
        snprintf(path, sizeof(path), "/%04d/001.mp3", album);

        _player.stop();
        _player.playSpecifiedDevicePath(DY::device_t::Sd, path);
        _isPlaying = true;
    }

    // Goes back to the normal volume, so whatever plays next isn't at the max volume
    void _endSoundCheck() {
        if (!_isSoundCheckActive) return;
        _isSoundCheckActive = false;
        _player.setVolume(_volume);
    }

    // Goes back to the normal volume if it was stopped in the middle of fading out
    void _cancelSleepTimer() {
        if (_isFadingOut()) _player.setVolume(_volume);
        _sleepState = SLEEP_OFF;
    }

    void _startSleepTimer() {
        _cancelSleepTimer();
        if (_sleepMinutes == 0) return;

        _sleepState = SLEEP_RUNNING;
        _sleepStartedAt = millis();
        _fadeVolume = 255;
    }

    bool _isFadingOut() {
        return _sleepState == SLEEP_RUNNING && millis() - _sleepStartedAt >= _sleepMinutes * SLEEP_MINUTE_MS;
    }

    // Once the time is up, fades the volume out and then pauses the playback
    void _updateSleepTimer() {
        if (!_isFadingOut()) return;

        unsigned long fadeElapsed = millis() - _sleepStartedAt - _sleepMinutes * SLEEP_MINUTE_MS;
        if (fadeElapsed >= SLEEP_FADE_MS) {
            Serial.println(F("Sleep timer done"));
            _player.pause();
            _isPlaying = false;
            _player.setVolume(_volume);
            _sleepState = SLEEP_DONE;
            return;
        }

        // Follows _volume, so turning the volume knob during the fade still has an effect
        uint8_t fadeVolume = _volume * (SLEEP_FADE_MS - fadeElapsed) / SLEEP_FADE_MS;
        if (fadeVolume != _fadeVolume) {
            _fadeVolume = fadeVolume;
            _player.setVolume(_fadeVolume);
        }
    }

    void _setVolume(uint8_t volume) {
        _volume = volume;
        // While fading out, the sleep timer sets the volume
        if (!_isFadingOut()) _player.setVolume(_volume);
        _volumeChangedAt = millis();
        _isVolumeUnsaved = true;
        Serial.print(F("Volume: "));
        Serial.println(_volume);
    }

    uint16_t _firstTrackInAlbum;
    uint8_t _volume = 15;
    uint8_t _maxVolume = VOLUME_LIMIT;
    unsigned long _volumeChangedAt = 0;
    bool _isVolumeUnsaved = false;
    bool _isSoundCheckActive = false;
    unsigned long _soundCheckStartedAt = 0;
    YPlayMode _playMode = ALBUM_PLAY_MODE;
    uint8_t _eq = 0;
    uint8_t _sleepMinutes = 0;
    YSleepState _sleepState = SLEEP_OFF;
    unsigned long _sleepStartedAt = 0;
    uint8_t _fadeVolume = 255;
    bool _isPlaying = false;
};
