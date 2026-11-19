#include <Arduino.h>
#include <LiquidCrystal.h>
#include <TimeLib.h>
#include "src/Hardware/AlarmHardware.h"
#include "src/Hardware/JoystickReader.h"
#include "src/Hardware/Buzzer.h"
#include "src/LCD/MainScreen.h"

// ==========================================
// 1. INSTANCIAÇÃO DE HARDWARE E INTERFACE
// ==========================================

// Pinos do LCD: RS, E, D4, D5, D6, D7 (compatível com Arduino Mega)
LiquidCrystal lcd(12, 11, 24, 26, 28, 30); 

AlarmHardware controlador;
JoystickReader joystick(A0, A1, 50); // Eixo X, Eixo Y e Pino Digital do Botão
Buzzer buzzer(2);                    // Pino PWM do Buzzer

// Interface gráfica do display
MainScreen menuInterface(&controlador, &lcd);

// ==========================================
// 2. MÁQUINA DE ESTADOS 
// ==========================================
enum EstadoSistema {
    ESTADO_RELOGIO,          // Tela principal do relógio e monitoramento
    ESTADO_MENU_PRINCIPAL,   // Navegação no menu principal
    ESTADO_CRIAR_ALARME,     // Criação de um novo alarme na memória
    ESTADO_LISTAR_ALARMES,   // Visualização e escolha dos alarmes salvos
    ESTADO_EDITAR_ALARME,    // Edição de hora/minuto do alarme selecionado
    ESTADO_AJUSTAR_DATA_HORA,// Ajuste da data e hora atual do sistema
    ESTADO_CONFIRMAR_RESET,  // Confirmação para apagar todos os alarmes
    ESTADO_ALARME_DISPARADO  // Alarme tocando (buzzer ativo)
};

EstadoSistema estadoAtual = ESTADO_RELOGIO;

// Variáveis de controle de navegação dos menus
uint8_t menuOpcao = 0;       // 0: Novo, 1: Listar, 2: Ajustar Data/H, 3: Apagar, 4: Voltar
uint8_t listIndex = 0;       // Índice do alarme na lista
uint8_t listAcao = 0;        // 0: EDIT, 1: DEL, 2: SAIR
uint8_t resetOpcao = 0;      // 0: NAO, 1: SIM

// Variáveis de criação e edição de alarme
uint8_t editIndex = 0;       // Índice do alarme na memória (0..4)
uint8_t editHour = 0;        // Hora configurada (0..23)
uint8_t editMinute = 0;      // Minuto configurado (0..59)
uint8_t editCampo = 0;       // 0: Hora, 1: Minuto, 2: [OK], 3: [X]
bool isNovoAlarme = false;   // Flag indicando se é novo alarme ou edição

// Variáveis de ajuste da data e hora atual
uint8_t adjHour = 0;
uint8_t adjMinute = 0;
uint8_t adjDay = 1;
uint8_t adjMonth = 1;
uint16_t adjYear = 2026;
uint8_t adjCampo = 0;        // 0: Hora, 1: Minuto, 2: Dia, 3: Mês, 4: Ano, 5: [SALVAR], 6: [CANC]

// Protótipos das funções de cada estado da FSM
void processarRelogio(JoyEvent evento);
void processarMenuPrincipal(JoyEvent evento);
void processarEditarAlarme(JoyEvent evento);
void processarListarAlarmes(JoyEvent evento);
void processarAjustarDataHora(JoyEvent evento);
void processarConfirmarReset(JoyEvent evento);
void processarAlarmeDisparado(JoyEvent evento);

// Função auxiliar para calcular dias máximos de cada mês (incluindo anos bissextos)
uint8_t calcularDiasNoMes(uint8_t mes, uint16_t ano) {
    if (mes == 2) {
        if ((ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0)) {
            return 29;
        }
        return 28;
    }
    if (mes == 4 || mes == 6 || mes == 9 || mes == 11) {
        return 30;
    }
    return 31;
}

// ==========================================
// 3. SETUP
// ==========================================
void setup() {
    Serial.begin(9600);

    // Configura a hora inicial utilizando o momento exato em que o código foi compilado
    controlador.syncCompilerTime();

    // Inicializa o Hardware
    lcd.begin(16, 2);
    joystick.begin();
    buzzer.begin();
    
    // Inicializa a UI
    menuInterface.begin();
    
    Serial.println("=== Relogio Despertador LCD Iniciado ===");
    Serial.println("Relogio sincronizado com a hora de compilacao.");
}

