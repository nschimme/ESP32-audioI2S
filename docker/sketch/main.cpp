#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"

Audio audio;

void setup() {
    Serial.begin(115200);
    WiFi.begin("ssid", "password");
    audio.setPinout(27, 26, 25);
    audio.setVolume(21);
    audio.connecttohost("http://stream.antennethueringen.de/live/aac-64/stream.antennethueringen.de/");
}

void loop() {
    audio.loop();
    vTaskDelay(1);
}
