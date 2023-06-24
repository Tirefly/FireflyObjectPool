// Copyright tzlFirefly, 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "UObject/ObjectMacros.h"
#include "Textures/SlateIcon.h"
#include "EdGraph/EdGraphNodeUtils.h"

#include "K2Node_ActorPool_SpawnActor.generated.h"

class FBlueprintActionDatabaseRegistrar;
class UEdGraph;

//
UCLASS()
class FIREFLYOBJECTPOOLDEVELOPER_API UK2Node_ActorPool_SpawnActor : public UK2Node
{
	GENERATED_UCLASS_BODY()

#pragma region UEdGraphNode

public:
	virtual void PostPlacedNewNode() override;
	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	virtual bool HasExternalDependencies(TArray<UStruct*>* OptionalOutput) const override;
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

#pragma endregion


#pragma region UK2Node

public:
	virtual FText GetMenuCategory() const override;
	virtual bool IsNodeSafeToIgnore() const override { return true; }
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void GetNodeAttributes(TArray<TKeyValuePair<FString, FString>>& OutNodeAttributes) const override;
	virtual class FNodeHandlingFunctor* CreateNodeHandler(FKismetCompilerContext& CompilerContext) const override;

#pragma endregion


#pragma region CustomNode

protected:
	// 当类型变更时更新节点的引脚
	void OnClassPinChanged();

	// 该节点是否使用WorldObjectContext输入
	virtual bool UseWorldContext() const;

public:
	// 创建新的引脚来显示原型的属性
	virtual void CreatePinsForClass(UClass* InClass, TArray<UEdGraphPin*>* OutClassPins = nullptr);

	// 查看这是一个派生变量管脚，还是一个‘默认’管脚
	virtual bool IsSpawnVarPin(UEdGraphPin* Pin) const;

	// 获取下一个执行输出的引脚
	UEdGraphPin* GetThenPin() const;

	// 获取世界上下文的引脚
	UEdGraphPin* GetWorldContextPin() const;

	// 获取蓝图类输入的引脚
	UEdGraphPin* GetClassPin(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;

	// 获取ActorID的引脚
	UEdGraphPin* GetIDPin() const;

	// 获取生成Transform矩阵的引脚
	UEdGraphPin* GetSpawnTransformPin() const;

	// 获取Actor的生命周期的引脚
	UEdGraphPin* GetLifetimePin() const;

	// 获取Actor的拥有者的引脚
	UEdGraphPin* GetOwnerPin() const;

	// 获取碰撞处理方式的引脚
	UEdGraphPin* GetCollisionHandlingPin() const;
	
	// 获取输出结果的引脚
	UEdGraphPin* GetResultPin() const;

	// 获取即将生成实例的类型，如果它有默认值
	UClass* GetClassToSpawn(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;

private:
	void MaybeUpdateCollisionPin(TArray<UEdGraphPin*>& OldPins);

protected:
	// Tooltip text for this node
	FText NodeTooltip;

	// Constructing FText strings can be costly, so we cache the node's title
	FNodeTextCache CachedNodeTitle;

#pragma endregion
};
