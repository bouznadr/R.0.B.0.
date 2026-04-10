// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraSweepComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UCameraSweepComponent::UCameraSweepComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bGoingRight = true;
}


// Called when the game starts
void UCameraSweepComponent::BeginPlay()
{
	Super::BeginPlay();

	float Distance = FMath::Abs(RightYaw - LeftYaw);
	SweepSpeed = Distance / Duration;


    if (!GetOwner()) return;
    beginRot = GetOwner()->GetActorRotation();

    NewRot = beginRot;

    CurrentYaw = LeftYaw + beginRot.Yaw;
    TargetYaw = RightYaw + beginRot.Yaw;


}

void UCameraSweepComponent::StartSweep()
{

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            SweepTimerHandle,
            this,
            &UCameraSweepComponent::PerformSweepStep,
            0.01f,
            true
        );
    }
}

void UCameraSweepComponent::StopSweep()
{
    if (GetWorld())
        GetWorld()->GetTimerManager().ClearTimer(SweepTimerHandle);
}

void UCameraSweepComponent::ResetSweep()
{
    StopSweep();
    bGoingRight = true;
    CurrentYaw = LeftYaw;
    TargetYaw = RightYaw;
}

void UCameraSweepComponent::PerformSweepStep()
{
    if (!GetOwner()) return;

    float DeltaYaw = SweepSpeed * 0.01f;
    if (!bGoingRight) DeltaYaw *= -1;


    NewRot.Yaw = CurrentYaw + DeltaYaw;

    GetOwner()->SetActorRotation(NewRot);

    CurrentYaw = NewRot.Yaw;

    if (bGoingRight && CurrentYaw >= RightYaw + beginRot.Yaw)
    {
        //CurrentYaw = RightYaw + beginRot.Yaw;
        OnReachedRightLimit.Broadcast();
        SwitchDirection();
    }
    else if (!bGoingRight && CurrentYaw <= LeftYaw + beginRot.Yaw)
    {
        //CurrentYaw = LeftYaw + beginRot.Yaw;
        OnReachedLeftLimit.Broadcast();
        SwitchDirection();
    }
}

void UCameraSweepComponent::SwitchDirection()
{
    bGoingRight = !bGoingRight;
}