// Fill out your copyright notice in the Description page of Project Settings.


#include "DelegateTeest.h"

#include "Engine/Engine.h"

// Sets default values
ADelegateTeest::ADelegateTeest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADelegateTeest::BeginPlay()
{
	Super::BeginPlay();

	OnTestSimple.BindUObject(this, &ADelegateTeest::PrintSimple);
	OnTest.AddDynamic(this, &ADelegateTeest::PrintMulticast);

	if (OnTestSimple.IsBound())
	{
		OnTestSimple.Execute();
	}
	OnTest.Broadcast();
}

// Called every frame
void ADelegateTeest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADelegateTeest::PrintSimple()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Emerald, TEXT("Simple delegate"));
	}	
}

void ADelegateTeest::PrintMulticast()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Silver, TEXT("Dynamic Multi delegate"));
	}	
}