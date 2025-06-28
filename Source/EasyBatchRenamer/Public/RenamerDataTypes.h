// Copyright Devil Dev, 2025.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameFramework/Actor.h"


enum class ERenameItemType : uint8
{
	Asset UMETA(DisplayName = "Asset"),
	Actor UMETA(DisplayName = "Actor")
};

enum class ETextCaseChange : uint8
{
	NoChange UMETA(DisplayName = "No Change"),
	ReplacedToLower UMETA(DisplayName = "ReplacedToLower"),
	ReplacedToUpper UMETA(DisplayName = "ReplacedToUpper"),
	ToLower UMETA(DisplayName = "To Lowercase"),
	ToUpper UMETA(DisplayName = "To Uppercase")
};

struct FRenameItem : TSharedFromThis<FRenameItem>
{
	FAssetData OriginalAssetData;
	TWeakObjectPtr<UObject> OriginalObject;

	FString OriginalName;
	FString PreviewName;
	FString ManualName;

	// 标记当前项是 Asset 还是 Actor
	ERenameItemType ItemType;

	// 引入一个标志来精确控制“手动命名”的更新行为
	bool bIsManualOverride;

	FRenameItem(const FAssetData& InAssetData)
		: OriginalAssetData(InAssetData)
		, OriginalName(InAssetData.AssetName.ToString())
		, PreviewName(InAssetData.AssetName.ToString())
		, ManualName(InAssetData.AssetName.ToString())
		, ItemType(ERenameItemType::Asset)
		, bIsManualOverride(false)
	{}

	FRenameItem(AActor* InActor)
		: OriginalObject(InActor)
		, OriginalName(InActor->GetActorLabel())
		, PreviewName(InActor->GetActorLabel())
		, ManualName(InActor->GetActorLabel())
		, ItemType(ERenameItemType::Actor)
		, bIsManualOverride(false)
	{}

	FString GetFinalRenameName() const
	{
		if (bIsManualOverride)
		{
			return ManualName;
		}
		return PreviewName;
	}

	/**
	 * @brief 将预览名称重置为原始名称。
	 *
	 * 当用户清除了所有重命名规则时，调用此函数可将 PreviewName 恢复为 OriginalName，
	 * 以便在 UI 中正确显示未经过任何规则处理的原始名称。
	 */
	void ResetPreviewName()
	{
		PreviewName = OriginalName;
	}

