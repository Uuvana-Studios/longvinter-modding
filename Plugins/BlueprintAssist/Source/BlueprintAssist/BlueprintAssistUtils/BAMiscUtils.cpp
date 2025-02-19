// Fill out your copyright notice in the Description page of Project Settings.

#include "BAMiscUtils.h"

#include "Editor/Kismet/Public/SSCSEditor.h"
#include "Misc/AsciiSet.h"

#if BA_UE_VERSION_OR_LATER(5, 0)
#include "SSubobjectEditor.h"
#endif

AActor* FBAMiscUtils::GetSCSNodeDefaultActor(TSharedPtr<BA_SUBOBJECT_EDITOR_TREE_NODE> Node, UBlueprint* Blueprint)
{
#if BA_UE_VERSION_OR_LATER(5, 0)
	FSubobjectData* Data = Node->GetDataSource();
	return Data ? const_cast<AActor*>(Data->GetObjectForBlueprint<AActor>(Blueprint)) : nullptr;
#elif BA_UE_VERSION_OR_LATER(4, 26)
	return Node->GetEditableObjectForBlueprint<AActor>(Blueprint);
#else
	return (Blueprint != nullptr && Blueprint->GeneratedClass != nullptr) ? Blueprint->GeneratedClass->GetDefaultObject<AActor>() : nullptr;
#endif
}

bool FBAMiscUtils::IsSCSActorNode(TSharedPtr<BA_SUBOBJECT_EDITOR_TREE_NODE> Node)
{
#if BA_UE_VERSION_OR_LATER(5, 0)
	FSubobjectData* Source = Node->GetDataSource();
	return Source ? Source->IsActor() : false;
#elif BA_UE_VERSION_OR_LATER(4, 26)
	return Node->IsActorNode();
#else
	return Node->GetNodeType() == FSCSEditorTreeNode::ENodeType::RootActorNode;
#endif
}

TArray<FString> FBAMiscUtils::ParseStringIntoArray(const FString& String, bool bToLower)
{
	constexpr FAsciiSet Delimiters("_,.|/;");

	int StartSlice = -1;

	TArray<FString> OutArray;

	// slice is a start index and end index
	TArray<TPair<int, int>> Slices;

	// UE_LOG(LogTemp, Warning, TEXT("%s"), *String);

	bool bIsPrevUpper = false;
	for (int i = 0; i < String.Len(); ++i)
	{
		const TCHAR& Char = String[i];
		const bool bSkipChar = FChar::IsWhitespace(Char) || Delimiters.Contains(Char);
		const bool bIsLastChar = i == String.Len() - 1;
		const bool bIsUpper = FChar::IsUpper(Char);

		if (StartSlice >= 0)
		{
			if (bSkipChar || bIsLastChar || bIsUpper && !bIsPrevUpper)
			{
				const int EndSlice = bIsLastChar ? i : i - 1;
				// Slices.Add(TPair<int, int>(StartSlice, EndSlice));
				const int Length = EndSlice - StartSlice + 1;
				FString Slice = String.Mid(StartSlice, Length);
				if (bToLower)
				{
					Slice.ToLowerInline();
				}
				OutArray.Add(Slice);

				// reset the slice index or start it here if we don't skip the char
				StartSlice = bSkipChar ? -1 : i;
			}
		}
		else if (!bSkipChar && StartSlice < 0)
		{
			// if we are a new slice but we are the last char then we need a 1 length slice
			if (bIsLastChar)
			{
				TCHAR LowerChar = bToLower ? FChar::ToLower(Char) : Char;
				OutArray.Add(FString(&LowerChar));
			}
			else
			{
				StartSlice = i;
			}
		}

		bIsPrevUpper = bIsUpper;
	}

	// for (const FString& S : OutArray)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("%s"), *S);
	// }

	return OutArray;
}

FString FBAMiscUtils::ParseSearchTextByFilter(const FString& SearchText, const FString& FilterString)
{
	TArray<FString> SearchWords = ParseStringIntoArray(SearchText);

	int LastMatchingLetter = -1;
	for (int i = 0; i < SearchWords.Num(); ++i)
	{
		// add first letter of search word
		const FString& Word = SearchWords[i];

		// if the character matches and we aren't the last word use the character instead
		if (FilterString.Len() > i && FChar::ToLower(FilterString[i]) == FChar::ToLower(Word[0]) && i != SearchWords.Num())
		{
			LastMatchingLetter = i;
		}
		else
		{
			break;
		}
	}

	// make new search text by ignoring words when the first letter matches 
	FString NewSearchText;
	for (int i = 0; i < SearchWords.Num(); ++i)
	{
		// add first letter of search word
		const FString& Word = SearchWords[i].ToLower();
		if (i <= LastMatchingLetter)
		{
			NewSearchText += Word[0];
		}
		else
		{
			NewSearchText += Word;
		}
	}

	// UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *SearchText, *NewSearchText);

	return NewSearchText;
}