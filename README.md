# ForgeModInjector

A lightweight tool designed to inject Forge mods into Minecraft for educational and research purposes.

---

## **Overview**
This tool consists of two main components:
1. **Injector (Executable):** A command-line application that injects a DLL (the Payload) into the Minecraft process.
2. **Payload (DLL):** A dynamic library that hooks into the JVM and facilitates loading the Forge mod into Minecraft.

---

## **Usage**
### **Injector**
The Injector is a command-line tool that injects the Payload into the Minecraft process. You can use it in two ways:

1. **Using the Minecraft Process ID:**
   ```bash
   Injector.exe -p <Minecraft Process ID> -f <Payload Path>
    ```
   Replace `<Minecraft Process ID>` with the PID of the Minecraft process and <Payload Path> with the full path to the DLL Payload file.

2. **Using the Minecraft Window Name:**
    ```bash
    Injector.exe -w <Minecraft Window Name> -f <Payload Path>
    ```

### Payload
The Payload is a DLL that gets injected into Minecraft by the Injector. It hooks into the JVM running the game and loads the Forge mod into the Minecraft environment.

# Mixins
Getting mixins to work is a little more tricky if not done at boot time. So either you hook up the game process
during the startup and manually register your mixins, or you go ahead and do something scuffed like run-time hotswapping or such.
However, doing that will require special JVMs like the JetBrains JVM and a HotSwap agent so it's not really viable.

# Important Notes for Developers
Educational Use Only:
This tool is intended solely for educational purposes and modding research. The author does not condone or take responsibility for misuse of this tool.

No Support for Cheat Development:
While the tool could theoretically be adapted for malicious purposes, this README provides no support for creating cheats or exploiting Minecraft. An additional subdirectory in the repository contains tools for running fully in-memory JAR files. However, no guidance will be provided for this feature to discourage misuse.

