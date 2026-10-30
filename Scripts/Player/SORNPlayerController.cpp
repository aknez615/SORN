// Copyright 2025 Sentinel Games All rights reserved.


#include "SORNPlayerController.h"

#include "DefaultLevelSequenceInstanceData.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MechPlayerState.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "TaggedActorManagerSubsystem.h"
#include "Core/MissionGamemode.h"
#include "Core/SORNGameInstance.h"
#include "Core/SORNGameState.h"
#include "Core/GameIntanceHubNotifications/MissionResultsHubNotification.h"
#include "Kismet/GameplayStatics.h"
#include "Mech/PlayerMechCharacter.h"
#include "MechParts/Weapons/WeaponBase.h"
#include "Systems/Mission/Components/MissionReplicatorComponent.h"
#include "UI/SORNUIRoot.h"
#include "UI/Viewmodels/CharacterStateViewModel.h"
#include "UI/Viewmodels/WeaponViewModel.h"

// Enhanced Input
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/ThirdPersonCameraManagerComponent.h"
#include "Core/SORNGameSettings.h"
#include "Core/Abilities/SORNGameplayTags.h"
#include "Core/Save/SaveManagerSubsystem.h"

class ULevelSequencePlayer;
class ALevelSequenceActor;


ASORNPlayerController::ASORNPlayerController()
{
	MissionReplicatorComponent = CreateDefaultSubobject<UMissionReplicatorComponent>(TEXT("MissionReplicatorComponent"));
}


void ASORNPlayerController::OnMissionStartSequenceComplete()
{
	if (UIRootWidget)
	{
		UIRootWidget->PushWidgetToStack(HudWidgetClass, ESORNUIStack::ESUS_HUD);
		UIRootWidget->ClearStack(ESORNUIStack::ESUS_Menu);
	}

	SetViewTarget(GetPawn());

}


USORNUIRoot* ASORNPlayerController::GetUIRootWidget() const
{
	return UIRootWidget;
}

void ASORNPlayerController::StartRetrySequence()
{
	if (UIRootWidget)
	{
		UIRootWidget->ClearStack(ESORNUIStack::ESUS_HUD);
	}
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 1.0f, FLinearColor::Black, false, true);
	}


	// 3. Wait for fade to finish, then call server
	GetWorldTimerManager().SetTimer(FadeTimerHandle, this, &ASORNPlayerController::OnRetryFadeComplete, 1.0f, false);

}

void ASORNPlayerController::AttachToHUB()
{
	if (APlayerMechCharacter* MechCharacter = Cast<APlayerMechCharacter>(GetPawn()))
	{
		MechCharacter->AttachToHUBShip();
	}
}

void ASORNPlayerController::MissionEndSequenceComplete()
{
}


void ASORNPlayerController::Client_StartExtraction(TObjectPtr<ULevelSequence> ExtractionSequence)
{
	UTaggedActorManagerSubsystem* TaggedActorManager = GetWorld()->GetGameInstance()->GetSubsystem<UTaggedActorManagerSubsystem>();
	if (!TaggedActorManager)return;
	
	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bPauseAtEnd = true;
	
	ALevelSequenceActor* OutActor = nullptr;
	ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		this, ExtractionSequence, Settings, OutActor
	);

	if (SequencePlayer && OutActor)
	{
		SetViewTargetWithBlend(OutActor, 0.0f);

		SequencePlayer->Play();
	}
}

void ASORNPlayerController::Client_SetViewTarget_Implementation(AActor* NewViewTarget, float BlendTime)
{
	if (NewViewTarget && IsLocalController()) // Ensure it's the locally controlled PC
	{
		SetViewTargetWithBlend(NewViewTarget, BlendTime);
	}
}


void ASORNPlayerController::OnLeftWeaponEquipped(TScriptInterface<IEquippableInterface> NewWeapon, TScriptInterface<IEquippableInterface> OldWeapon)
{
	if (!NewWeapon)return;
	AWeaponBase* AsWeapon = Cast<AWeaponBase>(NewWeapon.GetObject());
	if (!AsWeapon)return;
	GetLeftWeaponViewModel()->BindToWeapon(AsWeapon);
}

void ASORNPlayerController::OnRightWeaponEquipped(TScriptInterface<IEquippableInterface> NewWeapon, TScriptInterface<IEquippableInterface> OldWeapon)
{
	if (!NewWeapon)return;
	AWeaponBase* AsWeapon = Cast<AWeaponBase>(NewWeapon.GetObject());
	if (!AsWeapon)return;
	GetRightWeaponViewModel()->BindToWeapon(AsWeapon);
}

void ASORNPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);

	if (IsLocalController())
	{
		if (UIRootWidget && !HUDWidgetInstance)
		{
			HUDWidgetInstance = UIRootWidget->PushWidgetToStack(HudWidgetClass, ESORNUIStack::ESUS_HUD);
		}

		if (UCharacterStateViewModel* VM = GetCharacterStateViewModel())
		{
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(P))
			{
				VM->InitializeGAS(ASI->GetAbilitySystemComponent());
			}
		}

		AMechCharacter* AsMech = Cast<AMechCharacter>(P);
		if (AsMech)
		{
			AsMech->OnLeftEquipped.RemoveAll(this);
			AsMech->OnLeftEquipped.AddDynamic(this, &ASORNPlayerController::OnLeftWeaponEquipped);
			AsMech->OnRightEquipped.RemoveAll(this);
			AsMech->OnRightEquipped.AddDynamic(this, &ASORNPlayerController::OnRightWeaponEquipped);

			OnLeftWeaponEquipped(AsMech->GetLeftEquippable(), nullptr);
			OnRightWeaponEquipped(AsMech->GetRightEquippable(), nullptr);
		}

		APlayerMechCharacter* AsPlayerMech = Cast<APlayerMechCharacter>(P);
		if (AsPlayerMech)
		{
			UThirdPersonCameraManagerComponent* CameraManager = AsPlayerMech->GetCameraManagerComponent();
			if (CameraManager)
			{
				CameraManager->SetPlayerControllerAndCameraManager(this, PlayerCameraManager);
			}
		}
		
		//PlayerCameraManager->StartCameraFade(1, 0, 0.5f, FLinearColor::Black, false, true);
	}
}

void ASORNPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	APlayerMechCharacter* AsMech = Cast<APlayerMechCharacter>(InPawn);
	if (AsMech)
	{
		if (AMechPlayerState* MechPlayerState = GetPlayerState<AMechPlayerState>())
		{
			AsMech->SetCustomizationData(MechPlayerState->MechCustomizationData);
		}
	}
}

void ASORNPlayerController::TeleportToMissionStart(FTransform StartTransform)
{
	if (GetPawn())
	{
		GetPawn()->SetActorTransform(StartTransform);
	}
}

UCharacterStateViewModel* ASORNPlayerController::GetCharacterStateViewModel()
{
	if (!CharacterStateViewModel)
	{
		CharacterStateViewModel = NewObject<UCharacterStateViewModel>(this);
	}
	return CharacterStateViewModel;
}

UWeaponViewModel* ASORNPlayerController::GetLeftWeaponViewModel()
{
	if (!LeftWeaponViewModel)
	{
		LeftWeaponViewModel = NewObject<UWeaponViewModel>(this);
	}
	return LeftWeaponViewModel;
}

UWeaponViewModel* ASORNPlayerController::GetRightWeaponViewModel()
{
	if (!RightWeaponViewModel)
	{
		RightWeaponViewModel = NewObject<UWeaponViewModel>(this);
	}
	return RightWeaponViewModel;
}


void ASORNPlayerController::GiveMissionRewards_Implementation(const FCurrencies& SessionCurrencies)
{
	USORNGameInstance* GameInstance = Cast<USORNGameInstance>(GetGameInstance());
	ASORNGameState* GameState = GetWorld()->GetGameState<ASORNGameState>();
	if (GameInstance)
	{
		UMissionResultsHubNotification* HubNotification = Cast<UMissionResultsHubNotification>(GameInstance->CreateHubNotification(MissionResultsHubNotificationClass));
		if (HubNotification)
		{
			HubNotification->MissionReward = SessionCurrencies;
			HubNotification->SetMission(GameState->GetCurrentMissionGraphAsset());
			GameInstance->PushHubNotification(HubNotification);	
		}
	}
}

void ASORNPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (IsLocalController())
	{
		SornGameSettings = USORNGameSettings::Get();
		
			//PlayerCameraManager->SetManualCameraFade(1.0f, FLinearColor::Black, false);
		//lazily create if not done already
#if WITH_EDITOR
	USaveManagerSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
		USORNSaveGame* CurrentSave = SaveSubsystem ? SaveSubsystem->GetSaveGame() : nullptr;
#endif
		

		USORNGameInstance* GameInstance = GetGameInstance<USORNGameInstance>();
		AMechPlayerState* CurPlayerState = GetPlayerState<AMechPlayerState>();
		
		if (GameInstance && CurPlayerState)
		{
			CurPlayerState->SetMechCustomizationData(GameInstance->SavedMechData);
		}

		ASORNGameState* CurGameState = GetWorld()->GetGameState<ASORNGameState>();
		if (CurGameState && GameInstance && CurGameState->HasMatchStarted())
		{
			GameInstance->RemoveLoadingScreenFromViewport();
		}

		GetWorld()->OnWorldMatchStarting.AddUObject(this, &ASORNPlayerController::EndLoadingScreen);
	}
}


FVector ASORNPlayerController::TransformPlayerInputToWorldDirection(const FVector2D& Input) const
{
	if (Input.IsNearlyZero())return FVector::ZeroVector;
	const FRotator CurrentControlRotation = GetControlRotation();
	const FRotator YawRotation(0.0f, CurrentControlRotation.Yaw, 0.0f);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector WorldDirection = (Forward * Input.Y + Right * Input.X);
	return WorldDirection.GetSafeNormal();
}

void ASORNPlayerController::Server_RestartMission_Implementation()
{
	if (AMissionGamemode* GM = GetWorld()->GetAuthGameMode<AMissionGamemode>())
	{
		GM->ReturnToLastCheckpoint();
	}
}

void ASORNPlayerController::OnRetryFadeComplete()
{
	Server_RestartMission();
}


// --- Cutscene playback and skip implementations ---
void ASORNPlayerController::PlayCutsceneLocal(TSoftObjectPtr<ULevelSequence> Sequence, const FTransform& StartTransform, bool bSkippable)
{
	if (!IsLocalController())
	{
		// Only local clients should perform view target changes
		return;
	}

	// If a fade duration is set, fade to black first then start sequence
	if (CutsceneFadeDuration > KINDA_SMALL_NUMBER && PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(0.0f, 1.0f, CutsceneFadeDuration, FLinearColor::Black, false, true);

		FTimerDelegate StartDel;
		StartDel.BindUObject(this, &ASORNPlayerController::PlayCutsceneLocal_Internal, Sequence, StartTransform, bSkippable);
		GetWorldTimerManager().SetTimer(CutsceneStartFadeTimerHandle, StartDel, CutsceneFadeDuration, false);
		return;
	}

	// No fade path
	PlayCutsceneLocal_Internal(Sequence, StartTransform, bSkippable);
}

void ASORNPlayerController::PlayCutsceneLocal_Internal(TSoftObjectPtr<ULevelSequence> Sequence, FTransform StartTransform, bool bSkippable)
{
	// This contains the original PlayCutsceneLocal implementation moved here for deferred start
	if (!IsLocalController())
	{
		return;
	}

	// If there is already a playing sequence, stop it first
	StopCutsceneLocal();

	ULevelSequence* SequenceAsset = nullptr;
	if (Sequence.IsValid())
	{
		SequenceAsset = Sequence.Get();
	}
	else
	{
		SequenceAsset = Cast<ULevelSequence>(Sequence.LoadSynchronous());
	}

	if (!SequenceAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCutsceneLocal_Internal: Sequence asset invalid"));
		return;
	}

	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bPauseAtEnd = false;

	ALevelSequenceActor* OutActor = nullptr;
	ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		this,
		SequenceAsset,
		Settings,
		OutActor
	);

	if (!SequencePlayer || !OutActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCutsceneLocal_Internal: Failed to create level sequence player"));
		return;
	}
	OutActor->SetActorTransform(StartTransform);

	CurrentLevelSequencePlayer = SequencePlayer;
	CurrentLevelSequenceActor = OutActor;
	bLocalCutsceneSkippable = bSkippable;

	// Add the cutscene input mapping context so the local player can skip (if configured)
	if (CutsceneInputMappingContext && IsLocalController())
	{
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(CutsceneInputMappingContext, CutsceneInputMappingPriority);
			}
		}
	}

	// Set camera to sequence actor
	SetViewTargetWithBlend(OutActor, 0.0f);

	// Play and bind finish handler
	SequencePlayer->OnNativeFinished.BindUObject(this, &ASORNPlayerController::OnCutsceneStop);

	if (CharacterStateViewModel)
	{
		CharacterStateViewModel->SetIsInCutscene(true);
	}
	
	SequencePlayer->Play();

	if (GetPawn())
	{
		GetPawn()->DisableInput(this);
	}

	APlayerMechCharacter* AsMech = Cast<APlayerMechCharacter>(GetPawn());
	if (AsMech)
	{
		AsMech->EndAllInput();
	}
	if (BlackBarsWidgetClass)
	{
		if (!BlackBarsWidgetInstance)
		{
			BlackBarsWidgetInstance = CreateWidget(this, BlackBarsWidgetClass);
		}
		BlackBarsWidgetInstance->AddToViewport(100);
	}

	// Fade back in to reveal the cutscene if we faded out before starting
	if (CutsceneFadeDuration > KINDA_SMALL_NUMBER && PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(1.0f, 0.0f, CutsceneFadeDuration, FLinearColor::Black, false, true);
	}
}

