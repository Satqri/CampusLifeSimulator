#include "core/Localization.h"

#include "core/AssetPath.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

namespace cls {
namespace {

Language gCurrentLanguage = Language::English;
std::unordered_map<std::string, std::string> gEnglishTexts;
std::unordered_map<std::string, std::string> gChineseTexts;

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

    {"hud.energy", {"EN", "体力"}},
    {"hud.health", {"HP", "健康"}},
    {"hud.gold", {"GLD", "金钱"}},
    {"hud.san", {"SAN", "心态"}},
    {"hud.academic", {"KNW", "知识"}},
    {"hud.social", {"SOC", "社交"}},
    {"hud.san_level", {"SAN Level", "SAN 等级"}},
    {"hud.buff", {"Buff", "增益"}},
    {"hud.win", {"[Win]", "[胜利]"}},
    {"hud.none", {"[-]", "[-]"}},

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
    {"help.stress", {"C: SAN shock, lower SAN, may spawn emotion enemies", "C：触发 SAN 冲击，降低 SAN，并可能生成情绪敌人"}},
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
    {"quest.earned_titles", {"Earned Titles", "获得称号"}},
    {"quest.semester_summary", {"Semester Summary", "学期总结"}},
    {"quest.return_title", {"Press Enter to return to title", "按 Enter 返回标题"}},
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
    {"prompt.choice1234", {"Press 1, 2, 3, or 4", "按 1、2、3 或 4"}},
    {"time.current", {"Current time", "当前时间"}},
    {"time.passes", {"Time passes...", "时间流逝中……"}},
    {"time.class_passes", {"Class time passes...", "上课时间流逝中……"}},
    {"time.skip_class", {"Skipping class...", "逃课中……"}},
    {"time.sleeping", {"Sleeping...", "睡觉中……"}},
    {"status.new_game", {"New game started", "新游戏开始"}},
    {"status.save_failed", {"Save failed", "保存失败"}},
    {"status.saved", {"Game saved", "游戏已保存"}},
    {"status.no_save", {"No save file found", "未找到存档"}},
    {"status.loaded", {"Game loaded", "已读取存档"}},
    {"status.quest_completed", {"Quest completed", "任务完成"}},
    {"status.stress", {"Stress spike lowered SAN", "压力上升，SAN 降低"}},
    {"status.rest", {"Short rest restored SAN", "短暂休息恢复了 SAN"}},
    {"status.buff_ready", {"Prepared a +2 combat buff", "已准备 +2 战斗加值"}},
    {"status.combat_intro", {"Combat encounter ready", "战斗遭遇已准备"}},
    {"time.day_phase", {"{dayLabel} | {status}", "{dayLabel}｜{status}"}},
    {"ui.enter_to_enter", {"Press Enter to enter", "按 Enter 进入"}},
    {"ui.entering", {"Entering...", "进入中……"}},
    {"combat.you", {"You", "你"}},
    {"hint.interact", {"[Enter] ", "[Enter] "}},

