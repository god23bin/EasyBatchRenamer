// Copyright Devil Dev, 2025.


#include "SRenamerWidget.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Selection.h"
#include "SlateOptMacros.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"

#include "Styling/AppStyle.h"

#include "Misc/MessageDialog.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/FeedbackContext.h"

#include "Editor.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "Editor/EditorEngine.h"
#include "Editor/Transactor.h"
#include "Editor/UnrealEd/Public/Editor.h"

#include "Engine/World.h"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

#define LOCTEXT_NAMESPACE "SRenamerWidget"

SRenamerWidget::SRenamerWidget()
	: FindText(TEXT(""))
	, ReplaceText(TEXT(""))
	, bEnableRegexFind(false)
	, RegexFindText(TEXT(""))
	, LastRegexFindText(TEXT(""))
	, CachedRegexPattern(nullptr)
	, PrefixText(TEXT(""))
	, SuffixText(TEXT(""))
	, bUseNumbering(false)
	, StartNumber(DefaultStartNumber)
	, PaddingDigits(DefaultPaddingDigits)
	, CaseChangeOption(ETextCaseChange::NoChange)
	, bAnyItemNeedsRename(false)
{
	CaseChangeOptions.Add(MakeShared<ETextCaseChange>(ETextCaseChange::NoChange));
	CaseChangeOptions.Add(MakeShared<ETextCaseChange>(ETextCaseChange::ReplacedToLower));
	CaseChangeOptions.Add(MakeShared<ETextCaseChange>(ETextCaseChange::ReplacedToUpper));
	CaseChangeOptions.Add(MakeShared<ETextCaseChange>(ETextCaseChange::ToLower));
	CaseChangeOptions.Add(MakeShared<ETextCaseChange>(ETextCaseChange::ToUpper));
}

void SRenamerWidget::Construct(const FArguments& InArgs)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.GetOnAssetSelectionChanged().AddRaw(this, &SRenamerWidget::OnContentBrowserSelectionChanged);

	if (GEditor && GEditor->GetSelectedActors())
	{
		GEditor->GetSelectedActors()->SelectionChangedEvent.AddRaw(this, &SRenamerWidget::OnActorSelectionChanged);
	}
	
	ChildSlot
	[
		CreateMainContentLayout()
	];

	// 初始化 RenameList
	UpdateRenameListFromSelection();
}

SRenamerWidget::~SRenamerWidget()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.GetOnAssetSelectionChanged().RemoveAll(this);

	if (GEditor) 
	{
		GEditor->GetSelectedActors()->SelectionChangedEvent.RemoveAll(this);
	}
}

TSharedRef<SWidget> SRenamerWidget::CreateMainContentLayout()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			CreateRenameOptionsArea()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(20)
		[
			CreateNoSelectionMessageArea()
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(5)
		[
			CreateRenameListViewArea()
		];
}

