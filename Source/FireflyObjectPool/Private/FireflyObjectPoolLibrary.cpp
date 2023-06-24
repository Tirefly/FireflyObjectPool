// Copyright tzlFirefly, 2023. All Rights Reserved.

#include "FireflyObjectPoolLibrary.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"

void UFireflyObjectPoolLibrary::UniversalBeginPlay_Actor(const UObject* WorldContextObject, AActor* Actor)
{
	Actor->SetActorTickEnabled(true);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorHiddenInGame(false);

	TInlineComponentArray<UActorComponent*>Components;
	Actor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (UParticleSystemComponent* ParticleSystem = Cast<UParticleSystemComponent>(Component))
		{
			ParticleSystem->SetActive(true, true);
			ParticleSystem->ActivateSystem();

			continue;
		}

		if (UNiagaraComponent* Niagara = Cast<UNiagaraComponent>(Component))
		{
			Niagara->SetActive(true, true);
			Niagara->ActivateSystem();
			
			continue;
		}

		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
		{
			Primitive->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			Primitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Primitive->SetComponentTickEnabled(true);
			Primitive->SetVisibility(true, true);

			Primitive->SetActive(true, true);

			continue;
		}

		if (UMovementComponent* Movement = Cast<UMovementComponent>(Component))
		{
			Movement->SetUpdatedComponent(Actor->GetRootComponent());
			Movement->SetActive(true, true);
			Movement->StopMovementImmediately();
			if (UProjectileMovementComponent* ProjectileMovement = Cast<UProjectileMovementComponent>(Movement))
			{
				ProjectileMovement->SetVelocityInLocalSpace(FVector::XAxisVector * ProjectileMovement->InitialSpeed);
			}			
		}

		Component->SetActive(true, true);
	}
}

void UFireflyObjectPoolLibrary::UniversalEndPlay_Actor(const UObject* WorldContextObject, AActor* Actor)
{
	Actor->SetActorTickEnabled(false);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorHiddenInGame(true);

	TInlineComponentArray<UActorComponent*>Components;
	Actor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (UParticleSystemComponent* ParticleSystem = Cast<UParticleSystemComponent>(Component))
		{
			ParticleSystem->DeactivateSystem();
			Component->SetActive(false);

			continue;
		}

		if (UNiagaraComponent* Niagara = Cast<UNiagaraComponent>(Component))
		{
			Niagara->DeactivateImmediate();
			Component->SetActive(false);

			continue;
		}

		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
		{
			Primitive->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			Primitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Primitive->SetComponentTickEnabled(false);
			Primitive->SetSimulatePhysics(false);
			Primitive->SetVisibility(false, true);
			Component->SetActive(false);

			continue;
		}

		if (UMovementComponent* Movement = Cast<UMovementComponent>(Component))
		{
			Movement->StopMovementImmediately();
			Movement->SetUpdatedComponent(nullptr);
		}

		Component->SetActive(false);
	}
}

void UFireflyObjectPoolLibrary::UniversalWarmUp_Actor(const UObject* WorldContextObject, AActor* Actor)
{
	UniversalEndPlay_Actor(WorldContextObject, Actor);
}

void UFireflyObjectPoolLibrary::UniversalBeginPlay_Pawn(const UObject* WorldContextObject, APawn* Pawn)
{
	UniversalBeginPlay_Actor(WorldContextObject, Pawn);

	Pawn->SpawnDefaultController();
	if (IsValid(Pawn->GetController()))
	{
		if (const AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
		{
			if (UBrainComponent* Brain = AIController->GetBrainComponent())
			{
				Brain->StartLogic();
			}			
		}
	}
}

void UFireflyObjectPoolLibrary::UniversalEndPlay_Pawn(const UObject* WorldContextObject, APawn* Pawn)
{
	UniversalEndPlay_Actor(WorldContextObject, Pawn);

	if (IsValid(Pawn->GetController()))
	{
		if (const AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
		{
			if (UBrainComponent* Brain = AIController->GetBrainComponent())
			{
				Brain->Cleanup();
			}
		}
	}
}

void UFireflyObjectPoolLibrary::UniversalWarmUp_Pawn(const UObject* WorldContextObject, APawn* Pawn)
{
	UniversalWarmUp_Actor(WorldContextObject, Pawn);

	Pawn->SpawnDefaultController();
	if (IsValid(Pawn->GetController()))
	{
		if (const AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
		{
			if (UBrainComponent* Brain = AIController->GetBrainComponent())
			{
				Brain->Cleanup();
			}
		}
	}
}

void UFireflyObjectPoolLibrary::UniversalBeginPlay_Character(const UObject* WorldContextObject, ACharacter* Character)
{
	UniversalBeginPlay_Pawn(WorldContextObject, Character);
}

void UFireflyObjectPoolLibrary::UniversalEndPlay_Character(const UObject* WorldContextObject, ACharacter* Character)
{
	UniversalEndPlay_Pawn(WorldContextObject, Character);
}

void UFireflyObjectPoolLibrary::UniversalWarmUp_Character(const UObject* WorldContextObject, ACharacter* Character)
{
	UniversalWarmUp_Pawn(WorldContextObject, Character);
}