	/**
	 * @brief 应用重命名规则到当前项的名称上，生成预览名称。
	 *
	 * 该函数根据提供的多种规则（如查找替换、正则表达式、前缀后缀添加、编号等）对原始名称进行修改，
	 * 并将结果存储在 PreviewName 中。此函数通常用于批量重命名工具中，允许用户在正式重命名之前看到效果。
	 *
	 * @param InFindText 要查找并替换的文本（非正则模式下使用）
	 * @param InReplaceText 替换为的文本（非正则模式下使用）
	 * @param bUseRegex 是否使用正则表达式进行查找替换
	 * @param InRegexFindText 正则表达式查找的文本（仅供显示或调试用途，实际使用传入的缓存模式）
	 * @param InCachedRegexPattern 已编译的正则表达式模式（避免重复编译提高性能）
	 * @param InPrefix 添加到名称前面的前缀
	 * @param InSuffix 添加到名称后面的后缀
	 * @param bShouldUseNumbering 是否启用编号功能
	 * @param InNumber 如果启用编号，该参数作为当前项的编号值
	 * @param InPaddingDigits 编号的最小位数，不足时前面补零（例如：3 -> 001）
	 * @param InCaseChange 指定是否将名称转换为大写或小写
	 */
	void ApplyRenameRules(const FString& InFindText, const FString& InReplaceText,
						  bool bUseRegex, const FString& InRegexFindText, TSharedPtr<FRegexPattern> InCachedRegexPattern,
						  const FString& InPrefix, const FString& InSuffix,
						  bool bShouldUseNumbering, int32 InNumber, int32 InPaddingDigits,
						  ETextCaseChange InCaseChange
						  )
	{
		FString TempName = OriginalName;
		FString ReplacedResult = FString(TEXT(""));

		if (bUseRegex && InCachedRegexPattern.IsValid())
		{
			FRegexMatcher RegexMatcher(*InCachedRegexPattern.Get(), TempName);
			FString ReplacedString;

			int32 LastMatchEnd = 0;

			while (RegexMatcher.FindNext())
			{
				int32 MatchBeginning = RegexMatcher.GetMatchBeginning();
				int32 MatchEnding = RegexMatcher.GetMatchEnding();

				ReplacedString += TempName.Mid(LastMatchEnd, MatchBeginning - LastMatchEnd);

				FString CurrentReplacement = InReplaceText;

				for (int32 i = 0; i < 10; ++i)
				{
					FString CaptureGroupPlaceholder = FString::Printf(TEXT("$%d"), i);
					FString CaptureGroupValue = RegexMatcher.GetCaptureGroup(i);

					CurrentReplacement.ReplaceInline(*CaptureGroupPlaceholder, *CaptureGroupValue, ESearchCase::CaseSensitive);
				}

				if (InCaseChange == ETextCaseChange::ReplacedToLower)
				{
					CurrentReplacement = CurrentReplacement.ToLower();
				}
				else if (InCaseChange == ETextCaseChange::ReplacedToUpper)
				{
					CurrentReplacement = CurrentReplacement.ToUpper();
				}

				ReplacedString += CurrentReplacement;
				LastMatchEnd = MatchEnding;
			}
			ReplacedString += TempName.Mid(LastMatchEnd);
			TempName = ReplacedString;
		}
		else if (!InFindText.IsEmpty())
		{
			// TempName.ReplaceInline(*InFindText, *InReplaceText, ESearchCase::CaseSensitive);
			int32 CurrentPos = 0;
			int32 FoundPos = INDEX_NONE;

			while ((FoundPos = TempName.Find(InFindText, ESearchCase::CaseSensitive, ESearchDir::FromStart, CurrentPos)) != INDEX_NONE)
			{
				ReplacedResult += TempName.Mid(CurrentPos, FoundPos - CurrentPos);

				FString CurrentReplacementText = InReplaceText;
				if (InCaseChange == ETextCaseChange::ReplacedToLower)
				{
					CurrentReplacementText = CurrentReplacementText.ToLower();
				}
				else if (InCaseChange == ETextCaseChange::ReplacedToUpper)
				{
					CurrentReplacementText = CurrentReplacementText.ToUpper();
				}

				ReplacedResult += CurrentReplacementText;
				CurrentPos = FoundPos + InFindText.Len();
			}
			ReplacedResult += TempName.Mid(CurrentPos);
			TempName = ReplacedResult;
		}

		FString FinalNameBuilder;
		FinalNameBuilder.Reserve(TempName.Len() + InPrefix.Len() + InSuffix.Len() + (bShouldUseNumbering ? 10 : 0));

		if (!InPrefix.IsEmpty())
		{
			FinalNameBuilder.Append(InPrefix);
		}

		FinalNameBuilder.Append(TempName);

		if (!InSuffix.IsEmpty())
		{
			FinalNameBuilder.Append(InSuffix);
		}

		if (bShouldUseNumbering)
		{
			FString NumberString = FString::Printf(TEXT("%0*d"), InPaddingDigits, InNumber);
			FinalNameBuilder.Append(NumberString);
		}

		TempName = FinalNameBuilder;

		switch (InCaseChange)
		{
		case ETextCaseChange::ToLower:
			TempName = TempName.ToLower();
			break;
		case ETextCaseChange::ToUpper:
			TempName = TempName.ToUpper();
			break;
		case ETextCaseChange::ReplacedToLower:
		case ETextCaseChange::ReplacedToUpper:
		case ETextCaseChange::NoChange:
			break;
		default:
			break;
		}

		PreviewName = TempName;
	}
};
