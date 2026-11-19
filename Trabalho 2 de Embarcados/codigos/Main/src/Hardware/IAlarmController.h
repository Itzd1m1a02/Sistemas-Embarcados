#pragma once
#include <stdint.h>

struct TimeData {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t weekDays; 
};

class IAlarmController {
public:
    virtual ~IAlarmController() {}
    
    // Ações disparadas pelas telas / máquina de estados
    virtual void saveAlarm(uint8_t alarmIndex, TimeData data) = 0;
    virtual void deleteAlarm(uint8_t alarmIndex) = 0;
    virtual void deleteAllAlarms() = 0;
    
    // Leitura e escrita de data e hora do sistema
    virtual TimeData getCurrentTime() = 0;
    virtual void setCurrentDateTime(uint8_t hr, uint8_t min, uint8_t sec, uint8_t day, uint8_t month, uint16_t year) = 0;
    virtual uint8_t getCurrentDay() = 0;
    virtual uint8_t getCurrentMonth() = 0;
    virtual uint16_t getCurrentYear() = 0;

    // Leitura de alarmes
    virtual TimeData getAlarmData(uint8_t alarmIndex) = 0;
    virtual uint8_t getTotalAlarms() = 0;

    // Controle do alarme
    virtual bool isAlarmRinging() = 0;
    virtual void stopAlarm() = 0;
};
