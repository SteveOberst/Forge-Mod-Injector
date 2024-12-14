# ForgeModInjector

A lightweight tool designed to inject Forge mods into Minecraft for educational and research purposes.


## **Overview**
This tool consists of two main components:
1. **Injector (Executable):** A command-line application that injects a DLL (the Payload) into the Minecraft process.
2. **Payload (DLL):** A dynamic library that hooks into the JVM and facilitates loading the Forge mod into Minecraft.


## **Supported Versions**  
Currently tested with:  
- **Minecraft Forge 1.8.x**  

## **Usage**
### **Injector**
The Injector is a command-line tool that injects the Payload into the Minecraft process. You can use it in two ways:

1. **Using the Minecraft Process ID:**
   ```bash
   Injector.exe -t <Minecraft Process ID> -p <Payload Path> -m <Mod File Path> -c <Main Class Name>
    ```
   Replace `<Minecraft Process ID>` with the PID of the Minecraft process and <Payload Path> with the full path to the DLL Payload file.

2. **Using the Minecraft Window Name:**
    ```bash
    Injector.exe -w <Minecraft Window Name> -p <Payload Path> -m <Mod File Path> -c <Main Class Name>
    ```
The `<Payload Path>` and `<Mod File Path>` can be supplied as relative paths. if they're in the same directory as the executable, you may only provide the file name.
### **Parameters**
- **-w, --target-window**: The window name to inject into (useful if you know the window name but not the process ID).
- **-t, --target-pid**: The Minecraft process ID to inject into (useful if you know the PID).
- **-p, --payload**: The path to the DLL Payload file to inject.
- **-m, --mod-file**: The path to the mod file to inject.
- **-c, --main-class**: The name of the mod's main class. **Important:** The mod's main class must implement a `void initialize()` method, which will be called when the mod is injected into Minecraft.


### Payload
The Payload is a DLL that gets injected into Minecraft by the Injector. It hooks into the JVM running the game and loads the Forge mod into the Minecraft environment.

# Showcase
![gif](https://pouch.jumpshare.com/preview/stMaHMc4hQmUMyd93-96dkNBDyeWbZ017q_bU6Gtbgz7BpAkN0gw-SHN2CuhDLVVvlTcRIu8guQ3El4ZNw2QyJXsbY9i-j22O5gaQ6ysCNQ)

# Mixins
Getting mixins to work is a little more tricky if not done at boot time. So either you hook up the game process
during the startup and manually register your mixins, or you go ahead and do something scuffed like run-time hotswapping or such.
However, doing that will require special JVMs like the JetBrains JVM and a HotSwap agent so it's not really viable.

# Important Notes for Developers
Educational Use Only:
This tool is intended solely for educational purposes and modding research. The author does not condone or take responsibility for misuse of this tool.

No Support for Cheat Development:
While the tool could theoretically be adapted for malicious purposes, this README provides no support for creating cheats or exploiting Minecraft. An additional subdirectory in the repository contains tools for running fully in-memory JAR files. However, no guidance will be provided for this feature to discourage misuse.

