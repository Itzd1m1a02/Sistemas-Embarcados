#include "AlarmHardware.h"

// Construtor: inicializa variáveis, sincroniza horário da compilação e carrega EEPROM
AlarmHardware::AlarmHardware() {
    totalAlarms = 0;
    ringAlarm = false;
    lastTriggeredMin = -1;
    lastTriggeredHr = -1;

    // Inicializa os slots de alarme com valores seguros (todos os dias ativos = 127)
    for (uint8_t i = 0; i < MAX_ALARMS; ++i) {
        alarms[i] = {0, 0, 0, 127};
    }

    // Leitura da EEPROM (posição 0 = total de alarmes, posições 1.. = array de alarmes)
    EEPROM.get(0, totalAlarms);

    // Se o valor lido for inválido (EEPROM virgem geralmente lê 0xFF = 255)
    if (totalAlarms > MAX_ALARMS) {
        totalAlarms = 0;
        EEPROM.put(0, totalAlarms);
        EEPROM.put(1, alarms);
    } else {
        EEPROM.get(1, alarms);
        // Sanitização de dados lidos da EEPROM para evitar valores corrompidos
        for (uint8_t i = 0; i < MAX_ALARMS; ++i) {
            if (alarms[i].hour >= 24) alarms[i].hour = 0;
            if (alarms[i].minute >= 60) alarms[i].minute = 0;
            if (alarms[i].weekDays == 0 || alarms[i].weekDays == 255) {
                alarms[i].weekDays = 127; // Todos os dias da semana
            }
        }
    }

    // Configura o relógio interno com o momento da compilação
    syncCompilerTime();
}

// =====================================
// Métodos locais para o loop do Arduino
// =====================================

// Atualiza o relógio interno e verifica se algum alarme deve ser disparado
void AlarmHardware::updateClock() {
    TimeData currentTime = getCurrentTime();

    // Se o minuto mudou, liberamos a trava para futuros alarmes
    if (currentTime.minute != lastTriggeredMin || currentTime.hour != lastTriggeredHr) {
        lastTriggeredMin = -1;
        lastTriggeredHr = -1;
    }

    // Se o alarme já está soando, não precisa reavaliar
    if (ringAlarm) return;

    for (uint8_t i = 0; i < totalAlarms; ++i) {
        if (alarms[i].hour == currentTime.hour &&
            alarms[i].minute == currentTime.minute &&
            ((alarms[i].weekDays & currentTime.weekDays) != 0)) {
            
            // Verifica se este alarme já não foi tocado ou dispensado neste mesmo minuto
            if (lastTriggeredMin != currentTime.minute || lastTriggeredHr != currentTime.hour) {
                ringAlarm = true;
                lastTriggeredMin = currentTime.minute;
                lastTriggeredHr = currentTime.hour;
                break;
            }
        }
    }
}

// Sincroniza a biblioteca TimeLib com a hora e data da compilação (__TIME__ e __DATE__)
void AlarmHardware::syncCompilerTime() {
    // __TIME__ formato: "HH:MM:SS"
    int hr = 0, min = 0, sec = 0;
    sscanf(__TIME__, "%d:%d:%d", &hr, &min, &sec);

    // __DATE__ formato: "Mmm dd yyyy" (ex: "Oct 27 2026")
    char monthStr[4];
    int day = 1, year = 2026, month = 0;
    sscanf(__DATE__, "%s %d %d", monthStr, &day, &year);

    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (month = 0; month < 12; month++) {
        if (strcmp(monthStr, months[month]) == 0) {
            break;
        }
    }
    month++; // Ajusta de índice 0-11 para 1-12

    setTime(hr, min, sec, day, month, year);
}

// ==============================================
// Leitura e escrita de dados de tempo e data (RTC)
// ==============================================
TimeData AlarmHardware::getCurrentTime() {
    TimeData currentTime;
    currentTime.second = second();
    currentTime.minute = minute();
    currentTime.hour = hour();

    // Dias da semana da TimeLib: 1 = Domingo, 2 = Segunda, ..., 7 = Sábado
    int diatimelib = weekday();
    int deslocamento;
    if (diatimelib == 1) { // Domingo
        deslocamento = 6;  // Bit 6
    } else {
        deslocamento = diatimelib - 2; // Segunda = bit 0, Terça = bit 1, ..., Sábado = bit 5
    }
    currentTime.weekDays = (1 << deslocamento);
    return currentTime;
}

void AlarmHardware::setCurrentDateTime(uint8_t hr, uint8_t min, uint8_t sec, uint8_t day, uint8_t month, uint16_t year) {
    setTime(hr, min, sec, day, month, year);
}

uint8_t AlarmHardware::getCurrentDay() {
    return day();
}

uint8_t AlarmHardware::getCurrentMonth() {
    return month();
}

uint16_t AlarmHardware::getCurrentYear() {
    return year();
}

TimeData AlarmHardware::getAlarmData(uint8_t alarmIndex) {
    if (alarmIndex < totalAlarms && alarmIndex < MAX_ALARMS) {
        return alarms[alarmIndex];
    }
    return {0, 0, 0, 127};
}

uint8_t AlarmHardware::getTotalAlarms() {
    return totalAlarms;
}

// =====================================================
// Ações disparadas pelas telas de [SET], [DELETE], etc.
// =====================================================
void AlarmHardware::saveAlarm(uint8_t alarmIndex, TimeData data) {
    if (alarmIndex < MAX_ALARMS) {
        data.second = 0;
        if (data.weekDays == 0) data.weekDays = 127; // Todos os dias por padrão

        alarms[alarmIndex] = data;

        // Se estiver adicionando além do total atual, incrementa
        if (alarmIndex >= totalAlarms) {
            totalAlarms = alarmIndex + 1;
        }

        EEPROM.put(0, totalAlarms);
        EEPROM.put(1, alarms);
    }
}

void AlarmHardware::deleteAlarm(uint8_t alarmIndex) {
    if (alarmIndex >= totalAlarms || totalAlarms == 0) return;

    // Desloca os alarmes posteriores uma posição para trás
    for (uint8_t i = alarmIndex; i < totalAlarms - 1; ++i) {
        alarms[i] = alarms[i + 1];
    }

    // Zera o último slot liberado
    alarms[totalAlarms - 1] = {0, 0, 0, 127};
    totalAlarms--;

    // Persiste alterações na EEPROM
    EEPROM.put(0, totalAlarms);
    EEPROM.put(1, alarms);
}

void AlarmHardware::deleteAllAlarms() {
    totalAlarms = 0;
    for (uint8_t i = 0; i < MAX_ALARMS; ++i) {
        alarms[i] = {0, 0, 0, 127};
    }

    EEPROM.put(0, totalAlarms);
    EEPROM.put(1, alarms);
}

// =========================
// Controle de alarme/buzzer
// =========================
bool AlarmHardware::isAlarmRinging() {
    return ringAlarm;
}

void AlarmHardware::stopAlarm() {
    ringAlarm = false;
}
