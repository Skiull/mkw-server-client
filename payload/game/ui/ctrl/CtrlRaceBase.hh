// See LICENSE-mkw-sp

#pragma once

#include "game/ui/UIControl.hh"

namespace UI {

class CtrlRaceBase : public LayoutUIControl {
public:
    CtrlRaceBase();
    ~CtrlRaceBase() override;
    void initSelf() override;
    void vf_28() override;
    const char *getTypeName() override;

protected:
    virtual void process();
    virtual void vf_40();
    virtual void vf_44();
    virtual bool vf_48();
    virtual void vf_4c();

    void REPLACED(setPaneColor)(const char *pane, bool teamColors);
    REPLACE void setPaneColor(const char *pane, bool teamColors);

public:
    s8 getPlayerId() const;
    // Hides the pane if the HUD Labels setting is "Hide" (or with 3-4 local players).
    void calcLabelVisibility(const char *pane);

private:
    u8 _174[0x190 - 0x174];

protected:
    u8 m_localPlayerId;

private:
    u8 _191[0x198 - 0x191];
};
static_assert(sizeof(CtrlRaceBase) == 0x198);

} // namespace UI
