// Fill out your copyright notice in the Description page of Project Settings.

#include "GravityGunComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

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

			// Update Niagara Beam position - le beam va de l'arme vers l'objet saisi
			if (ActiveBeam)
			{
				FVector GunLocation = Owner->GetActorLocation();
				FVector GrabbedObjectLocation = GrabbedComponent->GetComponentLocation();

				ActiveBeam->SetVectorParameter(TEXT("Beam Start"), GunLocation);
				ActiveBeam->SetVectorParameter(TEXT("Beam End"), GrabbedObjectLocation);
			}
		}
	}
	else if (ActiveBeam)
	{
		// Désactiver le beam quand aucun objet n'est saisi
		ActiveBeam->SetVectorParameter(TEXT("Beam Start"), FVector::ZeroVector);
		ActiveBeam->SetVectorParameter(TEXT("Beam End"), FVector::ZeroVector);
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

bool UGravityGunComponent::CanGrabObject() const
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	return (LastThrowTime < 0.f || CurrentTime - LastThrowTime > ThrowCooldownTime);
}

void UGravityGunComponent::GrabObject()
{
	if (!PhysicsHandle || !CanGrabObject()) return;

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

				// Activer le beam Niagara quand un objet est saisi
				if (BeamVFX && !ActiveBeam)
				{
					AActor* Owner = GetOwner();
					if (Owner)
					{
						FVector GunLocation = Owner->GetActorLocation();
						FRotator GunRotation = Owner->GetActorRotation();

						ActiveBeam = UNiagaraFunctionLibrary::SpawnSystemAttached(
							BeamVFX,
							Owner->GetRootComponent(),
							NAME_None,
							GunLocation,
							GunRotation,
							EAttachLocation::KeepWorldPosition,
							true
						);

						// Initialiser immédiatement la position du beam
						if (ActiveBeam)
						{
							FVector GrabbedObjectLocation = GrabbedComponent->GetComponentLocation();
							ActiveBeam->SetVectorParameter(TEXT("Beam Start"), GunLocation);
							ActiveBeam->SetVectorParameter(TEXT("Beam End"), GrabbedObjectLocation);
						}
					}
				}
			}
		}
	}
}

void UGravityGunComponent::ThrowObject(float Force)
{
	if (!GrabbedComponent) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector CameraLoc;
	FRotator CameraRot;
	if (APlayerController* PC = Cast<APlayerController>(Owner->GetInstigatorController()))
	{
		PC->GetPlayerViewPoint(CameraLoc, CameraRot);

		FVector PushDir = CameraRot.Vector();
		GrabbedComponent->AddImpulse(PushDir * Force, NAME_None, true);

		LastThrowTime = GetWorld()->GetTimeSeconds();

		ReleaseObject();
	}
}

void UGravityGunComponent::ReleaseObject()
{
	if (PhysicsHandle && GrabbedComponent)
	{
		PhysicsHandle->ReleaseComponent();
		GrabbedComponent = nullptr;

		// Désactiver le beam quand l'objet est relâché
		if (ActiveBeam)
		{
			ActiveBeam->Deactivate();
			ActiveBeam = nullptr;
		}
	}
}

void UGravityGunComponent::ActivateGravityGun()
{
	SetComponentTickEnabled(true);
	SetActive(true);
}

void UGravityGunComponent::DeactivateGravityGun()
{
	SetComponentTickEnabled(false);
	SetActive(false);

	if (ActiveBeam)
	{
		ActiveBeam->Deactivate();
		ActiveBeam = nullptr;
	}
}