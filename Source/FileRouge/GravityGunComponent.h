// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GravityGunComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FILEROUGE_API UGravityGunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGravityGunComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "GravityGun")
	void GrabObject();

	UFUNCTION(BlueprintCallable, Category = "GravityGun")
	void ThrowObject(float Force = 1000.f);

	UFUNCTION(BlueprintCallable, Category = "GravityGun")
	void ReleaseObject();

	UFUNCTION(BlueprintCallable, Category = "GravityGun")
	void ActivateGravityGun();

	UFUNCTION(BlueprintCallable, Category = "GravityGun")
	void DeactivateGravityGun();

private:
	UPROPERTY(EditAnywhere)
	float TraceDistance = 1500.f;

	UPROPERTY(EditAnywhere)
	float HoldDistance = 400.f;

	UPROPERTY(EditAnywhere)
	float InterpSpeed = 1.f;

	UPROPERTY(EditAnywhere)
	float ThrowCooldownTime = 0.5f;

	UPROPERTY()
	UPhysicsHandleComponent* PhysicsHandle;

	UPROPERTY()
	UPrimitiveComponent* GrabbedComponent;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* BeamVFX;

	UPROPERTY()
	UNiagaraComponent* ActiveBeam;

	float LastThrowTime = -1.f;

	bool TraceForGrab(FHitResult& HitResult) const;
	bool CanGrabObject() const;

};