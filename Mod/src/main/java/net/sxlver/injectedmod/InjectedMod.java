package net.sxlver.injectedmod;

import net.minecraft.client.Minecraft;
import net.minecraft.client.entity.EntityPlayerSP;
import net.minecraft.util.ChatComponentText;
import net.minecraftforge.fml.common.Mod;
import net.minecraftforge.fml.common.event.FMLInitializationEvent;
import org.lwjgl.opengl.Display;

@Mod(modid = InjectedMod.MODID, version = InjectedMod.VERSION)
public class InjectedMod {
    public static final String MODID = "injectedmod";
    public static final String VERSION = "0.1.1-alpha";
    
    /**
     * Called by the injector
     */
    public static void initialize() {
        new InjectedMod().start();
    }
    
    /**
     * Called by Forge, here to maintain compatibility with Forge
     *
     * @param event the event
     */
    @Mod.EventHandler
    public void onFMLInitialization(final FMLInitializationEvent event) {
        initialize();
    }
    
    public void start() {
        final Minecraft minecraft = Minecraft.getMinecraft();
        final EntityPlayerSP player = minecraft.thePlayer;
        if(player == null) {
            Display.setTitle("InjectedMod");
            return;
        }
        
        player.addChatMessage(new ChatComponentText("Mod has been injected!"));
        minecraft.ingameGUI.displayTitle("Forge Mod Injector", "Mod has been injected!", 10, 40, 10);
        
        // do normal startup stuff...
    }
    
}
