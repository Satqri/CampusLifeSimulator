#include "ui/TimePanel.h"
#include "core/Localization.h"
#include "core/TimeSystem.h"

TimePanel::TimePanel(sf::Font& font) : mFont(font) {}

void TimePanel::setTimeSystem(const TimeSystem* time) {
    mTimeSystem = time;
}

void TimePanel::attachToGui(TguiContext& ctx) {
    mTguiCtx = &ctx;
    createWidgets();
}

void TimePanel::createWidgets() {
    if (mWidgetsCreated) return;
    mWidgetsCreated = true;

    using namespace cls::tgui_theme;

    mContainer = tgui::Panel::create({960, 540});
    mContainer->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0));  // 透明
    mContainer->setVisible(false);
    mTguiCtx->gui().add(mContainer);

    // 折叠态 (52x24 at bottom-right)
    mCollapsedPanel = tgui::Panel::create({52, 24});
    mCollapsedPanel->setPosition({900, 506});
    mCollapsedPanel->getRenderer()->setBackgroundColor(tgui::Color(10, 18, 26, 218));
    mCollapsedPanel->getRenderer()->setBorders({1.0f});
    mCollapsedPanel->getRenderer()->setBorderColor(tgui::Color(80, 96, 118));
    mCollapsedPanel->onClick([this](tgui::Vector2f) { toggle(); });
    mContainer->add(mCollapsedPanel);

    mToggleLabel = createLabel(cls::text("time.toggle"), 11);
    mToggleLabel->setPosition({910, 511});
    mToggleLabel->getRenderer()->setTextColor(tgui::Color(235, 238, 220));
    mContainer->add(mToggleLabel);

    // 展开态 (216x52)
    mExpandedPanel = tgui::Panel::create({216, 52});
    mExpandedPanel->setPosition({736, 46});
    mExpandedPanel->getRenderer()->setBackgroundColor(tgui::Color(10, 18, 26, 230));
    mExpandedPanel->getRenderer()->setBorders({1.0f});
    mExpandedPanel->getRenderer()->setBorderColor(tgui::Color(80, 96, 118));
    mExpandedPanel->onClick([this](tgui::Vector2f) { toggle(); });
    mExpandedPanel->setVisible(false);
    mContainer->add(mExpandedPanel);

    mClockLabel = createLabel("", 12);
    mClockLabel->setPosition({748, 55});
    mClockLabel->getRenderer()->setTextColor(tgui::Color(235, 238, 220));
    mContainer->add(mClockLabel);

    mDayLabel = createLabel("", 10);
    mDayLabel->setPosition({748, 76});
    mDayLabel->getRenderer()->setTextColor(tgui::Color(155, 180, 205));
    mContainer->add(mDayLabel);
}

void TimePanel::setAlwaysExpanded(bool always) {
    mAlwaysExpanded = always;
    if (mWidgetsCreated && always) {
        mExpanded = true;
        mCollapsedPanel->setVisible(false);
        mToggleLabel->setVisible(false);
        mExpandedPanel->setVisible(true);
        mClockLabel->setVisible(true);
        mDayLabel->setVisible(true);
    }
}

void TimePanel::toggle() {
    if (mAlwaysExpanded) return;
    mExpanded = !mExpanded;
    mCollapsedPanel->setVisible(!mExpanded);
    mToggleLabel->setVisible(!mExpanded);
    mExpandedPanel->setVisible(mExpanded);
    mClockLabel->setVisible(mExpanded);
    mDayLabel->setVisible(mExpanded);
}

void TimePanel::refreshDisplay() {
    if (!mTimeSystem) return;

    bool midterm = mTimeSystem->isMidtermDay();
    tgui::Color outlineColor = midterm ? tgui::Color(255, 190, 90) : tgui::Color(80, 96, 118);

    mCollapsedPanel->getRenderer()->setBorderColor(outlineColor);
    mExpandedPanel->getRenderer()->setBorderColor(outlineColor);

    if (mExpanded) {
        mClockLabel->setText(mTimeSystem->clockText());
        mDayLabel->setText(mTimeSystem->dayLabel());
        mDayLabel->getRenderer()->setTextColor(midterm ? tgui::Color(255, 210, 120) : tgui::Color(155, 180, 205));
    }
}

void TimePanel::update(float deltaTime) {
    (void)deltaTime;
    refreshDisplay();
}

void TimePanel::render(sf::RenderWindow& window) {
    (void)window;
}

void TimePanel::setVisible(bool visible) {
    mVisible = visible;
    if (mContainer) mContainer->setVisible(visible);
}
