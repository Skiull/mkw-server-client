// See LICENSE-mkw-sp

#include "SettingsPage.hh"

#include "game/ui/page/RaceMenuPage.hh"

#include "game/ui/SectionManager.hh"

#include <sp/settings/Settings.hh>

namespace UI {

SettingsPage::SettingsPage() = default;

SettingsPage::~SettingsPage() = default;

void SettingsPage::onInit() {
    m_inputManager.init(0x1, false);
    setInputManager(&m_inputManager);
    m_inputManager.setWrappingMode(MultiControlInputManager::WrappingMode::Y);

    initChildren(4 + std::size(m_settingControls) + !!blackBack());
    insertChild(0, &m_pageTitleText, 0);
    insertChild(1, &m_categoryControl, 0);
    insertChild(2, instructionText(), 0);
    insertChild(3, &m_backButton, 0);
    for (u32 i = 0; i < std::size(m_settingControls); i++) {
        insertChild(4 + i, &m_settingControls[i], 0);
    }
    if (blackBack()) {
        insertChild(4 + std::size(m_settingControls), blackBack(), 0);
    }

    m_pageTitleText.load(false);
    m_categoryControl.load(getSheetCount(), 0, "control", "CategoryUpDownBase", "Category",
            "CategoryUpDownButtonR", "RightButton", "CategoryUpDownButtonL", "LeftButton",
            m_categoryValue.animator(), 0x1, false, false, true, true);
    m_categoryValue.load("ranking", "CategoryUpDownValue", "Value", "CategoryUpDownText", "Text");
    for (u32 i = 0; i < std::size(m_settingControls); i++) {
        char variant[0x20];
        snprintf(variant, sizeof(variant), "UpDown%u", i);
        m_settingControls[i].load(1, 0, "control", "SettingUpDownBase", variant,
                "SettingUpDownButtonR", "RightButton", "SettingUpDownButtonL", "LeftButton",
                m_settingValues[i].animator(), 0x1, false, false, true, true);
        m_settingValues[i].load("ranking", "SettingUpDownValue", "Value", "SettingUpDownText",
                "Text");
    }
    auto sectionId = SectionManager::Instance()->currentSection()->id();
    if (blackBack()) {
        instructionText()->load("bg", "ObiInstructionTextPopup", "ObiInstructionTextPopup",
                nullptr);
        m_backButton.load("button", "Back", "ButtonBackPopup", 0x1, false, true);
    } else if (Section::GetSceneId(sectionId) == System::SceneId::Race) {
        m_backButton.load("message_window", "Back", "ButtonBack", 0x1, false, true);
        instructionText()->load("bg", "RaceObiInstructionText", "RaceObiInstructionText", nullptr);
    } else {
        m_backButton.load("button", "Back", "ButtonBack", 0x1, false, true);
        instructionText()->load("bg", "MenuObiInstructionText", "MenuObiInstructionText", nullptr);
    }
    if (blackBack()) {
        blackBack()->load("message_window", "BlackBack", "BlackBack");
        blackBack()->m_zIndex = -1.0f;
    }

    m_inputManager.setHandler(MenuInputManager::InputId::Back, &m_onBack, false, false);
    m_categoryControl.setFrontHandler(&m_onCategoryControlFront);
    m_categoryControl.setSelectHandler(&m_onCategoryControlSelect);
    m_categoryValue.setChangeHandler(&m_onCategoryValueChange);
    m_backButton.setFrontHandler(&m_onBackButtonFront, false);
    for (u32 i = 0; i < std::size(m_settingControls); i++) {
        m_settingControls[i].setChangeHandler(&m_onSettingControlChange);
        m_settingControls[i].setFrontHandler(&m_onSettingControlFront);
        m_settingControls[i].setSelectHandler(&m_onSettingControlSelect);
    }

    m_pageTitleText.setMessage(10000);
}

void SettingsPage::onActivate() {
    instructionText()->setMessageAll(0);
    m_categoryControl.selectDefault(0);
}

BlackBackControl *SettingsPage::blackBack() {
    return nullptr;
}

void SettingsPage::onBack(u32 /* localPlayerId */) {
    if (m_handler) {
        m_handler->handle(this, nullptr);
    }

    startReplace(Anim::Prev, 0.0f);
}

void SettingsPage::onCategoryControlFront(UpDownControl * /* control */, u32 localPlayerId) {
    m_settingControls[0].select(localPlayerId);
}

void SettingsPage::onCategoryControlSelect(UpDownControl * /* control */, u32 /* localPlayerId */) {
    instructionText()->setMessageAll(0);
}

void SettingsPage::onCategoryValueChange(TextUpDownValueControl::TextControl *text, u32 index) {
    auto categoryInfo = getCategoryInfo(index);

    if (categoryInfo.categorySheetCount >= 2) {
        MessageInfo info{};
        info.messageIds[0] = Settings::categoryMessageIds[categoryInfo.categoryIndex];
        info.intVals[0] = categoryInfo.categorySheetIndex + 1;
        info.intVals[1] = categoryInfo.categorySheetCount;
        text->setMessageAll(10182, &info);
    } else {
        text->setMessageAll(Settings::categoryMessageIds[categoryInfo.categoryIndex]);
    }

    auto category = static_cast<Settings::Category>(categoryInfo.categoryIndex);
    u32 i = 0;
    for (u32 j = categoryInfo.settingIndex;
            j < Settings::settingCount && i < std::size(m_settingControls); j++) {
        if (Settings::settings[j].category != category) {
            continue;
        }
        if (Settings::settings[j].valueCount == 0) {
            continue;
        }
        m_settingControls[i].m_id = j << 16 | i;
        const Settings::SettingEntry &settingEntry = Settings::settings[j];
        m_settingControls[i].setMessageAll(settingEntry.messageId);
        u32 count = settingEntry.valueCount;
        u32 chosen = Settings::getSetting(j) - settingEntry.valueOffset;
        m_settingControls[i].reconfigure(count, chosen);
        m_settingControls[i].setVisible(true);
        m_settingControls[i].setPlayerFlags(0x1);
        auto *shownText = m_settingValues[i].shownText();
        auto *hiddenText = m_settingValues[i].hiddenText();
        if (settingEntry.valueNames) {
            shownText->setMessageAll(settingEntry.valueMessageIds[chosen]);
            hiddenText->setMessageAll(settingEntry.valueMessageIds[chosen]);
        } else {
            chosen += settingEntry.valueOffset;
            MessageInfo info{};
            info.intVals[0] = chosen;
            shownText->setMessageAll(settingEntry.valueMessageIds[0], &info);
            hiddenText->setMessageAll(settingEntry.valueMessageIds[0], &info);
        }
        i++;
    }
    for (; i < std::size(m_settingControls); i++) {
        m_settingControls[i].setVisible(false);
        m_settingControls[i].setPlayerFlags(0x0);
    }
}

void SettingsPage::onSettingControlChange(UpDownControl *control, u32 /* localPlayerId */,
        u32 index) {
    auto *text = m_settingValues[control->m_id & 0xffff].shownText();
    const Settings::SettingEntry &settingEntry = Settings::settings[control->m_id >> 16];
    if (settingEntry.valueNames) {
        text->setMessageAll(settingEntry.valueMessageIds[index]);
        instructionText()->setMessageAll(settingEntry.valueExplanationMessageIds[index]);
    } else {
        index += settingEntry.valueOffset;
        MessageInfo info{};
        info.intVals[0] = index;
        text->setMessageAll(settingEntry.valueMessageIds[0], &info);
        instructionText()->setMessageAll(settingEntry.valueExplanationMessageIds[0], &info);
    }
    Settings::setSetting(control->m_id >> 16, index);
}

void SettingsPage::onSettingControlFront(UpDownControl *control, u32 localPlayerId) {
    u32 index = control->m_id & 0xffff;
    if (index + 1 == std::size(m_settingControls) || !m_settingControls[index + 1].getVisible()) {
        m_categoryControl.select(localPlayerId);
    } else {
        m_settingControls[index + 1].select(localPlayerId);
    }
}

void SettingsPage::onSettingControlSelect(UpDownControl *control, u32 /* localPlayerId */) {
    u32 chosen = control->chosen();
    const Settings::SettingEntry &settingEntry = Settings::settings[control->m_id >> 16];
    if (settingEntry.valueNames) {
        instructionText()->setMessageAll(settingEntry.valueExplanationMessageIds[chosen]);
    } else {
        chosen += settingEntry.valueOffset;
        MessageInfo info{};
        info.intVals[0] = chosen;
        instructionText()->setMessageAll(settingEntry.valueExplanationMessageIds[0], &info);
    }
}

void SettingsPage::onBackButtonFront(PushButton *button, u32 /* localPlayerId */) {
    if (m_handler) {
        m_handler->handle(this, button);
    }

    f32 delay = button->getDelay();
    startReplace(Anim::Prev, delay);
}

u32 SettingsPage::getSheetCount() const {
    u32 sheetCount = 0;
    for (u32 i = 0; i < magic_enum::enum_count<Settings::Category>(); i++) {
        auto category = static_cast<Settings::Category>(i);
        u32 settingCount = 0;
        for (u32 j = 0; j < magic_enum::enum_count<Settings::Setting>(); j++) {
            if (Settings::settings[j].category != category) {
                continue;
            }
            if (Settings::settings[j].valueCount == 0) {
                continue;
            }
            settingCount++;
        }
        u32 controlCount = std::size(m_settingControls);
        sheetCount += (settingCount + controlCount - 1) / controlCount;
    }
    return sheetCount;
}

SettingsPage::CategoryInfo SettingsPage::getCategoryInfo(u32 sheetIndex) const {
    u32 controlCount = std::size(m_settingControls);
    CategoryInfo info{};
    u32 sheetCount = 0;
    u32 categoryCount = magic_enum::enum_count<Settings::Category>();
    for (; info.categoryIndex < categoryCount; info.categoryIndex++) {
        auto category = static_cast<Settings::Category>(info.categoryIndex);
        u32 settingCount = 0;
        for (u32 i = 0; i < magic_enum::enum_count<Settings::Setting>(); i++) {
            if (Settings::settings[i].category != category) {
                continue;
            }
            if (Settings::settings[i].valueCount == 0) {
                continue;
            }
            if (settingCount % controlCount == 0) {
                if (sheetCount == sheetIndex) {
                    info.categorySheetIndex = settingCount / std::size(m_settingControls);
                    info.settingIndex = i;
                }
                sheetCount++;
            }
            settingCount++;
        }
        if (sheetCount > sheetIndex) {
            info.categorySheetCount = (settingCount + controlCount - 1) / controlCount;
            return info;
        }
    }
    return info;
}

SettingsPagePopup::SettingsPagePopup() = default;

SettingsPagePopup::~SettingsPagePopup() = default;

void SettingsPagePopup::onInit() {
    SettingsPage::onInit();

    m_popRequested = false;
}

void SettingsPagePopup::beforeCalc() {
    if (state() == State::State4 && m_popRequested) {
        startReplace(Anim::Prev, 0.0f);
        m_popRequested = false;
    }
}

LayoutUIControl *SettingsPagePopup::instructionText() {
    return &m_instructionText;
}

BlackBackControl *SettingsPagePopup::blackBack() {
    return &m_blackBack;
}

void SettingsPagePopup::configure(IHandler *handler) {
    m_handler = handler;
}

void SettingsPagePopup::pop(Anim anim) {
    setAnim(anim);
    m_popRequested = true;
}

MenuSettingsPage::MenuSettingsPage() = default;

MenuSettingsPage::~MenuSettingsPage() = default;

PageId MenuSettingsPage::getReplacement() {
    return m_replacement;
}

void MenuSettingsPage::onInit() {
    SettingsPage::onInit();

    m_replacement = PageId::None; // Failsafe
}

void MenuSettingsPage::onActivate() {
    SettingsPage::onActivate();

    RaceMenuPage::OnSettingsActivated();
}

LayoutUIControl *MenuSettingsPage::instructionText() {
    return &m_instructionText;
}

BlackBackControl *MenuSettingsPage::blackBack() {
    return nullptr;
}

void MenuSettingsPage::configure(IHandler *handler, PageId replacement) {
    m_handler = handler;
    m_replacement = replacement;
}

} // namespace UI