    {"map.campus", {"Campus Map", "校园地图"}},
    {"map.dormitory", {"Dormitory", "宿舍"}},
    {"map.dorm.short", {"Dorm", "宿舍"}},
    {"map.gym", {"Gym", "健身房"}},
    {"map.library", {"Library", "图书馆"}},
    {"map.classroom", {"Classroom", "教室"}},
    {"map.cafeteria", {"Cafeteria", "食堂"}},
    {"map.store", {"Convenience Store", "便利店"}},
    {"map.exit_campus", {"Exit to Campus", "返回校园"}},
    {"campus.move_hint", {"WASD Move  |  Enter: enter highlighted area  |  0/6 Help", "WASD 移动  |  Enter：进入高亮区域  |  0/6 帮助"}},
    {"scene.dormitory.subtitle", {"Backpacks drop by the bed; the next plan starts from a quiet room.", "背包落在床边，下一步计划从安静的房间开始。"}},
    {"scene.gym.subtitle", {"Rubber mats, bright lights, and steady breathing make room for training.", "橡胶地垫、明亮灯光与平稳呼吸，为训练留出空间。"}},
    {"scene.gym.exit", {"Fresh air returns after the echo of treadmills and weights.", "跑步机与器械的回响淡去，清新的空气再次迎面而来。"}},
    {"scene.library.subtitle", {"Between shelves and desk lamps, tomorrow's answers begin to take shape.", "书架与台灯之间，明天的答案开始逐渐成形。"}},
    {"scene.classroom.subtitle", {"The bell rings softly; notes, questions, and pressure wait inside.", "铃声轻轻响起，笔记、问题与压力都在里面等待着你。"}},
    {"scene.cafeteria.subtitle", {"Warm food and noisy tables make the campus feel briefly lighter.", "热腾腾的饭菜和嘈杂的餐桌，让校园短暂轻松起来。"}},
    {"scene.store.subtitle", {"Instant noodles steam under bright lights; convenience and exhaustion meet here.", "白灯下泡面冒着热气，便利与疲惫在这里碰面。"}},
    {"notice.no_enemy.title", {"No Enemy Nearby", "附近没有敌人"}},
    {"notice.no_enemy.body", {"Get closer to an enemy or lower SAN to spawn one first.", "先靠近敌人，或降低 SAN 以触发敌人出现。"}},
    {"notice.shelf_browsed", {"Shelf Browsed", "已浏览书架"}},
    {"notice.meal_closed", {"Meal Time Closed", "当前不是供餐时间"}},
    {"notice.meal_holding", {"Already Holding Food", "你已经拿着食物了"}},
    {"notice.already_served", {"Already Served", "本时段已取餐"}},
    {"notice.choose_meal", {"Choose Meal", "选择餐食"}},
    {"notice.no_food", {"No Food", "还没有食物"}},
    {"notice.meal_complete", {"Meal Complete", "用餐完成"}},
    {"notice.training_complete", {"Training Complete", "训练完成"}},
    {"notice.study_complete", {"Study Complete", "学习完成"}},
    {"notice.game_break_complete", {"Game Break Complete", "娱乐休息完成"}},
    {"notice.overplayed", {"Overplayed", "玩得太久了"}},
    {"notice.quiet_moment", {"Quiet Moment", "安静时刻"}},
    {"notice.board_reviewed", {"Board Reviewed", "已查看黑板"}},
    {"notice.desk", {"Desk", "课桌"}},
    {"notice.event_result", {"Event Result", "事件结果"}},
    {"notice.midterm_morning", {"Midterm Morning", "期中考试早晨"}},
    {"notice.morning_class", {"Morning Class", "早课"}},
    {"notice.midterm_complete", {"Midterm Complete", "期中考试结束"}},
    {"notice.class_complete", {"Class Complete", "课程结束"}},
    {"notice.roll_call", {"Roll Call Notice", "点名通知"}},
    {"notice.not_enough_gold", {"Not Enough Gold", "金币不足"}},
    {"notice.food_taken", {"Food Taken", "已取餐"}},
    {"notice.too_early", {"Too Early", "现在还太早"}},
    {"notice.days_complete", {"Fourteen Days Complete", "十四天已结束"}},
    {"notice.new_day", {"New Day", "新的一天"}},
    {"notice.reading_complete", {"Reading Complete", "阅读完成"}},
    {"notice.exit_confirm_title", {"Exit Game", "退出游戏"}},
    {"notice.exit_confirm_body", {"Do you want to exit the game?\n1) Continue\n2) Exit", "要退出游戏吗？\n1）继续游戏\n2）退出程序"}},
    {"notice.exit_continue", {"Continue", "继续游戏"}},
    {"notice.exit_now", {"Exit", "退出程序"}},
    {"notice.standard_route_title", {"Standard Route", "标准通关路线"}},
    {"notice.standard_route_body", {"Explore campus, finish daily activities, trigger quests by progression events, survive SAN combat pressure, clear the quest chain, and reach the end of Day 14.", "在校园中探索，完成日常活动，通过推进事件触发主线任务，顶住 SAN 战斗压力，完成任务链，并坚持到第 14 天结束。"}},
    {"notice.minigame_title", {"Dormitory Mini Game", "宿舍小游戏"}},
    {"notice.minigame_prompt", {"Press Enter / Space when the cursor is inside the green zone.", "当指针进入绿色区域时按 Enter / Space。"}},
    {"notice.minigame_time", {"Time Left: {count}", "剩余时间：{count}"}},
    {"notice.minigame_perfect", {"Perfect Relaxation", "完美放松"}},
    {"notice.minigame_good", {"Good Break", "不错的休息"}},
    {"notice.minigame_messy", {"Messy Break", "手忙脚乱的休息"}},
    {"notice.minigame_timeout", {"Time Over", "时间结束"}},
    {"notice.minigame_perfect_body", {"Great timing. SAN +9, Energy +6.", "时机完美。SAN +9，精力 +6。"}},
    {"notice.minigame_good_body", {"Solid timing. SAN +6, Energy +4.", "节奏不错。SAN +6，精力 +4。"}},
    {"notice.minigame_messy_body", {"You still relaxed a little. SAN +2, Energy +1.", "虽然失误了，但还是放松了一点。SAN +2，精力 +1。"}},
    {"notice.minigame_timeout_body", {"You played too long without focus. SAN +1, Energy -2.", "玩了太久却没投入状态。SAN +1，精力 -2。"}},
    {"notice.campus_square", {"Campus Square", "校园广场"}},
    {"notice.campus_square.subtitle", {"The main paths open again; choose where the day goes next.", "主路再次展开，选择接下来一天要去哪里。"}},
    {"notice.study_complete", {"Study Complete", "学习完成"}},
    {"combat.encounter", {"Combat Encounter", "战斗遭遇"}},
    {"combat.action", {"Action", "行动"}},
    {"combat.enemy_dc", {"Enemy DC", "敌人 DC"}},
    {"combat.roll_buff", {"Your current roll buff", "当前检定加值"}},
    {"combat.press_enter", {"Press Enter to roll.", "按 Enter 掷骰。"}},
    {"combat.vs", {"vs", "对抗"}},
    {"combat.victory", {"VICTORY!", "胜利！"}},
    {"combat.defeat", {"DEFEAT!", "失败！"}},
    {"combat.result.victory", {"SAN restored. Positive buff prepared.", "SAN 已恢复，并准备了正面增益。"}},
    {"combat.result.defeat", {"SAN -15. Debuff applied.", "SAN -15，并施加了减益。"}},
    {"ui.press_enter_continue", {"Press Enter to continue", "按 Enter 继续"}},
    {"ui.press_number_range", {"Press 1-{count}", "按 1-{count}"}},
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

const std::string* findExternalText(Language language, const std::string& key) {
    const auto& table = language == Language::Chinese ? gChineseTexts : gEnglishTexts;
    const auto it = table.find(key);
    if (it == table.end() || it->second.empty()) return nullptr;
    return &it->second;
}

const std::string* findBuiltinText(Language language, const std::string& key) {
    const auto it = kTexts.find(key);
    if (it == kTexts.end()) return nullptr;
    return language == Language::Chinese ? &it->second.second : &it->second.first;
}

} // namespace

void setLanguage(Language language) {
    gCurrentLanguage = language;
}

bool loadLocaleFile(Language language, const std::string& relativePath) {
    const std::string resolved = resolveAssetPath(relativePath);
    std::ifstream file(resolved);
    if (!file.is_open()) {
        std::cerr << "[Locale] Cannot open: " << resolved << std::endl;
        return false;
    }

    nlohmann::json data;
    try {
        file >> data;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[Locale] JSON parse error in " << resolved << ": " << e.what() << std::endl;
        return false;
    }

    if (!data.is_object()) {
        std::cerr << "[Locale] Expected object: " << resolved << std::endl;
        return false;
    }

    auto& table = language == Language::Chinese ? gChineseTexts : gEnglishTexts;
    for (auto it = data.begin(); it != data.end(); ++it) {
        if (it.value().is_string()) {
            table[it.key()] = it.value().get<std::string>();
        }
    }
    std::cout << "[Locale] Loaded " << table.size() << " entries from " << resolved << std::endl;
    return true;
}

void loadDefaultLocales() {
    loadLocaleFile(Language::English, "assets/locales/en.json");
    loadLocaleFile(Language::Chinese, "assets/locales/zh.json");
}

Language currentLanguage() {
    return gCurrentLanguage;
}

std::string text(const std::string& key) {
    if (const auto* value = findExternalText(gCurrentLanguage, key)) return *value;
    if (const auto* value = findBuiltinText(gCurrentLanguage, key)) return *value;

    const Language fallbackLanguage = gCurrentLanguage == Language::Chinese
        ? Language::English
        : Language::Chinese;
    if (const auto* value = findExternalText(fallbackLanguage, key)) return *value;
    if (const auto* value = findBuiltinText(fallbackLanguage, key)) return *value;

    return key;
}

std::string format(const std::string& key,
                   const std::vector<std::pair<std::string, std::string>>& replacements) {
    return applyReplacements(text(key), replacements);
}

std::string languageName(Language language) {
    return language == Language::Chinese ? text("language.chinese") : text("language.english");
}

} // namespace cls
