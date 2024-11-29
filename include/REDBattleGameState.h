#pragma once

#include "TutorialGameState.h"

namespace RED
{
    class REDGameState;

    class REDBattleGameState : public Demo::GameState
    {
        REDGameState* mRenderGameState {};

    public:
        REDBattleGameState();

        void LoadBattleAssets();
        void NotifyRenderGameState(REDGameState* renderGameState);
        void update(float timeSinceLast) override;
    };
}