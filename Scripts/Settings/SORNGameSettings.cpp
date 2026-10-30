// Copyright 2025 Sentinel Games All rights reserved.


#include "SORNGameSettings.h"
#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"

#include "SettingsManagerSubsystems.h"

USORNGameSettings::USORNGameSettings()
{
	MasterVolume = 1.0f;
	MusicVolume = 0.60f;
	SFXVolume = 1.0f;
	MouseSensitivity = 1.0f;
	ControllerSensitivity = 1.0f;
	ControllerDeadzone = 0.15f;
	bInvertYAxis = false;
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(TEXT("/Game/SORN/Materials/PostProcess/MI_Brightness_PP"));
	
	if (MatFinder.Succeeded())
	{
		BrightnessBaseMaterial = MatFinder.Object;
	}
}

USORNGameSettings* USORNGameSettings::Get()
{
	return Cast<USORNGameSettings>(UGameUserSettings::GetGameUserSettings());
}

void USORNGameSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
	Super::ApplySettings(bCheckForCommandLineOverrides);
	
	//Engine only
	if (GEngine)
	{
		GEngine->bSubtitlesEnabled = bSubtitles;
	}
	
	UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
	if (!World)
	{
		SaveSettings();
		return;
	}
	
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		APostProcessVolume* PPV = *It;
		
		if (PPV && PPV->bUnbound)
		{
			if (!BrightnessMID && BrightnessBaseMaterial)
			{
				BrightnessMID = UMaterialInstanceDynamic::Create(BrightnessBaseMaterial, this);
				
				PPV->Settings.AddBlendable(BrightnessMID, 1.0f);
			}
			
			if (BrightnessMID)
			{
				BrightnessMID->SetScalarParameterValue(TEXT("Brightness"), Brightness);
			}
			
			break;
		}
	}
	
	//Moving Accessibility Apply Here
	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->SetFOV(FOVSettings);
			PC->PlayerCameraManager->DefaultFOV = FOVSettings;
			
			if (!bCameraShake)
			{
				PC->PlayerCameraManager->StopAllCameraShakes(true);
			}
		}
	}

	SaveSettings();
}

void USORNGameSettings::SetToDefaults()
{
	Super::SetToDefaults();
	
	//---Audio---
	MasterVolume = 1.0f;
	MusicVolume = 0.75f;
	SFXVolume = 1.0f;
	VoiceVolume = 1.0f;
	
	//---Gameplay---
	MouseSensitivity = 1.0f;
	ControllerSensitivity = 1.0f;
	ControllerDeadzone = 0.15;
	ControllerExponent = 1.7f;;
	bInvertYAxis = false;
	
	//---Accessibility---
	FOVSettings = 90.0f;
	Brightness = 1.0f;
	bSubtitles = true;
	bCameraShake = true;
	TextSize = ETextSize::Regular;
}

float USORNGameSettings::GetTextScale() const
{
	switch (TextSize)
	{
	case ETextSize::Small: return 0.8f;
	case ETextSize::Regular: return 1.0f;
	case ETextSize::Large: return 1.3f;
	case ETextSize::ExtraLarge: return 1.6f;
	}
	return 1.0f;
}

void USORNGameSettings::SetSubtitlesEnabled(bool bEnable)
{
	bSubtitles = bEnable;
}

void USORNGameSettings::SetCameraShakeEnabled(bool bEnable)
{
	bCameraShake = bEnable;
}

void USORNGameSettings::SetTextSize(ETextSize InSize)
{
	TextSize = InSize;
}

void USORNGameSettings::SetBrightness(float InBrightness)
{
	Brightness = FMath::Clamp(InBrightness, 0.5f, 2.0f);
	
	ApplySettings(false);
}

void USORNGameSettings::SetFOV(float InFOV)
{
	FOVSettings = FMath::Clamp(InFOV, 60.0f, 120.0f);
}
