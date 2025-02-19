#include "BlueprintAssistActionsBase.h"

#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistTabHandler.h"
#include "Framework/Application/SlateApplication.h"

TSharedPtr<FBAGraphHandler> FBAActionsBase::GetGraphHandler()
{
	return FBATabHandler::Get().GetActiveGraphHandler();
}

bool FBAActionsBase::CanExecuteActions() const
{
	return FSlateApplication::Get().IsInitialized() && !FBAUtils::IsGamePlayingAndHasFocus();
}
