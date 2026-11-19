#pragma once
#include <Arduino.h>
#include "IJoystickReader.h"

class JoystickReader : public IJoystickReader {
  private:
    uint8_t pinX;
    uint8_t pinY;
    uint8_t pinButton;

    // Controle de repetição e timing dos eixos analógicos
    uint32_t lastActionTime;
    const uint32_t actionDelay = 200; // intervalo entre leituras repetidas do joystick

    // Debounce do botão digital
    bool lastButtonReading;
    bool buttonPressed;
    uint32_t lastBtnDebounceTime;
    const uint32_t debounceDelay = 40; // tempo de debounce em ms

  public:
    JoystickReader(uint8_t xPin, uint8_t yPin, uint8_t buttonPin);
    void begin();
    JoyEvent readJoystick() override;
};
