// © 2021 Uuvana Studios Oy. All Rights Reserved.


#include "UMG_Navigation_Component.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "Framework/Application/NavigationConfig.h"

FSlateBrush Brush;

// Sets default values for this component's properties
UUMG_Navigation_Component::UUMG_Navigation_Component()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UUMG_Navigation_Component::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UUMG_Navigation_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UUMG_Navigation_Component::SetupNavigationKeys(bool KeyNavigation, bool TabNavigation, bool AnalogNavigation) {
	TSharedRef<FNavigationConfig> Navigation = FSlateApplication::Get().GetNavigationConfig();
	Navigation->bKeyNavigation = KeyNavigation;
	Navigation->bTabNavigation = TabNavigation;
	Navigation->bAnalogNavigation = AnalogNavigation;
	Navigation->AnalogHorizontalKey = FKey("Gamepad_RightX");
	Navigation->AnalogVerticalKey = FKey("Gamepad_RightY");
	FSlateApplication::Get().SetNavigationConfig(Navigation);
}

void UUMG_Navigation_Component::SetFocusBrush(FSlateBrush brush) {
	Brush = brush;
	FCoreStyle::SetFocusBrush(&Brush);
}