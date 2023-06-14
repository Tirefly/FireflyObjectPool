// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FireflyPoolingActorInterface.h"
#include "FireflyObjectPoolWorldSubsystem.generated.h"

/** 基于世界的对象池子系统 */
UCLASS()
class FIREFLYOBJECTPOOL_API UFireflyObjectPoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

#pragma region WorldSubsystem

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

#pragma endregion


#pragma region ActorPool_Clear

public:
	// 清理所有Actor池。
	// Clear all actor pools
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	static void ActorPool_ClearAll();

	// 清理指定类的Actor池。
	// Clear the actor pool of specified class.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	static void ActorPool_ClearByClass(TSubclassOf<AActor> ActorClass);

	// 清理指定ID的Actor池。
	// Clear the actor pool of specified ID.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	static void ActorPool_ClearByID(FName ActorID);

#pragma endregion


#pragma region ActorPool_Fetch

public:
	// 从Actor池里提取一个特定类的Actor实例。
	// Extract an actor instance of a specific class from the Actor pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (DisplayName = "Actor Pool Fetch Actor By Class", DeterminesOutputType = "ActorClass"))
	static AActor* K2_ActorPool_FetchActorByClass(TSubclassOf<AActor> ActorClass);

	template<typename T>
	static T* ActorPool_FetchActorByClass(TSubclassOf<T> ActorClass);

	// 从Actor池里提取一个特定ID的Actor实例。
	// Extract an actor instance of a specific ID from the Actor pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (DisplayName = "Actor Pool Fetch Actor By ID", DeterminesOutputType = "ActorClass"))
	static AActor* K2_ActorPool_FetchActorByID(FName ActorID, TSubclassOf<AActor> ActorClass);

	template<typename T>
	static T* ActorPool_FetchActorByID(FName ActorID);

	// 从Actor池里提取一个特定类的Actor实例集。
	// Extract a collection of Actor instances of a specific class from the Actor pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (DeterminesOutputType = "ActorClass"))
	static TArray<AActor*> ActorPool_FetchActorSetByClass(TSubclassOf<AActor> ActorClass, int32 Count = 16);

	// 从Actor池里提取一个特定ID的Actor实例集。
	// Extract a collection of Actor instances of a specific ID from the Actor pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (DeterminesOutputType = "ActorClass"))
	static TArray<AActor*> ActorPool_FetchActorSetByID(FName ActorID, TSubclassOf<AActor> ActorClass, int32 Count = 16);

#pragma endregion


#pragma region ActorPool_Spawn

public:
	// 从Actor池里生成一个Actor，或者根据ID，或者根据Actor类。
	// Spawn an Actor from the Actor pool, either based on ID or based on Actor class.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (DisplayName = "Actor Pool Spawn Actor"
		, WorldContext = "WorldContextObject", DeterminesOutputType = "ActorClass", AdvancedDisplay = "5"))
	static AActor* K2_ActorPool_SpawnActor(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, FName ActorID
		, const FTransform& Transform, AActor* Owner = nullptr, APawn* Instigator = nullptr
		, const ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	// 从Actor池里生成一个Actor，或者根据ID，或者根据Actor类。并且会在给定时间后，将该Actor回收到Actor池中。
	// Spawn an Actor from the Actor pool, either based on ID or based on Actor class. Additionally, the Actor will be recycled back into the Actor pool after a given time.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (DisplayName = "Actor Pool Spawn Actor"
		, WorldContext = "WorldContextObject", DeterminesOutputType = "ActorClass", AdvancedDisplay = "6"))
	static AActor* K2_ActorPool_SpawnActorWithLifetime(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, FName ActorID
		, const FTransform& Transform, float Lifetime, AActor* Owner = nullptr, APawn* Instigator = nullptr
		, const ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	template<typename T>
	static T* ActorPool_SpawnActor(const UObject* WorldContextObject, TSubclassOf<T> ActorClass, FName ActorID
		, const FTransform& Transform, AActor* Owner, APawn* Instigator
		, const ESpawnActorCollisionHandlingMethod CollisionHandling);

#pragma endregion


#pragma region ActorPool_Release

public:
	// 把Actor回收到Actor池里，如果Actor有ID（并且Actor实现了IFireflyPoolingActorInterface::GetActorID）则回到对应ID的Actor池，否则回到Actor类的Actor池。
	// Recycle the Actor back into the Actor pool. If the Actor has an ID (dn implements IFireflyPoolingActorInterface::GetActorID), return it to the ID-based Actor pool; otherwise, return it to the class-based Actor pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (DisplayName = "Actor Pool Release Actor"))
	static void ActorPool_ReleaseActor(AActor* Actor);

#pragma endregion


#pragma region ActorPool_WarmUp

public:
	// 生成特定数量的指定类的Actor并放进Actor池中待命。
	// Spawn a specific number of Actors of a specified class and place them in the Actor pool on standby.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", meta = (WorldContext = "WorldContextObject"))
	static void ActorPool_WarmUpActorByClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& Transform, int32 Count = 16);

	// 生成特定数量的指定ID的Actor并放进Actor池中待命。
	// Pre-generate a specific number of Actors of a specified ID and place them in the Actor pool on standby.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", meta = (WorldContext = "WorldContextObject"))
	static void ActorPool_WarmUpActorByID(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, FName ActorID, const FTransform& Transform, int32 Count = 16);