TSharedRef<SWidget> SRenamerWidget::CreateRenameOptionsArea()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(10)
		.Visibility_Lambda([this]() {return HasSelection() ? EVisibility::Visible : EVisibility::Collapsed;})
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 5))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.3f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(LOCTEXT("FindTextLabel", "Find Text:")).Font(FAppStyle::GetFontStyle("SmallFont"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.7f)
				[
					SAssignNew(FindTextBoxWidget, SEditableTextBox) // Assign to member variable
					.Text(FText::FromString(FindText))
					.OnTextChanged(this, &SRenamerWidget::OnFindTextChanged)
					.HintText(LOCTEXT("FindTextHint", "Text to find..."))
					.IsEnabled(this, &SRenamerWidget::GetFindTextBoxIsEnabled) // Bind IsEnabled
					.Font(FAppStyle::GetFontStyle("SmallFont"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 5))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.3f)
				.VAlign(VAlign_Center)
				[
					SNew(SCheckBox)
					.IsChecked(this, &SRenamerWidget::GetEnableRegexCheckBoxState)
					.OnCheckStateChanged(this, &SRenamerWidget::OnEnableRegexCheckBoxChanged)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("EnableRegexLabel", "Enable Regex:"))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.7f)
				[
					SAssignNew(RegexFindTextBoxWidget, SEditableTextBox) // Assign to member variable
					.Text(FText::FromString(RegexFindText))
					.OnTextChanged(this, &SRenamerWidget::OnRegexFindTextChanged)
					.HintText(LOCTEXT("RegexFindTextHint", "Regex pattern to find (e.g., '^(BP_)')"))
					.IsEnabled(this, &SRenamerWidget::GetRegexTextBoxIsEnabled) // Bind IsEnabled
					.Font(FAppStyle::GetFontStyle("SmallFont"))
				]
			]
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0, 0, 0, 5))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(0.3f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text(LOCTEXT("ReplaceTextLabel", "Replace With:")).Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.7f)
                [
                    SAssignNew(ReplaceTextBoxWidget, SEditableTextBox)
                    .Text(FText::FromString(ReplaceText))
                    .OnTextChanged(this, &SRenamerWidget::OnReplaceTextChanged)
                    .HintText(LOCTEXT("ReplaceTextHint", "Text to replace with..."))
                	.Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0, 0, 0, 5))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(0.3f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text(LOCTEXT("PrefixTextLabel", "Prefix:")).Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.7f)
                [
                    SAssignNew(PrefixTextBoxWidget, SEditableTextBox)
                    .Text(FText::FromString(PrefixText))
                    .OnTextChanged(this, &SRenamerWidget::OnPrefixTextChanged)
                    .HintText(LOCTEXT("PrefixTextHint", "Add prefix..."))
                	.Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0, 0, 0, 5))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(0.3f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text(LOCTEXT("SuffixTextLabel", "Suffix:")).Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.7f)
                [
                    SAssignNew(SufixTextBoxWidget, SEditableTextBox)
                    .Text(FText::FromString(SuffixText))
                    .OnTextChanged(this, &SRenamerWidget::OnSuffixTextChanged)
                    .HintText(LOCTEXT("SuffixTextHint", "Add suffix..."))
                	.Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0, 0, 0, 5))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(0.3f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text(LOCTEXT("NumberingLabel", "Use Numbering:")).Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.7f)
                [
                    SNew(SCheckBox)
                	.IsChecked(this, &SRenamerWidget::GetEnableUseNumberingCheckBoxState)
                    .OnCheckStateChanged(this, &SRenamerWidget::OnUseNumberingChanged)
                ]
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0, 0, 0, 5))
            [
                SNew(SHorizontalBox)
                .IsEnabled_Lambda([this]() { return bUseNumbering; })
                + SHorizontalBox::Slot()
                .FillWidth(0.3f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text(LOCTEXT("StartNumberLabel", "Start Number:")).Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.7f)
                [
                    SAssignNew(StartNumberSpinBoxWidget, SSpinBox<int32>)
                    .Value(StartNumber)
                    .OnValueChanged(this, &SRenamerWidget::OnStartNumberChanged)
                    .MinValue(1)
                    .MaxValue(9999)
                	.Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0, 0, 0, 5))
            [
                SNew(SHorizontalBox)
                .IsEnabled_Lambda([this]() { return bUseNumbering; })
                + SHorizontalBox::Slot()
                .FillWidth(0.3f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text(LOCTEXT("PaddingDigitsLabel", "Padding Digits:")).Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.7f)
                [
                    SAssignNew(PaddingDigitsSpinBoxWidget, SSpinBox<int32>)
                    .Value(PaddingDigits)
                    .OnValueChanged(this, &SRenamerWidget::OnPaddingDigitsChanged)
                    .MinValue(1)
                    .MaxValue(10)
                	.Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0, 0, 0, 5))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(0.3f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text(LOCTEXT("CaseChangeLabel", "Case Change:")).Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.7f)
                [
                    SNew(SComboBox<TSharedPtr<ETextCaseChange>>)
                    .OptionsSource(&CaseChangeOptions)
                    .OnGenerateWidget(this, &SRenamerWidget::GenerateCaseChangeComboBoxEntry)
                    .OnSelectionChanged(this, &SRenamerWidget::OnCaseChangeOptionChanged)
                    .InitiallySelectedItem(CaseChangeOptions[0])
                    .Content()
                    [
                        SNew(STextBlock)
                        .Text(this, &SRenamerWidget::GetCurrentCaseChangeComboBoxText)
                    	.Font(FAppStyle::GetFontStyle("SmallFont"))
                    ]
                ]
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .HAlign(HAlign_Right)
            .Padding(FMargin(0, 10, 0, 0))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .HAlign(HAlign_Left)
                .VAlign(VAlign_Center)
                .Padding(FMargin(0, 0, 10, 0))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("RenameHint", "Hint: English names are recommended."))
                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                    .ColorAndOpacity(FSlateColor::FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f)))
                    .Justification(ETextJustify::Left) // 确保文本左对齐
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(0, 0, 10, 0))
                [
                    SNew(SButton)
                    .OnClicked(this, &SRenamerWidget::OnResetAllRulesClicked)
                    .IsEnabled(this, &SRenamerWidget::AreRulesModified)
                    .ContentPadding(FMargin(15, 5))
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("ResetRulesButton", "Reset Rules"))
                        .Font(FAppStyle::GetFontStyle("BoldFont"))
                    ]
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .OnClicked(this, &SRenamerWidget::OnApplyRenameButtonClicked)
                    .IsEnabled(this, &SRenamerWidget::CanApplyRename)
                    .ContentPadding(FMargin(15, 5))
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("ApplyRenameButton", "Apply Rename"))
                        .Font(FAppStyle::GetFontStyle("BoldFont"))
                    ]
                ]
            ]
		];
}

