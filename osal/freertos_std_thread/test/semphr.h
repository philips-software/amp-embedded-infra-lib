#ifndef TEST_FREERTOS_STD_THREAD_SEMPHR_H
#define TEST_FREERTOS_STD_THREAD_SEMPHR_H

#include "gmock/gmock.h"
#include <chrono>

typedef void* xSemaphoreHandle;
typedef std::chrono::system_clock::duration::rep portTickType;

static const portTickType portMAX_DELAY = -1;
static const bool pdTRUE = true;

#define vSemaphoreCreateBinary(xSemaphoreHandle) xSemaphoreHandle = SemaphoreCreateBinary()
xSemaphoreHandle SemaphoreCreateBinary();
void vSemaphoreDelete(xSemaphoreHandle handle);
void xSemaphoreGive(xSemaphoreHandle handle);
bool xSemaphoreTake(xSemaphoreHandle handle, portTickType timeout);

class SemaphoreMock
{
public:
    SemaphoreMock()
    {
        instance = this;
    }

    static SemaphoreMock* instance;

    MOCK_METHOD0(SemaphoreCreateBinary, xSemaphoreHandle());
    MOCK_METHOD1(SemaphoreDelete, void(xSemaphoreHandle));
    MOCK_METHOD1(SemaphoreGive, void(xSemaphoreHandle));
    MOCK_METHOD2(SemaphoreTake, bool(xSemaphoreHandle, portTickType));
};

#endif
