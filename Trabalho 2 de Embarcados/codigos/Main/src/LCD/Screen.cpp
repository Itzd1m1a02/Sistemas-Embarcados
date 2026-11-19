#include "Screen.h"
#include <string.h>

Screen::Screen() {
    elementRows[0] = nullptr;
    elementRows[1] = nullptr;
    rowsCount[0] = 0;
    rowsCount[1] = 0;
    focusedRowElement = 0;
    focusedColElement = 0;
}

void Screen::setScreenElements(ScreenElement** row0, uint8_t count0, ScreenElement** row1, uint8_t count1) {
    elementRows[0] = row0;
    elementRows[1] = row1;
    rowsCount[0] = count0;
    rowsCount[1] = count1;
    focusedRowElement = 0;
    focusedColElement = 0;
}

void Screen::replaceElement(uint8_t row, uint8_t col, ScreenElement* subsequent) {
    if (row < 2 && col < rowsCount[row]) {
        elementRows[row][col] = subsequent;
    }
}

Screen* Screen::processInput(JoyEvent event) {
    static bool isEditMode = false; 

    if (rowsCount[0] == 0 && rowsCount[1] == 0) return this;

    ScreenElement* focused = nullptr;
    if (focusedRowElement < 2 && focusedColElement < rowsCount[focusedRowElement]) {
        focused = elementRows[focusedRowElement][focusedColElement];
    }

    if (focused == nullptr) return this;

    if (isEditMode) {
        if (event == JoyEvent::PRESS) {
            isEditMode = false; 
            return this;
        }
        else if (event == JoyEvent::UP || event == JoyEvent::DOWN) {
            focused->handleEvent(event);
        }
        else if (event == JoyEvent::LEFT) {
            if (focusedColElement > 0) {
                uint8_t tempCol = focusedColElement - 1;
                while (tempCol > 0 && !elementRows[focusedRowElement][tempCol]->isEditable()) {
                    tempCol--;
                }
                if (elementRows[focusedRowElement][tempCol]->isEditable()) {
                    focusedColElement = tempCol;
                }
            }
        }
        else if (event == JoyEvent::RIGHT) {
            if (focusedColElement < rowsCount[focusedRowElement] - 1) {
                uint8_t tempCol = focusedColElement + 1;
                while (tempCol < rowsCount[focusedRowElement] - 1 && !elementRows[focusedRowElement][tempCol]->isEditable()) {
                    tempCol++;
                }
                if (elementRows[focusedRowElement][tempCol]->isEditable()) {
                    focusedColElement = tempCol;
                }
            }
        }
        return this;
    }

    if (event == JoyEvent::PRESS) {
        if (focused->isEditable()) {
            isEditMode = true; 
            return this;
        } else if (focused->isSelectable()) {
            Screen* target = focused->handleEvent(event);
            if (target != nullptr) return target;
        }
    } 
    else if (event == JoyEvent::LEFT) {
        if (focusedColElement > 0) {
            do {
                focusedColElement--;
            } while (focusedColElement > 0 && 
                    !elementRows[focusedRowElement][focusedColElement]->isSelectable());
        }
    } 
    else if (event == JoyEvent::RIGHT) {
        if (focusedColElement < rowsCount[focusedRowElement] - 1) {
            do {
                focusedColElement++;
            } while (focusedColElement < rowsCount[focusedRowElement] - 1 && 
                    !elementRows[focusedRowElement][focusedColElement]->isSelectable());
        }
    }
    else if (event == JoyEvent::DOWN) {
        if (focusedRowElement == 0 && rowsCount[1] > 0) {
            focusedRowElement = 1;
            if (focusedColElement >= rowsCount[1]) focusedColElement = rowsCount[1] - 1;
            while (!elementRows[1][focusedColElement]->isSelectable() && focusedColElement > 0) {
                focusedColElement--;
            }
        }
    }
    else if (event == JoyEvent::UP) {
        if (focusedRowElement == 1 && rowsCount[0] > 0) {
            focusedRowElement = 0;
            if (focusedColElement >= rowsCount[0]) focusedColElement = rowsCount[0] - 1;
            while (!elementRows[0][focusedColElement]->isSelectable() && focusedColElement > 0) {
                focusedColElement--;
            }
        }
    }

    return this; 
}

void Screen::render(dataLCD& outData) {
    memset(outData.rows[0], ' ', 16);
    outData.rows[0][16] = '\0';
    memset(outData.rows[1], ' ', 16);
    outData.rows[1][16] = '\0';
    
    outData.highlightedChars[0] = 0;
    outData.highlightedChars[1] = 0;

    for (uint8_t r = 0; r < 2; r++) {
        uint8_t cursorX = 0; 
        for (uint8_t c = 0; c < rowsCount[r]; c++) {
            if (elementRows[r][c] != nullptr) {
                const char* text = elementRows[r][c]->getText();
                uint8_t len = strlen(text);
                
                if (c > 0 && c == rowsCount[r] - 1) {
                    uint8_t rightAlignPos = 16 - len;
                    if (rightAlignPos > cursorX) {
                        cursorX = rightAlignPos;
                    }
                }

                if (cursorX + len > 16) len = 16 - cursorX; 
                
                strncpy(&outData.rows[r][cursorX], text, len);
                
                if (r == focusedRowElement && c == focusedColElement) {
                    uint16_t elementMask = elementRows[r][c]->highlightTextIndexes();
                    outData.highlightedChars[r] |= (elementMask << cursorX);
                }
                
                cursorX += len + 1; 
            }
        }
    }
}