TSharedRef<SWidget> SRenamerWidget::CreateNoSelectionMessageArea()
{
	return SAssignNew(NoSelectionMessageText, STextBlock)
		.Text(LOCTEXT("NoSelectionMessage", "Please select assets in the Content Browser or actors in the Outliner to begin batch renaming."))
		.Visibility_Lambda([this]() { return HasSelection() ? EVisibility::Collapsed : EVisibility::Visible; })
		.Font(FAppStyle::GetFontStyle("HeadingSmall"))
		.ColorAndOpacity(FSlateColor::FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)))
		.AutoWrapText(true);
}

TSharedRef<SWidget> SRenamerWidget::CreateRenameListViewArea()
{
	return SNew(SVerticalBox)
        .Visibility_Lambda([this]() { return HasSelection() ? EVisibility::Visible : EVisibility::Collapsed; })
        
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNew(SScrollBox) 
            + SScrollBox::Slot()
            [
                SAssignNew(RenameListView, SListView<TSharedPtr<FRenameItem>>)
                .ListItemsSource(&RenameItems)
                .OnGenerateRow(this, &SRenamerWidget::HandleGenerateRow)
                .SelectionMode(ESelectionMode::Multi)
                .HeaderRow
                (
                    SNew(SHeaderRow)
                    // 为每一列都定义了一个 SHeaderRow::Column
                    + SHeaderRow::Column(TEXT("OriginalName"))                      
                    .DefaultLabel(LOCTEXT("OriginalNameColumn", "Original Name"))
                    .FillWidth(0.25f)
                    .ShouldGenerateWidget(true)
                    .HAlignCell(HAlign_Left)
                    .VAlignCell(VAlign_Center)
                    .HAlignHeader(HAlign_Left)
                    .VAlignHeader(VAlign_Center)
                    
                    + SHeaderRow::Column(TEXT("PreviewName"))
                    .DefaultLabel(LOCTEXT("PreviewNameColumn", "Preview Name"))
                    .FillWidth(0.25f)
                    .ShouldGenerateWidget(true)
                    .HAlignCell(HAlign_Left)
                    .VAlignCell(VAlign_Center)
                    .HAlignHeader(HAlign_Left)
                    .VAlignHeader(VAlign_Center)

                    + SHeaderRow::Column(TEXT("Type"))
                    .DefaultLabel(LOCTEXT("TypeColumn", "Type"))
                    .FillWidth(0.20f)
                    .ShouldGenerateWidget(true)
                    .HAlignCell(HAlign_Left)
                    .VAlignCell(VAlign_Center)
                    .HAlignHeader(HAlign_Left)
                    .VAlignHeader(VAlign_Center)

                    + SHeaderRow::Column(TEXT("ManualName"))
                    .DefaultLabel(LOCTEXT("ManualNameColumn", "Manual Name"))
                    .FillWidth(0.30f)
                    .ShouldGenerateWidget(true)
                    .HAlignCell(HAlign_Left)
                    .VAlignCell(VAlign_Center)
                    .HAlignHeader(HAlign_Left)
                    .VAlignHeader(VAlign_Center)
                )
            ]
        ];
}

