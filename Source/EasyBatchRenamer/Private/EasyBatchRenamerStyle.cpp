// Copyright Devil Dev, 2025.

#include "EasyBatchRenamerStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FEasyBatchRenamerStyle::StyleInstance = nullptr;

void FEasyBatchRenamerStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FEasyBatchRenamerStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FEasyBatchRenamerStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("EasyBatchRenamerStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FEasyBatchRenamerStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("EasyBatchRenamerStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("EasyBatchRenamer")->GetBaseDir() / TEXT("Resources"));

	Style->Set("EasyBatchRenamer.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

void FEasyBatchRenamerStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FEasyBatchRenamerStyle::Get()
{
	return *StyleInstance;
}
