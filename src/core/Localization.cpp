#include "core/Localization.h"

#include <unordered_map>

namespace cls {
namespace {

Language gCurrentLanguage = Language::English;

const std::unordered_map<std::string, std::pair<std::string, std::string>> kTexts = {
    {"title.name", {"Campus Life", "校园人生"}},
    {"title.subtitle", {"Semester begins here", "学期从这里开始"}},
    {"title.start", {"New Semester", "开启新学期"}},
    {"title.settings", {"Settings", "设置"}},
    {"title.help", {"Help", "帮助"}},
    {"title.nav", {"A/D or Left/Right: Select  |  Enter: Confirm", "A/D 或 左右键：切换  |  回车：确认"}},

    {"difficulty.title", {"Choose Difficulty", "选择难度"}},
    {"difficulty.subtitle", {"(you can tune it later)", "（之后也可以在设置中调整）"}},
    {"difficulty.easy.title", {"Easy", "简单"}},
    {"difficulty.easy.desc", {"More SAN and energy.\nExplore systems\nwith less pressure.", "更多 SAN 与精力。\n更轻松地体验\n各项系统。"}},
    {"difficulty.normal.title", {"Normal", "普通"}},
    {"difficulty.normal.desc", {"Balanced campus life.\nRecommended\nfirst run.", "校园生活较平衡。\n推荐第一次游玩\n选择此难度。"}},
    {"difficulty.hard.title", {"Hard", "困难"}},
    {"difficulty.hard.desc", {"Lower SAN buffer.\nStress appears\nearlier.", "更低的 SAN 缓冲。\n压力与敌人会\n更早出现。"}},
    {"difficulty.note", {"Keyboard: 1 Easy   2 Normal   3 Hard   Esc Back", "键盘：1 简单   2 普通   3 困难   Esc 返回"}},

    {"page.entity", {"Entity", "探索"}},
    {"page.simple", {"SimpleQuest", "简单任务"}},
    {"page.midterm", {"MidtermExam", "期中考试"}},
    {"page.final", {"FinalExam", "期末考试"}},
    {"page.quest_manager", {"QuestManager", "任务链"}},
    {"page.help_settings", {"Help / Settings", "帮助 / 设置"}},

    {"hud.gold", {"Gold", "金币"}},
    {"hud.san_level", {"SAN Level", "SAN 等级"}},
    {"hud.buff", {"Buff", "增益"}},
    {"hud.win", {"[Win]", "[胜利]"}},
    {"hud.none", {"[-]", "[-]"}},
    {"hud.san", {"SAN", "SAN"}},
    {"hud.energy", {"EN", "精力"}},
    {"hud.academic", {"ACD", "学业"}},
    {"hud.social", {"SOC", "社交"}},

    {"time.day", {"Day {day}", "第 {day} 天"}},
    {"time.early_morning", {"Early Morning", "清晨"}},
    {"time.noon", {"Noon", "中午"}},
    {"time.afternoon", {"Afternoon", "下午"}},
    {"time.evening", {"Evening", "傍晚"}},
    {"time.night", {"Night", "夜晚"}},
    {"time.finished", {"Semester Complete", "学期结束"}},

    {"help.title", {"Help / Settings", "帮助 / 设置"}},
    {"help.section.pages", {"Page Switch", "页面切换"}},
    {"help.pages", {"1 Entity Demo    2 Simple Quest    3 Midterm    4 Final    5 Quest Chain    0/6 Help", "1 探索演示    2 简单任务    3 期中考试    4 期末考试    5 任务链    0/6 帮助"}},
    {"help.section.explore", {"Exploration Controls", "探索操作"}},
    {"help.move", {"WASD / Arrow Keys: Move player", "WASD / 方向键：移动角色"}},
    {"help.click_move", {"Mouse click on map: guide player to target", "鼠标点击地图：引导角色移动到目标点"}},
    {"help.stress", {"C: Stress event, lower SAN, may spawn emotion enemies", "C：触发压力事件，降低 SAN，并可能生成情绪敌人"}},
    {"help.combat", {"F: Fight nearby enemy with d20 roll    V: Restore SAN    X: Set test buff", "F：与附近敌人进行 d20 战斗    V：恢复 SAN    X：设置测试 buff"}},
    {"help.section.quest", {"Quest Controls", "任务操作"}},
    {"help.quest_enter", {"Enter: Continue / Confirm / Roll exam dice", "Enter：继续 / 确认 / 掷考试骰子"}},
    {"help.quest_ud", {"Up / Down: Select quest choices or review option", "上 / 下：选择任务选项或复习选项"}},
    {"help.quest_chain", {"Quest Chain page: E adds event count, T checks trigger, C resets chain", "任务链页面：E 增加事件计数，T 检查触发条件，C 重置任务链"}},
    {"help.section.global", {"Global Shortcuts", "全局快捷键"}},
    {"help.global", {"H: Toggle Help    S: Toggle Settings", "H：切换帮助    S：切换设置"}},
    {"help.close", {"Press H again or Esc to return", "再次按 H 或 Esc 返回"}},

    {"settings.title", {"Settings", "设置"}},
    {"settings.bgm", {"BGM Volume", "背景音乐音量"}},
    {"settings.sfx", {"SFX Volume", "音效音量"}},
    {"settings.window", {"Window Size", "窗口大小"}},
    {"settings.language", {"Language", "语言"}},
    {"settings.back", {"Back", "返回"}},
    {"settings.nav", {"Up/Down: Select  Left/Right: Adjust  Enter: Confirm  Esc/S: Back", "上/下：切换  左/右：调整  Enter：确认  Esc/S：返回"}},
    {"settings.scale.1", {"Small (960x540)", "小（960x540）"}},
    {"settings.scale.2", {"Medium (1280x720)", "中（1280x720）"}},
    {"settings.scale.3", {"Large (1600x900)", "大（1600x900）"}},
    {"settings.scale.4", {"Huge (1920x1080)", "超大（1920x1080）"}},

    {"language.english", {"English", "英文"}},
    {"language.chinese", {"Chinese", "中文"}},

    {"quest.subject", {"Subject", "科目"}},
    {"quest.rounds_total", {"Total {count} rounds", "共 {count} 轮"}},
    {"quest.need_passes", {"Need {count} passes", "需要通过 {count} 轮"}},
    {"quest.review_prompt", {"Review before exam?", "考试前要复习吗？"}},
    {"quest.review_yes", {"> [YES]", "> [是]"}},
    {"quest.review_no", {"[NO]", "[否]"}},
    {"quest.review_skip", {"(skip review)", "（跳过复习）"}},
    {"quest.review_cost", {"(-{cost} Energy, +{bonus} bonus)", "（-{cost} 精力，+{bonus} 加值）"}},
    {"quest.round", {"Round {current}/{total}", "第 {current}/{total} 轮"}},
    {"quest.passed_rounds", {"Passed: {count} rounds", "已通过：{count} 轮"}},
    {"quest.academic_bonus", {"Academic Bonus", "学业加值"}},
    {"quest.review_bonus", {"Review Bonus", "复习加值"}},
    {"quest.total", {"Total", "总值"}},
    {"quest.pass", {"Pass!", "通过！"}},
    {"quest.fail", {"Failed", "失败"}},
    {"quest.final_result", {"Final Result", "最终结果"}},
    {"quest.passed", {"Passed", "通过"}},
    {"quest.failed", {"Failed", "失败"}},
    {"quest.passed_rounds_short", {"Passed Rounds", "通过轮数"}},
    {"quest.prompt.continue", {"[Press Enter to continue]", "[按 Enter 继续]"}},
    {"quest.prompt.choice", {"[Up/Down: Select  |  Enter: Confirm]", "[上/下：选择  |  Enter：确认]"}},
    {"quest.prompt.prep", {"[Up/Down: Toggle Review/Skip  |  Enter: Confirm]", "[上/下：切换复习/跳过  |  Enter：确认]"}},
    {"quest.prompt.roll", {"[Press Enter to roll!]", "[按 Enter 掷骰！]"}},
    {"quest.prompt.final", {"[Press Enter to confirm result]", "[按 Enter 确认结果]"}},
    {"quest.prompt.completed", {"[Quest completed -- press 2/3/4 to restart]", "[任务已完成——按 2/3/4 重新开始]"}},
    {"prompt.choice12", {"Press 1 or 2", "按 1 或 2"}},
    {"prompt.choice123", {"Press 1, 2, or 3", "按 1、2 或 3"}},
    {"time.current", {"Current time", "当前时间"}},
    {"time.passes", {"Time passes...", "时间流逝中……"}},
    {"time.class_passes", {"Class time passes...", "上课时间流逝中……"}},
    {"time.skip_class", {"Skipping class...", "逃课中……"}},
    {"time.sleeping", {"Sleeping...", "睡觉中……"}},
    {"ui.enter_to_enter", {"Press Enter to enter", "按 Enter 进入"}},
    {"ui.entering", {"Entering...", "进入中……"}},
    {"combat.you", {"You", "你"}},
    {"hint.interact", {"[Enter] ", "[Enter] "}},

    {"map.campus", {"Campus Map", "校园地图"}},
    {"map.dormitory", {"Dormitory", "宿舍"}},
    {"map.gym", {"Gym", "健身房"}},
    {"map.library", {"Library", "图书馆"}},
    {"map.classroom", {"Classroom", "教室"}},
    {"map.cafeteria", {"Cafeteria", "食堂"}},
    {"map.exit_campus", {"Exit to Campus", "返回校园"}},
    {"campus.move_hint", {"WASD Move  |  Enter: enter highlighted area  |  0/6 Help", "WASD 移动  |  Enter：进入高亮区域  |  0/6 帮助"}},
    {"gym.treadmill", {"Treadmill", "跑步机"}},
    {"gym.barbells", {"Barbells", "杠铃区"}}
};

std::string applyReplacements(std::string text,
                              const std::vector<std::pair<std::string, std::string>>& replacements) {
    for (const auto& replacement : replacements) {
        const std::string token = "{" + replacement.first + "}";
        std::size_t pos = 0;
        while ((pos = text.find(token, pos)) != std::string::npos) {
            text.replace(pos, token.size(), replacement.second);
            pos += replacement.second.size();
        }
    }
    return text;
}

} // namespace

void setLanguage(Language language) {
    gCurrentLanguage = language;
}

Language currentLanguage() {
    return gCurrentLanguage;
}

std::string text(const std::string& key) {
    const auto it = kTexts.find(key);
    if (it == kTexts.end()) return key;
    return gCurrentLanguage == Language::Chinese ? it->second.second : it->second.first;
}

std::string format(const std::string& key,
                   const std::vector<std::pair<std::string, std::string>>& replacements) {
    return applyReplacements(text(key), replacements);
}

std::string languageName(Language language) {
    return language == Language::Chinese ? text("language.chinese") : text("language.english");
}

} // namespace cls
