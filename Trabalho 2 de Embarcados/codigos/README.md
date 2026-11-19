# ⏰ Guia de Integração - Projeto Alarme Embarcado (Lab 2)

## 1. Visão Geral do Projeto

**Hardware Utilizado:**
*   **Microcontrolador:** Arduino (ATmega).
*   **Display:** LCD 16x2 (controlado via biblioteca `LiquidCrystal`).
*   **Entrada de Dados:** Joystick analógico (2 eixos) com botão digital embutido.
*   **Saída de Áudio:** Buzzer (saída analógica/PWM).

**Obrigatório pelo Roteiro:** 
*   Todo o controle lógico e leitura de botões **deve** usar **Máquinas de Estados (State Machines)**, incluindo *software debouncing* para o Joystick.

---

## 2. A Arquitetura do Software (O Segredo do Projeto)
Para evitar que o código vire uma bagunça e permita que pessoas diferentes programem ao mesmo tempo, dividimos o projeto em duas grandes camadas que se comunicam através de um "Contrato" (Interface).

1.  **Camada de Interface Visual (UI):** Cuida exclusivamente do que aparece no LCD, do cursor e da navegação entre menus. Não faz ideia de como o hardware funciona.
2.  **Camada de Hardware (Controlador):** Cuida dos Timers, interrupções, leitura do Joystick, toca o Buzzer e salva dados na memória. Não faz ideia de como as telas são desenhadas.

### O "Contrato" (`IAlarmController.h`)
Para que a Tela consiga salvar um alarme sem saber como a memória funciona, nós usamos a classe abstrata `IAlarmController`. Ela define as funções obrigatórias que a camada de hardware vai implementar:
*   `saveAlarm(index, data)`
*   `deleteAlarm(index)`
*   `getCurrentTime()`
*   ...entre outras.

---

## 3. Divisão de Tarefas: 

### 👨‍💻 Camada de Telas (UI)
**Função:** Desenhar as telas (versão 5) baseadas nos rascunhos em `.txt`, programar a navegação do cursor e as opções.

**Como trabalhar:**
1.  Você vai lidar apenas com as classes de interface: `MainScreen`, `Screen` e os herdeiros de `ScreenElement` (`StaticData`, `EditableData`, `MenuOption`, `NavOption`).
2.  **Montando as Telas:** No construtor da `MainScreen`, você instanciará os elementos (ex: `new MenuOption("[SET]")`) e os agrupará em objetos `Screen`.
3.  **Lógica Visual:** A sua lógica só deve se preocupar em calcular onde o cursor (pixel invertido) está, se a palavra cabe na tela (16 caracteres), para qual tela o usuário vai se apertar "PRESS" e interromper tudo para colocar uma tela de despertar na hora que despertar, e voltar ao normal quando acabar ou se o usuário desligar o despertador.
4.  **Testando sem hardware:** Como você só depende do `IAlarmController`, você pode criar um "Controlador Falso" (Mock) que apenas imprime no console do computador, permitindo testar a navegação de telas inteira no PC antes de jogar pro Arduino!

### ⚙️ Camada de Hardware e Lógica (Controlador)
**Função:** Fazer o relógio bater, os alarmes tocarem, gerenciar a memória do microcontrolador e ler o mundo físico (Joystick) usando **Máquinas de Estado**.

**Como trabalhar:**
1.  **O Controlador Principal:** Você deve criar uma classe (ex: `AlarmHardware`) que herde e implemente OBRIGATORIAMENTE os métodos de `IAlarmController`.
2.  **Máquinas de Estado (Obrigatório):** Você deve desenhar máquinas de estado para:
    *   **Debouncing do Joystick:** Ler a entrada analógica/digital, esperar um tempo para evitar ruídos, e traduzir para a tela no formato limpo (`JoyEvent::UP`, `DOWN`, `PRESS`).
    *   **Relógio/Timer:** Contar o tempo usando os timers internos do ATmega.
    *   **Alarme/Buzzer:** Verificar se `Hora Atual == Hora do Alarme` e disparar a máquina de estado que toca o Buzzer (com ritmos/frequências específicas) e levantar uma flag para a camada de telas exibir a tela de alarme.
3.  **Memória EEPROM:** Lidar com a gravação e leitura estruturada para que o alarme sobreviva se o Arduino for desligado.

---

## 4. Roteiro de Complexidade (V1 ao V5)
Nós projetamos 5 níveis de rascunhos de telas. O nosso alvo de entrega é iterativo:
*   **Fase 1 (O Básico):** Começar implementando a Tela V1 (1 alarme apenas, hora atual, [SET], [LIST], [RESET]). Garantir que o hardware e a tela funcionam juntos perfeitamente.
*   **Fase 2 (O Alvo - V5):** Com a V1 rodando, expandir a UI e a memória para suportar a Tela V5 (Múltiplos alarmes, dias da semana específicos, função para deletar individual ou geral).

---

## 5. ⚠️ Regras de Ouro do Projeto
*   **Não use `String` ou `std::string`:** O ATmega tem apenas 2KB de RAM. Use arrays de caracteres clássicos (`char buffer[16]`) para evitar fragmentação e travamentos.
*   **Não use `new` (alocação dinâmica) no Loop:** Instancie as telas e elementos **apenas uma vez** no momento de inicialização (Construtor da `MainScreen`) usando arrays fixos de ponteiros. Alocar e desalocar memória com a placa rodando vai crashar o sistema rapidamente.
*   **Use `#pragma once`:** Todo arquivo `.h` deve obrigatoriamente começar com essa diretiva.
*   **Comunicação Limpa:** A UI nunca deve acessar uma porta do Arduino diretamente (como `digitalRead`). O Hardware lê o pino, transforma num evento (ex: `JoyEvent::UP`) e avisa a UI.
