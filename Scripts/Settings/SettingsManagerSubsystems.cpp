// Copyright 2025 Sentinel Games All rights reserved.


#include "SettingsManagerSubsystems.h"

#include "SORNGameSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

void USettingsManagerSubsystems::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	MasterSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/SORN/Characters/Player/GameplayCues/Sound/SoundMixers/SC_Master.SC_Master"));
	MusicSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Engine/EngineSounds/Music.Music"));
	SFXSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/SORN/Characters/Player/GameplayCues/Sound/SoundMixers/SFXMix/SC_SFX.SC_SFX"));
	VoiceSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/SORN/Characters/Player/GameplayCues/Sound/SoundMixers/SC_Voicelines.SC_Voicelines"));
	// You need ONE SoundMix asset in your project to act as the "carrier" for overrides.
	MainSoundMixModifier = LoadObject<USoundMix>(nullptr, TEXT("/Game/SORN/Characters/Player/GameplayCues/Sound/SoundMixers/Mix_MainMix.Mix_MainMix"));

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &USettingsManagerSubsystems::OnMapLoaded);
	FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &USettingsManagerSubsystems::OnWorldInitializedActors);

	// On startup: load persisted values from disk then apply them to the engine.
	LoadAllSettings();
	ApplyAllSettings();
}

void USettingsManagerSubsystems::ApplyAllSettings()
{
	USORNGameSettings* Settings = USORNGameSettings::Get();
	if (Settings)
	{
		// Apply current in-memory settings to the engine. Do NOT load from disk here; callers
		// (e.g., UI Apply button) typically want to apply their current adjustments without
		// overwriting them with disk values.
		Settings->ApplySettings(false);

		UE_LOG(LogTemp, Log, TEXT("SettingsManager: Applied settings - Master=%.3f Music=%.3f SFX=%.3f Voice=%.3f MouseSens=%.3f ControllerSens=%.3f InvertY=%d"),
			Settings->MasterVolume, Settings->MusicVolume, Settings->SFXVolume, Settings->VoiceVolume,
			Settings->MouseSensitivity, Settings->ControllerSensitivity, Settings->bInvertYAxis ? 1 : 0);
	}
	UpdateAudioDevice();
	
	//ApplyAccessibilitySettings();
}

// Explicit save/load helpers so UI or callers can control when the values are persisted to disk.
void USettingsManagerSubsystems::SaveAllSettings()
{
	if (USORNGameSettings* Settings = USORNGameSettings::Get())
	{
		// Save current in-memory settings to the GameUserSettings INI on disk.
		Settings->SaveSettings();
		UE_LOG(LogTemp, Log, TEXT("SettingsManager: Settings saved to disk."));
	}
}

void USettingsManagerSubsystems::LoadAllSettings()
{
	if (USORNGameSettings* Settings = USORNGameSettings::Get())
	{
		// Load settings from disk into memory. Does not automatically apply them to the engine
		// unless ApplyAllSettings() is called afterwards.
		Settings->LoadSettings(false);
		UE_LOG(LogTemp, Log, TEXT("SettingsManager: Loaded settings - Master=%.3f Music=%.3f SFX=%.3f Voice=%.3f MouseSens=%.3f ControllerSens=%.3f InvertY=%d"),
			Settings->MasterVolume, Settings->MusicVolume, Settings->SFXVolume, Settings->VoiceVolume,
			Settings->MouseSensitivity, Settings->ControllerSensitivity, Settings->bInvertYAxis ? 1 : 0);
		
		if (!GConfig->DoesSectionExist(TEXT("/Script/SORN.SORNGameSettings"), GGameUserSettingsIni))
		{
			UE_LOG(LogTemp, Log, TEXT("First run, applying defaults"));
			
			Settings->SetToDefaults();
			Settings->SaveSettings();
		}
	}
}

void USettingsManagerSubsystems::SetControllerDeadzone(float Deadzone)
{
	if (USORNGameSettings* Settings = USORNGameSettings::Get())
	{
		Settings->ControllerDeadzone = FMath::Clamp(Deadzone, 0.0f, 0.8f);
	}
}

float USettingsManagerSubsystems::GetControllerDeadzone() const
{
	if (USORNGameSettings* Settings = USORNGameSettings::Get())
	{
		return Settings->ControllerDeadzone;
	}
	return 0.15f;
}

void USettingsManagerSubsystems::SetMasterVolume(float Volume)
{
	if (USORNGameSettings* Settings = USORNGameSettings::Get())
	{
		Settings->MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
		// Do not auto-save to disk here. Let UI call SaveAllSettings() explicitly when the user confirms.
		UpdateAudioDevice();
	}
}

