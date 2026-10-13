#include <Arduino.h>
#include <DYPlayerArduino.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

class YMP3Player {
   public:
    // The loudest volume the DY-SV5W supports
    static const uint8_t VOLUME_LIMIT = 30;

    YMP3Player(byte rxPin, byte txPin) : _serial(rxPin, txPin), _player(&_serial) {}

    bool begin() {
        // A never-written EEPROM reads 0xFF, so fall back to the defaults
        uint8_t savedMaxVolume = EEPROM.read(EEPROM_MAX_VOLUME_ADDR);
        if (savedMaxVolume >= 1 && savedMaxVolume <= VOLUME_LIMIT) _maxVolume = savedMaxVolume;

        uint8_t savedVolume = EEPROM.read(EEPROM_VOLUME_ADDR);
        if (savedVolume <= VOLUME_LIMIT) _volume = savedVolume;
        if (_volume > _maxVolume) _volume = _maxVolume;

        Serial.print(F("Starting volume: "));
        Serial.print(_volume);
        Serial.print(F(", max volume: "));
        Serial.println(_maxVolume);

        _player.begin();
        _player.setVolume(_volume);
        _player.setCycleMode(DY::play_mode_t::RepeatDir);
        _player.stop();

        return !(_player.checkPlayState() == DY::play_state_t::Fail);
    }

    void playAlbum(uint16_t album) {
        _endSoundCheck();
        _playFirstTrack(album);
        delay(1000);
        _firstTrackInAlbum = _player.getFirstInDir();
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
    }

    bool isPlaying() { return _isPlaying; }

    uint16_t currentTrack() { return _player.getPlayingSound() - _firstTrackInAlbum + 1; }

      private:
    SoftwareSerial _serial;
    DY::Player _player;

    void _playFirstTrack(uint16_t album) {
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

    void _setVolume(uint8_t volume) {
        _volume = volume;
        _player.setVolume(_volume);
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
    bool _isPlaying = false;
};
