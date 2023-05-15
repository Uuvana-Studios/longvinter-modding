// Copyright Sam Bonifacio. All Rights Reserved.

#include "CoreTypes.h"
#include "PlayerInputMappings.h"
#include "Misc/AutomationTest.h"
#include "Misc/AutoSettingsInputConfig.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Check that when null base preset is specified, the resulting layout is empty
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmptyPresetTest, "AutoSettings.Input.EmptyPreset", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FEmptyPresetTest::RunTest(const FString& Parameters)
{
	UAutoSettingsInputConfig* Config = NewObject<UAutoSettingsInputConfig>();
	const FPlayerInputMappings PlayerInputMappings = FPlayerInputMappings(true, Config);

	// Check layout has nothing bound
	const FInputMappingLayout MergedLayout = PlayerInputMappings.BuildMergedMappingLayout();
	TestEqual(TEXT("Layout must have no actions bound"), MergedLayout.GetActions().Num(), 0);
	TestEqual(TEXT("Layout must have no axes bound"), MergedLayout.GetAxes().Num(), 0);
	return true;
}

/**
 * Check that when an empty base preset tag is specified, the resulting layout is built on a default preset from the config
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDefaultPresetTest, "AutoSettings.Input.DefaultPreset", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FDefaultPresetTest::RunTest(const FString& Parameters)
{
	UAutoSettingsInputConfig* Config = NewObject<UAutoSettingsInputConfig>();

	// Add a preset to config
	Config->InputPresets.Add(FInputMappingPreset(FGameplayTag::EmptyTag, false, Config));
	FInputMappingPreset& Preset = Config->InputPresets.Last();
	
	Preset.InputLayout.MappingGroups.Add(FInputMappingGroup(Config));
	FInputMappingGroup& MappingGroup = Preset.InputLayout.MappingGroups.Last();

	MappingGroup.ActionMappings.Add(FConfigActionKeyMapping(FInputActionKeyMapping(TEXT("Jump"), EKeys::SpaceBar)));

	// Null base preset false, because we want to test with config preset
	const FPlayerInputMappings PlayerInputMappings = FPlayerInputMappings(false, Config);

	// Check second action is now bound
	const FInputActionKeyMapping JumpAction = PlayerInputMappings.BuildMergedMappingLayout().GetAction(0, FName(TEXT("Jump")));
	TestEqual(TEXT("Must inherit from default preset"), JumpAction.Key, EKeys::SpaceBar);

	return true;
}

/**
 * Check that the config object is registered at different levels of the input structure
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigRegisteredTest, "AutoSettings.Input.ConfigRegistered", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FConfigRegisteredTest::RunTest(const FString& Parameters)
{
	UAutoSettingsInputConfig* Config = NewObject<UAutoSettingsInputConfig>();
	FPlayerInputMappings PlayerInputMappings = FPlayerInputMappings(true, Config);
	PlayerInputMappings.AddAxisOverride(FInputAxisKeyMapping(FName(TEXT("MoveForward")), EKeys::I), 0, false);

	TestEqual(TEXT("MappingOverrides must have the created config"), PlayerInputMappings.MappingOverrides.Config.GetObject(), Cast<UObject>(Config));
	TestEqual(TEXT("MappingGroup must have the created config"), PlayerInputMappings.MappingOverrides.MappingGroups[0].Config.GetObject(), Cast<UObject>(Config));
	return true;
}

/**
 * Check that when a button is bound to an axis, the binding is applied correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMapPreviousAxisButtonTest, "AutoSettings.Input.BindPreviousAxisButton", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FMapPreviousAxisButtonTest::RunTest(const FString& Parameters)
{
	UAutoSettingsInputConfig* Config = NewObject<UAutoSettingsInputConfig>();
	FPlayerInputMappings PlayerInputMappings = FPlayerInputMappings(true, Config);
	PlayerInputMappings.AddAxisOverride(FInputAxisKeyMapping(FName(TEXT("MoveForward")), EKeys::MouseY), 0, false);
	PlayerInputMappings.AddAxisOverride(FInputAxisKeyMapping(FName(TEXT("MoveForward")), EKeys::W), 0, false);
	const FInputAxisKeyMapping Axis = PlayerInputMappings.BuildMergedMappingLayout().GetAxis(0, FName(TEXT("MoveForward")), 1.f);
	TestEqual(TEXT("Latest override must be current"), Axis.Key, EKeys::W);
	return true;
}

/**
 * Check that Multiple Bindings Per Key set to false correctly unbinds previous mappings
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultipleBindingsPerKeyDisallowTest, "AutoSettings.Input.MultipleBindingsPerKey.Disallow", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FMultipleBindingsPerKeyDisallowTest::RunTest(const FString& Parameters)
{
	UAutoSettingsInputConfig* Config = NewObject<UAutoSettingsInputConfig>();
	ensure(Config->AllowMultipleBindingsPerKey == false);
	FPlayerInputMappings PlayerInputMappings = FPlayerInputMappings(true, Config);

	// Bind two actions to the same key
	PlayerInputMappings.AddActionOverride(FInputActionKeyMapping(FName(TEXT("FireWeapon")), EKeys::LeftMouseButton), 0, false);
	PlayerInputMappings.AddActionOverride(FInputActionKeyMapping(FName(TEXT("Jump")), EKeys::LeftMouseButton), 0, false);

	// Check second action is now bound
	const FInputActionKeyMapping JumpAction = PlayerInputMappings.BuildMergedMappingLayout().GetAction(0, FName(TEXT("Jump")));
	TestEqual(TEXT("Second action must be bound"), JumpAction.Key, EKeys::LeftMouseButton);
	
	// Check first action is now unbound
	const FInputActionKeyMapping FireWeaponAction = PlayerInputMappings.BuildMergedMappingLayout().GetAction(0, FName(TEXT("FireWeapon")));
	TestEqual(TEXT("First action must be unbound by the second"), FireWeaponAction.Key, EKeys::Invalid);
	return true;
}

/**
 * Check that Multiple Bindings Per Key set to true correctly preserves previous mappings
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultipleBindingsPerKeyAllowTest, "AutoSettings.Input.MultipleBindingsPerKey.Allow", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FMultipleBindingsPerKeyAllowTest::RunTest(const FString& Parameters)
{
	UAutoSettingsInputConfig* Config = NewObject<UAutoSettingsInputConfig>();
	Config->AllowMultipleBindingsPerKey = true;
	FPlayerInputMappings PlayerInputMappings = FPlayerInputMappings(true, Config);

	// Bind two actions to the same key
	PlayerInputMappings.AddActionOverride(FInputActionKeyMapping(FName(TEXT("FireWeapon")), EKeys::LeftMouseButton), 0, false);
	PlayerInputMappings.AddActionOverride(FInputActionKeyMapping(FName(TEXT("Jump")), EKeys::LeftMouseButton), 0, false);

	// Check second action is now bound
	const FInputActionKeyMapping JumpAction = PlayerInputMappings.BuildMergedMappingLayout().GetAction(0, FName(TEXT("Jump")));
	TestEqual(TEXT("Second action must be bound"), JumpAction.Key, EKeys::LeftMouseButton);
	
	// Check first action is still bound
	const FInputActionKeyMapping FireWeaponAction = PlayerInputMappings.BuildMergedMappingLayout().GetAction(0, FName(TEXT("FireWeapon")));
	TestEqual(TEXT("First action must still be bound"), FireWeaponAction.Key, EKeys::LeftMouseButton);
	return true;
}

/**
 * Check that linked mapping groups functions correctly by unbinding mappings on linked groups, but not on other groups
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLinkedMappingGroupsTest, "AutoSettings.Input.LinkedMappingGroups", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FLinkedMappingGroupsTest::RunTest(const FString& Parameters)
{
	UAutoSettingsInputConfig* Config = NewObject<UAutoSettingsInputConfig>();
	Config->AllowMultipleBindingsPerKey = true;
	Config->MappingGroupLinks.Add(FMappingGroupLink({0, 1}));
	FPlayerInputMappings PlayerInputMappings = FPlayerInputMappings(true, Config);

	// Bind two actions on linked groups to the same key
	PlayerInputMappings.AddActionOverride(FInputActionKeyMapping(FName(TEXT("FireWeapon")), EKeys::LeftMouseButton), 0, false);
	PlayerInputMappings.AddActionOverride(FInputActionKeyMapping(FName(TEXT("Jump")), EKeys::LeftMouseButton), 1, false);

	// Bind another action on an unlinked group
	PlayerInputMappings.AddActionOverride(FInputActionKeyMapping(FName(TEXT("Crouch")), EKeys::LeftMouseButton), 2, false);

	// Check second action is now bound
	const FInputActionKeyMapping JumpAction = PlayerInputMappings.BuildMergedMappingLayout().GetAction(1, FName(TEXT("Jump")));
	TestEqual(TEXT("Second action must be bound"), JumpAction.Key, EKeys::LeftMouseButton);
	
	// Check first action is unbound
	const FInputActionKeyMapping FireWeaponAction = PlayerInputMappings.BuildMergedMappingLayout().GetAction(0, FName(TEXT("FireWeapon")));
	TestEqual(TEXT("First action must be unbound by the second"), FireWeaponAction.Key, EKeys::Invalid);

	// Check third action is bound
	const FInputActionKeyMapping CrouchAction = PlayerInputMappings.BuildMergedMappingLayout().GetAction(2, FName(TEXT("Crouch")));
	TestEqual(TEXT("Third action must be bound"), CrouchAction.Key, EKeys::LeftMouseButton);

	return true;
}



#endif
