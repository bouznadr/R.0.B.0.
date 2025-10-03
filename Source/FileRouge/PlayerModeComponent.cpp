// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerModeComponent.h"

UPlayerModeComponent::UPlayerModeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Valeurs par défaut
    CurrentModeIndex = 0;
    AvailableModes = { EPlayerMode::Weapon, EPlayerMode::GravityGun, EPlayerMode::FreezeGun };
    CurrentMode = AvailableModes[0];
}

void UPlayerModeComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPlayerModeComponent::SwitchMode(int32 Direction)
{
    if (AvailableModes.Num() == 0) return;

    int32 Length = AvailableModes.Num();
    CurrentModeIndex = (CurrentModeIndex + Direction + Length) % Length;
    CurrentMode = AvailableModes[CurrentModeIndex];
}

void UPlayerModeComponent::NextMode()
{
    SwitchMode(+1);
}

void UPlayerModeComponent::PreviousMode()
{
    SwitchMode(-1);
}

