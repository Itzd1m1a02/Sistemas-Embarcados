#pragma once
#include "../LCD/ScreenElement.h" // Necessário para acessar o enum JoyEvent

class IJoystickReader {
public:
    virtual ~IJoystickReader() {}
    
    // Único método que a tela precisa conhecer
    virtual JoyEvent readJoystick() = 0;
};