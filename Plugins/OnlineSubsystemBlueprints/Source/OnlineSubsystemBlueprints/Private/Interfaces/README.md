# Additional interfaces for Online Subsystems

These interfaces provide a standard way of accessing additional functionality for online subsystems in Unreal Engine.

## If you want to use the interfaces in your game or plugin

- Download the files in this repository and place them under the `Private` folder of your game or module's source code.
- You can also add this repository as a Git submodule if you prefer.

For example, you could place them in:

- `YourGame/Source/Private/RedpointInterfaces` so that e.g. `YourGame/Source/Private/RedpointInterfaces/OnlineLobbyInterface.h` exists.

Then to include the interface in your game source files, you would include it like so:

```
#include "RedpointInterfaces/OnlineLobbyInterface.h"
// Or include the other headers, depending on what you want to use...
```

Once you've included the headers, you can then get each of the interfaces at runtime by using the following functions:

- `Online::GetLobbyInterface(OSS)` where OSS is an `IOnlineSubsystem*`
- `Online::GetAvatarInterface(OSS)` where OSS is an `IOnlineSubsystem*`
- `Online::GetVoiceAdminInterface(OSS)` where OSS is an `IOnlineSubsystem*`

## If you want to implement the interfaces in your own online subsystem

If you are an implementor of online subsystems, and you want to support these interfaces, add the header files to the `Private` folder of your module and then follow the instructions in `OnlineLobbyInterface.h` for more details.

## List of available interfaces

The following interfaces are provided by this repository:

### OnlineAvatarInterface

This interface provides a standard way of accessing avatars of players in online subsystems.

### OnlineLobbyInterface

This interface provides a standard way of accessing lobby functionality of online subsystems.

Unreal Engine currently ships with two interfaces which map be mapped onto lobbies: sessions and parties. Unfortunately, sessions do not provide any kind of attribute system, and parties do not provide search functionality, so neither of them map cleanly onto lobbies in a way that the Matchmaking plugin can use. This interface covers that gap.

### OnlineVoiceAdminInterface

This interface provides a standard way of controlling voice channels on dedicated servers. It allows you to create credentials for voice channels as well as mute and kick participants.