float USettingsManagerSubsystems::GetMasterVolume() const
{
	USORNGameSettings* Settings = USORNGameSettings::Get();
	if (Settings)
	{
		Settings->LoadSettings();
		return  Settings->MasterVolume;
	}
	return 1;
}

void USettingsManagerSubsystems::SetMusicVolume(float Volume)
{
	if (USORNGameSettings* Settings = USORNGameSettings::Get())
	{
		Settings->MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
		// Intentionally not saving to disk here; call SaveAllSettings() explicitly elsewhere.
		UpdateAudioDevice();
	}
}

float USettingsManagerSubsystems::GetMusicVolume() const
{
	USORNGameSettings* Settings = USORNGameSettings::Get();
	if (Settings)
	{
		Settings->LoadSettings();
		return  Settings->MusicVolume;
	}
	return 1;
}

void USettingsManagerSubsystems::SetVoiceVolume(float Volume)
{
	if (USORNGameSettings* Settings = USORNGameSettings::Get())
	{
		Settings->VoiceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
		// Intentionally not saving here.
		UpdateAudioDevice();
	}
}

float USettingsManagerSubsystems::GetVoiceVolume() const
{
	USORNGameSettings* Settings = USORNGameSettings::Get();
	if (Settings)
	{
		Settings->LoadSettings();
		return  Settings->VoiceVolume;
	}
	return 1;
}

void USettingsManagerSubsystems::SetSFXVolume(float Volume)
{
	if (USORNGameSettings* Settings = USORNGameSettings::Get())
	{
		Settings->SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
		// Intentionally not saving here.
		UpdateAudioDevice();
	}
}

float USettingsManagerSubsystems::GetSFXVolume() const
{
	USORNGameSettings* Settings = USORNGameSettings::Get();
	if (Settings)
	{
		Settings->LoadSettings();
		return  Settings->SFXVolume;
	}
	return 1;
}

void USettingsManagerSubsystems::UpdateAudioDevice(UWorld* World)
{ 
	USORNGameSettings* Settings = USORNGameSettings::Get();
	if (!Settings || !MainSoundMixModifier) return;

	World = World ? World : GetWorld();
	if (!World) return;
	
	UGameplayStatics::PushSoundMixModifier(World, MainSoundMixModifier);

	
	if (MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MainSoundMixModifier, MasterSoundClass, Settings->MasterVolume, 1.0f, 0.0f);
	}

	if (MusicSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MainSoundMixModifier, MusicSoundClass, Settings->MusicVolume, 1.0f, 0.0f);
	}

	if (SFXSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MainSoundMixModifier, SFXSoundClass, Settings->SFXVolume, 1.0f, 0.0f);
	}

	if (VoiceSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MainSoundMixModifier, VoiceSoundClass, Settings->VoiceVolume, 1.0f, 0.0f);
	}
}

void USettingsManagerSubsystems::OnMapLoaded(UWorld* World, FWorldInitializationValues WorldInitializationValues)
{
	if (World && World->IsGameWorld())
	{
		LoadAllSettings();
		
		UpdateAudioDevice(World);
		ApplyAllSettings();
	}
}

void USettingsManagerSubsystems::OnWorldInitializedActors(const FActorsInitializedParams& ActorsInitializedParams)
{
	if (ActorsInitializedParams.World && ActorsInitializedParams.World->IsGameWorld())
	{
		LoadAllSettings();
		UpdateAudioDevice(ActorsInitializedParams.World);
		ApplyAllSettings();
	}
}

/*void USettingsManagerSubsystems::ApplyAccessibilitySettings(UWorld* World)
{
	USORNGameSettings* Settings = USORNGameSettings::Get();
	if (!Settings) return;
	
	World = World ? World : GetWorld();
	if (!World) return;
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC || !PC->PlayerCameraManager) return;
	
	//--- FOV ---
	PC->PlayerCameraManager->SetFOV(Settings->FOVSettings);
	
	//--- Camera Shake ---
	if (!Settings->bCameraShake)
	{
		PC->PlayerCameraManager->StopAllCameraShakes((true));
	}
	
	//--- Brightness ---
	FPostProcessSettings& PPS = PC->PlayerCameraManager->ViewTarget.POV.PostProcessSettings;
	
	//Lock exposure
	PPS.bOverride_AutoExposureMinBrightness = true;
	PPS.bOverride_AutoExposureMaxBrightness = true;
	
	PPS.AutoExposureMinBrightness = 1.0f;
	PPS.AutoExposureMaxBrightness = 1.0f;
	
	//Exposure Bias instead of Gamma
	PPS.bOverride_AutoExposureBias = true;
	PPS.AutoExposureBias = Settings->Brightness;
	
	UMaterialInterface* SelectedMat = nullptr;
	
	if (SelectedMat)
	{
		PPS.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, SelectedMat));
	}
}*/
