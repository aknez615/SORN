// Copyright 2025 Sentinel Games All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "SORNGameSettings.generated.h"

/**
* Custom User Settings class that extends the engine's built-in graphics settings
 * to include Audio and Gameplay options.
 * Automatically saves to GameUserSettings.ini.
 */

UENUM(BlueprintType)
enum class ETextSize : uint8
{
	Small UMETA(DisplayName="Small"),
	Regular UMETA(DisplayName="Regular"),
	Large UMETA(DisplayName="Large"),
	ExtraLarge UMETA(DisplayName="ExtraLarge")
};

UCLASS(BlueprintType, config=GameUserSettings)
class SORN_API USORNGameSettings : public UGameUserSettings
{
	GENERATED_BODY()
	
public:
	USORNGameSettings();

	// Helper to get the correct instance
	static USORNGameSettings* Get();

	// --- Audio Settings (0.0 to 1.0) ---
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MasterVolume = 1.0f;

	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MusicVolume = 0.8f;

	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float SFXVolume = 1.0f;
	
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float VoiceVolume = 1.0f;
	
	// --- Gameplay Settings ---
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float MouseSensitivity = 1.0f;
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float ControllerSensitivity = 1.0f;
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta=(ClampMin="0.0", ClampMax="0.8"))
	float ControllerDeadzone = 0.15f;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta=(ClampMin="1.0", ClampMax="3.0"))
	float ControllerExponent = 1.7f;
	
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bInvertYAxis = false;

	// --- Accessibility Settings ---
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta=(ClampMin="60.0", ClampMax="120.0"))
	float FOVSettings = 90.0f;
	
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	float Brightness = 1.0f;
	
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	bool bSubtitles = true;
	
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	bool bCameraShake = true;
	
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void SetSubtitlesEnabled(bool bEnable);
	
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void SetCameraShakeEnabled(bool bEnable);
	
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void SetTextSize(ETextSize InSize);
	
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void SetBrightness(float InBrightness);
	
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void SetFOV(float InFOV);
	
	UPROPERTY()
	UMaterialInstanceDynamic* BrightnessMID;
	
	UPROPERTY()
	UMaterialInterface* BrightnessBaseMaterial;
	
	//For all camera shakes if (USORNGameSettings::Get()->bCameraShake needs to be in it
	UFUNCTION(BlueprintPure)
	static bool CanPlayCameraShake()
	{
		return Get()->bCameraShake;
	}
	
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	ETextSize TextSize = ETextSize::Regular;
	
	UFUNCTION(BlueprintPure, Category = "Accessibility")
	float GetTextScale() const;
	
	// Overriding ApplySettings to ensure our custom logic runs when settings are applied.
	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;
	virtual void SetToDefaults() override;
};
