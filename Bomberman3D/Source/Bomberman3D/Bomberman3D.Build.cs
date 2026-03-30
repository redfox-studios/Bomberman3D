// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO; // for dihcord

public class Bomberman3D : ModuleRules
{
	public Bomberman3D(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Niagara" });

        // PrivateDependencyModuleNames.AddRange(new string[] {  });

        PublicIncludePaths.AddRange(new string[] {
            "Bomberman3D",
            "Bomberman3D/Grid",
            "Bomberman3D/Player",
            "Bomberman3D/Bomb",
            "Bomberman3D/Core",
            "Bomberman3D/Enemies",
            "Bomberman3D/Door",
            "Bomberman3D/Upgrades",
            // "Bomberman3D/AntiCheat",
            "Bomberman3D/Debug"
        });

        // dihcord
        string DiscordRPCPath = Path.Combine(ModuleDirectory, "../../ThirdParty/DiscordRPC");
        PublicIncludePaths.Add(Path.Combine(DiscordRPCPath, "include"));
        PublicAdditionalLibraries.Add(Path.Combine(DiscordRPCPath, "lib", "discord-rpc.lib"));
        RuntimeDependencies.Add("$(BinaryOutputDir)/discord-rpc.dll", Path.Combine(DiscordRPCPath, "bin", "discord-rpc.dll"));
        // end dihcord

        // Uncomment if you are using Slate UI
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