// ==========================================
// 4. LOOP PRINCIPAL (EXECUÇÃO DA FSM)
// ==========================================
void loop() {
    // A. Atualiza o relógio interno e verifica alarmes
    controlador.updateClock();

    // B. Leitura de entrada do Joystick (UP, DOWN, LEFT, RIGHT, PRESS)
    JoyEvent evento = joystick.readJoystick();

    // C. Prioridade: se o alarme disparou, vai para o estado de alarme sonoro
    if (controlador.isAlarmRinging() && estadoAtual != ESTADO_ALARME_DISPARADO) {
        buzzer.activate();
        estadoAtual = ESTADO_ALARME_DISPARADO;
    }

    // D. Máquina de Estados Finita (FSM)
    switch (estadoAtual) {
        case ESTADO_RELOGIO:
            processarRelogio(evento);
            break;

        case ESTADO_MENU_PRINCIPAL:
            processarMenuPrincipal(evento);
            break;

        case ESTADO_CRIAR_ALARME:
        case ESTADO_EDITAR_ALARME:
            processarEditarAlarme(evento);
            break;

        case ESTADO_LISTAR_ALARMES:
            processarListarAlarmes(evento);
            break;

        case ESTADO_AJUSTAR_DATA_HORA:
            processarAjustarDataHora(evento);
            break;

        case ESTADO_CONFIRMAR_RESET:
            processarConfirmarReset(evento);
            break;

        case ESTADO_ALARME_DISPARADO:
            processarAlarmeDisparado(evento);
            break;
    }
}

// ============================================================
// 5. IMPLEMENTAÇÃO DOS ESTADOS DA MÁQUINA DE ESTADOS
// ============================================================

// --- ESTADO: RELÓGIO PRINCIPAL ---
void processarRelogio(JoyEvent evento) {
    TimeData agora = controlador.getCurrentTime();
    menuInterface.renderRelogio(agora, controlador.getTotalAlarms());

    // Pressionar o botão ou mover para a direita/cima abre o menu
    if (evento == JoyEvent::PRESS || evento == JoyEvent::RIGHT || evento == JoyEvent::UP) {
        menuOpcao = 0;
        estadoAtual = ESTADO_MENU_PRINCIPAL;
    }
}

// --- ESTADO: MENU PRINCIPAL ---
void processarMenuPrincipal(JoyEvent evento) {
    menuInterface.renderMenuPrincipal(menuOpcao);

    if (evento == JoyEvent::UP) {
        menuOpcao = (menuOpcao == 0) ? 4 : menuOpcao - 1;
    } else if (evento == JoyEvent::DOWN) {
        menuOpcao = (menuOpcao + 1) % 5;
    } else if (evento == JoyEvent::LEFT) {
        // Voltar direto para o relógio
        estadoAtual = ESTADO_RELOGIO;
    } else if (evento == JoyEvent::PRESS) {
        switch (menuOpcao) {
            case 0: // 1. Novo Alarme
                if (controlador.getTotalAlarms() >= MAX_ALARMS) {
                    menuInterface.mostrarMensagem("Memoria Cheia!", "Max: 5 alarmes");
                } else {
                    editIndex = controlador.getTotalAlarms();
                    TimeData agora = controlador.getCurrentTime();
                    editHour = agora.hour;
                    editMinute = agora.minute;
                    editCampo = 0; // Foco na Hora
                    isNovoAlarme = true;
                    estadoAtual = ESTADO_CRIAR_ALARME;
                }
                break;

            case 1: // 2. Listar Alarmes
                if (controlador.getTotalAlarms() == 0) {
                    menuInterface.mostrarMensagem("Nenhum alarme", "cadastrado!");
                } else {
                    listIndex = 0;
                    listAcao = 0; // 0: EDIT, 1: DEL, 2: SAIR
                    estadoAtual = ESTADO_LISTAR_ALARMES;
                }
                break;

            case 2: // 3. Ajustar Data e Hora Atual
                {
                    TimeData agora = controlador.getCurrentTime();
                    adjHour = agora.hour;
                    adjMinute = agora.minute;
                    adjDay = controlador.getCurrentDay();
                    adjMonth = controlador.getCurrentMonth();
                    adjYear = controlador.getCurrentYear();
                    if (adjYear < 2000) adjYear = 2026;
                    adjCampo = 0; // Começa na Hora
                    estadoAtual = ESTADO_AJUSTAR_DATA_HORA;
                }
                break;

            case 3: // 4. Apagar Todos
                if (controlador.getTotalAlarms() == 0) {
                    menuInterface.mostrarMensagem("Sem alarmes", "para apagar!");
                } else {
                    resetOpcao = 0; // 0: NAO, 1: SIM
                    estadoAtual = ESTADO_CONFIRMAR_RESET;
                }
                break;

            case 4: // 5. Voltar ao Relógio
                estadoAtual = ESTADO_RELOGIO;
                break;
        }
    }
}

