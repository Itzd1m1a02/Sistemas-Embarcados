#pragma once
#include <Wire.h>
#include <TimeLib.h>          // Biblioteca para manipulação de tempo
#include "IAlarmController.h" // Interface e struct TimeData
#include <Arduino.h>          // Tipos Arduino (uint8_t, etc.)
#include <EEPROM.h>
#include "Buzzer.h"

// Capacidade máxima de alarmes suportada
#define MAX_ALARMS 5

class AlarmHardware : public IAlarmController {
private:
    bool ringAlarm;             // Flag indicando se o alarme está tocando
    TimeData alarms[MAX_ALARMS];
    uint8_t totalAlarms;

    // Travas para evitar que o alarme redispare continuamente no mesmo minuto
    int8_t lastTriggeredMin;
    int8_t lastTriggeredHr;

public:
    AlarmHardware();

    // ==========================================
    // Métodos do Contrato IAlarmController
    // ==========================================
    TimeData getCurrentTime() override;
    void setCurrentDateTime(uint8_t hr, uint8_t min, uint8_t sec, uint8_t day, uint8_t month, uint16_t year) override;
    uint8_t getCurrentDay() override;
    uint8_t getCurrentMonth() override;
    uint16_t getCurrentYear() override;

    TimeData getAlarmData(uint8_t alarmIndex) override;
    uint8_t getTotalAlarms() override;

    void saveAlarm(uint8_t alarmIndex, TimeData data) override;
    void deleteAlarm(uint8_t alarmIndex) override;
    void deleteAllAlarms() override;

    bool isAlarmRinging() override;
    void stopAlarm() override;

    // =====================================
    // Métodos de utilidade / sincronização
    // =====================================
    void updateClock();
    void syncCompilerTime();
};
