using UnrealBuildTool;
using System.Collections.Generic;

public class PinballGameTarget : TargetRules
{
	public PinballGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange(new string[] { "PinballGame" });
	}
}