// --- ESTADO: CRIAR OU EDITAR ALARME ---
void processarEditarAlarme(JoyEvent evento) {
    menuInterface.renderEditarAlarme(editIndex, editHour, editMinute, editCampo, isNovoAlarme);

    if (evento == JoyEvent::LEFT) {
        if (editCampo > 0) editCampo--;
    } else if (evento == JoyEvent::RIGHT) {
        if (editCampo < 3) editCampo++;
    } else if (evento == JoyEvent::UP) {
        if (editCampo == 0) {
            editHour = (editHour + 1) % 24;
        } else if (editCampo == 1) {
            editMinute = (editMinute + 1) % 60;
        }
    } else if (evento == JoyEvent::DOWN) {
        if (editCampo == 0) {
            editHour = (editHour == 0) ? 23 : editHour - 1;
        } else if (editCampo == 1) {
            editMinute = (editMinute == 0) ? 59 : editMinute - 1;
        }
    } else if (evento == JoyEvent::PRESS) {
        if (editCampo == 0) {
            // Avança para configurar o minuto
            editCampo = 1;
        } else if (editCampo == 1) {
            // Avança para o botão [OK]
            editCampo = 2;
        } else if (editCampo == 2) {
            // [OK] - Grava o alarme na memória EEPROM
            TimeData dadosAlarme;
            dadosAlarme.hour = editHour;
            dadosAlarme.minute = editMinute;
            dadosAlarme.second = 0;
            dadosAlarme.weekDays = 127; // Todos os dias da semana ativos

            controlador.saveAlarm(editIndex, dadosAlarme);
            menuInterface.mostrarMensagem("Alarme Salvo!", "Com sucesso!");

            if (isNovoAlarme) {
                estadoAtual = ESTADO_MENU_PRINCIPAL;
            } else {
                estadoAtual = ESTADO_LISTAR_ALARMES;
            }
        } else if (editCampo == 3) {
            // [X] - Cancela a edição sem salvar
            if (isNovoAlarme) {
                estadoAtual = ESTADO_MENU_PRINCIPAL;
            } else {
                estadoAtual = ESTADO_LISTAR_ALARMES;
            }
        }
    }
}

// --- ESTADO: LISTAR ALARMES CADASTRADOS ---
void processarListarAlarmes(JoyEvent evento) {
    uint8_t total = controlador.getTotalAlarms();

    if (total == 0) {
        menuInterface.mostrarMensagem("Sem alarmes!", "");
        estadoAtual = ESTADO_MENU_PRINCIPAL;
        return;
    }

    if (listIndex >= total) {
        listIndex = total - 1;
    }

    TimeData alarmeAtual = controlador.getAlarmData(listIndex);
    menuInterface.renderListarAlarmes(listIndex, total, alarmeAtual, listAcao);

    if (evento == JoyEvent::UP) {
        if (listIndex > 0) listIndex--;
        else listIndex = total - 1;
    } else if (evento == JoyEvent::DOWN) {
        listIndex = (listIndex + 1) % total;
    } else if (evento == JoyEvent::LEFT) {
        if (listAcao > 0) listAcao--;
    } else if (evento == JoyEvent::RIGHT) {
        if (listAcao < 2) listAcao++;
    } else if (evento == JoyEvent::PRESS) {
        if (listAcao == 0) {
            // EDIT: Entra em modo de edição para este alarme
            editIndex = listIndex;
            editHour = alarmeAtual.hour;
            editMinute = alarmeAtual.minute;
            editCampo = 0;
            isNovoAlarme = false;
            estadoAtual = ESTADO_EDITAR_ALARME;
        } else if (listAcao == 1) {
            // DEL: Apaga este alarme da memória EEPROM
            controlador.deleteAlarm(listIndex);
            menuInterface.mostrarMensagem("Alarme Apagado", "com sucesso!");

            if (controlador.getTotalAlarms() == 0) {
                estadoAtual = ESTADO_MENU_PRINCIPAL;
            } else if (listIndex >= controlador.getTotalAlarms()) {
                listIndex = controlador.getTotalAlarms() - 1;
            }
        } else if (listAcao == 2) {
            // SAIR: Volta ao menu principal
            estadoAtual = ESTADO_MENU_PRINCIPAL;
        }
    }
}

