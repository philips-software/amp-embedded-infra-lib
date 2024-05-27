#ifndef SERVICES_CUCUMBER_STEP_HPP
#define SERVICES_CUCUMBER_STEP_HPP

#include "infra/syntax/Json.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "services/cucumber/CucumberContext.hpp"

namespace services
{
    class CucumberStep
        : public infra::IntrusiveList<CucumberStep>::NodeType
    {
    public:
        CucumberStep(infra::BoundedConstString stepName, infra::BoundedConstString sourceLocation);
        virtual ~CucumberStep() = default;
        CucumberStep& operator=(const CucumberStep& other) = delete;
        CucumberStep(CucumberStep& other) = delete;

        bool operator==(const CucumberStep& other) const;

        infra::BoundedConstString StepName() const;
        infra::BoundedConstString SourceLocation() const;

        virtual void Invoke(infra::JsonArray& arguments) = 0;

        virtual bool HasStringArguments() const = 0;
        virtual uint16_t NrArguments() const = 0;
        virtual uint16_t NrFields() const = 0;

        services::CucumberContext& Context();

    private:
        infra::BoundedConstString stepName;
        infra::BoundedConstString sourceLocation;
    };

    class CucumberStepArguments
        : public CucumberStep
    {
    public:
        using CucumberStep::CucumberStep;

        void Invoke(infra::JsonArray& arguments) override;

        bool HasStringArguments() const override;
        uint16_t NrArguments() const override;
        uint16_t NrFields() const override;

        bool ContainsTableArgument(infra::BoundedConstString fieldName) const;
        infra::JsonArray GetTable() const;
        std::optional<infra::JsonString> GetTableArgument(infra::BoundedConstString fieldName) const;
        std::optional<infra::JsonString> GetStringArgument(uint8_t argumentNumber) const;
        std::optional<uint32_t> GetUIntegerArgument(uint8_t argumentNumber) const;
        std::optional<bool> GetBooleanArgument(uint8_t argumentNumber) const;
        bool ContainsStringArgument(uint8_t index) const;

    protected:
        virtual void Execute() = 0;

        template<size_t Size>
        infra::BoundedConstString::WithStorage<Size> GetStringArgumentValue(uint8_t argumentNumber) const
        {
            infra::BoundedString::WithStorage<Size> param;
            GetStringArgument(argumentNumber)->ToString(param);
            return param;
        }

    private:
        void SkipOverStringArguments(infra::JsonArrayIterator& iterator) const;

        infra::JsonArray* invokeArguments{ nullptr };
    };

    class CucumberStepProgress
        : public CucumberStepArguments
    {
    public:
        using CucumberStepArguments::CucumberStepArguments;

        virtual void Success();
        virtual void Error(infra::BoundedConstString failReason);

        void Invoke(infra::JsonArray& arguments) override;

        bool isActive{ false };
    };
}

#endif
