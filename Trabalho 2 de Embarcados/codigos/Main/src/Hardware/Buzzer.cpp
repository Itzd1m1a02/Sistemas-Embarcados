#include "Buzzer.h"

namespace {
struct Note {
    uint16_t frequency;
    uint16_t duration;
};

// Frequências das notas usadas na 5ª Sinfonia (em Dó Menor)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_DS4 311 // Mi Bemol (Eb4)
#define NOTE_F4  349
#define NOTE_G4  392

const Note alarmMelody[] = {
    // Tã - tã - tã...
    {NOTE_G4, 120}, {0, 50},
    {NOTE_G4, 120}, {0, 50},
    {NOTE_G4, 120}, {0, 50},
    // TÃAAA! (Mi bemol longo)
    {NOTE_DS4, 800}, {0, 400}, 

    // Tã - tã - tã...
    {NOTE_F4, 120}, {0, 50},
    {NOTE_F4, 120}, {0, 50},
    {NOTE_F4, 120}, {0, 50},
    // TÃAAA! (Ré longo, ainda mais dramático)
    {NOTE_D4, 1000}, {0, 500},

    // Uma leve variação para dar continuidade antes do loop
    {NOTE_G4, 120}, {0, 50},
    {NOTE_G4, 120}, {0, 50},
    {NOTE_G4, 120}, {0, 50},
    {NOTE_DS4, 600}, {0, 200},
    
    {NOTE_F4, 120}, {0, 50},
    {NOTE_F4, 120}, {0, 50},
    {NOTE_F4, 120}, {0, 50},
    {NOTE_C4, 1000}, {0, 1000} // Finaliza em Dó (C4) com uma pausa grande antes de repetir
};

const uint8_t melodyLength = sizeof(alarmMelody) / sizeof(alarmMelody[0]);
}

Buzzer::Buzzer(uint8_t buzzerPin)
    : pinBuzzer(buzzerPin), isActive(false), melodyIndex(0), noteStartedAt(0) {}

void Buzzer::begin() {
    pinMode(pinBuzzer, OUTPUT);
    deactivate();
}

void Buzzer::activate() {
    if (isActive) {
        return;
    }

    isActive = true;
    melodyIndex = 0;
    noteStartedAt = millis();
    playCurrentNote();
}

void Buzzer::update() {
    if (!isActive || millis() - noteStartedAt < alarmMelody[melodyIndex].duration) {
        return;
    }

    // A mágica matemática que faz a música repetir para sempre
    melodyIndex = (melodyIndex + 1) % melodyLength;
    noteStartedAt = millis();
    playCurrentNote();
}

void Buzzer::playCurrentNote() {
    const uint16_t frequency = alarmMelody[melodyIndex].frequency;
    if (frequency == 0) {
        noTone(pinBuzzer);
    } else {
        tone(pinBuzzer, frequency);
    }
}

void Buzzer::deactivate() {
    noTone(pinBuzzer);
    isActive = false;
}

bool Buzzer::IsActive() {
    return isActive;
}