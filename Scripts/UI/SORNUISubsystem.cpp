// Copyright 2025 Sentinel Games All rights reserved.


#include "SORNUISubsystem.h"

#include "SORNUIRoot.h"

void USORNUISubsystem::RegisterLayout(USORNUIRoot* NewLayout)
{
	ActiveLayout = NewLayout;
}

void USORNUISubsystem::UnregisterLayout(USORNUIRoot* LayoutToRemove)
{
	if (ActiveLayout == LayoutToRemove)
	{
		ActiveLayout = nullptr;
	}
}

UCommonActivatableWidget* USORNUISubsystem::PushHUDWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (ActiveLayout)
	{
		return ActiveLayout->PushWidgetToStack(WidgetClass, ESORNUIStack::ESUS_HUD);
	}
	return nullptr;
}

UCommonActivatableWidget* USORNUISubsystem::PushMenuWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	SaveMousePosition();
	
	UCommonActivatableWidget* Result = nullptr;
	
	if (ActiveLayout)
	{
		Result = ActiveLayout->PushWidgetToStack(WidgetClass, ESORNUIStack::ESUS_Menu);
	}
	
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeUIOnly());
	}
	
	RestoreMousePosition();
	
	return Result;
}

UCommonActivatableWidget* USORNUISubsystem::PushPromptWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (ActiveLayout)
	{
		return ActiveLayout->PushWidgetToStack(WidgetClass, ESORNUIStack::ESUS_Prompt);
	}
	return nullptr;
}

void USORNUISubsystem::ClearHUDWidgetStack()
{
	if (ActiveLayout)
	{
		ActiveLayout->ClearStack(ESORNUIStack::ESUS_HUD);
	}
}

void USORNUISubsystem::ClearMenuWidgetStack()
{
	if (ActiveLayout)
	{
		ActiveLayout->ClearStack(ESORNUIStack::ESUS_Menu);
	}
}

void USORNUISubsystem::ClearPromptWidgetStack()
{
	if (ActiveLayout)
	{
		ActiveLayout->ClearStack(ESORNUIStack::ESUS_Prompt);
	}
}

UCommonActivatableWidgetStack* USORNUISubsystem::GetMenuStack() const
{
    if (ActiveLayout)
    {
        return ActiveLayout->GetMenuStack();
    }
    return nullptr;
}

void USORNUISubsystem::SaveMousePosition()
{
	if (!GetWorld()) return;
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;
	
	float X, Y;
	if (PC->GetMousePosition(X, Y))
	{
		SavedMouseX = X;
		SavedMouseY = Y;
	}
}

void USORNUISubsystem::RestoreMousePosition()
{
	if (!GetWorld()) return;
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;
	
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(
		Handle,
		[PC, this]()
		{
			PC->SetMouseLocation(
				FMath::RoundToInt(SavedMouseX),
				FMath::RoundToInt(SavedMouseY)
			);
		},
		0.01f,
		false
	);
}