TSharedRef<SWidget> SRenamerWidget::GenerateCaseChangeComboBoxEntry(TSharedPtr<ETextCaseChange> InOption)
{
	FText DisplayText;
	switch (*InOption)
	{
	case ETextCaseChange::NoChange:
		DisplayText = LOCTEXT("CaseChange_NoChange", "No Change");
		break;
	case ETextCaseChange::ReplacedToLower:
		DisplayText = LOCTEXT("CaseChange_ReplacedToLower", "Replaced Part To Lower");
		break;
	case ETextCaseChange::ReplacedToUpper:
		DisplayText = LOCTEXT("CaseChange_ReplacedToUpper", "Replaced Part To Upper");
		break;
	case ETextCaseChange::ToLower:
		DisplayText = LOCTEXT("CaseChange_ToLower", "Whole Name To Lower");
		break;
	case ETextCaseChange::ToUpper:
		DisplayText = LOCTEXT("CaseChange_ToUpper", "Whole Name To Upper");
		break;
	default:
		DisplayText = LOCTEXT("CaseChange_Unknown", "Unknown");
		break;
	}
	return SNew(STextBlock).Text(DisplayText);
}

void SRenamerWidget::ResetAllRulesInternal()
{
	FindText = FString(TEXT(""));
	ReplaceText = FString(TEXT(""));
	RegexFindText = FString(TEXT(""));
	LastRegexFindText = FString(TEXT(""));
	PrefixText = FString(TEXT(""));
	SuffixText = FString(TEXT(""));
	bEnableRegexFind = false;
	bUseNumbering = false;
	StartNumber = 1;
	PaddingDigits = 3;
	CaseChangeOption = ETextCaseChange::NoChange;

	if (FindTextBoxWidget.IsValid())
	{
		FindTextBoxWidget->SetText(FText::GetEmpty());
	}
	if (RegexFindTextBoxWidget.IsValid())
	{
		RegexFindTextBoxWidget->SetText(FText::GetEmpty());
	}
	if (ReplaceTextBoxWidget.IsValid())
	{
		ReplaceTextBoxWidget->SetText(FText::GetEmpty());
	}
	if (PrefixTextBoxWidget.IsValid())
	{
		PrefixTextBoxWidget->SetText(FText::GetEmpty());
	}
	if (SufixTextBoxWidget.IsValid())
	{
		SufixTextBoxWidget->SetText(FText::GetEmpty());
	}
	if (StartNumberSpinBoxWidget.IsValid())
	{
		StartNumberSpinBoxWidget->SetValue(DefaultStartNumber);
	}
	if (PaddingDigitsSpinBoxWidget.IsValid())
	{
		PaddingDigitsSpinBoxWidget->SetValue(DefaultPaddingDigits);
	}
}

void SRenamerWidget::ApplyAllRenameRules(bool bNeedRequireRefreshListViewByCurrentFunction)
{
	if (bEnableRegexFind && !RegexFindText.IsEmpty())
	{
		if (!CachedRegexPattern.IsValid() || LastRegexFindText != RegexFindText)
		{
			CachedRegexPattern = MakeShared<FRegexPattern>(RegexFindText);
			LastRegexFindText = RegexFindText;
		}
	}
	else
	{
		CachedRegexPattern.Reset();
		LastRegexFindText = TEXT("");
	}

	int32 CurrentNumber = StartNumber;
	bAnyItemNeedsRename = false;

	for (TSharedPtr<FRenameItem>& Item : RenameItems)
	{
		if (Item.IsValid())
		{
			FString InitialManualName = Item->ManualName;

			Item->ApplyRenameRules(FindText, ReplaceText,
				bEnableRegexFind, RegexFindText, CachedRegexPattern,
				PrefixText, SuffixText,
				bUseNumbering, CurrentNumber, PaddingDigits,
				CaseChangeOption);

			if (!Item->bIsManualOverride)
			{
				Item->ManualName = Item->PreviewName;
			}
			if (bUseNumbering)
			{
				CurrentNumber++;
			}

			if (!Item->GetFinalRenameName().Equals(Item->OriginalName, ESearchCase::CaseSensitive))
			{
				bAnyItemNeedsRename = true;
			}
		}
	}

	if (bNeedRequireRefreshListViewByCurrentFunction)
	{
		if (RenameListView.IsValid())
		{
			RenameListView->RequestListRefresh();
		}
	}
	
	
}

