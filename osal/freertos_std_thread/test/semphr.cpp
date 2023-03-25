#include "semphr.h"

SemaphoreMock* SemaphoreMock::instance = nullptr;

xSemaphoreHandle SemaphoreCreateBinary()
{
    return SemaphoreMock::instance->SemaphoreCreateBinary();
}

void vSemaphoreDelete(xSemaphoreHandle handle)
{
    SemaphoreMock::instance->SemaphoreDelete(handle);
}

void xSemaphoreGive(xSemaphoreHandle handle)
{
    SemaphoreMock::instance->SemaphoreGive(handle);
}

bool xSemaphoreTake(xSemaphoreHandle handle, portTickType timeout)
{
    return SemaphoreMock::instance->SemaphoreTake(handle, timeout);
}
