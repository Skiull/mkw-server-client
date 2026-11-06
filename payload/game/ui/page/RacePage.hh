#pragma once

#include "game/ui/page/Page.hh"

namespace UI {

class RacePage : public Page {
public:
    ~RacePage() override;

    static TypeInfo *GetTypeInfo();
    static RacePage *Instance() {
        return s_instance;
    }

    u32 watchedPlayerId() const {
        return m_watchedPlayerId;
    }

protected:
    virtual void vf_64() = 0;
    virtual u32 getControls() const = 0;
    virtual u32 getNameBalloonCount() const;
    virtual void vf_70();
    virtual void vf_74();
    virtual void vf_78();
    virtual void vf_7c();

private:
    u8 REPLACED(getControlCount)(u32 controls) const;
    REPLACE u8 getControlCount(u32 controls) const;
    void REPLACED(initControls)(u32 controls);
    REPLACE void initControls(u32 controls);

    bool pingEnabled() const;

    PageId m_replacement;
    u8 _048[0x064 - 0x048];
    u32 m_watchedPlayerId;
    u8 _068[0x1dc - 0x068];

    static RacePage *s_instance;
};
static_assert(sizeof(RacePage) == 0x1dc);

} // namespace UI
