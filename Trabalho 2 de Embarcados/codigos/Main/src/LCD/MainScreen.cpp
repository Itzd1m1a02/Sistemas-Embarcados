#include "MainScreen.h"
#include <stdio.h>
#include <string.h>

MainScreen::MainScreen(IAlarmController* ctrl, LiquidCrystal* lcdDisplay)
    : controller(ctrl), lcd(lcdDisplay) {
    memset(bufferL0, 0, sizeof(bufferL0));
    memset(bufferL1, 0, sizeof(bufferL1));
}

MainScreen::~MainScreen() {
}

void MainScreen::begin() {
    clear();
}

void MainScreen::clear() {
    lcd->clear();
    memset(bufferL0, ' ', 16);
    bufferL0[16] = '\0';
    memset(bufferL1, ' ', 16);
    bufferL1[16] = '\0';
}

void MainScreen::writeLine(uint8_t row, const char* str) {
    char formatted[17];
    uint8_t len = strlen(str);
    for (uint8_t i = 0; i < 16; i++) {
        if (i < len) {
            formatted[i] = str[i];
        } else {
            formatted[i] = ' ';
        }
    }
    formatted[16] = '\0';

    char* prev = (row == 0) ? bufferL0 : bufferL1;

    // Se o conteúdo da linha não mudou, NÃO reescreve no LCD (elimina flickering)
    if (strcmp(prev, formatted) == 0) {
        return;
    }

    lcd->setCursor(0, row);
    lcd->print(formatted);
    strcpy(prev, formatted);
}

void MainScreen::renderRelogio(const TimeData& horaAtual, uint8_t qtdAlarmes) {
    const char* dias[] = {"SEG", "TER", "QUA", "QUI", "SEX", "SAB", "DOM"};
    uint8_t diaIdx = 0;
    for (uint8_t i = 0; i < 7; i++) {
        if (horaAtual.weekDays & (1 << i)) {
            diaIdx = i;
            break;
        }
    }

    char l0[17];
    char l1[17];

    snprintf(l0, sizeof(l0), " %02d:%02d:%02d  %s", horaAtual.hour, horaAtual.minute, horaAtual.second, dias[diaIdx]);
    snprintf(l1, sizeof(l1), "Alm:%d   >[MENU]", qtdAlarmes);

    writeLine(0, l0);
    writeLine(1, l1);
}

void MainScreen::renderMenuPrincipal(uint8_t opcaoSelecionada) {
    char l0[17];
    char l1[17];

    if (opcaoSelecionada <= 1) {
        snprintf(l0, sizeof(l0), "%s1.Novo Alarme",  (opcaoSelecionada == 0) ? ">" : " ");
        snprintf(l1, sizeof(l1), "%s2.Listar Alarms", (opcaoSelecionada == 1) ? ">" : " ");
    } else if (opcaoSelecionada <= 3) {
        snprintf(l0, sizeof(l0), "%s3.Ajustar Data",  (opcaoSelecionada == 2) ? ">" : " ");
        snprintf(l1, sizeof(l1), "%s4.Apagar Todos",  (opcaoSelecionada == 3) ? ">" : " ");
    } else {
        snprintf(l0, sizeof(l0), "%s4.Apagar Todos",  " ");
        snprintf(l1, sizeof(l1), "%s5.Voltar Relog", (opcaoSelecionada == 4) ? ">" : " ");
    }

    writeLine(0, l0);
    writeLine(1, l1);
}

void MainScreen::renderEditarAlarme(uint8_t indexAlarme, uint8_t hora, uint8_t minuto, uint8_t campoFocado, bool isNovo) {
    char l0[17];
    char l1[17];

    if (isNovo) {
        snprintf(l0, sizeof(l0), "Novo Alarme #%d", indexAlarme + 1);
    } else {
        snprintf(l0, sizeof(l0), "Editar Alm  #%d", indexAlarme + 1);
    }

    if (campoFocado == 0) {
        // Foco na hora
        snprintf(l1, sizeof(l1), ">[%02d]:%02d [OK][X]", hora, minuto);
    } else if (campoFocado == 1) {
        // Foco no minuto
        snprintf(l1, sizeof(l1), " %02d:>[%02d][OK][X]", hora, minuto);
    } else if (campoFocado == 2) {
        // Foco em Salvar [OK]
        snprintf(l1, sizeof(l1), " %02d:%02d >[OK] [X]", hora, minuto);
    } else {
        // Foco em Cancelar [X]
        snprintf(l1, sizeof(l1), " %02d:%02d  [OK]>[X]", hora, minuto);
    }

    writeLine(0, l0);
    writeLine(1, l1);
}