TSharedRef<ITableRow> SRenamerWidget::HandleGenerateRow(TSharedPtr<FRenameItem> InItem,
                                                        const TSharedRef<STableViewBase>& OwnerTable)
{
    TSharedPtr<SHeaderRow> CurrentHeaderRow = OwnerTable->GetHeaderRow();

    if (!CurrentHeaderRow.IsValid())
    {
        return SNew(STableRow<TSharedPtr<FRenameItem>>, OwnerTable)
            .Padding(2)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(0.25f).Padding(FMargin(5))
                    [SNew(STextBlock).Text(this, &SRenamerWidget::GetOriginalNameText, InItem).Font(FAppStyle::GetFontStyle("NormalFont"))]
                + SHorizontalBox::Slot().FillWidth(0.25f).Padding(FMargin(5))
                    [SNew(STextBlock).Text(this, &SRenamerWidget::GetPreviewNameText, InItem).Font(FAppStyle::GetFontStyle("NormalFont"))]
                + SHorizontalBox::Slot().FillWidth(0.20f).Padding(FMargin(5))
                    [SNew(STextBlock).Text(this, &SRenamerWidget::GetItemTypeText, InItem).Font(FAppStyle::GetFontStyle("NormalFont"))]
                + SHorizontalBox::Slot().FillWidth(0.30f).Padding(FMargin(5))
                    [SNew(SEditableTextBox)
                        .Text(this, &SRenamerWidget::GetManualNameText, InItem)
                        .OnTextChanged(this, &SRenamerWidget::OnManualNameTextChanged, InItem)
                        .OnTextCommitted(this, &SRenamerWidget::OnManualNameTextCommitted, InItem)
                        .Font(FAppStyle::GetFontStyle("NormalFont"))]
            ];
    }

    auto GetColumnWidthAttribute = [WeakHeaderRow = TWeakPtr<SHeaderRow>(CurrentHeaderRow)](const FName ColumnId) -> float
    {
        if (TSharedPtr<SHeaderRow> SharedHeaderRow = WeakHeaderRow.Pin())
        {
            for (const SHeaderRow::FColumn& HeaderColumn : SharedHeaderRow->GetColumns())
            {
                if (HeaderColumn.ColumnId == ColumnId)
                {
                    return HeaderColumn.Width.Get(); 
                }
            }
        }
        return 0.f;
    };
    return SNew(STableRow<TSharedPtr<FRenameItem>>, OwnerTable)
        .Padding(2)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(TAttribute<float>::CreateLambda([GetColumnWidthAttribute]() { return GetColumnWidthAttribute(TEXT("OriginalName")); }))
            .Padding(FMargin(5))
            [
                SNew(STextBlock)
                .Text(this, &SRenamerWidget::GetOriginalNameText, InItem)
                .Font(FAppStyle::GetFontStyle("NormalFont"))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(TAttribute<float>::CreateLambda([GetColumnWidthAttribute]() { return GetColumnWidthAttribute(TEXT("PreviewName")); })) 
            .Padding(FMargin(5))
            [
                SNew(STextBlock)
                .Text(this, &SRenamerWidget::GetPreviewNameText, InItem)
                .Font(FAppStyle::GetFontStyle("NormalFont"))
                .ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(TAttribute<float>::CreateLambda([GetColumnWidthAttribute]() { return GetColumnWidthAttribute(TEXT("Type")); }))
            .Padding(FMargin(5))
            [
                SNew(STextBlock)
                .Text(this, &SRenamerWidget::GetItemTypeText, InItem)
                .Font(FAppStyle::GetFontStyle("NormalFont"))
                .ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(TAttribute<float>::CreateLambda([GetColumnWidthAttribute]() { return GetColumnWidthAttribute(TEXT("ManualName")); }))
            .Padding(FMargin(5))
            [
                SNew(SEditableTextBox)
                .Text(this, &SRenamerWidget::GetManualNameText, InItem)
                .OnTextChanged(this, &SRenamerWidget::OnManualNameTextChanged, InItem)
                .OnTextCommitted(this, &SRenamerWidget::OnManualNameTextCommitted, InItem)
                .Font(FAppStyle::GetFontStyle("NormalFont"))
            ]
        ];
}

void SRenamerWidget::OnContentBrowserSelectionChanged(const TArray<FAssetData>& SelectedAssets, bool bFromUser)
{
	UpdateRenameListFromSelection();
}

void SRenamerWidget::OnActorSelectionChanged(UObject* InSelectedActor)
{
	UpdateRenameListFromSelection();
}

void SRenamerWidget::UpdateRenameListFromSelection()
{
	RenameItems.Empty();

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> CurrentSelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(CurrentSelectedAssets);

	for (const FAssetData& Asset : CurrentSelectedAssets)
	{
		RenameItems.Add(MakeShared<FRenameItem>(Asset));
	}

	if (GEditor && GEditor->GetSelectedActors())
	{
		USelection* SelectedActors = GEditor->GetSelectedActors();
		TArray<AActor*> CurrentSelectedActors;
		SelectedActors->GetSelectedObjects<AActor>(CurrentSelectedActors);

		for (AActor* Actor : CurrentSelectedActors) 
		{
			RenameItems.Add(MakeShared<FRenameItem>(Actor));
		}
	}

	RenameItems.Sort([](const TSharedPtr<FRenameItem>& A, const TSharedPtr<FRenameItem>& B)
	{
		return A->OriginalName < B->OriginalName;
	});

	ApplyAllRenameRules();

	if (RenameListView.IsValid())
	{
		RenameListView->RequestListRefresh();
	}
}

bool SRenamerWidget::HasSelection() const
{
	return RenameItems.Num() > 0;
}

bool SRenamerWidget::CanApplyRename() const
{
	return RenameItems.Num() > 0 && bAnyItemNeedsRename;
}

FReply SRenamerWidget::OnApplyRenameButtonClicked()
{
	FScopedTransaction Transaction(LOCTEXT("ApplyRenameTransaction", "Apply Rename"));

	int32 InitialItemsToProcess = 0;
	int32 EstimatedRenamedCount = 0;
	int32 EstimatedFailedCount = 0;

	TArray<FAssetRenameData> AssetsToRename;
	TArray<TTuple<AActor*, FString>> ActorsToRename;

	TMap<FString, FString> OriginalNameToProposedFinalNameMap;

	for (TSharedPtr<FRenameItem>& Item : RenameItems)
	{
		if (!Item.IsValid()) continue;

		const FString CurrentFinalName = Item->GetFinalRenameName();

		if (CurrentFinalName.IsEmpty() || CurrentFinalName.Equals(Item->OriginalName, ESearchCase::CaseSensitive))
		{
			continue; 
		}
		
		OriginalNameToProposedFinalNameMap.Add(Item->OriginalName, CurrentFinalName);

		if (Item->ItemType == ERenameItemType::Asset)
		{
			if (Item->OriginalAssetData.IsValid())
			{
				if (UObject* Asset = Item->OriginalAssetData.GetAsset())
				{
					FString CurrentPackagePath = Item->OriginalAssetData.PackagePath.ToString();
					AssetsToRename.Add(FAssetRenameData(Asset, CurrentPackagePath, CurrentFinalName));
					InitialItemsToProcess++;
				}
				else
				{
					EstimatedFailedCount++;
					UE_LOG(LogTemp, Warning, TEXT("Skipped renaming invalid asset '%s' to '%s'."), *Item->OriginalName, *CurrentFinalName);
				}
			}
		}
		else if (Item->ItemType == ERenameItemType::Actor)
		{
			if (AActor* Actor = Cast<AActor>(Item->OriginalObject.Get()))
			{
				ActorsToRename.Emplace(Actor, CurrentFinalName);
				InitialItemsToProcess++;
			}
			else
			{
				EstimatedFailedCount++;
				UE_LOG(LogTemp, Warning, TEXT("Skipped renaming invalid or deleted actor '%s' to '%s'."), *Item->OriginalName, *CurrentFinalName);
			}
		}
	}

	if (InitialItemsToProcess == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoRenameNeeded", "No items needed renaming (names might be unchanged or no items selected)."));
		return FReply::Handled();
	}

	FScopedSlowTask SlowTask(static_cast<float>(InitialItemsToProcess), LOCTEXT("RenamingAssetsAndActors", "Renaming Assets and Actors..."));
	SlowTask.MakeDialog(false, false);

	if (AssetsToRename.Num() > 0)
	{
		SlowTask.EnterProgressFrame(static_cast<float>(AssetsToRename.Num()), LOCTEXT("ProcessingAssets", "Processing Assets..."));

		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

		if (AssetToolsModule.Get().RenameAssets(AssetsToRename))
		{
			EstimatedRenamedCount += AssetsToRename.Num();
		}
		else
		{
			EstimatedFailedCount += AssetsToRename.Num();
			UE_LOG(LogTemp, Error, TEXT("Some asset renames failed. Check Asset Tools log for more details."));
		}
	}

	for (const auto& ActorData : ActorsToRename)
	{
		SlowTask.EnterProgressFrame(1.0f, LOCTEXT("ProcessingActors", "Processing Actors..."));

		AActor* Actor = ActorData.Get<0>();
		const FString& NewName = ActorData.Get<1>();

		if (Actor && Actor->IsValidLowLevel())
		{
			FActorLabelUtilities::RenameExistingActor(Actor, NewName);
			EstimatedRenamedCount++;
		}
		else
		{
			EstimatedFailedCount++;
			UE_LOG(LogTemp, Warning, TEXT("Skipped renaming actor as it is no longer valid: '%s'"), *NewName);
		}

		SlowTask.TickProgress();
		if (SlowTask.ShouldCancel())
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("RenameCanceled", "Batch rename operation was canceled. Some items might not have been renamed."));
			break; 
		}
	}

	UpdateRenameListFromSelection();

	if (EstimatedRenamedCount > 0 && EstimatedFailedCount == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("RenameSuccessFormat", "Successfully renamed {0} items."), FText::AsNumber(EstimatedRenamedCount)));
	}
	else if (EstimatedRenamedCount > 0 && EstimatedFailedCount > 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::Format(LOCTEXT("RenamePartialSuccessFormat", "Renamed {0} items. Failed to rename {1} items."),
				FText::AsNumber(EstimatedRenamedCount), FText::AsNumber(EstimatedFailedCount)),
			LOCTEXT("RenamePartialSuccess", "Batch Rename Partial Success"));
	}
	else if (EstimatedRenamedCount == 0 && InitialItemsToProcess > 0) // Failed all attempts
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::Format(LOCTEXT("RenameFailedFormat", "Failed to rename {0} items. See log for details."),
				FText::AsNumber(InitialItemsToProcess)),
			LOCTEXT("RenameFailed", "Batch Rename Failed"));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("RenameNoChangeMessage", "No items were renamed (names might be unchanged or no items selected)."),
			LOCTEXT("RenameNoChange", "Batch Rename Info"));
	}
	
	return FReply::Handled();
}

