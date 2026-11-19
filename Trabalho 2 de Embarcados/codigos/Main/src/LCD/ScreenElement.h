#pragma once
#include <stdint.h>
#include <string.h> // Mudança aqui: padrão de C clássico

// Eventos mapeados do Joystick
enum class JoyEvent { UP, DOWN, LEFT, RIGHT, PRESS, NONE };

class Screen; // Forward declaration

class ScreenElement {
protected:
    char* textLabel;
    const bool selectable;
    const bool editable;

    virtual void setText(char* text) {
        textLabel = text;
    };

public:
    ScreenElement(char* label, bool selectable, bool editable) :
            textLabel(label),
            selectable(selectable),
            editable(editable) {}
    virtual ~ScreenElement() {}

    virtual char* getText() {
        return textLabel;
    }

    bool isSelectable() {
        return selectable;
    };
    
    virtual uint16_t highlightTextIndexes() {
        return (1 << getTextLength()) - 1;
    };

    bool isEditable() {
        return editable;
    };

    virtual uint8_t getTextLength() {
        // Mudança aqui: removido o std::
        return (uint8_t) strlen(textLabel);
    };

    virtual void update() {};

    virtual Screen* handleEvent(JoyEvent event) {
        return nullptr;
    };
};