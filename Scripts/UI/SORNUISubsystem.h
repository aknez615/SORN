// Copyright 2025 Sentinel Games All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SORNUISubsystem.generated.h"

class USORNUIRoot;
class UCommonActivatableWidget;
/**
 * 
 */
UCLASS()
class SORN_API USORNUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// --- Registration ---
    
	// Called by the Root Widget when it loads
	void RegisterLayout(USORNUIRoot* NewLayout);
    
	// Called by the Root Widget when it is destroyed
	void UnregisterLayout(USORNUIRoot* LayoutToRemove);

	// --- Public API for Blueprints ---

	UFUNCTION(BlueprintCallable, Category = "SORN|UI")
	UCommonActivatableWidget* PushHUDWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass);
	
	UFUNCTION(BlueprintCallable, Category = "SORN|UI")
	UCommonActivatableWidget* PushMenuWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "SORN|UI")
	UCommonActivatableWidget* PushPromptWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "SORN|UI")
	void ClearHUDWidgetStack();

	UFUNCTION(BlueprintCallable, Category = "SORN|UI")
	void ClearMenuWidgetStack();

	UFUNCTION(BlueprintCallable, Category = "SORN|UI")
	void ClearPromptWidgetStack();
	
	UFUNCTION(BlueprintCallable, Category = "SORN|UI")
	UCommonActivatableWidgetStack* GetMenuStack() const;
	
	UFUNCTION(BlueprintCallable, Category = "SORN|UI")
	void SaveMousePosition();
	
	UFUNCTION(BlueprintCallable, Category = "SORN|UI")
	void RestoreMousePosition();


private:
	UPROPERTY()
	TObjectPtr<USORNUIRoot> ActiveLayout;
	
	float SavedMouseX = 0.0f;
	float SavedMouseY = 0.0f;
};
