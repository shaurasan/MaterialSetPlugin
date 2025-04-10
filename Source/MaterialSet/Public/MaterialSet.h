// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

#include "Materials/MaterialInterface.h"


class FMaterialSetModule : public IModuleInterface
{
private:
	TSharedPtr<FExtender> Extender;

	TWeakPtr<SWindow> RootWindow;

	TWeakPtr<SWindow> MyWindow;

	TArray<TSharedPtr<UMaterialInterface>> MaterialOptions;

	TWeakObjectPtr<UMaterialInterface> SelectedMaterial;

	TSharedPtr<FAssetThumbnailPool> ThumbnailPool;

	UPROPERTY()
	TObjectPtr<UTexture2D> SelectedAlbedo;
	UPROPERTY()
	TObjectPtr<UTexture2D> SelectedNormal;
	UPROPERTY()
	TObjectPtr<UTexture2D> SelectedRoughness;

public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	void OnWindowMenuExtension(FMenuBuilder & MenuBuilder);

	void OnMyToolMenu();

	void OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow);

	void PopulateMaterialOptions();

};
