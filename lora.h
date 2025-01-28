
#ifndef LORA_MODULE
#define LORA_MODULE

void LoRaInit();
bool LoRaService();
bool LoRaSendPacket(const char* packet, bool debugOn);

#endif
