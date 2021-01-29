#ifndef ESPNTPTIME_H
#define ESPNTPTIME_H

#include <Arduino.h>
#include "time.h"

class EspNtpTime
{
public:
    EspNtpTime();
    void init(const char *server1 = (const char *)nullptr, const char *server2 = (const char *)nullptr, const char *server3 = (const char *)nullptr);
    boolean isSet();
    boolean waitForTime(unsigned long timeoutMs = 60000);
    int getUtcIsoString(char *t);
};

#endif //ESPNTPTIME_H