// --- ESTADO: AJUSTAR DATA E HORA ATUAL ---
void processarAjustarDataHora(JoyEvent evento) {
    menuInterface.renderAjustarDataHora(adjHour, adjMinute, adjDay, adjMonth, adjYear, adjCampo);

    if (evento == JoyEvent::LEFT) {
        if (adjCampo > 0) adjCampo--;
    } else if (evento == JoyEvent::RIGHT) {
        if (adjCampo < 6) adjCampo++;
    } else if (evento == JoyEvent::UP) {
        if (adjCampo == 0) {
            adjHour = (adjHour + 1) % 24;
        } else if (adjCampo == 1) {
            adjMinute = (adjMinute + 1) % 60;
        } else if (adjCampo == 2) {
            uint8_t maxD = calcularDiasNoMes(adjMonth, adjYear);
            adjDay = (adjDay >= maxD) ? 1 : adjDay + 1;
        } else if (adjCampo == 3) {
            adjMonth = (adjMonth >= 12) ? 1 : adjMonth + 1;
            uint8_t maxD = calcularDiasNoMes(adjMonth, adjYear);
            if (adjDay > maxD) adjDay = maxD;
        } else if (adjCampo == 4) {
            adjYear = (adjYear >= 2040) ? 2024 : adjYear + 1;
            uint8_t maxD = calcularDiasNoMes(adjMonth, adjYear);
            if (adjDay > maxD) adjDay = maxD;
        }
    } else if (evento == JoyEvent::DOWN) {
        if (adjCampo == 0) {
            adjHour = (adjHour == 0) ? 23 : adjHour - 1;
        } else if (adjCampo == 1) {
            adjMinute = (adjMinute == 0) ? 59 : adjMinute - 1;
        } else if (adjCampo == 2) {
            uint8_t maxD = calcularDiasNoMes(adjMonth, adjYear);
            adjDay = (adjDay <= 1) ? maxD : adjDay - 1;
        } else if (adjCampo == 3) {
            adjMonth = (adjMonth <= 1) ? 12 : adjMonth - 1;
            uint8_t maxD = calcularDiasNoMes(adjMonth, adjYear);
            if (adjDay > maxD) adjDay = maxD;
        } else if (adjCampo == 4) {
            adjYear = (adjYear <= 2024) ? 2040 : adjYear - 1;
            uint8_t maxD = calcularDiasNoMes(adjMonth, adjYear);
            if (adjDay > maxD) adjDay = maxD;
        }
    } else if (evento == JoyEvent::PRESS) {
        if (adjCampo < 5) {
            // Pressionar avança para o próximo campo (Hora -> Minuto -> Dia -> Mês -> Ano -> Salvar)
            adjCampo++;
        } else if (adjCampo == 5) {
            // [SALVAR]: Aplica a nova data e hora no sistema
            controlador.setCurrentDateTime(adjHour, adjMinute, 0, adjDay, adjMonth, adjYear);
            menuInterface.mostrarMensagem("Data e Hora", "Atualizadas!");
            estadoAtual = ESTADO_MENU_PRINCIPAL;
        } else if (adjCampo == 6) {
            // [CANC]: Sai sem modificar
            estadoAtual = ESTADO_MENU_PRINCIPAL;
        }
    }
}

// --- ESTADO: CONFIRMAÇÃO DE RESET ---
void processarConfirmarReset(JoyEvent evento) {
    menuInterface.renderConfirmarReset(resetOpcao);

    if (evento == JoyEvent::LEFT) {
        resetOpcao = 0; // NAO
    } else if (evento == JoyEvent::RIGHT) {
        resetOpcao = 1; // SIM
    } else if (evento == JoyEvent::PRESS) {
        if (resetOpcao == 1) {
            controlador.deleteAllAlarms();
            menuInterface.mostrarMensagem("Todos apagados!", "Memoria limpa");
        }
        estadoAtual = ESTADO_MENU_PRINCIPAL;
    }
}

// --- ESTADO: ALARME DISPARADO (TOCANDO) ---
void processarAlarmeDisparado(JoyEvent evento) {
    TimeData agora = controlador.getCurrentTime();
    menuInterface.renderAlarmeDisparando(agora);

    buzzer.activate();
    buzzer.update();

    // Qualquer interação com o joystick interrompe o alarme sonoro
    if (evento == JoyEvent::PRESS || evento != JoyEvent::NONE) {
        controlador.stopAlarm();
        buzzer.deactivate();
        menuInterface.mostrarMensagem("Alarme Parado", "");
        estadoAtual = ESTADO_RELOGIO;
    }
}
