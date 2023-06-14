// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FireflyObjectPoolLibrary.generated.h"

/** 用于对象池的通用函数API */
UCLASS()
class FIREFLYOBJECTPOOL_API UFireflyObjectPoolLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

#pragma region Actor_Universal_Pool_Operation

	// Actor通用的从对象池中取出后进行初始化的操作。
	// Common operation for an Actor to initialize after being taken out from the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	void UniversalBeginPlay_Actor(AActor* Actor);

	// Actor通用的回到对象池后进入冻结状态的操作。
	// Common operation for an Actor to enter a frozen state after returning to the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	void UniversalEndPlay_Actor(AActor* Actor);

	// Actor通用的在对象池中生成后进入待命状态的操作。
	// Common operation for an Actor to enter a standby state after being generated in the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	void UniversalWarmUp_Actor(AActor* Actor);

#pragma endregion


#pragma region Pawn_Universal_Pool_Operation

	// Pawn通用的从对象池中取出后进行初始化的操作。
	// Common operation for an Pawn to initialize after being taken out from the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	void UniversalBeginPlay_Pawn(APawn* Pawn);

	// Pawn通用的回到对象池后进入冻结状态的操作。
	// Common operation for an Pawn to enter a frozen state after returning to the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	void UniversalEndPlay_Pawn(APawn* Pawn);

	// Pawn通用的在对象池中生成后进入待命状态的操作。
	// Common operation for an Pawn to enter a standby state after being generated in the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	void UniversalWarmUp_Pawn(APawn* Pawn);

#pragma endregion


#pragma region Character_Universal_Pool_Operation

	// Character通用的从对象池中取出后进行初始化的操作。
	// Common operation for an Character to initialize after being taken out from the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	void UniversalBeginPlay_Character(ACharacter* Character);

	// Character通用的回到对象池后进入冻结状态的操作。
	// Common operation for an Character to enter a frozen state after returning to the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	void UniversalEndPlay_Character(ACharacter* Character);

	// Character通用的在对象池中生成后进入待命状态的操作。
	// Common operation for an Character to enter a standby state after being generated in the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool")
	void UniversalWarmUp_Character(ACharacter* Character);

#pragma endregion
};
