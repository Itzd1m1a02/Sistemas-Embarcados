#pragma once
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "../Hardware/IAlarmController.h"

class MainScreen {
private:
    IAlarmController* controller;
    LiquidCrystal* lcd;

    // Buffers para controle de redesenho e eliminação de flicker no LCD
    char bufferL0[17];
    char bufferL1[17];

    void writeLine(uint8_t row, const char* str);

public:
    MainScreen(IAlarmController* controller, LiquidCrystal* lcdDisplay);
    ~MainScreen();

    void begin();
    void clear();

    // Funções de desenho de cada estado da Máquina de Estados (FSM)
    void renderRelogio(const TimeData& horaAtual, uint8_t qtdAlarmes);
    void renderMenuPrincipal(uint8_t opcaoSelecionada);
    void renderEditarAlarme(uint8_t indexAlarme, uint8_t hora, uint8_t minuto, uint8_t campoFocado, bool isNovo);
    void renderListarAlarmes(uint8_t indexAtual, uint8_t total, const TimeData& alarme, uint8_t acaoSelecionada);
    void renderAjustarDataHora(uint8_t hour, uint8_t minute, uint8_t day, uint8_t month, uint16_t year, uint8_t campoFocado);
    void renderConfirmarReset(uint8_t opcaoSelecionada);
    void renderAlarmeDisparando(const TimeData& horaAtual);

    // Mensagens informativas na tela
    void mostrarMensagem(const char* linha0, const char* linha1, uint16_t delayMs = 1200);
};
