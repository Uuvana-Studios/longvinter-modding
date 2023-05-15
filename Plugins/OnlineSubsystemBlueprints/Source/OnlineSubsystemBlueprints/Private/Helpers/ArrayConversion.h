// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "CoreMinimal.h"
#include <functional>

/**
 * Converts an array of one type into an array of another type. This helper function is used in auto-generated
 * code when we need to convert arrays.
 */
template <typename TSrc, typename TDest>
TArray<TDest> ConvertArrayElements(const TArray<TSrc> &Src, std::function<TDest(const TSrc &Src)> Convert)
{
    TArray<TDest> Dest;
    for (auto i = 0; i < Src.Num(); i++)
    {
        Dest.Add(Convert(Src[i]));
    }
    return Dest;
}

/**
 * Validates that all the elements of an array are valid.
 */
template <typename TSrc>
bool ValidateArrayElements(const TArray<TSrc> &Src, std::function<bool(const TSrc &Src)> Validate)
{
    for (auto i = 0; i < Src.Num(); i++)
    {
        if (!Validate(Src[i]))
        {
            return false;
        }
    }
    return true;
}

/**
 * Converts an map of one type into an map of another type. This helper function is used in auto-generated
 * code when we need to convert maps.
 */
template <typename TKeySrc, typename TKeyDest, typename TValueSrc, typename TValueDest>
TMap<TKeyDest, TValueDest> ConvertMapElements(
    const TMap<TKeySrc, TValueSrc> &Src,
    std::function<TKeyDest(const TKeySrc &Src)> ConvertKey,
    std::function<TValueDest(const TValueSrc &Src)> ConvertValue)
{
    TMap<TKeyDest, TValueDest> Dest;
    for (auto KV : Src)
    {
        Dest.Add(ConvertKey(KV.Key), ConvertValue(KV.Value));
    }
    return Dest;
}

/**
 * Validates that all the elements of an array are valid.
 */
template <typename TKeySrc, typename TValueSrc>
bool ValidateMapElements(
    const TMap<TKeySrc, TValueSrc> &Src,
    std::function<bool(const TKeySrc &Src)> ValidateKey,
    std::function<bool(const TValueSrc &Src)> ValidateValue)
{
    for (auto KV : Src)
    {
        if (!ValidateKey(KV.Key) || !ValidateValue(KV.Value))
        {
            return false;
        }
    }
    return true;
}