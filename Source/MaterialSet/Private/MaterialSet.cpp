// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaterialSet.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/UIAction.h"
#include "Interfaces/IMainFrameModule.h"

#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"

#include "Materials/MaterialInterface.h"

#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PropertyCustomizationHelpers.h"

#include "Engine/Texture2D.h"

#include "MakeNode.h"

#include "TimerManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "FMaterialSetModule"

void FMaterialSetModule::StartupModule()
{
	if(IsRunningCommandlet()){return;}
	Extender = MakeShareable(new FExtender);
	Extender -> AddMenuExtension
	(
		"LevelEditor",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FMaterialSetModule::OnWindowMenuExtension)
	);

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(Extender);

	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
	MainFrameModule.OnMainFrameCreationFinished().AddRaw(this, &FMaterialSetModule::OnMainFrameLoad);


}

void FMaterialSetModule::ShutdownModule()
{
	if(Extender.IsValid() && FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule &LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(Extender);
	}
	
	if(FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		MainFrameModule.OnMainFrameCreationFinished().RemoveAll(this);
	}


}

void FMaterialSetModule::OnWindowMenuExtension(FMenuBuilder & MenuBuilder)
{
	MenuBuilder.BeginSection("MyMenuHook", LOCTEXT("MyMenu", "CustomPlugin"));
	MenuBuilder.AddMenuEntry(
		LOCTEXT("MyMenuTitle", "SetMaterial"),
		LOCTEXT("MyMenuToolTip", "マテリアルにテクスチャを自動で割り当てます"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FMaterialSetModule::OnMyToolMenu))
	);
	MenuBuilder.EndSection();
}

void FMaterialSetModule::OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow)
{
	if((!bIsNewProjectWindow) && (InRootWindow.IsValid()))
	{
		RootWindow = InRootWindow;
	}
}


