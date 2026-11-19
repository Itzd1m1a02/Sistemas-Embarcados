#pragma once
#include <Arduino.h>

class Buzzer {
  private:
    uint8_t pinBuzzer;
    bool isActive;
    uint8_t melodyIndex;
    unsigned long noteStartedAt;

    void playCurrentNote();

  public:
    Buzzer(uint8_t buzzerPin);
    void begin(); // Inicializa o pino do buzzer
    void activate(); // Ativa o buzzer
    void update(); // Avança a melodia sem bloquear o loop principal
    void deactivate(); // Desativa o buzzer
    bool IsActive(); // Retorna o estado do buzzer
};