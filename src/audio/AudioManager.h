#ifndef CLS_AUDIO_AUDIOMANAGER_H
#define CLS_AUDIO_AUDIOMANAGER_H

#include "core/GameSettings.h"
#include "core/TimeSystem.h"
#include "core/Types.h"
#include "map/MapPortal.h"
#include "utils/AssetPath.h"
#include <SFML/Audio.hpp>
#include <memory>
#include <string>

namespace cls {

class AudioManager {
public:
    AudioManager() = default;

    /** @brief 加载全部音频资产，返回 true 表示全部加载成功 */
    bool initialize() {
        const std::string bgmDir = "assets/audio/bgm/";
        const std::string sfxDir = "assets/audio/sfx/";

        if (!mBgmDaytime.openFromFile(resolveAssetPath(bgmDir + "daytime.wav"))) return false;
        mBgmDaytime.setLooping(true);

        if (!mBgmNight.openFromFile(resolveAssetPath(bgmDir + "night.wav"))) return false;
        mBgmNight.setLooping(true);

        if (!mBgmBattle.openFromFile(resolveAssetPath(bgmDir + "battle.wav"))) return false;
        mBgmBattle.setLooping(true);

        if (!mClassStartBuf.loadFromFile(resolveAssetPath(sfxDir + "class_start.wav")))
            return false;
        mClassStartSfx = std::make_unique<sf::Sound>(mClassStartBuf);
        mClassStartSfx->setLooping(false);

        if (!mClassEndBuf.loadFromFile(resolveAssetPath(sfxDir + "class_end.wav")))
            return false;
        mClassEndSfx = std::make_unique<sf::Sound>(mClassEndBuf);
        mClassEndSfx->setLooping(false);

        mInitialized = true;
        return true;
    }

    /**
     * @brief 每帧更新：BGM 切换、上下课铃检测、战斗音乐管理
     * @param timeSystem 时间系统引用
     * @param combatResult 战斗结果状态（用于检测战斗起止）
     * @param settings 运行时设置（读取音量值）
     */
    void update(const TimeSystem& timeSystem, const CombatResult& combatResult,
                const GameSettings& settings, CampusPlace currentPlace) {
        if (!mInitialized) return;

        applyVolume(settings);

        const int curMinute = timeSystem.getMinuteOfDay();
        const TimePhase curPhase = timeSystem.currentPhase();

        // ── 铃声播放中 ──────────────────────────────────────
        if (mSfxPlaying) {
            // 离开教室 → 立即停止铃声
            if (currentPlace != CampusPlace::Classroom) {
                mActiveSfx->stop();
                mActiveSfx = nullptr;
                mSfxPlaying = false;
                if (!mInBattle) playBgm(mCurrentBgm);
            } else if (mActiveSfx->getStatus() != sf::SoundSource::Status::Playing) {
                // 铃声自然结束
                mActiveSfx = nullptr;
                mSfxPlaying = false;
                if (!mInBattle) playBgm(mCurrentBgm);
            }
            mLastMinute = curMinute;
            mLastPhase = curPhase;
            return;
        }

        // ── 战斗音乐切换 ────────────────────────────────────
        if (!mInBattle && combatResult.active) {
            pauseCurrentBgm();
            mPrevBgm = mCurrentBgm;
            mBgmBattle.play();
            mCurrentBgm = BgmTrack::Battle;
            mInBattle = true;
        } else if (mInBattle && !combatResult.active) {
            mBgmBattle.stop();
            mInBattle = false;
            playBgm(mPrevBgm);
        }

        // ── 非战斗时按时间段切换 BGM ────────────────────────
        if (!mInBattle) {
            const BgmTrack desired = trackForPhase(curPhase);
            if (desired != mCurrentBgm)
                playBgm(desired);
        }

        // ── 上下课铃检测（仅在教室播放）─────────────────────
        if (mLastMinute >= 0 && currentPlace == CampusPlace::Classroom) {
            sf::Sound* bell = nullptr;
            if (crossedThreshold(mLastMinute, curMinute, TimeSystem::kClassMinute))
                bell = mClassStartSfx.get();
            else if (crossedThreshold(mLastMinute, curMinute, TimeSystem::kClassEndMinute))
                bell = mClassEndSfx.get();

            if (bell) {
                pauseCurrentBgm();
                bell->play();
                mActiveSfx = bell;
                mSfxPlaying = true;
            }
        }

        mLastMinute = curMinute;
        mLastPhase = curPhase;
    }

    /** @brief 将音量设置同步到所有音频源 */
    void applyVolume(const GameSettings& settings) {
        const float bgmVol = static_cast<float>(settings.bgmVolume);
        const float sfxVol = static_cast<float>(settings.sfxVolume);
        mBgmDaytime.setVolume(bgmVol);
        mBgmNight.setVolume(bgmVol);
        mBgmBattle.setVolume(bgmVol);
        if (mClassStartSfx) mClassStartSfx->setVolume(sfxVol);
        if (mClassEndSfx) mClassEndSfx->setVolume(sfxVol);
    }

private:
    enum class BgmTrack { Daytime, Night, Battle, None };

    /** @brief 时间段 → BGM 曲目映射 */
    static BgmTrack trackForPhase(TimePhase phase) {
        switch (phase) {
            case TimePhase::EarlyMorning:
            case TimePhase::Noon:
            case TimePhase::Afternoon:
                return BgmTrack::Daytime;
            case TimePhase::Evening:
            case TimePhase::Night:
                return BgmTrack::Night;
        }
        return BgmTrack::Daytime;
    }

    /** @brief 检测时间跨越阈值（prev < threshold && cur >= threshold） */
    static bool crossedThreshold(int prevMin, int curMin, int threshold) {
        return prevMin < threshold && curMin >= threshold;
    }

    void pauseCurrentBgm() {
        switch (mCurrentBgm) {
            case BgmTrack::Daytime: mBgmDaytime.pause(); break;
            case BgmTrack::Night:   mBgmNight.pause(); break;
            case BgmTrack::Battle:  mBgmBattle.pause(); break;
            default: break;
        }
    }

    void playBgm(BgmTrack track) {
        pauseCurrentBgm();
        mCurrentBgm = track;
        switch (track) {
            case BgmTrack::Daytime: mBgmDaytime.play(); break;
            case BgmTrack::Night:   mBgmNight.play(); break;
            case BgmTrack::Battle:  mBgmBattle.play(); break;
            default: break;
        }
    }

    // ── 音频资产 ──────────────────────────────────────────
    sf::Music mBgmDaytime;
    sf::Music mBgmNight;
    sf::Music mBgmBattle;
    sf::SoundBuffer mClassStartBuf;
    sf::SoundBuffer mClassEndBuf;
    std::unique_ptr<sf::Sound> mClassStartSfx;
    std::unique_ptr<sf::Sound> mClassEndSfx;

    // ── 状态 ──────────────────────────────────────────────
    BgmTrack mCurrentBgm = BgmTrack::None;
    BgmTrack mPrevBgm = BgmTrack::Daytime;
    TimePhase mLastPhase = TimePhase::EarlyMorning;
    int mLastMinute = -1;
    bool mInBattle = false;
    bool mSfxPlaying = false;
    sf::Sound* mActiveSfx = nullptr;
    bool mInitialized = false;
};

} // namespace cls

#endif // CLS_AUDIO_AUDIOMANAGER_H
