// Copyright Firefly Games, 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FireflyPoolingActorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UFireflyPoolingActorInterface : public UInterface
{
	GENERATED_BODY()
};

/** Interface that actors spawned from actor pool should implement */
/** Actor池生成的Actor需要实现的接口 */
class FIREFLYOBJECTPOOL_API IFireflyPoolingActorInterface
{
	GENERATED_BODY()
	
public:
	// Actor从对象池中生成后执行的BeginPlay
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyObjectPool")
	void PoolingBeginPlay();
	virtual void PoolingBeginPlay_Implementation() {}

	// Actor被放回对象池中后执行的EndPlay
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyObjectPool")
	void PoolingEndPlay();
	virtual void PoolingEndPlay_Implementation() {}

	// Actor从对象池中生成后等待使用执行的WarmUp
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyObjectPool")
	void PoolingWarmUp();
	virtual void PoolingWarmUp_Implementation() {}

	// 获取Actor的ID
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyObjectPool")
	FName PoolingGetActorID() const;
	virtual FName PoolingGetActorID_Implementation() const { return NAME_None; }

	// 设置Actor的ID
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyObjectPool")
	void PoolingSetActorID(FName NewActorID);
	virtual void PoolingSetActorID_Implementation(FName NewActorID) {}
};