void ASORNPlayerController::OnCutsceneStop()
{
	// When the sequence finishes, perform a fade out and defer the rest of the shutdown until fade completes
	/*if (CutsceneFadeDuration > KINDA_SMALL_NUMBER && PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(0.0f, 1.0f, CutsceneFadeDuration, FLinearColor::Black, false, true);
		GetWorldTimerManager().SetTimer(CutsceneStopFadeTimerHandle, this, &ASORNPlayerController::FinishStopCutscene_Internal, CutsceneFadeDuration, false);
		return;
	}*/

	// No fade path
	FinishStopCutscene_Internal();
}

void ASORNPlayerController::FinishStopCutscene_Internal()
{
	// This contains the original OnCutsceneStop logic but deferred until after fade
	APlayerMechCharacter* AsMech = Cast<APlayerMechCharacter>(GetPawn());
	if (AsMech)
	{
		//AsMech->GetAbilitySystemComponent()->RemoveLooseGameplayTag(FSORNGameplayTags::Get().Status_Movement_DisableAutoAim);
		//AsMech->GetAbilitySystemComponent()->RemoveLooseGameplayTag(FSORNGameplayTags::Get().Status_Movement_LockTorsoRotation);
	}
	if (GetPawn())
	{
		GetPawn()->EnableInput(this);
	}

	if (CurrentLevelSequenceActor)
	{
		SetViewTarget(GetPawn());
		// Optionally destroy the sequence actor since it was spawned for playback
		CurrentLevelSequenceActor = nullptr;
	}

	bLocalCutsceneSkippable = false;

	if (CharacterStateViewModel)
	{
		CharacterStateViewModel->SetIsInCutscene(false);
	}
	// Remove the cutscene input mapping context when stopping
	if (CutsceneInputMappingContext && IsLocalController())
	{
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->RemoveMappingContext(CutsceneInputMappingContext);
			}
		}
	}
	ASORNGameState* GameState = GetWorld()->GetGameState<ASORNGameState>();
	if (GameState)
	{
		GameState->StopCutscene();
	}

	if (BlackBarsWidgetInstance)
	{
		BlackBarsWidgetInstance->RemoveFromParent();
	}

	// Fade back in to show gameplay
	if (CutsceneFadeDuration > KINDA_SMALL_NUMBER && PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(1.0f, 0.0f, CutsceneFadeDuration, FLinearColor::Black, false, true);
	}
}

void ASORNPlayerController::StopCutsceneLocal()
{
	if (!IsLocalController()) return;
	if (CurrentLevelSequencePlayer && CurrentLevelSequenceActor)
	{
		CurrentLevelSequencePlayer->GoToEndAndStop();
		OnCutsceneStop();
	}
}

void ASORNPlayerController::Server_RequestSkipCutscene_Implementation()
{
	// Server side: forward request to the GameMode which will validate and act
	if (AMissionGamemode* GM = GetWorld()->GetAuthGameMode<AMissionGamemode>())
	{
		GM->ProcessSkipCutsceneRequest(this);
	}
}

void ASORNPlayerController::EndLoadingScreen() const
{
	USORNGameInstance* GameInstance = GetGameInstance<USORNGameInstance>();
	if (GameInstance)
	{
		GameInstance->RemoveLoadingScreenFromViewport();
	}
}


