#ifndef MONITORLOGIC_H
#define MONITORLOGIC_H

struct AlarmFlags
{
    bool temperatureExceeded = false;
    bool humidityExceeded = false;
};

inline AlarmFlags evaluateAlarm(double temperature, double humidity,
                                double temperatureLimit, double humidityLimit)
{
    return {temperature > temperatureLimit, humidity > humidityLimit};
}

#endif // MONITORLOGIC_H
