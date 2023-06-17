// Copyright Firefly Games, 2023. All Rights Reserved.

#include "FireflyObjectPoolWorldSubsystem.h"

#include "TimerManager.h"


TMap<TSubclassOf<AActor>, UFireflyObjectPoolWorldSubsystem::TActorPoolList> UFireflyObjectPoolWorldSubsystem::ActorPoolOfClass;
TMap<FName, UFireflyObjectPoolWorldSubsystem::TActorPoolList> UFireflyObjectPoolWorldSubsystem::ActorPoolOfID;


void UFireflyObjectPoolWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UFireflyObjectPoolWorldSubsystem::Deinitialize()
{
	ActorPool_ClearAll();

	Super::Deinitialize();
}

void UFireflyObjectPoolWorldSubsystem::ActorPool_ClearAll()
{
	for (auto Pool : ActorPoolOfClass)
	{
		for (auto Actor : Pool.Value)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy(true);
			}
		}
	}

	for (auto Pool : ActorPoolOfID)
	{
		for (auto Actor : Pool.Value)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy(true);
			}
		}
	}

	ActorPoolOfClass.Empty();
	ActorPoolOfID.Empty();
}

void UFireflyObjectPoolWorldSubsystem::ActorPool_ClearByClass(TSubclassOf<AActor> ActorClass)
{
	TActorPoolList* Pool = ActorPoolOfClass.Find(ActorClass);
	if (Pool)
	{
		for (auto Actor : *Pool)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy(true);
			}
		}
		Pool->Empty();
	}
}

void UFireflyObjectPoolWorldSubsystem::ActorPool_ClearByID(FName ActorID)
{
	TActorPoolList* Pool = ActorPoolOfID.Find(ActorID);
	if (Pool)
	{
		for (auto Actor : *Pool)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy(true);
			}
		}
		Pool->Empty();
	}
}

AActor* UFireflyObjectPoolWorldSubsystem::K2_ActorPool_FetchActorByClass(TSubclassOf<AActor> ActorClass)
{
	return ActorPool_FetchActorByClass<AActor>(ActorClass);
}

AActor* UFireflyObjectPoolWorldSubsystem::K2_ActorPool_FetchActorByID(FName ActorID, TSubclassOf<AActor> ActorClass)
{
	return ActorPool_FetchActorByID<AActor>(ActorID);
}

TArray<AActor*> UFireflyObjectPoolWorldSubsystem::ActorPool_FetchActorSetByClass(TSubclassOf<AActor> ActorClass,
	int32 Count)
{
	TArray<AActor*> ActorCollection;

	TActorPoolList* Pool = ActorPoolOfClass.Find(ActorClass);
	if (Pool && Pool->Num() > Count)
	{
		for (int32 i = 0; i < Count; ++i)
		{
			AActor* Actor = Pool->Pop(false);
			ActorCollection.Add(Actor);
		}
	}

	return ActorCollection;
}

TArray<AActor*> UFireflyObjectPoolWorldSubsystem::ActorPool_FetchActorSetByID(FName ActorID, TSubclassOf<AActor> ActorClass, int32 Count)
{
	TArray<AActor*> ActorCollection;

	TActorPoolList* Pool = ActorPoolOfID.Find(ActorID);
	if (Pool && Pool->Num() > Count)
	{
		for (int32 i = 0; i < Count; ++i)
		{
			AActor* Actor = Pool->Pop(false);
			ActorCollection.Add(Actor);
		}
	}

	return ActorCollection;
}

AActor* UFireflyObjectPoolWorldSubsystem::K2_ActorPool_SpawnActor(const UObject* WorldContextObject,
	TSubclassOf<AActor> ActorClass, FName ActorID, const FTransform& Transform, AActor* Owner, APawn* Instigator,
	const ESpawnActorCollisionHandlingMethod CollisionHandling)
{
	return ActorPool_SpawnActor<AActor>(WorldContextObject, ActorClass, ActorID, Transform, Owner, Instigator, CollisionHandling);
}

