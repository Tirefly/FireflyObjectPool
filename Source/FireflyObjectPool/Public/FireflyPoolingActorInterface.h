// Copyright tzlFirefly, 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FireflyPoolingActorInterface.generated.h"


UINTERFACE(MinimalAPI, BlueprintType)
class UFireflyPoolingActorInterface : public UInterface
{
	GENERATED_BODY()
};

/** Actor池生成的Actor需要实现的接口 */
/** Interface that actors spawned from actor pool should implement */
class FIREFLYOBJECTPOOL_API IFireflyPoolingActorInterface
{
	GENERATED_BODY()
	
public:
	// Actor从对象池中生成后执行的BeginPlay。
	// BeginPlay executed after the Actor is spawned from the object pool.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FireflyObjectPool")
	void PoolingBeginPlay();
	virtual void PoolingBeginPlay_Implementation() {}

	// Actor被放回对象池中后执行的EndPlay。
	// EndPlay executed after the Actor is returned to the object pool.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FireflyObjectPool")
	void PoolingEndPlay();
	virtual void PoolingEndPlay_Implementation() {}

	// Actor从对象池中生成后等待使用执行的WarmUp。
	// WarmUp executed after the Actor is spawned from the object pool, waiting to be used.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FireflyObjectPool")
	void PoolingWarmUp();
	virtual void PoolingWarmUp_Implementation() {}

	// 获取Actor的ID。
	// Get the ID of the Actor.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FireflyObjectPool")
	FName PoolingGetActorID() const;
	virtual FName PoolingGetActorID_Implementation() const { return NAME_None; }

	// 设置Actor的ID。
	// Set the ID of the Actor.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FireflyObjectPool")
	void PoolingSetActorID(FName NewActorID);
	virtual void PoolingSetActorID_Implementation(FName NewActorID) {}
};
