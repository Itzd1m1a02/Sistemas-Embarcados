#include "UIElements.h"
#include <stdio.h>
#include <stdlib.h>

// --- LABEL ---
Label::Label(char* label) : ScreenElement(label, false, false) {}

// --- FLOW DATA ---
FlowData::FlowData(char* label, IAlarmController* controller) 
    : ScreenElement("00:00", false, false), controller(controller)
{
    setText(label);
}

void FlowData::update() {
    currentData = controller->getCurrentTime();
    snprintf(displayBuffer, sizeof(displayBuffer), "%02u:%02u", currentData.hour, currentData.minute);
    setText(displayBuffer);
}

// --- EDITABLE DATA ---
EditableData::EditableData(char* label, Screen* target)
    : ScreenElement("00:00", true, false), targetScreen(target)
{
    setText(label);
}

Screen* EditableData::handleEvent(JoyEvent event) {
    if (event == JoyEvent::PRESS) {
        return targetScreen;
    }
    return nullptr; // Correção: Retorna nulo se não for PRESS
}

// --- SELECTABLE DIGIT ---
SelectableDigit::SelectableDigit(uint8_t valorInicial, uint8_t maximo)
    : ScreenElement(charBuffer, true, true), valorAtual(valorInicial), valorMaximo(maximo) 
{
    updateBuffer();
}

void SelectableDigit::updateBuffer() {
    charBuffer[0] = valorAtual + '0'; 
    charBuffer[1] = '\0';
}

uint16_t SelectableDigit::highlightTextIndexes() {
    return 1; 
}

Screen* SelectableDigit::handleEvent(JoyEvent event) {
    if (event == JoyEvent::UP) {
        valorAtual = (valorAtual == valorMaximo) ? 0 : valorAtual + 1;
        updateBuffer();
    } 
    else if (event == JoyEvent::DOWN) {
        valorAtual = (valorAtual == 0) ? valorMaximo : valorAtual - 1;
        updateBuffer();
    }
    return nullptr;
}

uint8_t SelectableDigit::getValue() {
    return valorAtual;
}

// --- MENU OPTION ---
MenuOption::MenuOption(char* label, Screen* target, IAlarmController* ctrl, void (*callback)(IAlarmController*)) 
    : ScreenElement(label, true, false), 
      targetScreen(target),
      controller(ctrl),
      actionCallback(callback) 
{
}

uint16_t MenuOption::highlightTextIndexes() {
    if (textLabel[0] == '[') {
        uint8_t len = getTextLength();
        return ((1 << (len - 1)) - 1) & ~1; 
    }
    return (1 << getTextLength()) - 1;
}

Screen* MenuOption::handleEvent(JoyEvent event) {
    if (event == JoyEvent::PRESS) {
        if (actionCallback != nullptr && controller != nullptr) {
            actionCallback(controller);
        }
        return targetScreen; 
    }
    return nullptr;
}