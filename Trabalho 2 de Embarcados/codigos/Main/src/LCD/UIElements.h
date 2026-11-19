#pragma once
#include "ScreenElement.h"
#include "../Hardware/IAlarmController.h"

// Um Label inerte (usaremos para os dois pontos ":")
class Label : public ScreenElement {
public:
    Label(char* label);
    uint16_t highlightTextIndexes() override { return 0; } 
    Screen* handleEvent(JoyEvent event) override { return nullptr; }
};

class FlowData : public ScreenElement {
private:
    IAlarmController* controller;
    TimeData currentData;
    char displayBuffer[6];

public:
    FlowData(char* label, IAlarmController* controller);
    void update() override;
};

// Dados selecionaveis pra edicao
class EditableData : public ScreenElement {
private:
    Screen* targetScreen;
    
public:
    EditableData(char* label, Screen* targetScreen);
    Screen* handleEvent(JoyEvent event) override;
};

// Um único dígito que pode ser incrementado/decrementado (0-9 ou 0-5)
class SelectableDigit : public ScreenElement {
private:
    uint8_t valorAtual;
    uint8_t valorMaximo;
    char charBuffer[2]; // Guarda "1" + \0
    
public:
    SelectableDigit(uint8_t valorInicial, uint8_t maximo);
    uint8_t getValue();
    void updateBuffer(); 
    
    uint16_t highlightTextIndexes() override;
    Screen* handleEvent(JoyEvent event) override; 
};

// Opção de Menu ([SET], [LIST], [DELETE])
class MenuOption : public ScreenElement {
private:
    Screen* targetScreen;
    void (*actionCallback)(IAlarmController*);
    IAlarmController* controller;

public:
    MenuOption(char* label,
                Screen* target,
                IAlarmController* controller,
                void (*callback)(IAlarmController*) = nullptr);
    uint16_t highlightTextIndexes() override;
    Screen* handleEvent(JoyEvent event) override; 
};