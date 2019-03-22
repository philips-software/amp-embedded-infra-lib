#include "upgrade/deploy_pack_to_external/DeployPackToExternal.hpp"

namespace application
{
    DeployPackToExternal::DeployPackToExternal(hal::Flash& from_, hal::Flash& to_)
        : from(from_)
        , to(to_)
        , header()
    {
        sequencer.Load([this]() {
            sequencer.Step([this]() { from.ReadBuffer(infra::MakeByteRange(header), 0, [this]() { sequencer.Continue(); }); });
            sequencer.Execute([this]() { sizeToDo = header.signedContentsLength + sizeof(UpgradePackHeaderPrologue) + header.signatureLength; });
            sequencer.If([this]() { return header.status != UpgradePackStatus::readyToDeploy; });
                sequencer.Execute([this]() { GetObserver().NotDeployable(); });
            sequencer.ElseIf([this]() { return sizeToDo <= from.TotalSize(); });
                sequencer.Step([this]() { to.EraseAll([this]() { sequencer.Continue(); }); });
                sequencer.While([this]() { return sizeToDo != 0; });
                    sequencer.Step([this]()
                    {
                        currentSize = std::min(buffer.size(), sizeToDo);
                        from.ReadBuffer(buffer, currentReadAddress, [this]() { sequencer.Continue(); });
                    });
                    sequencer.Step([this]()
                    {
                        to.WriteBuffer(buffer, currentWriteAddress, [this]() { sequencer.Continue(); });
                    });
                    sequencer.Execute([this]()
                    {
                        currentReadAddress += currentSize;
                        currentWriteAddress += currentSize;
                        sizeToDo -= currentSize;
                    });
                sequencer.EndWhile();
                sequencer.Step([this]()
                {
                    static const UpgradePackStatus statusDeployed = UpgradePackStatus::deployed;
                    from.WriteBuffer(infra::MakeByteRange(statusDeployed), 0, [this]() { sequencer.Continue(); });
                });
                sequencer.Execute([this]() { GetObserver().Done(); });
            sequencer.Else();
                sequencer.Execute([this]() { GetObserver().DoesntFit(); });
            sequencer.EndIf();
        });
    }
}
