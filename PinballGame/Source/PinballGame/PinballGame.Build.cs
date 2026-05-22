using UnrealBuildTool;

public class PinballGame : ModuleRules
{
	public PinballGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"PhysicsCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
