// Fill out your copyright notice in the Description page of Project Settings.

#include "GravityGunComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values for this component's properties
UGravityGunComponent::UGravityGunComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BeamVFXAsset(TEXT("/Game/05-Niagara/NS_GravityGunBeam"));

	if (BeamVFXAsset.Succeeded())
	{
		BeamVFX = BeamVFXAsset.Object;
	}
}

// Called when the game starts
void UGravityGunComponent::BeginPlay()
{
	UActorComponent::BeginPlay();

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
	UActorComponent::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PhysicsHandle && GrabbedComponent)
	{
		if (!IsValid(GrabbedComponent))
		{
			ReleaseObject();
			return;
		}

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
			HitResult, CameraLoc, End, ECC_PhysicsBody, Params
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
	if (!IsActive() || !PhysicsHandle || !CanGrabObject()) return;
	if (GrabbedComponent) return;

	FHitResult Hit;
	if (!TraceForGrab(Hit)) return;

	AActor* HitActor = Hit.GetActor();
	if (!HitActor) return;

	TArray<UStaticMeshComponent*> MeshComps;
	HitActor->GetComponents<UStaticMeshComponent>(MeshComps);

	if (MeshComps.Num() == 0) return;

	UPrimitiveComponent* HitPrim = Hit.GetComponent();

	UPrimitiveComponent* TargetComp = HitPrim ? HitPrim : MeshComps[0];

	TargetComp->SetSimulatePhysics(true);
	TargetComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TargetComp->WakeAllRigidBodies();

	for (UStaticMeshComponent* Mesh : MeshComps)
	{
		if (Mesh && Mesh != TargetComp)
		{
			// Désactiver leur physique indépendante
			Mesh->SetSimulatePhysics(false);

			// Les souder au composant cible
			Mesh->WeldTo(TargetComp, NAME_None);
		}
	}

	GrabbedComponent = TargetComp;

	PhysicsHandle->GrabComponentAtLocation(
		GrabbedComponent,
		NAME_None,
		GrabbedComponent->GetComponentLocation()
	);

	// Beam Niagara
	if (BeamVFX && !ActiveBeam)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			ActiveBeam = UNiagaraFunctionLibrary::SpawnSystemAttached(
				BeamVFX, Owner->GetRootComponent(), NAME_None,
				GunLocation, Owner->GetActorRotation(),
				EAttachLocation::KeepWorldPosition, true
			);

			if (GrabLoopSound && !ActiveGrabSound)
			{
				ActiveGrabSound = UGameplayStatics::SpawnSoundAttached(
					GrabLoopSound, GetOwner()->GetRootComponent(),
					NAME_None, FVector::ZeroVector,
					EAttachLocation::KeepRelativeOffset, false
				);
			}

			if (ActiveBeam)
			{
				ActiveBeam->SetVectorParameter(TEXT("Beam Start"), GunLocation);
				ActiveBeam->SetVectorParameter(TEXT("Beam End"),
					GrabbedComponent->GetComponentLocation());
			}
		}
	}
}

void UGravityGunComponent::ThrowObject(float Force)
{
	if (!IsActive()) return;

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
	if (!PhysicsHandle) return;

	UPrimitiveComponent* ReleasedComponent = GrabbedComponent;
	AActor* HitActor = ReleasedComponent ? ReleasedComponent->GetOwner() : nullptr;

	PhysicsHandle->ReleaseComponent();

	if (IsValid(ReleasedComponent))
	{
		if (HitActor)
		{
			TArray<UStaticMeshComponent*> MeshComps;
			HitActor->GetComponents<UStaticMeshComponent>(MeshComps);
			for (UStaticMeshComponent* Mesh : MeshComps)
			{
				if (Mesh && Mesh != ReleasedComponent)
				{
					Mesh->UnWeldFromParent();
				}
			}
		}

		ReleasedComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ReleasedComponent->SetSimulatePhysics(true);
		ReleasedComponent->WakeAllRigidBodies();
	}

	GrabbedComponent = nullptr;

	if (ActiveBeam)
	{
		ActiveBeam->Deactivate();
		ActiveBeam = nullptr;
	}

	if (ActiveGrabSound)
	{
		ActiveGrabSound->FadeOut(0.15f, 0.f);
		ActiveGrabSound = nullptr;
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

	ReleaseObject();

	if (ActiveBeam)
	{
		ActiveBeam->Deactivate();
		ActiveBeam = nullptr;
	}
}