#ifndef HAL_GPIO_STUB_HPP
#define HAL_GPIO_STUB_HPP

#include "hal/interfaces/Gpio.hpp"
#include "infra/timer/TimerService.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/VariantDetail.hpp"
#include <array>
#include <functional>
#include <map>
#include <vector>

namespace hal
{
    class GpioPinStub
        : public GpioPin
    {
    public:
        virtual ~GpioPinStub() = default;

        virtual bool Get() const override;
        virtual void Set(bool value) override;
        virtual bool GetOutputLatch() const override;
        virtual void SetAsInput() override;
        virtual bool IsInput() const override;
        virtual void Config(PinConfigType config) override;
        virtual void Config(PinConfigType config, bool startOutputState) override;
        virtual void ResetConfig() override;
        virtual void EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger) override;
        virtual void DisableInterrupt() override;

        void SetStubState(bool value);
        bool GetStubState() const;

    private:
        bool state = false;
        bool input = true;

        infra::Optional<std::pair<infra::Function<void()>, InterruptTrigger>> triggerOnChange;
    };

    struct PinChange
    {
        PinChange() = default;
        PinChange(infra::Duration duration, bool state);

        bool operator==(const PinChange& other) const;
        bool operator!=(const PinChange& other) const;

        infra::Duration duration;
        bool state = false;
    };

    class GpioPinSpy
        : public GpioPinStub
    {
    public:
        GpioPinSpy();

        virtual void Set(bool value) override;

        std::vector<PinChange> PinChanges() const;

    private:
        infra::TimePoint start;
        std::vector<PinChange> pinChanges;
    };
}

#endif
