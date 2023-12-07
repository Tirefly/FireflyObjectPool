// Copyright tzlFirefly, 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FireflyPoolingActorInterface.h"
#include "FireflyObjectPoolWorldSubsystem.generated.h"

/** 基于世界的对象池子系统 */
/** World based object pool subsystem */
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
	// 从Actor池里提取一个特定类的Actor实例。请确保要使用的对象池存在，且对象池中确实有可使用的Actor实例。
	// Extract an actor instance of a specific class from the Actor pool. Make sure that the object pool you want to use exists and that there are Actor instances available in the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (DisplayName = "Actor Pool Fetch Actor", DeterminesOutputType = "ActorClass"))
	static AActor* K2_ActorPool_FetchActor(TSubclassOf<AActor> ActorClass, FName ActorID);

	template<typename T>
	static T* ActorPool_FetchActor(TSubclassOf<T> ActorClass, FName ActorID);

	// 从Actor池里提取一个特定类的Actor实例集。请确保要使用的对象池存在，且对象池中确实有可使用的Actor实例。
	// Extract a collection of Actor instances of a specific class from the Actor pool. Make sure that the object pool you want to use exists and that there are Actor instances available in the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (DisplayName = "Actor Pool Fetch Actors", DeterminesOutputType = "ActorClass"))
	static TArray<AActor*> K2_ActorPool_FetchActors(TSubclassOf<AActor> ActorClass, FName ActorID, int32 Count = 16);

	template<typename T>
	static TArray<T*> ActorPool_FetchActors(TSubclassOf<T> ActorClass, FName ActorID, int32 Count = 16);

#pragma endregion


#pragma region ActorPool_Spawn

protected:
	AActor* SpawnActor_Internal(TSubclassOf<AActor> ActorClass, FName ActorID, const FTransform& Transform, float Lifetime
		, AActor* Owner, APawn* Instigator, const ESpawnActorCollisionHandlingMethod CollisionHandling);

public:
	// 从ActorPool生成执行指定Actor类的实例，但不会自动运行其构造脚本及其ActorPool初始化。
	// Spawns an instance of the specified actor class from ActorPool, but does not automatically run its construction script and its ActorPool initialization.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", meta = (WorldContext = "WorldContext"
		, UnsafeDuringActorConstruction = "true", BlueprintInternalUseOnly = "true"))
	static AActor* ActorPool_BeginDeferredActorSpawn(const UObject* WorldContext
		, TSubclassOf<AActor> ActorClass, FName ActorID, const FTransform& SpawnTransform, AActor* Owner = nullptr
		, ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	// 完成从ActorPool生成一个Actor实例，并且会执行Actor的构造脚本和ActorPool初始化。
	// 'Finish' spawning an actor from ActorPool.  This will run the construction script and the ActorPool initialization.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", meta = (WorldContext = "WorldContext"
		, UnsafeDuringActorConstruction = "true", BlueprintInternalUseOnly = "true"))
	static AActor* ActorPool_FinishSpawningActor(const UObject* WorldContext, AActor* Actor, const FTransform& SpawnTransform, float Lifetime);

	template<typename T>
	T* ActorPool_SpawnActor(TSubclassOf<T> ActorClass, FName ActorID, const FTransform& Transform, float Lifetime
		, AActor* Owner, APawn* Instigator, const ESpawnActorCollisionHandlingMethod CollisionHandling);

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
	// 生成特定数量的指定类以及指定ID的Actor并放进Actor池中待命。
	// Spawn a specific number of Actors of a specified class and a specified ID ,and place them in the Actor pool on standby.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", meta = (WorldContext = "WorldContextObject"))
	static void ActorPool_WarmUp(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, FName ActorID, const FTransform& Transform, int32 Count = 16);
	
#pragma endregion


#pragma region ActorPool_Debug

public:
	// 返回在Actor类对象池中所有的Actor类型。
	// Return all Actor classes of ActorPoolOfClass.
	UFUNCTION(BlueprintPure, Category = "FireflyObjectPool")
	static TArray<TSubclassOf<AActor>> ActorPool_DebugActorClasses();

	// 返回在ActorID对象池中所有的ActorID。
	// Return all Actor IDs of ActorPoolOfID.
	UFUNCTION(BlueprintPure, Category = "FireflyObjectPool")
	static TArray<FName> ActorPool_DebugActorIDs();

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

#pragma region ActorPool_FunctionTemplate

template <typename T>
T* UFireflyObjectPoolWorldSubsystem::ActorPool_FetchActor(TSubclassOf<T> ActorClass, FName ActorID)
{
	TActorPoolList* Pool = ActorPoolOfID.Find(ActorID);
	if (!Pool)
	{
		Pool = ActorPoolOfClass.Find(ActorClass);
	}

	if (Pool && Pool->Num() > 0)
	{
		T* Actor = Pool->Pop(false);

		return Actor;
	}

	return nullptr;
}

template <typename T>
TArray<T*> UFireflyObjectPoolWorldSubsystem::ActorPool_FetchActors(TSubclassOf<T> ActorClass, FName ActorID,
	int32 Count)
{
	TArray<T*> ActorCollection;
	for (int32 i = 0; i < Count; ++i)
	{
		if (T* Actor = ActorPool_FetchActor<T>(ActorClass, ActorID))
		{
			ActorCollection.Add(Actor);
		}
	}

	return ActorCollection;
}

template<typename T>
T* UFireflyObjectPoolWorldSubsystem::ActorPool_SpawnActor(TSubclassOf<T> ActorClass, FName ActorID
	, const FTransform& Transform, float Lifetime, AActor* Owner, APawn* Instigator
	, const ESpawnActorCollisionHandlingMethod CollisionHandling)
{
	return Cast<T>(SpawnActor_Internal(ActorClass, ActorID, Transform, Lifetime, Owner, Instigator, CollisionHandling));
}

#pragma endregion