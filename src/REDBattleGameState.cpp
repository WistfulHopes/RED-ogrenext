#include "REDBattleGameState.h"

#include <OgreResourceManager.h>

#include "Game/REDGameCommon.h"

namespace RED
{
    REDBattleGameState::REDBattleGameState() = default;

    void REDBattleGameState::LoadBattleAssets()
    {
    }

    void REDBattleGameState::NotifyRenderGameState(REDGameState* renderGameState)
    {
        mRenderGameState = renderGameState;
    }

    void REDBattleGameState::update(float timeSinceLast)
    {
        REDGameCommon::GetInstance()->Tick(timeSinceLast);        
        GameState::update(timeSinceLast);
    }
}