void FMaterialSetModule::OnMyToolMenu()
{
	if(!MyWindow.IsValid())
	{

		SelectedMaterial.Reset();
		SelectedAlbedo = nullptr;
		SelectedNormal = nullptr;
		SelectedRoughness = nullptr;

		TSharedPtr<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("MyWindow", "MyPlugin"))
		.ClientSize(FVector2D(400.f, 450.f));
		MyWindow = TWeakPtr<SWindow>(Window);
		Window -> SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>&){
			MyWindow.Reset();
		}));
		if(RootWindow.IsValid())
		{
			FSlateApplication::Get().AddWindowAsNativeChild(
				Window.ToSharedRef(),
				RootWindow.Pin().ToSharedRef()
			);
		}

		//----------------表示項目

		ThumbnailPool = MakeShareable(new FAssetThumbnailPool(32));

		Window -> SetContent(
			SNew(SVerticalBox)
			//-------------マテリアル-------------
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[				
				SNew(SHorizontalBox)
					
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 5.0, 0)
				[	
					SNew(SBorder)
					.Padding(FMargin(5))
					.VAlign(VAlign_Center)
					.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.Content()
					[
						SNew(STextBlock)
						.Text(FText::FromString("Material"))
						.MinDesiredWidth(80)
					]
	
				]
	
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SBorder)
					.Padding(FMargin(5))
					.VAlign(VAlign_Center)
					.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.Content()
					[

						SNew(SObjectPropertyEntryBox)
						.AllowedClass(UMaterialInterface::StaticClass())
						.ObjectPath_Lambda([this]() -> FString {
							return SelectedMaterial.IsValid() ? SelectedMaterial -> GetPathName() : FString();
						})
						.OnObjectChanged_Lambda([this](const FAssetData& AssetData) {
							UMaterialInterface* NewMaterial = Cast<UMaterialInterface>(AssetData.GetAsset());
							if (NewMaterial)
							{
								SelectedMaterial = NewMaterial;
							}
						})
						.AllowClear(true)
						.ThumbnailPool(ThumbnailPool.ToSharedRef())
	
					]
	

				]
					

				
			]
			//-------------ベースカラー-------------
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 5.0, 0)
				[
					SNew(SBorder)
					.Padding(FMargin(5))
					.VAlign(VAlign_Center)
					.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.Content()
					[
						SNew(STextBlock)
						.Text(FText::FromString("BaseColor"))
						.MinDesiredWidth(80)
					]
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SBorder)
					.Padding(FMargin(5))
					.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.Content()
					[
						SNew(SObjectPropertyEntryBox)
						.AllowedClass(UTexture2D::StaticClass())
						.ObjectPath_Lambda([this]() -> FString {
							return IsValid(SelectedAlbedo) ? SelectedAlbedo -> GetPathName() : FString();
						})
						.OnObjectChanged_Lambda([this](const FAssetData& AssetData) {
							UTexture2D* NewTexture = Cast<UTexture2D>(AssetData.GetAsset());
							if (NewTexture)
							{
								SelectedAlbedo = NewTexture;
							}
						})
						.AllowClear(true)
						.ThumbnailPool(ThumbnailPool.ToSharedRef())
					]
				]


			]

			//-------------ノーマル-------------
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 5.0, 0)
				[
					SNew(SBorder)
					.Padding(FMargin(5))
					.VAlign(VAlign_Center)
					.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.Content()
					[
						SNew(STextBlock)
						.Text(FText::FromString("Normal"))
						.MinDesiredWidth(80)
					]
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SBorder)
					.Padding(FMargin(5))
					.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.Content()
					[
						SNew(SObjectPropertyEntryBox)
						.AllowedClass(UTexture2D::StaticClass())
						.ObjectPath_Lambda([this]() -> FString {
							return IsValid(SelectedNormal) ? SelectedNormal -> GetPathName() : FString();
						})
						.OnObjectChanged_Lambda([this](const FAssetData& AssetData) {
							UTexture2D* NewTexture = Cast<UTexture2D>(AssetData.GetAsset());
							if (NewTexture)
							{
								SelectedNormal = NewTexture;
							}
						})
						.AllowClear(true)
						.ThumbnailPool(ThumbnailPool.ToSharedRef())
					]
				]
			]

			//-------------ラフネス-------------
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 5.0, 0)
				[
					SNew(SBorder)
					.Padding(FMargin(5))
					.VAlign(VAlign_Center)
					.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.Content()
					[
						SNew(STextBlock)
						.Text(FText::FromString("Roughness"))
						.MinDesiredWidth(80)
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SBorder)
					.Padding(FMargin(5))
					.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.Content()
					[
						SNew(SObjectPropertyEntryBox)
						.AllowedClass(UTexture2D::StaticClass())
						.ObjectPath_Lambda([this]() -> FString {
							return IsValid(SelectedRoughness) ? SelectedRoughness -> GetPathName() : FString();
						})
						.OnObjectChanged_Lambda([this](const FAssetData& AssetData) {
							UTexture2D* NewTexture = Cast<UTexture2D>(AssetData.GetAsset());
							if (NewTexture)
							{
								SelectedRoughness = NewTexture;
							}
						})
						.AllowClear(true)
						.ThumbnailPool(ThumbnailPool.ToSharedRef())
					]
				]

			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10)
			.HAlign(HAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5)
				[
					SNew(SButton)
					.Text(LOCTEXT("Execute", "実行"))
					.OnClicked_Lambda([this]() -> FReply {
						FMakeNode::CreateTextureSampleNode(
							Cast<UMaterial>(SelectedMaterial.Get()),
							SelectedAlbedo.Get(),
							SelectedNormal.Get(),
							SelectedRoughness.Get()
						);
						
						FNotificationInfo Info(FText::FromString(TEXT("適用しました")));
						Info.ExpireDuration = 2.0f;
						FSlateNotificationManager::Get().AddNotification(Info);
	
						if (MyWindow.IsValid()){
							MyWindow.Pin() -> RequestDestroyWindow();
						}
	
						return FReply::Handled();
					})
				]
	
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5)
				[
					SNew(SButton)
					.Text(LOCTEXT("Next", "次へ"))
					.OnClicked_Lambda([this]() -> FReply {
						FMakeNode::CreateTextureSampleNode(
							Cast<UMaterial>(SelectedMaterial.Get()),
							SelectedAlbedo.Get(),
							SelectedNormal.Get(),
							SelectedRoughness.Get()
						);

						if (MyWindow.IsValid()) {
							MyWindow.Pin() -> RequestDestroyWindow();
						}
	
						FTimerHandle TimerHandle;
						GEditor -> GetTimerManager() -> SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]() {
							this -> OnMyToolMenu();
						}), 0.1f, false);

						FNotificationInfo Info(FText::FromString(TEXT("適用しました")));
						Info.ExpireDuration = 2.0f;
						FSlateNotificationManager::Get().AddNotification(Info);
	
						return FReply::Handled();
					})
				]
			]
			
		);


	}
	if (MyWindow.IsValid())
	{
		MyWindow.Pin() -> BringToFront();
	}
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMaterialSetModule, MaterialSet)