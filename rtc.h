#ifndef RTC_MODULE
#define RTC_MODULE

bool RtcSetTime(const char* time, char* feedbackMsg);
char* RtcGetTime();
bool RtcSetUnixTime(uint32_t unixTime);
uint32_t RtcGetUnixTime();
void RtcSetAlarmTime(uint32_t unixTime);
bool RtcGetAlarmFlag();

void RtcInit();
bool RtcService();

#endif
