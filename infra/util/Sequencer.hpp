#ifndef INFRA_SEQUENCER_HPP
#define INFRA_SEQUENCER_HPP

#include "infra/util/BoundedVector.hpp"
#include "infra/util/Function.hpp"

#ifndef INFRA_SEQUENCER_FUNCTION_EXTRA_SIZE
#define INFRA_SEQUENCER_FUNCTION_EXTRA_SIZE (INFRA_DEFAULT_FUNCTION_EXTRA_SIZE + (2 * sizeof(void*)))
#endif

namespace infra
{
    class Sequencer
    {
    public:
        using Function = infra::Function<void(), INFRA_SEQUENCER_FUNCTION_EXTRA_SIZE>;

        void Load(const Function& newSequence);

        void Step(const Function& action);
        void Execute(const Function& action);
        void If(const infra::Function<bool()>& condition);
        void ElseIf(const infra::Function<bool()>& condition);
        void Else();
        void EndIf();
        void While(const infra::Function<bool()>& condition);
        void EndWhile();
        void DoWhile();
        void EndDoWhile(const infra::Function<bool()>& condition);
        void ForEach(uint32_t& variable, uint32_t from, uint32_t to);
        void EndForEach(uint32_t& variable);

        void Continue();
        bool Finished() const;

    private:
        void ExecuteWithoutContext(const Function& action);

        void IncreaseCurrentStep();
        bool ExecuteCurrentStep() const;

        void ExecutePreviousStep();
        void ExecuteNextStep();

        void PushContext();
        void PopContext();

    private:
        Function sequence;

        infra::BoundedVector<uint8_t>::WithMaxSize<8> execute;
        infra::BoundedVector<uint8_t>::WithMaxSize<8> examine;
        bool repeat = false;
    };
}

#endif