void ASORNPlayerController::EOSLogin()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface(); // This is the generic OSS interface that will access the EOS OSS.
 
    // If you're logged in, don't try to login again.
    // This can happen if your player travels to a dedicated server or different maps as BeginPlay() will be called each time.
    
    FUniqueNetIdPtr NetId = Identity->GetUniquePlayerId(0);
 
    if (NetId != nullptr && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        return; 
    }
    
    /* This binds a delegate so we can run our function when the callback completes. 0 represents the player number.
    You should parametrize this Login function and pass the parameter here for splitscreen. 
    */
    LoginDelegateHandle = 
	Identity->AddOnLoginCompleteDelegate_Handle(
        0,
        FOnLoginCompleteDelegate::CreateUObject(
            this,
            &ThisClass::HandleLoginCompleted));
 
    // Grab command line parameters. If empty call hardcoded login function - Hardcoded login function useful for Play In Editor. 
    FString AuthType; 
    FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), AuthType);
 
    if (!AuthType.IsEmpty()) //If parameter is NOT empty we can autologin.
    {
        /* 
        In most situations you will want to automatically log a player in using the parameters passed via CLI.
        For example, using the exchange code for the Epic Games Store.
        */
        UE_LOG(LogTemp, Log, TEXT("Logging into EOS...")); // Log to the UE logs that we are trying to log in. 
      
        if (!Identity->AutoLogin(0))
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to login... ")); // Log to the UE logs that we are trying to log in.
			// Clear our handle and reset the delegate.
			Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginDelegateHandle);
            LoginDelegateHandle.Reset();			
        }
    }
    else
    {
        /* 
        Fallback if the CLI parameters are empty.Useful for PIE.
        The type here could be developer if using the DevAuthTool, ExchangeCode if the game is launched via the Epic Games Launcher, etc...
        */
        FOnlineAccountCredentials Credentials("AccountPortal","", "");
 
        UE_LOG(LogTemp, Log, TEXT("Logging into EOS...")); // Log to the UE logs that we are trying to log in. 
        
        if (!Identity->Login(0, Credentials))
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to login... ")); // Log to the UE logs that we are trying to log in.
			// Clear our handle and reset the delegate. 
            Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginDelegateHandle);
            LoginDelegateHandle.Reset();			
        }        
    }
}

void ASORNPlayerController::RequestSkipCutscene()
{
	Server_RequestSkipCutscene();
}

void ASORNPlayerController::HandleLoginCompleted(int LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
                                                 const FString& Error)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Login callback completed!"));;

		FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
        
		// Reload the level
		UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
	}
	else //Login failed
	{
		// If your game is online only, you may want to return an errror to the user and return to a menu that uses a different GameMode/PlayerController.
 
		UE_LOG(LogTemp, Warning, TEXT("EOS login failed.")); //Print sign in failure in logs as a warning.
	}
 
	Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginDelegateHandle);
	LoginDelegateHandle.Reset();
}

static float ApplyDeadzone(float Value, float Deadzone)
{
	if (FMath::Abs(Value) < Deadzone)
	{
		return 0.0f;
	}
	
	return (Value - FMath::Sign(Value) * Deadzone) / (1.0f - Deadzone);
}

static float ApplyResponseCurve(float Value, float Exponent)
{
	return FMath::Sign(Value) * FMath::Pow(FMath::Abs(Value), Exponent);
}

void ASORNPlayerController::ApplySensitivity(float& X, float& Y, bool bIsGamepad) const
{
	if (!SornGameSettings) return;
	
	if (bIsGamepad)
	{
		X = ApplyDeadzone(X, SornGameSettings->ControllerDeadzone);
		Y = ApplyDeadzone(Y, SornGameSettings->ControllerDeadzone);
		
		X = ApplyResponseCurve(X, SornGameSettings->ControllerExponent);
		Y = ApplyResponseCurve(Y, SornGameSettings->ControllerExponent);
		
		X *= SornGameSettings->ControllerSensitivity;
		Y *= SornGameSettings->ControllerSensitivity;
	}
	else
	{
		const float MouseScale = 0.01f;
		
		X *= SornGameSettings->MouseSensitivity * MouseScale;
		Y *= SornGameSettings->MouseSensitivity * MouseScale;
	}
	
	if (SornGameSettings->bInvertYAxis)
	{
		Y *= -1.0f;
	}
}

void ASORNPlayerController::Client_PlayCameraFade_Implementation(float FromAlpha, float ToAlpha, float Duration, FLinearColor Color, bool bShouldFadeAudio, bool bHoldWhenFinished)
{
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, Duration, Color, bShouldFadeAudio, bHoldWhenFinished);
	}
}
