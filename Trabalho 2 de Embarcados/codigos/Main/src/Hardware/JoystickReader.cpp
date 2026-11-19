#include "JoystickReader.h"
#include "../LCD/ScreenElement.h" // Acesso ao enum JoyEvent

JoystickReader::JoystickReader(uint8_t xPin, uint8_t yPin, uint8_t buttonPin) {
    pinX = xPin;
    pinY = yPin;
    pinButton = buttonPin;

    lastActionTime = 0;
    lastButtonReading = HIGH;
    buttonPressed = false;
    lastBtnDebounceTime = 0;
}

void JoystickReader::begin() {
    pinMode(pinButton, INPUT_PULLUP);
}

JoyEvent JoystickReader::readJoystick() {
    uint32_t currentMillis = millis();

    // 1. Debounce e detecção de borda do botão digital
    bool reading = digitalRead(pinButton);

    if (reading != lastButtonReading) {
        lastBtnDebounceTime = currentMillis;
        lastButtonReading = reading;
    }

    if ((currentMillis - lastBtnDebounceTime) > debounceDelay) {
        // Se o pino está em nível LOW (pressionado com INPUT_PULLUP) e ainda não registramos o clique
        if (reading == LOW && !buttonPressed) {
            buttonPressed = true;
            lastActionTime = currentMillis;
            return JoyEvent::PRESS;
        } else if (reading == HIGH && buttonPressed) {
            buttonPressed = false;
        }
    }

    // 2. Leitura analógica dos eixos X e Y
    if (currentMillis - lastActionTime > actionDelay) {
        int xVal = analogRead(pinX);
        int yVal = analogRead(pinY);

        JoyEvent event = JoyEvent::NONE;

        // Limiares para evitar zona morta / drift (central = ~512)
        if (yVal < 200) event = JoyEvent::LEFT;
        else if (yVal > 800) event = JoyEvent::RIGHT;
        else if (xVal < 200) event = JoyEvent::DOWN;
        else if (xVal > 800) event = JoyEvent::UP;

        if (event != JoyEvent::NONE) {
            lastActionTime = currentMillis;
            return event;
        }
    }

    return JoyEvent::NONE;
}