bool SRenamerWidget::GetFindTextBoxIsEnabled() const
{
	return !bEnableRegexFind;
}

bool SRenamerWidget::GetRegexTextBoxIsEnabled() const
{
	return bEnableRegexFind;
}

bool SRenamerWidget::AreRulesModified() const
{
	if (!FindText.IsEmpty()) return true;
	if (!ReplaceText.IsEmpty()) return true;
	if (!PrefixText.IsEmpty()) return true;
	if (!SuffixText.IsEmpty()) return true;
	if (bUseNumbering) return true;
	if (StartNumber != DefaultStartNumber) return true;
	if (PaddingDigits != DefaultPaddingDigits) return true;
	if (CaseChangeOption != ETextCaseChange::NoChange) return true;
	if (!RegexFindText.IsEmpty()) return true;
	if (bEnableRegexFind) return true;

	return false;
}

ECheckBoxState SRenamerWidget::GetEnableRegexCheckBoxState() const
{
	return bEnableRegexFind ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SRenamerWidget::GetEnableUseNumberingCheckBoxState() const
{
	return bUseNumbering ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

FText SRenamerWidget::GetCurrentCaseChangeComboBoxText() const
{
	switch (CaseChangeOption)
	{
	case ETextCaseChange::NoChange:
		return LOCTEXT("CaseChange_NoChange_Current", "No Change");
	case ETextCaseChange::ReplacedToLower:
		return LOCTEXT("CaseChange_ReplacedToLower", "Replaced Part To Lower");
	case ETextCaseChange::ReplacedToUpper:
		return LOCTEXT("CaseChange_ReplacedToUpper", "Replaced Part To Upper");
	case ETextCaseChange::ToLower:
		return LOCTEXT("CaseChange_ToLower_Current", "Whole Name To Lower");
	case ETextCaseChange::ToUpper:
		return LOCTEXT("CaseChange_ToUpper_Current", "Whole Name To Upper");
	default:
		return LOCTEXT("CaseChange_Unknown_Current", "Unknown");
	}
}

FText SRenamerWidget::GetOriginalNameText(TSharedPtr<FRenameItem> InItem) const
{
	return InItem.IsValid() ? FText::FromString(InItem->OriginalName) : FText::GetEmpty();
}

FText SRenamerWidget::GetPreviewNameText(TSharedPtr<FRenameItem> InItem) const
{
	return InItem.IsValid() ? FText::FromString(InItem->PreviewName) : FText::GetEmpty();
}

FText SRenamerWidget::GetItemTypeText(TSharedPtr<FRenameItem> InItem) const
{
	if (!InItem.IsValid())
	{
		return FText::GetEmpty();
	}

	switch (InItem->ItemType)
	{
	case ERenameItemType::Asset:
		return LOCTEXT("ItemType_Asset", "Asset");
	case ERenameItemType::Actor:
		return LOCTEXT("ItemType_Actor", "Actor");
	default:
		return LOCTEXT("ItemType_Unknown", "Unknown");
	}
}

FText SRenamerWidget::GetManualNameText(TSharedPtr<FRenameItem> InItem) const
{
	return InItem.IsValid() ? FText::FromString(InItem->ManualName) : FText::GetEmpty();
}

void SRenamerWidget::OnFindTextChanged(const FText& NewText)
{
	FindText = FString(NewText.ToString());
	ApplyAllRenameRules();
}

void SRenamerWidget::OnEnableRegexCheckBoxChanged(ECheckBoxState NewState)
{
	bEnableRegexFind = (NewState == ECheckBoxState::Checked);
    
	if (bEnableRegexFind)
	{
		FindText = FString(TEXT(""));
		if (FindTextBoxWidget.IsValid())
		{
			FindTextBoxWidget->SetText(FText::GetEmpty());
		}
	}
	else
	{
		RegexFindText = FString(TEXT(""));
		if (RegexFindTextBoxWidget.IsValid())
		{
			RegexFindTextBoxWidget->SetText(FText::GetEmpty());
		}
	}

	ApplyAllRenameRules();
}

void SRenamerWidget::OnRegexFindTextChanged(const FText& NewText)
{
	RegexFindText = FString(NewText.ToString());
	if (LastRegexFindText != RegexFindText)
	{
		CachedRegexPattern.Reset();
		LastRegexFindText = FString(RegexFindText);
	}
	ApplyAllRenameRules();
}

void SRenamerWidget::OnReplaceTextChanged(const FText& NewText)
{
	ReplaceText = FString(NewText.ToString());
	ApplyAllRenameRules();
}

void SRenamerWidget::OnPrefixTextChanged(const FText& NewText)
{
	PrefixText = FString(NewText.ToString());
	ApplyAllRenameRules();
}

void SRenamerWidget::OnSuffixTextChanged(const FText& NewText)
{
	SuffixText = FString(NewText.ToString());
	ApplyAllRenameRules();
}

void SRenamerWidget::OnUseNumberingChanged(ECheckBoxState NewState)
{
	bUseNumbering = (NewState == ECheckBoxState::Checked);
	ApplyAllRenameRules();
}

void SRenamerWidget::OnStartNumberChanged(int32 NewValue)
{
	StartNumber = NewValue;
	ApplyAllRenameRules();
}

void SRenamerWidget::OnPaddingDigitsChanged(int32 NewValue)
{
	PaddingDigits = NewValue;
	ApplyAllRenameRules();
}

void SRenamerWidget::OnCaseChangeOptionChanged(TSharedPtr<ETextCaseChange> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid())
	{
		CaseChangeOption = *NewSelection;
		ApplyAllRenameRules();
	}
}

FReply SRenamerWidget::OnUndoLastRenameClicked()
{
	if (GEditor && GEditor->Trans)
	{
		GEditor->Trans->Undo();
		UpdateRenameListFromSelection(); 
	}
	return FReply::Handled();
}

bool SRenamerWidget::CanUndoLastRename() const
{
	return GEditor && GEditor->Trans && GEditor->Trans->CanUndo();
}

FReply SRenamerWidget::OnResetAllRulesClicked()
{
	ResetAllRulesInternal();
	ApplyAllRenameRules(true); 
    
	for (TSharedPtr<FRenameItem>& Item : RenameItems)
	{
		if (Item.IsValid())
		{
			Item->bIsManualOverride = false;
		}
	}

	return FReply::Handled();
}

void SRenamerWidget::OnManualNameTextChanged(const FText& NewText, TSharedPtr<FRenameItem> InItem)
{
	if (InItem.IsValid())
	{
		InItem->ManualName = FString(NewText.ToString());
	}
}

void SRenamerWidget::OnManualNameTextCommitted(const FText& NewText, ETextCommit::Type CommitType,
	TSharedPtr<FRenameItem> InItem)
{
	if (InItem.IsValid())
	{
		InItem->ManualName = FString(NewText.ToString());
		InItem->bIsManualOverride = true; 
		ApplyAllRenameRules();
	}
}

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
