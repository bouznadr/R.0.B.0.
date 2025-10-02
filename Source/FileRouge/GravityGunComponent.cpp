// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityGunComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UGravityGunComponent::UGravityGunComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UGravityGunComponent::BeginPlay()
{
	Super::BeginPlay();

	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (!PhysicsHandle)
	{
		PhysicsHandle = NewObject<UPhysicsHandleComponent>(GetOwner());
		PhysicsHandle->RegisterComponent();
		GetOwner()->AddInstanceComponent(PhysicsHandle);
	}
}


// Called every frame
void UGravityGunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PhysicsHandle && GrabbedComponent)
	{
		AActor* Owner = GetOwner();
		if (!Owner) return;

		FVector CameraLoc;
		FRotator CameraRot;
		if (APlayerController* PC = Cast<APlayerController>(Owner->GetInstigatorController()))
		{
			PC->GetPlayerViewPoint(CameraLoc, CameraRot);

			FVector TargetLoc = CameraLoc + CameraRot.Vector() * HoldDistance;
			PhysicsHandle->SetTargetLocationAndRotation(TargetLoc, CameraRot);
		}
	}
}

bool UGravityGunComponent::TraceForGrab(FHitResult& HitResult) const
{
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	FVector CameraLoc;
	FRotator CameraRot;
	if (APlayerController* PC = Cast<APlayerController>(Owner->GetInstigatorController()))
	{
		PC->GetPlayerViewPoint(CameraLoc, CameraRot);

		FVector End = CameraLoc + CameraRot.Vector() * TraceDistance;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Owner);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			CameraLoc,
			End,
			ECC_PhysicsBody,
			Params
		);

		if (bHit && HitResult.GetActor() && HitResult.GetActor()->ActorHasTag(FName("Grab")))
		{
			return true;
		}
	}

	return false;
}

void UGravityGunComponent::PullObject()
{
	if (!PhysicsHandle) return;

	if (!GrabbedComponent)
	{
		FHitResult Hit;
		if (TraceForGrab(Hit))
		{
			GrabbedComponent = Hit.GetComponent();
			if (GrabbedComponent && GrabbedComponent->IsSimulatingPhysics())
			{
				PhysicsHandle->GrabComponentAtLocation(
					GrabbedComponent,
					NAME_None,
					GrabbedComponent->GetComponentLocation()
				);
			}
		}
	}
}

void UGravityGunComponent::ReleaseObject()
{
	if (PhysicsHandle && GrabbedComponent)
	{
		PhysicsHandle->ReleaseComponent();
		GrabbedComponent = nullptr;
	}
}