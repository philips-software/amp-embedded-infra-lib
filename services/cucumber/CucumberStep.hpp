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

        virtual void Invoke(infra::JsonArray& arguments) override;

        virtual bool HasStringArguments() const override;
        virtual uint16_t NrArguments() const override;
        virtual uint16_t NrFields() const override;

        bool ContainsTableArgument(infra::BoundedConstString fieldName) const;
        infra::JsonArray GetTable() const;
        infra::Optional<infra::JsonString> GetTableArgument(infra::BoundedConstString fieldName) const;
        infra::Optional<infra::JsonString> GetStringArgument(uint8_t argumentNumber) const;
        infra::Optional<uint32_t> GetUIntegerArgument(uint8_t argumentNumber) const;
        infra::Optional<bool> GetBooleanArgument(uint8_t argumentNumber) const;
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
    };
}

#endif
