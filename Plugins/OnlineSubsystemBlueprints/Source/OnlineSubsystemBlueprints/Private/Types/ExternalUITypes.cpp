// Copyright June Rhodes. All Rights Reserved.

#include "Types/ExternalUITypes.h"

FLoginFlowResultBP FLoginFlowResultBP::FromNative(const FLoginFlowResult &InObj)
{
    FLoginFlowResultBP Result;
    Result.Token = InObj.Token;
    Result.ErrorMessage = InObj.Error.ErrorMessage;
    Result.ErrorRaw = InObj.Error.ErrorRaw;
    Result.NumericErrorCode = InObj.NumericErrorCode;
    return Result;
}

FLoginFlowResult FLoginFlowResultBP::ToNative()
{
    FLoginFlowResult Result;
    Result.Token = this->Token;
    Result.NumericErrorCode = this->NumericErrorCode;
    return Result;
}

FShowWebUrlParameters FShowWebUrlParameters::FromNative(const FShowWebUrlParams &InObj)
{
    FShowWebUrlParameters Result;
    Result.Embedded = InObj.bEmbedded;
    Result.ShowCloseButton = InObj.bShowCloseButton;
    Result.ShowBackground = InObj.bShowBackground;
    Result.HideCursor = InObj.bHideCursor;
    Result.ResetCookies = InObj.bResetCookies;
    Result.OffsetX = InObj.OffsetX;
    Result.OffsetY = InObj.OffsetY;
    Result.SizeX = InObj.SizeX;
    Result.SizeY = InObj.SizeY;
    Result.AllowedDomains = InObj.AllowedDomains;
    Result.CallbackPath = InObj.CallbackPath;
    return Result;
}

FShowWebUrlParams FShowWebUrlParameters::ToNative()
{
    FShowWebUrlParams Result;
    Result.bEmbedded = this->Embedded;
    Result.bShowCloseButton = this->ShowCloseButton;
    Result.bShowBackground = this->ShowBackground;
    Result.bHideCursor = this->HideCursor;
    Result.bResetCookies = this->ResetCookies;
    Result.OffsetX = this->OffsetX;
    Result.OffsetY = this->OffsetY;
    Result.SizeX = this->SizeX;
    Result.SizeY = this->SizeY;
    Result.AllowedDomains = this->AllowedDomains;
    Result.CallbackPath = this->CallbackPath;
    return Result;
}

FShowStoreParameters FShowStoreParameters::FromNative(const FShowStoreParams &InObj)
{
    FShowStoreParameters Result;
    Result.Category = InObj.Category;
    Result.ProductId = InObj.ProductId;
#if defined(UE_4_25_OR_LATER)
    Result.AddToCart = InObj.bAddToCart;
#endif
    return Result;
}

FShowStoreParams FShowStoreParameters::ToNative()
{
    FShowStoreParams Result;
    Result.Category = this->Category;
    Result.ProductId = this->ProductId;
#if defined(UE_4_25_OR_LATER)
    Result.bAddToCart = this->AddToCart;
#endif
    return Result;
}

FShowSendMessageParameters FShowSendMessageParameters::FromNative(const FShowSendMessageParams &InObj)
{
    FShowSendMessageParameters Result;
    Result.DisplayTitle = InObj.DisplayTitle;
    Result.DisplayTitle_Loc = InObj.DisplayTitle_Loc;
    Result.DisplayMessage = InObj.DisplayMessage;
    Result.DisplayDetails = InObj.DisplayDetails;
    Result.DisplayDetails_Loc = InObj.DisplayDetails_Loc;
    // todo: DisplayThumbnail
    Result.DataPayload = FOnlineMessagePayloadData::FromNative(InObj.DataPayload);
    Result.MaxRecipients = InObj.MaxRecipients;
    return Result;
}

FShowSendMessageParams FShowSendMessageParameters::ToNative()
{
    FShowSendMessageParams Result;
    Result.DisplayTitle = this->DisplayTitle;
    Result.DisplayTitle_Loc = this->DisplayTitle_Loc;
    Result.DisplayMessage = this->DisplayMessage;
    Result.DisplayDetails = this->DisplayDetails;
    Result.DisplayDetails_Loc = this->DisplayDetails_Loc;
    // todo: DisplayThumbnail
    Result.DataPayload = this->DataPayload.ToNative();
    Result.MaxRecipients = this->MaxRecipients;
    return Result;
}