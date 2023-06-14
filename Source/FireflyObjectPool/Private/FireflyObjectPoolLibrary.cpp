// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyObjectPoolLibrary.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/Character.h"

#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"

void UFireflyObjectPoolLibrary::UniversalBeginPlay_Actor(AActor* Actor)
{
	Actor->SetActorTickEnabled(true);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorHiddenInGame(false);

	if (UActorComponent* Movement = Actor->GetComponentByClass(UMovementComponent::StaticClass()))
	{
		Cast<UMovementComponent>(Movement)->StopMovementImmediately();
	}
}

void UFireflyObjectPoolLibrary::UniversalEndPlay_Actor(AActor* Actor)
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

		Component->SetActive(false);

		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
		{
			Primitive->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			Primitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Primitive->SetComponentTickEnabled(false);
			Primitive->SetSimulatePhysics(false);
			Primitive->SetVisibility(false, true);
		}

		if (UMovementComponent* Movement = Cast<UMovementComponent>(Component))
		{
			Movement->StopMovementImmediately();
		}
	}
}

void UFireflyObjectPoolLibrary::UniversalWarmUp_Actor(AActor* Actor)
{
	UniversalEndPlay_Actor(Actor);
}

void UFireflyObjectPoolLibrary::UniversalBeginPlay_Pawn(APawn* Pawn)
{
	UniversalBeginPlay_Actor(Pawn);

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

void UFireflyObjectPoolLibrary::UniversalEndPlay_Pawn(APawn* Pawn)
{
	UniversalEndPlay_Actor(Pawn);

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

void UFireflyObjectPoolLibrary::UniversalWarmUp_Pawn(APawn* Pawn)
{
	UniversalWarmUp_Actor(Pawn);

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

void UFireflyObjectPoolLibrary::UniversalBeginPlay_Character(ACharacter* Character)
{
	UniversalBeginPlay_Pawn(Character);
}

void UFireflyObjectPoolLibrary::UniversalEndPlay_Character(ACharacter* Character)
{
	UniversalEndPlay_Pawn(Character);
}

void UFireflyObjectPoolLibrary::UniversalWarmUp_Character(ACharacter* Character)
{
	UniversalWarmUp_Pawn(Character);
}
