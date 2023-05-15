// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineExternalUISubsystem.h"

void UOnlineExternalUISubsystem::AddLoginFlowHandler(const TScriptInterface<IExternalUIFlowHandler> &Handler)
{
    TSharedPtr<class IOnlineExternalUI, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        FExternalUIFlowHandlerRegistration Register;
        Register.Interface = Handler.GetObject();
        TWeakObjectPtr<UOnlineExternalUISubsystem> SubsystemPtr(this);
        Register.LoginFlowUIRequired = Handle->AddOnLoginFlowUIRequiredDelegate_Handle(
            FOnLoginFlowUIRequired::FDelegate::CreateLambda([this, SubsystemPtr, Handler](
                                                                const FString &RequestedURL,
                                                                const FOnLoginRedirectURL &OnLoginRedirect,
                                                                const FOnLoginFlowComplete &OnLoginFlowComplete,
                                                                bool &bOutShouldContinueLogin) {
                if (SubsystemPtr.IsValid())
                {
                    auto RequestID = SubsystemPtr->NextFlowHandlersRequestID++;
                    SubsystemPtr->LoginFlowCompleteDelegates.Add(RequestID, OnLoginFlowComplete);
                    SubsystemPtr->LoginRedirectURLDelegates.Add(RequestID, OnLoginRedirect);
                    bOutShouldContinueLogin = IExternalUIFlowHandler::Execute_OnLoginFlowUIRequired(
                        Handler.GetObject(),
                        RequestedURL,
                        SubsystemPtr.Get(),
                        RequestID);
                }
            }));
        Register.CreateAccountFlowUIRequired = Handle->AddOnCreateAccountFlowUIRequiredDelegate_Handle(
            FOnCreateAccountFlowUIRequired::FDelegate::CreateLambda([this, SubsystemPtr, Handler](
                                                                        const FString &RequestedURL,
                                                                        const FOnLoginRedirectURL &OnLoginRedirect,
                                                                        const FOnLoginFlowComplete &OnLoginFlowComplete,
                                                                        bool &bOutShouldContinueLogin) {
                if (SubsystemPtr.IsValid())
                {
                    auto RequestID = SubsystemPtr->NextFlowHandlersRequestID++;
                    SubsystemPtr->LoginFlowCompleteDelegates.Add(RequestID, OnLoginFlowComplete);
                    SubsystemPtr->LoginRedirectURLDelegates.Add(RequestID, OnLoginRedirect);
                    bOutShouldContinueLogin = IExternalUIFlowHandler::Execute_OnCreateAccountFlowUIRequired(
                        Handler.GetObject(),
                        RequestedURL,
                        SubsystemPtr.Get(),
                        RequestID);
                }
            }));
        this->FlowHandlers.Add(Register);
    }
}

void UOnlineExternalUISubsystem::RemoveLoginFlowHandler(const TScriptInterface<IExternalUIFlowHandler> &Handler)
{
    TSharedPtr<class IOnlineExternalUI, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (int i = this->FlowHandlers.Num() - 1; i >= 0; i--)
        {
            if (this->FlowHandlers[i].Interface == Handler.GetObject())
            {
                Handle->ClearOnLoginFlowUIRequiredDelegate_Handle(this->FlowHandlers[i].LoginFlowUIRequired);
                Handle->ClearOnCreateAccountFlowUIRequiredDelegate_Handle(
                    this->FlowHandlers[i].CreateAccountFlowUIRequired);
                this->FlowHandlers.RemoveAt(i);
            }
        }
    }
}

FLoginFlowResultBP UOnlineExternalUISubsystem::NotifyLoginRedirectURL(int RequestID, const FString &URL)
{
    if (this->LoginRedirectURLDelegates.Contains(RequestID) && this->LoginRedirectURLDelegates[RequestID].IsBound())
    {
        return FLoginFlowResultBP::FromNative(this->LoginRedirectURLDelegates[RequestID].Execute(URL));
    }
    return FLoginFlowResultBP();
}

void UOnlineExternalUISubsystem::LoginFlowComplete(int RequestID, FLoginFlowResultBP Result)
{
    if (this->LoginFlowCompleteDelegates.Contains(RequestID))
    {
        this->LoginFlowCompleteDelegates[RequestID].ExecuteIfBound(Result.ToNative());
    }
}