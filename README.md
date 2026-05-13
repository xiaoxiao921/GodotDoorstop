<h1 align="center">Godot Doorstop</h1>

***

Godot Doorstop is a tool to execute managed .NET assemblies inside Godot as early as possible.

## Features

* **Runs first**: Godot Doorstop runs its code before Godot can do so
* **Configurable**: An elementary configuration file allows you to specify your assembly to execute
* **Multiplatform**: Supports Windows, Linux, macOS
* **Debugger support**: Allows to debug managed assemblies in Visual Studio, Rider or dnSpy

## Godot runtime support

Godot Doorstop supports executing .NET assemblies in Godot.
Godot Doorstop tries to run your assembly as follows:

* Your assembly is executed in the same runtime. As a result
  * You don't need to include your custom Common Language Runtime (CLR); the one bundled with the game is used
  * Your assembly is run alongside other Godot code
  * You can access all Godot API directly

## Building

Godot Doorstop uses [xmake](https://xmake.io/) to build the project. To build, run `build.bat`, `build.ps1` or `build.sh`.

Available build options:

* `-with_logging`: build with logging enabled
* `-arch`: the architectures to build for, separated by commas (e.g. `-arch x86,x64`)
* `-debug`: build in debug mode (currently only for *nix)

> **Note:** Initial build times are usually slower because the build script automatically downloads and installs xmake.  
> On Unix, xmake is built directly from the source code.

## Minimal injection example

To have Godot Doorstop inject your code, create `Main` class into `GodotPlugins.Game` namespace.
Define a private static `InitializeFromGameProject` method in it:

```cs
using System.Runtime.InteropServices;
using Godot.NativeInterop;

namespace GodotPlugins.Game;

internal static class Main
{
    [UnmanagedCallersOnlyAttribute(EntryPoint = "godotsharp_game_main_init")]
    private static godot_bool InitializeFromGameProject(IntPtr godotDllHandle, IntPtr outManagedCallbacks,
       IntPtr unmanagedCallbacks, int unmanagedCallbacksSize)
    {
        return godot_bool.True;
    }
}

```

You can then define any code you want in `InitializeFromGameProject`.

### Debugging

Godot Doorstop supports debugging the assemblies in the runtime.

#### Debugging

Debugging is automatically enabled in CoreCLR. 

To start debugging, compile your DLL in debug mode (with embedded or portable symbols) and start the game with the debugger of your choice.  
Alternatively, attach a debugger to the game once it is running. All standard CoreCLR debuggers should detect the CoreCLR runtime in the game.

Moreover, hot reloading is supported for Visual Studio, Rider and other debuggers with .NET 6 hot reloading feature enabled.

## Doorstop configuration

Doorstop is highly configurable based on your needs and the environment you want to use.
There are two ways to configure Doorstop: via config and CLI arguments.

### Via configuration file

Refer to [`doorstop_config.ini`](assets/windows/doorstop_config.ini) (Windows) or [`run.sh`](assets/nix/run.sh) for all available configuration options.

### CLI arguments

The following CLI arguments are available on both *nix, and Windows builds:

All Doorstop arguments start with `--doorstop-` and always contain an argument. The arguments can be of the following type:

* `bool` = `true` or `false`
* `string` = any sequence of characters and numbers. Wrap into `"`s if the string contains spaces

| Argument                                          | Description                                                                                          |
| ------------------------------------------------- |------------------------------------------------------------------------------------------------------|
| `--doorstop-enabled bool`                         | Enable or disable Doorstop.                                                                          |
| `--doorstop-target-assembly string`               | Path to the assembly to load and execute.                                                            |

## License

Godot Doorstop is licensed under LGPLv2.1. You can view the entire license [here](LICENSE).
