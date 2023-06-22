#ifndef SERVICES_CUCUMBER_STEP_HPP
#define SERVICES_CUCUMBER_STEP_HPP

#include "infra/syntax/Json.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/IntrusiveList.hpp"

namespace services
{
    enum class InvokeStatus : uint8_t
    {
        success,
        failure,
        pending
    };

    struct InvokeResult
    {
        InvokeStatus status;
        infra::BoundedConstString reason;

        bool Success() const
        {
            return status == InvokeStatus::success;
        }
    };

    static InvokeResult invokeSuccess{ InvokeStatus::success, "" };

    class CucumberStep
        : public infra::IntrusiveList<CucumberStep>::NodeType
    {
    public:
        CucumberStep(infra::BoundedConstString stepName, infra::BoundedConstString sourceLocation);
        CucumberStep& operator=(const CucumberStep&) = delete;
        CucumberStep& operator=(CucumberStep&&) noexcept = delete;
        CucumberStep(CucumberStep&) = delete;
        CucumberStep(CucumberStep&&) noexcept = delete;
        virtual ~CucumberStep() = default;

        infra::BoundedConstString StepName() const;
        infra::BoundedConstString SourceLocation() const;

        bool Matches(infra::BoundedConstString name) const;

        bool ContainsTableArgument(infra::JsonArray& arguments, infra::BoundedConstString fieldName) const;
        infra::JsonArray GetTable(infra::JsonArray& arguments) const;
        infra::Optional<infra::JsonString> GetTableArgument(infra::JsonArray& arguments, infra::BoundedConstString fieldName) const;
        infra::Optional<infra::JsonString> GetStringArgument(infra::JsonArray& arguments, uint8_t argumentNumber) const;
        infra::Optional<uint32_t> GetUIntegerArgument(infra::JsonArray& arguments, uint8_t argumentNumber) const;
        infra::Optional<bool> GetBooleanArgument(infra::JsonArray& arguments, uint8_t argumentNumber) const;
        bool HasArguments() const;
        bool ContainsArgument(infra::JsonArray& arguments, uint8_t index) const;
        uint16_t NrArguments() const;
        uint16_t NrFields(infra::JsonArray& arguments) const;

        InvokeResult Invoke(infra::JsonArray& arguments);

        virtual void StepImplementation(infra::JsonArray& arguments) = 0;

    private:
        void SkipOverArguments(infra::JsonArrayIterator& iterator) const;

    private:
        infra::BoundedConstString stepName;
        infra::BoundedConstString sourceLocation;
    };
}

#endif
