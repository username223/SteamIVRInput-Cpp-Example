This file is written in the present tense. The date of this document is `2019-01-18`. OpenVR version `1.1.3b`. SteamVR beta version `1.2.7`. The [current documentation](https://github.com/ValveSoftware/openvr/wiki/SteamVR-Input) is `yassermalaika edited this page on Nov 21, 2018 · 9 revisions`.

>- [Quick Start Guide](#quick-start-guide)
>- [General Pointers](#general-pointers)
>- [In Depth Guide](#in-depth-guide)
>  - [The Actions Manifest](#the-actions-manifest)
>  - [The Default Bindings Manifest](#the-default-bindings-manifest)
>  - [Initialization](#initialization)
>    - [VR_Init](#vr_init)
>    - [SetActionManifestPath](#setactionmanifestpath)
>    - [GetActionHandle](#getactionhandle)
>    - [GetActionSetHandle](#getactionsethandle)
>    - [VRActiveActionSet_t Struct Initialization](#vractiveactionset_t-struct-initialization)
>  - [Loop/Tick/Frame/Update](#looptickframeupdate)
>    - [UpdateActionState](#updateactionstate)
>  - [Getting Action State](#getting-action-state)
>    - [GetDigitalActionData](#getdigitalactiondata)
>    - [InputDigitalActionData_t Struct](#inputdigitalactiondata_t-struct)
>- [Further Reading](#further-reading)
>- [Issues/PRs](#issuesprs)

# Steam IVRInput Hello World in C++

The [current documentation](https://github.com/ValveSoftware/openvr/wiki/SteamVR-Input) for OpenVR's IVRInput is not very thorough, outdated and mostly intended for Unity. 
I wrote this partly to isolate the possible reasons why my implementation in [OpenVR Advanced Settings](https://github.com/OpenVR-Advanced-Settings/OpenVR-AdvancedSettings) didn't work, and partly because there are no code examples for a working implementation of even the first few functions you need. This repo was quickly put together because _some_ working code is better than no working code.

The source code should be seen as supplementary material. It might even make more sense for experienced devs to only look at the source code and skim the README.

This document is not intended to be an example of good C++, it's intended to quickly showcase which functions should be called when in order to get started. It is implemented as a class in order to be copy pastable into an existing project in order to test that nothing interferes with IVRInput. Having a known working copy to copy paste into my project would have stopped me from chasing shadows in the depths of the legacy code.

See the last section for links to references that helped me.

## Quick Start Guide

You'll need Visual Studio 2017 and a Vive controller. No other controllers are currently supported, because I only have a Vive controller.

* Build the x64 debug version. No other versions are supported (by me, they'll obviously still work with OpenVR).

* Copy `action_manifest.json` and `legacy_actions.json` into the executable directory `x64\Debug\`.

* Run the program. SteamVR should start and the console window for the program should open. Output should be blank unless there's an error, or until you turn on your Vive controller.

* After turning on your Vive controller, the console window will spam `Action Set Active!`.

* Pressing the menu button will print `Next song set!` and exit the application. SteamVR will still be open. 

* If you get those two prints when those actions are taken, the interface works on your machine.

## General Pointers

* Errors might be shown on the desktop or they might be shown in the headset. They might also not be shown at all.

* The `C:\Program Files (x86)\Steam\logs\vrserver.txt` contains helpful logs for SteamVR. Sometimes it'll display messages relevant to input.

## In Depth Guide

The [current documentation](https://github.com/ValveSoftware/openvr/wiki/SteamVR-Input) does not do a very good job of explaining why things are done, and what is basically a list of function calls to make could have been better represented with actual source code. Like we're about to do.

### The Actions Manifest

The actions manifest (`action_manifest.json` in the repo) contains the following:

* The default bindings.

Whenever a user starts your program without having set any bindings, this is what they use. The defaults are specific to various controllers. In our case we're using `vive_controller` and pointing at the `legacy_actions.json` file. The `legacy_actions.json` file was created by me by entering the SteamVR bindings interface. The [current documentation](https://github.com/ValveSoftware/openvr/wiki/SteamVR-Input) explains this process.

* Actions

An action represents, well, an action. It could be something like Open Inventory, Grab Gun or Play The Next Song. Your side of the API doesn't bother with _how_ the user activated an action, just that he did. Which exact button the user pressed isn't relevant anyway.

In our example we'll be using `/actions/main/in/playnexttrack` (the actions do not seem to be case sensitive).

* Action Sets

Action Sets contain a series of actions. I haven't messed with them so I can't give any specifics. In our example we'll just use `/actions/main`.

* Localization

The localization string must be `en_US` and NOT `en` as the official documentation says, otherwise the localization will not show up and you will not get any errors.

The actions manifest that we'll use can be found [here](action_manifest.json)

### The Default Bindings Manifest

The bindings manifest contains direct links between an action and an input on the controller. This can be generated by the SteamVR bindings interface. See the [current documentation](https://github.com/ValveSoftware/openvr/wiki/SteamVR-Input) for how to enable. 

The default bindings that we'll use can be found [here](legacy_actions.json).

### Initialization

Initialization is done in the Init function [here](https://github.com/username223/SteamIVRInput-Cpp-Example/blob/ecc3c56cefa89390f2a10e8d365c8cab3a144546/SteamVRBindings.cpp#L59). Initialization isn't being done in the constructor because the project I was trying to implement this in didn't support it.

#### VR_Init

```vr::VR_Init(&initError, vr::VRApplication_Overlay);```

This initializes SteamVR. If this isn't here, nothing else works.

#### SetActionManifestPath

```vr::VRInput()->SetActionManifestPath(m_actionManifestPath.c_str());```

This points at the action manifest we want to use. I couldn't get it to work without the action manifest being in the executable directory. There weren't error codes, it just didn't work. If your action manifest isn't valid JSON this will not error, but an error might be shown on the screen/in the headset.

#### GetActionHandle

```vr::VRInput()->GetActionHandle(k_actionPlayNextTrack, &m_nextSongHandler);```

Gets a handle to a specific action. If you attempt to get the handle of an action that isn't in the manifest there is generally no warning bells set off. ```GetDigitalActionData()``` _might_ give error 3, InvalidHandle, but I'm not sure. Make sure your strings are spelled correctly.

#### GetActionSetHandle

```vr::VRInput()->GetActionSetHandle(k_actionSetMain, &m_mainSetHandler);```

This gets a handle to the action set we want to monitor. 

#### VRActiveActionSet_t Struct Initialization

```
m_activeActionSet.ulActionSet = m_mainSetHandler;
m_activeActionSet.ulRestrictedToDevice = vr::k_ulInvalidInputValueHandle;
// When I didn't manually set priority zero it didn't work for me, for unknown reasons.
m_activeActionSet.nPriority = 0;
```

The struct should point to the set from GetActionSetHandle. I have no experience with anything other than `vr::k_ulInvalidInputValueHandle` in `ulRestrictedToDevice`.

Deliberately setting `nPriority = 0` made it work for me at one point.

### Loop/Tick/Frame/Update

Updates are done in the Loop function [here](https://github.com/username223/SteamIVRInput-Cpp-Example/blob/ecc3c56cefa89390f2a10e8d365c8cab3a144546/SteamVRBindings.cpp#L130).

#### UpdateActionState
```vr::VRInput()->UpdateActionState(&m_activeActionSet, sizeof(m_activeActionSet), 1);```

Updates the action sets. This should be called every frame/tick/whatever. After this, `GetDigitalActionData` can be called.

### Getting Action State

The action state for our one action is gotten [here](https://github.com/username223/SteamIVRInput-Cpp-Example/blob/ecc3c56cefa89390f2a10e8d365c8cab3a144546/SteamVRBindings.cpp#L106).

#### GetDigitalActionData

```
vr::VRInput()->GetDigitalActionData(
        m_nextSongHandler,
        &m_nextSongData,
        sizeof(m_nextSongData),
        vr::k_ulInvalidInputValueHandle);
```

This gets the action data of type digital (bool) into the `m_nextSongData` struct. I initially tried to call the `GetAnalogActionData` function, and forgot to change the sizeof to match the Digital struct when I changed the function call. Make sure you use the correct parameters. No errors will be shown if you mess this up.

#### InputDigitalActionData_t Struct

See the [current documentation](https://github.com/ValveSoftware/openvr/wiki/SteamVR-Input) for full documentation.

In my experience, `bActive` is enabled whenever your set is active and a controller is enabled. `bState` is enabled when the button is pressed down. I have not experimented with holding down a button and losing power, so I don't know what happens.

## Further Reading

https://github.com/ValveSoftware/openvr/wiki/SteamVR-Input

https://github.com/ValveSoftware/openvr/wiki/Action-manifest

https://github.com/ValveSoftware/openvr/wiki/API-Documentation

The `C:\Program Files (x86)\Steam\logs\vrserver.txt` file.

Output from the application run in debug mode. Seems to be mostly the same as `vrserver.txt`.

If you have inconsistent issues try rebooting your computer. The same binary/manifests can be completely broken one minute and then work perfectly after a reboot.

## Issues/PRs

If you have any questions just create an issue. Do not PM me. PMs are not indexed by search engines and will not help other people with the same problem. 

If you have any suggestions for changing the repo feel free to create a PR. Typo/wording/grammar PRs are welcome.
