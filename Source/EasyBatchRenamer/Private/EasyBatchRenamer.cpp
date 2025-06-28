// Copyright Devil Dev, 2025.

#include "EasyBatchRenamer.h"
#include "EasyBatchRenamerStyle.h"
#include "EasyBatchRenamerCommands.h"
#include "LevelEditor.h"
#include "SRenamerWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName EasyBatchRenamerTabName("EasyBatchRenamer");

#define LOCTEXT_NAMESPACE "FEasyBatchRenamerModule"

void FEasyBatchRenamerModule::StartupModule()
{
	FEasyBatchRenamerStyle::Initialize();
	FEasyBatchRenamerStyle::ReloadTextures();

	FEasyBatchRenamerCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FEasyBatchRenamerCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FEasyBatchRenamerModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FEasyBatchRenamerModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(EasyBatchRenamerTabName, FOnSpawnTab::CreateRaw(this, &FEasyBatchRenamerModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FEasyBatchRenamerTabTitle", "EasyBatchRenamer"))
		.SetMenuType(ETabSpawnerMenuType::Hidden)
		.SetIcon(FSlateIcon(FEasyBatchRenamerStyle::GetStyleSetName(), "EasyBatchRenamer.OpenPluginWindow"));
}

void FEasyBatchRenamerModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FEasyBatchRenamerStyle::Shutdown();

	FEasyBatchRenamerCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(EasyBatchRenamerTabName);
}

TSharedRef<SDockTab> FEasyBatchRenamerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SRenamerWidget)
		];
}

void FEasyBatchRenamerModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(EasyBatchRenamerTabName);
}

void FEasyBatchRenamerModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("Tools");
			Section.AddMenuEntryWithCommandList(FEasyBatchRenamerCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FEasyBatchRenamerCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEasyBatchRenamerModule, EasyBatchRenamer)