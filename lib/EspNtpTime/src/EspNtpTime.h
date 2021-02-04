#ifndef ESPNTPTIME_H
#define ESPNTPTIME_H

#include <Arduino.h>
#include "time.h"

#define DEFAULT_NTP_SERVERS "europe.pool.ntp.org", "time.nist.gov", "time.google.com"

class EspNtpTime
{
public:
    void init(const char *server1, const char *server2 = (const char *)nullptr, const char *server3 = (const char *)nullptr);
    void init();
    boolean isSet();
    boolean waitForTime(unsigned long timeoutMs = 60000);
    int getUtcIsoString(char *t);
};

#endif //ESPNTPTIME_H