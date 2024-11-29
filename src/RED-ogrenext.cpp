
#include "REDGameState.h"
#include "GraphicsSystem.h"

#include "Compositor/OgreCompositorManager2.h"
#include "OgreCamera.h"
#include "OgreConfigFile.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreWindow.h"

// Declares WinMain / main
#include <WIN32/OgreTimerImp.h>

#include "LogicSystem.h"
#include "MainEntryPointHelper.h"
#include "REDBattleGameState.h"
#include "SdlInputHandler.h"
#include "System/MainEntryPoints.h"
#include "System/Desktop/UnitTesting.h"

extern const double cFrametime;
const double cFrametime = 1.0 / 60.0;

extern bool gFakeSlowmo;
bool gFakeSlowmo = false;

extern bool gFakeFrameskip;
bool gFakeFrameskip = false;

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
#    include <errno.h>
#    include <pwd.h>
#    include <sys/stat.h>
#    include <sys/types.h>
#    include <unistd.h>
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#    include "shlobj.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#    include <Foundation/Foundation.h>
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#    include "OSX/macUtils.h"
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#    include "iOS/macUtils.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char *argv[] )
#endif
{
    Demo::GameState *graphicsGameState = 0;
    Demo::GraphicsSystem *graphicsSystem = 0;
    Demo::GameState *logicGameState = 0;
    Demo::LogicSystem *logicSystem = 0;

    Demo::MainEntryPoints::createSystems( &graphicsGameState, &graphicsSystem, &logicGameState, &logicSystem );

    try
    {
        graphicsSystem->initialize( Demo::MainEntryPoints::getWindowTitle() );
        if( logicSystem )
            logicSystem->initialize();

        if( graphicsSystem->getQuit() )
        {
            if( logicSystem )
                logicSystem->deinitialize();
            graphicsSystem->deinitialize();

            Demo::MainEntryPoints::destroySystems( graphicsGameState, graphicsSystem, logicGameState,
                                             logicSystem );

            return 0;  // User cancelled config
        }
        
        Ogre::Window *renderWindow = graphicsSystem->getRenderWindow();

        graphicsSystem->createScene01();
        if( logicSystem )
            logicSystem->createScene01();

        graphicsSystem->createScene02();
        if( logicSystem )
            logicSystem->createScene02();

#if OGRE_USE_SDL2
        // Do this after creating the scene for easier the debugging (the mouse doesn't hide itself)
        Demo::SdlInputHandler *inputHandler = graphicsSystem->getInputHandler();
        inputHandler->setGrabMousePointer( true );
        inputHandler->setMouseVisible( false );
        inputHandler->setMouseRelative( true );
#endif

        Ogre::Timer timer;
        Ogre::uint64 startTime = timer.getMicroseconds();
        double accumulator = Demo::MainEntryPoints::Frametime;

        double timeSinceLast = 1.0 / 60.0;

        while( !graphicsSystem->getQuit() )
        {
            while( accumulator >= cFrametime )
            {
                graphicsSystem->beginFrameParallel();

                logicSystem->beginFrameParallel();
                logicSystem->update( static_cast<float>( cFrametime ) );
                logicSystem->finishFrameParallel();

                graphicsSystem->finishFrameParallel();

                logicSystem->finishFrame();
                graphicsSystem->finishFrame();

                accumulator -= cFrametime;

                if( gFakeSlowmo )
                    Ogre::Threads::Sleep( 40 );
            }

            graphicsSystem->beginFrameParallel();
            graphicsSystem->update( static_cast<float>( timeSinceLast ) );
            graphicsSystem->finishFrameParallel();
            if( !logicSystem )
                graphicsSystem->finishFrame();
            
            if( !renderWindow->isVisible() )
            {
                // Don't burn CPU cycles unnecessary when we're minimized.
                Ogre::Threads::Sleep( 500 );
            }

            if( gFakeFrameskip )
                Ogre::Threads::Sleep( 40 );

            Ogre::uint64 endTime = timer.getMicroseconds();
            timeSinceLast = double( endTime - startTime ) / 1000000.0;
            timeSinceLast = std::min( 1.0, timeSinceLast );  // Prevent from going haywire.
            accumulator += timeSinceLast;
            startTime = endTime;
        }

        graphicsSystem->destroyScene();
        if( logicSystem )
        {
            logicSystem->destroyScene();
            logicSystem->deinitialize();
        }
        graphicsSystem->deinitialize();

        Demo::MainEntryPoints::destroySystems( graphicsGameState, graphicsSystem, logicGameState,
                                         logicSystem );
    }
    catch( Ogre::Exception &e )
    {
        Demo::MainEntryPoints::destroySystems( graphicsGameState, graphicsSystem, logicGameState,
                                         logicSystem );
        throw e;
    }
    catch( ... )
    {
        Demo::MainEntryPoints::destroySystems( graphicsGameState, graphicsSystem, logicGameState,
                                         logicSystem );
    }

    return 0;}

namespace Demo
{    
    class REDGraphicsSystem final : public GraphicsSystem
    {
        Ogre::CompositorWorkspace *setupCompositor() override
        {
            return GraphicsSystem::setupCompositor();
        }

        void setupResources() override
        {
            GraphicsSystem::setupResources();

            Ogre::ConfigFile cf;
            cf.load( mResourcePath + "resources2.cfg" );

            Ogre::String origDataFolder = cf.getSetting( "DoNotUseAsResource", "Hlms", "" );

            if( origDataFolder.empty() )
                origDataFolder = "./";
            else if( *( origDataFolder.end() - 1 ) != '/' )
                origDataFolder += "/";

            auto dataFolder = origDataFolder + "2.0/scripts/materials/PbsMaterials";
            addResourceLocation( dataFolder, "FileSystem", "General" );
            
            auto assetsFolder = origDataFolder + "assets";
            addResourceLocation( assetsFolder, "FileSystem", "General" );
        }

