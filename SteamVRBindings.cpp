#include "pch.h"
#include <openvr.h>
#include <iostream>

/*
C++ Hello World example for the SteamVR IVRInput system. I had a lot of problems setting this up because 
SteamVR apparently caches some things, but not all of them, and it isn't always apparent what has been cached
and what hasn't been cached. 

Sometimes I've had weird errors that were fixed by restarting SteamVR, other times they required 
restarting the computer. Sometimes I changed literally nothing and it magically worked.

This example will only work with a Vive controller. I haven't implemented default bindings for other controllers
because I don't have any others.

In order to run, copy "action_manifest.json" and "legacy_actions.json" into the executable directory and run the program.

When SteamVR has started, turn on your Vive controller. The console should be spammed with "Action Set Active!". Press the menu
button once. The console will print "Next song set!" and exit. SteamVR will still be running.
*/



class SteamIVRInput {
public:
    void Init(const bool init);
    void Loop();
    bool nextSongSet();

private:
    //Using an action manifest from a path that isn't the executable path doesn't work for whatever reason.
    const std::string m_actionManifestPath = PROJECTDIR "\\x64\\Debug\\action_manifest.json";
    vr::VRActionHandle_t m_nextSongHandler = {};
    vr::VRActionSetHandle_t m_mainSetHandler = {};
    vr::VRActiveActionSet_t m_activeActionSet = {};
    vr::InputDigitalActionData_t m_nextSongData = {};
};

// These two are set in action_manifest.json. They must match or you'll get errors/things that don't work with no errors.
constexpr auto k_actionSetMain = "/actions/main";
constexpr auto k_actionPlayNextTrack = "/actions/main/in/PlayNextTrack";

void SteamIVRInput::Init(const bool initializeSteamVR)
{

    // If something already initialized SteamVR we don't want to do it again.
    // Intended for being able to copy paste into existing codebase to make 
    // sure everything works with a known working example, hence why this is toggleable.
    // Should always be run with true if third party doesn't init SteamVR.
    if (initializeSteamVR)
    {
        auto initError = vr::VRInitError_None;
        vr::VR_Init(&initError, vr::VRApplication_Scene);
        if (initError != vr::VRInitError_None)
        {
            std::cerr << "SteamVR init error.";
        }
    }

    // Set the action manifest. This should be in the executable directory.
    // Defined by m_actionManifestPath.
    auto error = vr::VRInput()->SetActionManifestPath(m_actionManifestPath.c_str());
    if (error != vr::EVRInputError::VRInputError_None)
    {
        std::cerr << "Action manifest error\n";
    }

    // Get action handle
    error = vr::VRInput()->GetActionHandle(k_actionPlayNextTrack,
                                           &m_nextSongHandler);
    if (error != vr::EVRInputError::VRInputError_None)
    {
        std::cerr << "Handle error.\n";
    }

    // Get set handle
    error = vr::VRInput()->GetActionSetHandle(k_actionSetMain,
                                              &m_mainSetHandler);
    if (error != vr::EVRInputError::VRInputError_None)
    {
        std::cerr << "Handle error.\n";
    }

    m_activeActionSet.ulActionSet = m_mainSetHandler;
    m_activeActionSet.ulRestrictedToDevice = vr::k_ulInvalidInputValueHandle;
    // When I didn't manually set priority zero it didn't work for me, for unknown reasons.
    m_activeActionSet.nPriority = 0;
}

bool SteamIVRInput::nextSongSet()
{
    auto e = vr::VRInput()->GetDigitalActionData(
        m_nextSongHandler,
        &m_nextSongData,
        sizeof(m_nextSongData),
        vr::k_ulInvalidInputValueHandle);

    if (e != vr::EVRInputError::VRInputError_None)
    {
        // Print the rror code.
        std::cerr << e << '\n';
        std::cerr << "GetDigitalAction error.\n";
    }

    // Will basically just spam the console. To make sure it's visible even from a distance.
    if (m_nextSongData.bActive)
    {
        std::cout << "Action Set Active!\n";
    }

    return m_nextSongData.bState;
}

void SteamIVRInput::Loop()
{
    // Getting the correct sizeof is critical.
    // Make sure to match digital/analog with the function you're calling.
    auto error = vr::VRInput()->UpdateActionState(
        &m_activeActionSet, sizeof(m_activeActionSet), 1);

    if (error != vr::EVRInputError::VRInputError_None)
    {
        std::cerr << "Loop error.\n";
    }
}

int main()
{


    SteamIVRInput a;

    a.Init(true);

    a.Loop();

    while (!a.nextSongSet())
    {
        a.Loop();
        if (a.nextSongSet())
        {
            std::cout << "Next song set!\n";
        }
    }

    vr::VR_Shutdown();
}
