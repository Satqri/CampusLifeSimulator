#ifndef CLS_STATE_DICEGAMESTATE_H
#define CLS_STATE_DICEGAMESTATE_H

#include "state/GameState.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <random>

class Player;
class Student;

/**
 * @class DiceGameState
 * @brief 楠板瓙姣旂偣鏁版父鎴忕姸鎬?
 *
 * 鐜╁涓庡鑸嶅鐢熻繘琛屾瘮鐐规暟娓告垙锛?
 * - 涓嬪崐灞忓箷鏄帺瀹剁晫闈紝涓婂崐灞忓箷鏄鐢熺晫闈?
 * - 鍙屾柟鍚勭敓鎴?3 涓?1-6 鐨勯殢鏈烘暟锛屾寜瑙勫垯璁＄畻鎬荤偣鏁?
 * - 鎸?L 鍋滄涓€涓瀛愶紝鎸変笁娆?L 閿佸畾鍏ㄩ儴楠板瓙
 * - 鎸?G 鏌ョ湅瑙勫垯/鍏紡
 * - 宸€煎奖鍝嶇帺瀹剁殑 social 灞炴€?
 */
class DiceGameState : public GameState {
public:
    DiceGameState(Game* game, Player* player, Student* student);

    void handleInput(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    /** @brief 娓告垙鏄惁缁撴潫 */
    bool isFinished() const { return finished; }

    /** @brief 鑾峰彇绀句氦鍊煎閲?*/
    int getSocialDelta() const { return socialDelta; }

    /** @brief 鏄惁姝ｅ湪杩愯 */
    bool isRunning() const { return running; }

    /** @brief 璁剧疆瀛椾綋 */
    void setFont(const sf::Font* f) { font = f; }

private:
    /// 楠板瓙鏁版嵁
    struct DiceData {
        int value = 1;
        bool locked = false;
    };

    /// 娓告垙鏂?
    enum class Side {
        PLAYER,
        STUDENT
    };

    // 鈹€鈹€ 鐐规暟璁＄畻 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
    static int calculateTotalScore(const std::array<DiceData, 3>& dice);

    // 鈹€鈹€ 娓叉煋杈呭姪 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
    void renderDiceRow(sf::RenderWindow& window,
                       const std::array<DiceData, 3>& dice,
                       float startY,
                       int totalScore,
                       bool showScore,
                       const std::string& label);
    void renderDiceFace(sf::RenderWindow& window, int value,
                        float cx, float cy, float size, bool locked);
    void renderRulesPanel(sf::RenderWindow& window);
    void renderResultOverlay(sf::RenderWindow& window);

    // 鈹€鈹€ 鎴愬憳鍙橀噺 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
    Player* player;
    Student* student;
    const sf::Font* font;

    std::array<DiceData, 3> playerDice;
    std::array<DiceData, 3> studentDice;
    int playerTotalScore;
    int studentTotalScore;

    int playerLockCount;    // 鐜╁宸查攣瀹氶瀛愭暟 (0-3)
    int studentLockCount;   // 瀛︾敓宸查攣瀹氶瀛愭暟 (0-3)

    bool playerAllLocked;   // 鐜╁涓変釜楠板瓙鍏ㄩ儴閿佸畾
    bool studentAllLocked;  // 瀛︾敓涓変釜楠板瓙鍏ㄩ儴閿佸畾

    bool gameStarted;       // 鏄惁宸插紑濮嬶紙鎸夎繃L锛?
    bool finished;          // 娓告垙鏄惁缁撴潫
    bool running;           // 娓告垙鏄惁澶勪簬娲昏穬鐘舵€?

    bool showRules;         // 鏄惁鏄剧ず瑙勫垯闈㈡澘
    bool showFormula;       // 鏄惁鏄剧ず鍏紡闈㈡澘

    int socialDelta;        // 绀句氦鍊煎彉鍖栭噺

    float resultTimer;      // 缁撴灉鏄剧ず璁℃椂鍣?

    std::mt19937 rng;       // 闅忔満鏁扮敓鎴愬櫒
    std::uniform_int_distribution<int> dist;

    // 鈹€鈹€ 鍔ㄧ敾鐢?鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
    float animTimer;
    int displayValues[3];   // 褰撳墠鍔ㄧ敾鏄剧ず鐨勯瀛愬€?
};

#endif // CLS_STATE_DICEGAMESTATE_H
