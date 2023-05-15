// Copyright Sam Bonifacio. All Rights Reserved.

#include "KeyMappingTypes.h"
#include "Misc/AutoSettingsInputConfig.h"

FConfigActionKeyMapping::FConfigActionKeyMapping(FInputActionKeyMapping Base)
{
	ActionName = Base.ActionName;
	Key = Base.Key;
	bShift = Base.bShift;
	bCtrl = Base.bCtrl;
	bAlt = Base.bAlt;
	bCmd = Base.bCmd;
}

FConfigAxisKeyMapping::FConfigAxisKeyMapping(FInputAxisKeyMapping Base)
{
	AxisName = Base.AxisName;
	Key = Base.Key;
	Scale = Base.Scale;
}
