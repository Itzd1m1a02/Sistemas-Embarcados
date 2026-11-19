#pragma once
#include "ScreenElement.h"
#include <stdint.h>

struct dataLCD {
    char rows[2][17];
    uint16_t highlightedChars[2];
};

class Screen {
private:
    ScreenElement** elementRows[2];
    uint8_t rowsCount[2];

    volatile uint8_t focusedRowElement;
    volatile uint8_t focusedColElement;

public:
    Screen(); // Correção: Apenas declarativo, sem chaves {}

    // Correção: Assinatura simplificada para evitar confusão de ponteiros do C++
    void setScreenElements(ScreenElement** row0, uint8_t count0, ScreenElement** row1, uint8_t count1);

    void replaceElement(uint8_t row, uint8_t col, ScreenElement* subsequent);
    Screen* processInput(JoyEvent event);
    void render(dataLCD& outData);
};