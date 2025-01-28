#ifndef RTC_MODULE
#define RTC_MODULE

bool RtcSetTime(const char* time, char* feedbackMsg);
char* RtcGetTime();

#endif
