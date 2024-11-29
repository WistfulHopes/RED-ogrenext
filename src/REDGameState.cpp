
#include "REDGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreItem.h"
#include "OgreSceneManager.h"

#include "OgreMesh2.h"
#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"

#include "OgreCamera.h"
#include "OgreWindow.h"

#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsSamplerblock.h"

#include "OgreHlmsManager.h"
#include "OgreHlmsPbs.h"
#include "OgreRoot.h"

using namespace RED;

extern const double cFrametime;
extern bool gFakeSlowmo;
extern bool gFakeFrameskip;

namespace RED
{
    REDGameState::REDGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription )
    {
    }
    //-----------------------------------------------------------------------------------
    void REDGameState::createScene01()
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::Item *item = sceneManager->createItem(
            "Cube_d.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
            Ogre::SCENE_DYNAMIC );

        mSceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )
                         ->createChildSceneNode( Ogre::SCENE_DYNAMIC );

        mSceneNode->attachObject( item );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void REDGameState::update( float timeSinceLast )
    {
        float weight = mGraphicsSystem->getAccumTimeSinceLastLogicFrame() / (float)cFrametime;
        weight = std::min( 1.0f, weight );
        
        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void REDGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2 to fake a GPU bottleneck (frame skip). ";
        outText += gFakeFrameskip ? "[On]" : "[Off]";
        outText += "\nPress F3 to fake a CPU Logic bottleneck. ";
        outText += gFakeSlowmo ? "[On]" : "[Off]";
    }
    //-----------------------------------------------------------------------------------
    void REDGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( ( arg.keysym.mod & ~( KMOD_NUM | KMOD_CAPS ) ) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.sym == SDLK_F2 )
        {
            gFakeFrameskip = !gFakeFrameskip;
        }
        else if( arg.keysym.sym == SDLK_F3 )
        {
            gFakeSlowmo = !gFakeSlowmo;
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}  // namespace RED