    public:
        REDGraphicsSystem( GameState *gameState ) : GraphicsSystem( gameState )
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
            mResourcePath = "Data/";
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
            mResourcePath = Ogre::macBundlePath() + "/Contents/Resources/Data/";
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            mResourcePath = Ogre::macBundlePath() + "/Data/";
#else
            mResourcePath = "../Data/";
#endif

            // It's recommended that you set this path to:
            //	%APPDATA%/RED-ogrenext/ on Windows
            //	~/.config/RED-ogrenext/ on Linux
            //	macCachePath() + "/RED-ogrenext/" (NSCachesDirectory) on Apple -> Important because
            //	on iOS your app could be rejected from App Store when they see iCloud
            //	trying to backup your Ogre.log & ogre.cfg auto-generated without user
            //	intervention. Also convenient because these settings will be deleted
            //	if the user removes cached data from the app, so the settings will be
            //	reset.
            //  Obviously you can replace "RED-ogrenext" by your app's name.
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            mWriteAccessFolder = +"/";
            TCHAR path[MAX_PATH];
            if( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path ) !=
                           S_OK ) )
            {
                // Need to convert to OEM codepage so that fstream can
                // use it properly on international systems.
#    if defined( _UNICODE ) || defined( UNICODE )
                int size_needed =
                    WideCharToMultiByte( CP_OEMCP, 0, path, (int)wcslen( path ), NULL, 0, NULL, NULL );
                mWriteAccessFolder = std::string( size_needed, 0 );
                WideCharToMultiByte( CP_OEMCP, 0, path, (int)wcslen( path ), &mWriteAccessFolder[0],
                                     size_needed, NULL, NULL );
#    else
                TCHAR oemPath[MAX_PATH];
                CharToOem( path, oemPath );
                mWriteAccessFolder = std::string( oemPath );
#    endif
                mWriteAccessFolder += "/RED-ogrenext/";

                // Attempt to create directory where config files go
                if( !CreateDirectoryA( mWriteAccessFolder.c_str(), NULL ) &&
                    GetLastError() != ERROR_ALREADY_EXISTS )
                {
                    // Couldn't create directory (no write access?),
                    // fall back to current working dir
                    mWriteAccessFolder = "";
                }
            }
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
            const char *homeDir = getenv( "HOME" );
            if( homeDir == 0 )
                homeDir = getpwuid( getuid() )->pw_dir;
            mWriteAccessFolder = homeDir;
            mWriteAccessFolder += "/.config";
            int result = mkdir( mWriteAccessFolder.c_str(), S_IRWXU | S_IRWXG );
            int errorReason = errno;

            // Create "~/.config"
            if( result && errorReason != EEXIST )
            {
                printf( "Error. Failing to create path '%s'. Do you have access rights?",
                        mWriteAccessFolder.c_str() );
                mWriteAccessFolder = "";
            }
            else
            {
                // Create "~/.config/RED-ogrenext"
                mWriteAccessFolder += "/RED-ogrenext/";
                result = mkdir( mWriteAccessFolder.c_str(), S_IRWXU | S_IRWXG );
                errorReason = errno;

                if( result && errorReason != EEXIST )
                {
                    printf( "Error. Failing to create path '%s'. Do you have access rights?",
                            mWriteAccessFolder.c_str() );
                    mWriteAccessFolder = "";
                }
            }
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            NSURL *libUrl = [NSFileManager.defaultManager URLForDirectory:NSLibraryDirectory
                                                                 inDomain:NSUserDomainMask
                                                        appropriateForURL:nil
                                                                   create:YES
                                                                    error:nil];
            // Create "pathToCache/RED-ogrenext"
            mWriteAccessFolder = Ogre::String( libUrl.absoluteURL.path.UTF8String ) + "/RED-ogrenext/";
            const int result = mkdir( mWriteAccessFolder.c_str(), S_IRWXU | S_IRWXG );
            const int errorReason = errno;

            if( result && errorReason != EEXIST )
            {
                printf( "Error. Failing to create path '%s'. Do you have access rights?",
                        mWriteAccessFolder.c_str() );
                mWriteAccessFolder = "";
            }
#endif
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState, LogicSystem **outLogicSystem )
    {
        using namespace RED;
        
        REDGameState *gfxGameState = new REDGameState( "RED-ogrenext" );

        GraphicsSystem *graphicsSystem = new REDGraphicsSystem( gfxGameState );

        gfxGameState->_notifyGraphicsSystem( graphicsSystem );

        REDBattleGameState *logicGameState = new REDBattleGameState();

        LogicSystem* logicSystem = new LogicSystem(logicGameState);
        
        graphicsSystem->_notifyLogicSystem( logicSystem );

        logicGameState->NotifyRenderGameState(gfxGameState);
        
        *outGraphicsGameState = gfxGameState;
        *outGraphicsSystem = graphicsSystem;
        *outLogicGameState = logicGameState;
        *outLogicSystem = logicSystem;
    }

    void MainEntryPoints::destroySystems( GameState *graphicsGameState, GraphicsSystem *graphicsSystem,
                                          GameState *logicGameState, LogicSystem *logicSystem )
    {
        delete graphicsSystem;
        delete graphicsGameState;
        delete logicGameState;
        delete logicSystem;
    }

    const char *MainEntryPoints::getWindowTitle() { return "RED"; }
}  // namespace RED