void MainScreen::renderListarAlarmes(uint8_t indexAtual, uint8_t total, const TimeData& alarme, uint8_t acaoSelecionada) {
    char l0[17];
    char l1[17];

    if (total == 0) {
        snprintf(l0, sizeof(l0), "Nenhum alarme!");
        snprintf(l1, sizeof(l1), ">[VOLTAR]");
    } else {
        snprintf(l0, sizeof(l0), "Alm %d/%d:  %02d:%02d", indexAtual + 1, total, alarme.hour, alarme.minute);

        if (acaoSelecionada == 0) {
            snprintf(l1, sizeof(l1), ">EDIT   DEL  SAIR");
        } else if (acaoSelecionada == 1) {
            snprintf(l1, sizeof(l1), " EDIT  >DEL  SAIR");
        } else {
            snprintf(l1, sizeof(l1), " EDIT   DEL >SAIR");
        }
    }

    writeLine(0, l0);
    writeLine(1, l1);
}

void MainScreen::renderAjustarDataHora(uint8_t hour, uint8_t minute, uint8_t day, uint8_t month, uint16_t year, uint8_t campoFocado) {
    char l0[17];
    char l1[17];

    // Linha 0: Pré-visualização completa em tempo real
    snprintf(l0, sizeof(l0), "%02d:%02d  %02d/%02d/%02d", hour, minute, day, month, year % 100);

    // Linha 1: Campo focado para edição ou opções de salvar/cancelar
    if (campoFocado == 0) {
        snprintf(l1, sizeof(l1), ">Hora:   [ %02d ] ", hour);
    } else if (campoFocado == 1) {
        snprintf(l1, sizeof(l1), ">Minuto: [ %02d ] ", minute);
    } else if (campoFocado == 2) {
        snprintf(l1, sizeof(l1), ">Dia:    [ %02d ] ", day);
    } else if (campoFocado == 3) {
        snprintf(l1, sizeof(l1), ">Mes:    [ %02d ] ", month);
    } else if (campoFocado == 4) {
        snprintf(l1, sizeof(l1), ">Ano:    [%4d] ", year);
    } else if (campoFocado == 5) {
        snprintf(l1, sizeof(l1), " >[SALVAR]  CANC");
    } else {
        snprintf(l1, sizeof(l1), "  SALVAR  >[CANC]");
    }

    writeLine(0, l0);
    writeLine(1, l1);
}

void MainScreen::renderConfirmarReset(uint8_t opcaoSelecionada) {
    char l0[17];
    char l1[17];

    snprintf(l0, sizeof(l0), "Apagar todos?");
    if (opcaoSelecionada == 0) {
        snprintf(l1, sizeof(l1), " >[NAO]    [SIM]");
    } else {
        snprintf(l1, sizeof(l1), "  [NAO]   >[SIM]");
    }

    writeLine(0, l0);
    writeLine(1, l1);
}

void MainScreen::renderAlarmeDisparando(const TimeData& horaAtual) {
    char l0[17];
    char l1[17];

    snprintf(l0, sizeof(l0), "!ALARME:  %02d:%02d!", horaAtual.hour, horaAtual.minute);
    snprintf(l1, sizeof(l1), " >[PARAR ALARME]");

    writeLine(0, l0);
    writeLine(1, l1);
}

void MainScreen::mostrarMensagem(const char* linha0, const char* linha1, uint16_t delayMs) {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(linha0);
    lcd->setCursor(0, 1);
    lcd->print(linha1);

    // Reseta buffers para forçar renderização completa na próxima chamada
    memset(bufferL0, 0, sizeof(bufferL0));
    memset(bufferL1, 0, sizeof(bufferL1));

    delay(delayMs);
}