#pragma endregion


#pragma region ActorPool_Debug

public:
	// 返回在对象池中待命的指定类的Actor的数量，如果不存在指定类的Actor的对象池，则返回-1。
	// Return the number of Actors of a specified class on standby in the object pool. If the object pool for the specified class of Actors does not exist, return -1.
	UFUNCTION(BlueprintPure, Category = "FireflyObjectPool")
	static int32 ActorPool_DebugActorNumberOfClass(TSubclassOf<AActor> ActorClass);

	// 返回在对象池中待命的指定ID的Actor的数量，如果不存在指定ID的Actor的对象池，则返回-1。
	// Return the number of Actors of a specified ID on standby in the object pool. If the object pool for the specified ID of Actors does not exist, return -1.
	UFUNCTION(BlueprintPure, Category = "FireflyObjectPool")
	static int32 ActorPool_DebugActorNumberOfID(FName ActorID);

#pragma endregion


#pragma region ActorPool_Declaration

protected:
	typedef TArray<TObjectPtr<AActor>> TActorPoolList;

	static TMap<TSubclassOf<AActor>, TActorPoolList> ActorPoolOfClass;

	static TMap<FName, TActorPoolList> ActorPoolOfID;

#pragma endregion
};

template <typename T>
T* UFireflyObjectPoolWorldSubsystem::ActorPool_FetchActorByClass(TSubclassOf<T> ActorClass)
{
	TActorPoolList* Pool = ActorPoolOfClass.Find(ActorClass);
	if (Pool && Pool->Num() > 0)
	{
		T* Actor = Pool->Pop(false);

		return Actor;
	}

	return nullptr;
}

template <typename T>
T* UFireflyObjectPoolWorldSubsystem::ActorPool_FetchActorByID(FName ActorID)
{
	TActorPoolList* Pool = ActorPoolOfID.Find(ActorID);
	if (Pool && Pool->Num() > 0)
	{
		T* Actor = Pool->Pop(false);

		return Actor;
	}

	return nullptr;
}

template<typename T>
T* UFireflyObjectPoolWorldSubsystem::ActorPool_SpawnActor(const UObject* WorldContextObject,
	TSubclassOf<T> ActorClass, FName ActorID, const FTransform& Transform, AActor* Owner, APawn* Instigator,
	const ESpawnActorCollisionHandlingMethod CollisionHandling)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World) || (!IsValid(ActorClass) && ActorID == NAME_None))
	{
		return nullptr;
	}

	T* Actor = ActorPool_FetchActorByID<T>(ActorID);
	if (!IsValid(Actor))
	{
		Actor = ActorPool_FetchActorByClass<T>(ActorClass);
	}

	if (Actor)
	{
		Actor->SetActorTransform(Transform, true, nullptr, ETeleportType::ResetPhysics);
		Actor->SetOwner(Owner);
		Actor->SetInstigator(Instigator);

		if (Actor->Implements<UFireflyPoolingActorInterface>())
		{
			IFireflyPoolingActorInterface::Execute_PoolingBeginPlay(Actor);
		}

		return Actor;
	}

	if (!IsValid(ActorClass))
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Owner;
	SpawnParameters.Instigator = Instigator;
	SpawnParameters.SpawnCollisionHandlingOverride = CollisionHandling;

	Actor = World->SpawnActor<T>(ActorClass, Transform, SpawnParameters);
	if (Actor->Implements<UFireflyPoolingActorInterface>())
	{
		IFireflyPoolingActorInterface::Execute_PoolingSetActorID(Actor, ActorID);
		IFireflyPoolingActorInterface::Execute_PoolingBeginPlay(Actor);
	}

	return Actor;
}
