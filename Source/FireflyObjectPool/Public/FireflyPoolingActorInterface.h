// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "FireflyPoolingActorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UFireflyPoolingActorInterface : public UInterface
{
	GENERATED_BODY()
};

/** Actor池生成的Actor需要继承的接口 */
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
