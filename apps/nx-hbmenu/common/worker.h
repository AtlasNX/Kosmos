#pragma once
#include "common.h"

bool workerInit(void);
void workerExit(void);
void workerSchedule(workerThreadFunc func, void* data);
