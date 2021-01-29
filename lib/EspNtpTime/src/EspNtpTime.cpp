#include "EspNtpTime.h"

EspNtpTime::EspNtpTime(){}

void EspNtpTime::init(const char *server1, const char *server2, const char *server3) {
    configTime(0, 0, server1, server2, server3);
}

/**
 * Checks if internal time was set from NTP
 * @return bool
 */
boolean EspNtpTime::isSet()
{
    struct tm t{};
    return getLocalTime(&t);
}

/**
 * Waits until internal time is set from NTP
 * @returns bool
 */
boolean EspNtpTime::waitForTime(unsigned long timeoutMs)
{
    struct tm t{};
    return getLocalTime(&t, timeoutMs);
}

/**
 * Writes current time in UTC ISO 8601 format (21 chars)
 * @returns 0 if success, otherwise -1
 */
int EspNtpTime::getUtcIsoString(char *t) {
    struct tm timeinfo{};

    if (!getLocalTime(&timeinfo)) {
        return -1;
    }

    sprintf(t, "%04d-%02d-%02dT%02d:%02d:%02dZ", 1900 + timeinfo.tm_year, 1 + timeinfo.tm_mon,
            timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    return 0;
}