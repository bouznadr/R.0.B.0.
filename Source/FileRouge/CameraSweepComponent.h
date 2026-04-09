// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraSweepComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSweepLimitReached);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FILEROUGE_API UCameraSweepComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCameraSweepComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	FTimerHandle SweepTimerHandle;
	bool bGoingRight;

	//float CurrentYaw;
    //float TargetYaw;
    float SweepSpeed;
    FRotator NewRot;
    FRotator beginRot;

public:	
    // ----- CONFIG -----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSweep")
    float LeftYaw = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSweep")
    float RightYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSweep")
    float Duration = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "CameraSweep")
    float CurrentYaw = 0;

    UPROPERTY(BlueprintReadOnly, Category = "CameraSweep")
    float TargetYaw;

    // ----- DISPATCHERS -----
    UPROPERTY(BlueprintAssignable, Category = "CameraSweep")
    FOnSweepLimitReached OnReachedLeftLimit;

    UPROPERTY(BlueprintAssignable, Category = "CameraSweep")
    FOnSweepLimitReached OnReachedRightLimit;

    // ----- FUNCTIONS -----
    UFUNCTION(BlueprintCallable, Category = "CameraSweep")
    void StartSweep();

    UFUNCTION(BlueprintCallable, Category = "CameraSweep")
    void StopSweep();

    UFUNCTION(BlueprintCallable, Category = "CameraSweep")
    void ResetSweep();

private:
    void PerformSweepStep();
    void SwitchDirection();
		
};
