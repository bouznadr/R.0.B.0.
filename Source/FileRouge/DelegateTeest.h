// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DelegateTeest.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTestMulticast);
DECLARE_DELEGATE(FTestSimple);

UCLASS()
class FILEROUGE_API ADelegateTeest : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADelegateTeest();
	UPROPERTY(BlueprintAssignable)
	FTestMulticast OnTest;

	FTestSimple OnTestSimple;

	void PrintSimple();
	
	UFUNCTION()
	void PrintMulticast();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
