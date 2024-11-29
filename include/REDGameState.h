
#ifndef _Demo_EmptyProjectGameState_H_
#define _Demo_EmptyProjectGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace RED
{
    class REDGameState : public Demo::TutorialGameState
    {
        Ogre::SceneNode *mSceneNode {};
        
        void generateDebugText( float timeSinceLast, Ogre::String &outText ) override;

    public:
        REDGameState( const Ogre::String &helpDescription );

        void createScene01() override;

        void update( float timeSinceLast ) override;

        void keyReleased( const SDL_KeyboardEvent &arg ) override;
    };
}  // namespace RED

#endif
