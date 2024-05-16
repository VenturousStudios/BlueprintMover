// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class BlueprintMover : ModuleRules
{
	public BlueprintMover(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "NetworkPrediction", "AnimGraphRuntime", "Engine", "InputCore", "Mover" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Chaos",
                "CoreUObject",
                "Engine",
                "PhysicsCore",
                "DeveloperSettings", });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
