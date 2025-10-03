// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerModeComponent.generated.h"


UENUM(BlueprintType)
enum class EPlayerMode : uint8
{
    Weapon      UMETA(DisplayName = "Weapon"),
    GravityGun  UMETA(DisplayName = "Gravity Gun"),
    FreezeGun   UMETA(DisplayName = "Freeze Gun")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FILEROUGE_API UPlayerModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerModeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modes")
    TArray<EPlayerMode> AvailableModes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modes")
    int32 CurrentModeIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modes")
    EPlayerMode CurrentMode;

    // Passe au mode suivant
    UFUNCTION(BlueprintCallable, Category = "Modes")
    void NextMode();

    // Passe au mode précéden
    UFUNCTION(BlueprintCallable, Category = "Modes")
    void PreviousMode();

    // Change de mode en fonction d'une direction (+1 ou -1)
    UFUNCTION(BlueprintCallable, Category = "Modes")
    void SwitchMode(int32 Direction);

    // Retourne le mode actuel
    UFUNCTION(BlueprintPure, Category = "Modes")
    EPlayerMode GetCurrentMode() const { return CurrentMode; }
};
