#pragma once
#include <switch.h>

Result ipinfoInit();
void ipinfoExit();
Result ipinfoGetIpConfig(u32* address);
Result ipinfoGetIpConfig(u32* address, u32* netmask);