AActor* UFireflyObjectPoolWorldSubsystem::K2_ActorPool_SpawnActorWithLifetime(const UObject* WorldContextObject,
	TSubclassOf<AActor> ActorClass, FName ActorID, const FTransform& Transform, float Lifetime, AActor* Owner,
	APawn* Instigator, const ESpawnActorCollisionHandlingMethod CollisionHandling)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World) || (!IsValid(ActorClass) && ActorID == NAME_None))
	{
		return nullptr;
	}

	AActor* Actor = ActorPool_SpawnActor<AActor>(WorldContextObject, ActorClass, ActorID
		, Transform, Owner, Instigator, CollisionHandling);

	if (IsValid(Actor))
	{
		FTimerHandle TimerHandle;
		auto TimerLambda = [Actor]() { ActorPool_ReleaseActor(Actor); };
		World->GetTimerManager().SetTimer(TimerHandle, TimerLambda, Lifetime, false);
	}

	return Actor;
}

void UFireflyObjectPoolWorldSubsystem::ActorPool_ReleaseActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	FName ActorID = NAME_None;
	if (Actor->Implements<UFireflyPoolingActorInterface>())
	{
		ActorID = IFireflyPoolingActorInterface::Execute_PoolingGetActorID(Actor);
		IFireflyPoolingActorInterface::Execute_PoolingEndPlay(Actor);
	}

	if (ActorID != NAME_None)
	{
		TActorPoolList& Pool = ActorPoolOfID.FindOrAdd(ActorID);
		Pool.Push(Actor);

		return;
	}

	TActorPoolList& Pool = ActorPoolOfClass.FindOrAdd(Actor->GetClass());	
	Pool.Push(Actor);	
}

void UFireflyObjectPoolWorldSubsystem::ActorPool_WarmUpActorByClass(const UObject* WorldContextObject,
	TSubclassOf<AActor> ActorClass, const FTransform& Transform, int32 Count)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	if (!IsValid(World) || Count <= 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = nullptr;
	SpawnParameters.Instigator = nullptr;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TActorPoolList& Pool = ActorPoolOfClass.FindOrAdd(ActorClass);
	Pool.Reserve(Count);
	for (int32 i = 0; i < Count; i++)
	{
		AActor* Actor = World->SpawnActor<AActor>(ActorClass, Transform, SpawnParameters);
		if (Actor->Implements<UFireflyPoolingActorInterface>())
		{
			IFireflyPoolingActorInterface::Execute_PoolingWarmUp(Actor);
		}

		Pool.Push(Actor);
	}
}

void UFireflyObjectPoolWorldSubsystem::ActorPool_WarmUpActorByID(const UObject* WorldContextObject,
	TSubclassOf<AActor> ActorClass, FName ActorID, const FTransform& Transform, int32 Count)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	if (!IsValid(World) || !IsValid(ActorClass) || ActorID == NAME_None || Count <= 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = nullptr;
	SpawnParameters.Instigator = nullptr;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TActorPoolList& Pool = ActorPoolOfID.FindOrAdd(ActorID);
	Pool.Reserve(Count);
	for (int32 i = 0; i < Count; i++)
	{
		AActor* Actor = World->SpawnActor<AActor>(ActorClass, Transform, SpawnParameters);
		if (Actor->Implements<UFireflyPoolingActorInterface>())
		{
			IFireflyPoolingActorInterface::Execute_PoolingWarmUp(Actor);
			IFireflyPoolingActorInterface::Execute_PoolingSetActorID(Actor, ActorID);
		}

		Pool.Push(Actor);
	}
}

int32 UFireflyObjectPoolWorldSubsystem::ActorPool_DebugActorNumberOfClass(TSubclassOf<AActor> ActorClass)
{
	if (!ActorPoolOfClass.Contains(ActorClass))
	{
		return -1;
	}

	return ActorPoolOfClass[ActorClass].Num();
}

int32 UFireflyObjectPoolWorldSubsystem::ActorPool_DebugActorNumberOfID(FName ActorID)
{
	if (!ActorPoolOfID.Contains(ActorID))
	{
		return -1;
	}

	return ActorPoolOfID[ActorID].Num();
}
