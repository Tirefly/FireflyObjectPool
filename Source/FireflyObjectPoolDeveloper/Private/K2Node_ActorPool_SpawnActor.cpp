// Copyright tzlFirefly, 2023. All Rights Reserved.


#include "K2Node_ActorPool_SpawnActor.h"

#include "GameFramework/Actor.h"
#include "UObject/UnrealType.h"
#include "Engine/EngineTypes.h"

#include "FireflyObjectPoolWorldSubsystem.h"

#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompilerMisc.h"
#include "KismetCompiler.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "EdGraph/EdGraph.h"

#include "K2Node_CallFunction.h"
#include "K2Node_EnumLiteral.h"
#include "K2Node_Select.h"


struct FK2Node_ActorPool_SpawnActor_Helper
{
	static FName WorldContextPinName;
	static FName ClassPinName;
	static FName IDPinName;
	static FName SpawnTransformPinName;
	static FName LifetimePinName;
	static FName OwnerPinName;	
	static FName CollisionHandlingPinName;
	static FName ActorPinName;

	static FName SpawnEvenIfCollidingPinName;
	static FName NoCollisionFailPinName;
};

FName FK2Node_ActorPool_SpawnActor_Helper::WorldContextPinName(TEXT("WorldContext"));
FName FK2Node_ActorPool_SpawnActor_Helper::ClassPinName(TEXT("ActorClass"));
FName FK2Node_ActorPool_SpawnActor_Helper::IDPinName(TEXT("ActorID"));
FName FK2Node_ActorPool_SpawnActor_Helper::SpawnTransformPinName(TEXT("SpawnTransform"));
FName FK2Node_ActorPool_SpawnActor_Helper::LifetimePinName(TEXT("Lifetime"));
FName FK2Node_ActorPool_SpawnActor_Helper::OwnerPinName(TEXT("Owner"));
FName FK2Node_ActorPool_SpawnActor_Helper::CollisionHandlingPinName(TEXT("CollisionHandling"));
FName FK2Node_ActorPool_SpawnActor_Helper::ActorPinName(TEXT("Actor"));

FName FK2Node_ActorPool_SpawnActor_Helper::SpawnEvenIfCollidingPinName(TEXT("SpawnEvenIfColliding"));		// deprecated pin, name kept for backwards compat
FName FK2Node_ActorPool_SpawnActor_Helper::NoCollisionFailPinName(TEXT("bNoCollisionFail"));		// deprecated pin, name kept for backwards compat


#define LOCTEXT_NAMESPACE "FireflyObjectPool"


UK2Node_ActorPool_SpawnActor::UK2Node_ActorPool_SpawnActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeTooltip = LOCTEXT("NodeTooltip", "Attempts to Spawn an Actor from ActorPool and will create a new Actor if there is no available Actor in ActorPool.");
}

void UK2Node_ActorPool_SpawnActor::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();
	
	UClass* ClassToSpawn = GetClassToSpawn();
	if (ClassToSpawn != nullptr)
	{
		TArray<UEdGraphPin*> ClassPins;
		CreatePinsForClass(ClassToSpawn, &ClassPins);
	}
}

void UK2Node_ActorPool_SpawnActor::AllocateDefaultPins()
{
	// Add execution pins
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	// If required add the world context pin
	if (UseWorldContext())
	{
		CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass(), FK2Node_ActorPool_SpawnActor_Helper::WorldContextPinName);
	}
	
	// ActorClass pin
	UEdGraphPin* ClassPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Class, AActor::StaticClass(), FK2Node_ActorPool_SpawnActor_Helper::ClassPinName);

	// ActorID pin
	UEdGraphPin* IDPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Name, FK2Node_ActorPool_SpawnActor_Helper::IDPinName);
	IDPin->DefaultValue = FName(NAME_None).ToString();

	// Transform pin
	UScriptStruct* TransformStruct = TBaseStructure<FTransform>::Get();
	UEdGraphPin* SpawnTransformPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, TransformStruct, FK2Node_ActorPool_SpawnActor_Helper::SpawnTransformPinName);

	// Lifetime pin
	UEdGraphPin* LifetimePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Double, FK2Node_ActorPool_SpawnActor_Helper::LifetimePinName);
	LifetimePin->DefaultValue = FString::SanitizeFloat(-1.f);

	// Owner pin
	UEdGraphPin* OwnerPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, AActor::StaticClass(), FK2Node_ActorPool_SpawnActor_Helper::OwnerPinName);
	OwnerPin->bAdvancedView = true;
	
	// Collision handling method pin
	UEnum* const CollisionHandlingEnum = FindObjectChecked<UEnum>(nullptr, TEXT("/Script/Engine.ESpawnActorCollisionHandlingMethod"), true);
	UEdGraphPin* const CollisionHandlingOverridePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Byte
		, CollisionHandlingEnum, FK2Node_ActorPool_SpawnActor_Helper::CollisionHandlingPinName);
	CollisionHandlingOverridePin->DefaultValue = CollisionHandlingEnum->GetNameStringByValue(static_cast<int>(ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
	CollisionHandlingOverridePin->bAdvancedView = true;

	if (ENodeAdvancedPins::NoPins == AdvancedPinDisplay)
	{
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
	}

	// Result pin
	UEdGraphPin* ResultPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Object, AActor::StaticClass(), UEdGraphSchema_K2::PN_ReturnValue);

	Super::AllocateDefaultPins();
}

FText UK2Node_ActorPool_SpawnActor::GetTooltipText() const
{
	return NodeTooltip;
}

void UK2Node_ActorPool_SpawnActor::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	if (Pin && (Pin->PinName == FK2Node_ActorPool_SpawnActor_Helper::ClassPinName))
	{
		OnClassPinChanged();
	}
}

void UK2Node_ActorPool_SpawnActor::PinConnectionListChanged(UEdGraphPin* Pin)
{
	if (Pin && (Pin->PinName == FK2Node_ActorPool_SpawnActor_Helper::ClassPinName))
	{
		OnClassPinChanged();
	}
}

FSlateIcon UK2Node_ActorPool_SpawnActor::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), "GraphEditor.SpawnActor_16x");
	return Icon;
}

FText UK2Node_ActorPool_SpawnActor::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FText NodeTitle = NSLOCTEXT("K2Node", "ActorPool_SpawnActor_BaseTitle", "Actor Pool Spawn Actor");
	if (TitleType != ENodeTitleType::MenuTitle)
	{
		if (UEdGraphPin* ClassPin = GetClassPin())
		{
			if (ClassPin->LinkedTo.Num() > 0)
			{
				// Blueprint will be determined dynamically, so we don't have the name in this case
				NodeTitle = NSLOCTEXT("K2Node", "ActorPool_SpawnActor_Title_Unknown", "ActorPool_SpawnActor");
			}
			else if (ClassPin->DefaultObject == nullptr)
			{
				NodeTitle = NSLOCTEXT("K2Node", "ActorPool_SpawnActor_Title_NONE", "ActorPool_SpawnActor NONE");
			}
			else
			{
				if (CachedNodeTitle.IsOutOfDate(this))
				{
					FText ClassName;
					if (UClass* PickedClass = Cast<UClass>(ClassPin->DefaultObject))
					{
						ClassName = PickedClass->GetDisplayNameText();
					}

					FFormatNamedArguments Args;
					Args.Add(TEXT("ClassName"), ClassName);

					// FText::Format() is slow, so we cache this to save on performance
					CachedNodeTitle.SetCachedText(FText::Format(NSLOCTEXT("K2Node", "ActorPool_SpawnActor_Title_Class", "ActorPool_SpawnActor {ClassName}"), Args), this);
				}
				NodeTitle = CachedNodeTitle;
			}
		}
		else
		{
			NodeTitle = NSLOCTEXT("K2Node", "ActorPool_SpawnActor_Title_NONE", "ActorPool_SpawnActor NONE");
		}
	}
	return NodeTitle;
}

bool UK2Node_ActorPool_SpawnActor::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);

	return Super::IsCompatibleWithGraph(TargetGraph)
		&& (!Blueprint || (FBlueprintEditorUtils::FindUserConstructionScript(Blueprint) != TargetGraph 
			&& Blueprint->GeneratedClass->GetDefaultObject()->ImplementsGetWorld()));
}

bool UK2Node_ActorPool_SpawnActor::HasExternalDependencies(TArray<UStruct*>* OptionalOutput) const
{
	UClass* SourceClass = GetClassToSpawn();
	const UBlueprint* SourceBlueprint = GetBlueprint();
	const bool bResult = (SourceClass && (SourceClass->ClassGeneratedBy != SourceBlueprint));
	if (bResult && OptionalOutput)
	{
		OptionalOutput->AddUnique(SourceClass);
	}
	const bool bSuperResult = Super::HasExternalDependencies(OptionalOutput);
	return bSuperResult || bResult;
}

void UK2Node_ActorPool_SpawnActor::GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	
	if (UEdGraphPin* ClassPin = GetClassPin())
	{
		K2Schema->ConstructBasicPinTooltip(*ClassPin
			, LOCTEXT("ClassPinDescription", "The ActorClass to Spawn from ActorPool.")
			, ClassPin->PinToolTip);
	}

	if (UEdGraphPin* IDPin = GetIDPin())
	{
		K2Schema->ConstructBasicPinTooltip(*IDPin
			, LOCTEXT("IDPinDescription", "ActorID is used to spawn Actor from object pools or to initialize newly spawned Actor instances based on a data-driven way.")
			, IDPin->PinToolTip);
	}

	if (UEdGraphPin* SpawnTransformPin = GetSpawnTransformPin())
	{
		K2Schema->ConstructBasicPinTooltip(*SpawnTransformPin
			, LOCTEXT("TransformPinDescription", "The transform to spawn the Actor with.")
			, SpawnTransformPin->PinToolTip);
	}

	if (UEdGraphPin* LifetimePin = GetLifetimePin())
	{
		K2Schema->ConstructBasicPinTooltip(*LifetimePin
			, LOCTEXT("LifetimePinDescription", "Lifetime of Actor spawned from the object pool. When the time arrives, Actor will return to ActorPool. If the value is -1, there is no lifetime limit for Actor.")
			, LifetimePin->PinToolTip);
	}

	if (UEdGraphPin* OwnerPin = GetOwnerPin())
	{
		K2Schema->ConstructBasicPinTooltip(*OwnerPin
			, LOCTEXT("OwnerPinDescription", "Can be left empty; primarily used for replication or visibility.")
			, OwnerPin->PinToolTip);
	}

	if (UEdGraphPin* CollisionHandlingPin = GetCollisionHandlingPin())
	{
		K2Schema->ConstructBasicPinTooltip(*CollisionHandlingPin
			, LOCTEXT("CollisionHandlingPinDescription", "Specifies how to handle collisions at the Spawn Point. If undefined, uses Actor Class Settings.")
			, CollisionHandlingPin->PinToolTip);
	}	

	if (UEdGraphPin* ResultPin = GetResultPin())
	{
		K2Schema->ConstructBasicPinTooltip(*ResultPin
			, LOCTEXT("ResultPinDescription", "Actor instance spawned from the object pool of the specified ActorClass. If the spawning fails, the value is null.")
			, ResultPin->PinToolTip);
	}

	return Super::GetPinHoverText(Pin, HoverTextOut);
}

void UK2Node_ActorPool_SpawnActor::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	if (!IsValid(this) || !IsValid(SourceGraph))
	{
		BreakAllNodeLinks(); return;
	}

	static const FName BeginSpawningBlueprintFuncName = GET_FUNCTION_NAME_CHECKED(UFireflyObjectPoolWorldSubsystem, ActorPool_BeginDeferredActorSpawn);
	static const FName FinishSpawningFuncName = GET_FUNCTION_NAME_CHECKED(UFireflyObjectPoolWorldSubsystem, ActorPool_FinishSpawningActor);

	UK2Node_ActorPool_SpawnActor* TheNode = this;
	UEdGraphPin* TheNodeExec = TheNode->GetExecPin();
	UEdGraphPin* TheNodeThen = TheNode->GetThenPin();
	UEdGraphPin* TheNodeResult = TheNode->GetResultPin();
	UEdGraphPin* TheWorldContextPin = TheNode->GetWorldContextPin();
	UEdGraphPin* TheClassPin = TheNode->GetClassPin();
	UEdGraphPin* TheIDPin = TheNode->GetIDPin();
	UEdGraphPin* TheSpawnTransformPin = TheNode->GetSpawnTransformPin();
	UEdGraphPin* TheLifetimePin = TheNode->GetLifetimePin();
	UEdGraphPin* TheOwnerPin = TheNode->GetOwnerPin();
	UEdGraphPin* TheCollisionHandlingPin = GetCollisionHandlingPin();

	// Cache the class to spawn. Note, this is the compile time class that the pin was set to or the variable type it was connected to. Runtime it could be a child.
	UClass* ClassToSpawn = GetClassToSpawn();
	UClass* SpawnClass = (TheClassPin != NULL) ? Cast<UClass>(TheClassPin->DefaultObject) : NULL;
	if (!TheClassPin || ((0 == TheClassPin->LinkedTo.Num()) && (NULL == SpawnClass)))
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("SpawnActorNodeMissingClass_Error", "Spawn node @@ must have a @@ specified.").ToString(), TheNode, TheClassPin);
		// we break exec links so this is the only error we get, don't want the SpawnActor node being considered and giving 'unexpected node' type warnings
		TheNode->BreakAllNodeLinks();
		return;
	}
	
#pragma region BeginSpawn

	UK2Node_CallFunction* CallBeginTheNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(TheNode, SourceGraph);
	CallBeginTheNode->FunctionReference.SetExternalMember(BeginSpawningBlueprintFuncName, UFireflyObjectPoolWorldSubsystem::StaticClass());
	CallBeginTheNode->AllocateDefaultPins();

	UEdGraphPin* CallBeginExec = CallBeginTheNode->GetExecPin();
	UEdGraphPin* CallBeginResult = CallBeginTheNode->GetReturnValuePin();
	UEdGraphPin* CallBeginWorldContextPin = CallBeginTheNode->FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::WorldContextPinName);
	UEdGraphPin* CallBeginClassPin = CallBeginTheNode->FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::ClassPinName);
	UEdGraphPin* CallBeginIDPin = CallBeginTheNode->FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::IDPinName);
	UEdGraphPin* CallBeginSpawnTransformPin = CallBeginTheNode->FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::SpawnTransformPinName);
	UEdGraphPin* CallBeginOwnerPin = CallBeginTheNode->FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::OwnerPinName);
	UEdGraphPin* CallBeginCollisionHandlingPin = CallBeginTheNode->FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::CollisionHandlingPinName);

	// Copy the world context connection from the spawn node to 'begin spawn' if necessary
	if (TheWorldContextPin)
	{
		CompilerContext.MovePinLinksToIntermediate(*TheWorldContextPin, *CallBeginWorldContextPin);
	}

	if (TheClassPin->LinkedTo.Num() > 0)
	{
		CompilerContext.MovePinLinksToIntermediate(*TheClassPin, *CallBeginClassPin);
	}
	else
	{
		CallBeginClassPin->DefaultObject = SpawnClass;
	}
	
	if (TheOwnerPin != nullptr)
	{
		CompilerContext.MovePinLinksToIntermediate(*TheOwnerPin, *CallBeginOwnerPin);
	}
	
	CompilerContext.MovePinLinksToIntermediate(*TheNodeExec, *CallBeginExec);
	CompilerContext.MovePinLinksToIntermediate(*TheIDPin, *CallBeginIDPin);
	CompilerContext.MovePinLinksToIntermediate(*TheSpawnTransformPin, *CallBeginSpawnTransformPin);
	CompilerContext.MovePinLinksToIntermediate(*TheCollisionHandlingPin, *CallBeginCollisionHandlingPin);

#pragma endregion


#pragma region FinishSpawn

	UK2Node_CallFunction* CallFinishTheNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(TheNode, SourceGraph);
	CallFinishTheNode->FunctionReference.SetExternalMember(FinishSpawningFuncName, UFireflyObjectPoolWorldSubsystem::StaticClass());
	CallFinishTheNode->AllocateDefaultPins();

	UEdGraphPin* CallFinishExec = CallFinishTheNode->GetExecPin();
	UEdGraphPin* CallFinishThen = CallFinishTheNode->GetThenPin();
	UEdGraphPin* CallFinishResult = CallFinishTheNode->GetReturnValuePin();
	UEdGraphPin* CallFinishWorldContextPin = CallFinishTheNode->FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::WorldContextPinName);
	UEdGraphPin* CallFinishActorPin = CallFinishTheNode->FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::ActorPinName);
	UEdGraphPin* CallFinishSpawnTransformPin = CallFinishTheNode->FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::SpawnTransformPinName);
	UEdGraphPin* CallFinishLifetimePin = CallFinishTheNode->FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::LifetimePinName);
	
	CompilerContext.MovePinLinksToIntermediate(*TheNodeThen, *CallFinishThen);
	CompilerContext.CopyPinLinksToIntermediate(*CallBeginWorldContextPin, *CallFinishWorldContextPin);
	CompilerContext.CopyPinLinksToIntermediate(*CallBeginSpawnTransformPin, *CallFinishSpawnTransformPin);
	CompilerContext.MovePinLinksToIntermediate(*TheLifetimePin, *CallFinishLifetimePin);
	
	// Move result connection from spawn node to 'finish spawn'
	CallBeginResult->MakeLinkTo(CallFinishActorPin);
	CallFinishResult->PinType = TheNodeResult->PinType; // Copy type so it uses the right actor subclass
	CompilerContext.MovePinLinksToIntermediate(*TheNodeResult, *CallFinishResult);

#pragma endregion

	
#pragma region SetVariable

	// Get 'result' pin from 'begin spawn', this is the actual actor we want to set properties on
	UEdGraphPin* LastThen = FKismetCompilerUtilities::GenerateAssignmentNodes(CompilerContext, SourceGraph, CallBeginTheNode, TheNode, CallBeginResult, ClassToSpawn);
	// Make exec connection between 'then' on last node and 'finish'
	LastThen->MakeLinkTo(CallFinishExec);
	// Break any links to the expanded node
	TheNode->BreakAllNodeLinks();

#pragma endregion
}

FText UK2Node_ActorPool_SpawnActor::GetMenuCategory() const
{
	return LOCTEXT("PoolCategory", "FireflyObjectPool");
}

void UK2Node_ActorPool_SpawnActor::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();

	if (UClass* ClassToSpawn = GetClassToSpawn(&OldPins))
	{
		TArray<UEdGraphPin*>ClassPins;
		CreatePinsForClass(ClassToSpawn, &ClassPins);
	}
	
	MaybeUpdateCollisionPin(OldPins);
	RestoreSplitPins(OldPins);
}

void UK2Node_ActorPool_SpawnActor::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	// actions get registered under specific object-keys; the idea is that 
	// actions might have to be updated (or deleted) if their object-key is  
	// mutated (or removed)... here we use the node's class (so if the node 
	// type disappears, then the action should go with it)
	UClass* ActionKey = GetClass();
	// to keep from needlessly instantiating a UBlueprintNodeSpawner, first   
	// check to make sure that the registrar is looking for actions of this type
	// (could be regenerating actions for a specific asset, and therefore the 
	// registrar would only accept actions corresponding to that asset)
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

void UK2Node_ActorPool_SpawnActor::GetNodeAttributes(TArray<TKeyValuePair<FString, FString>>& OutNodeAttributes) const
{
	UClass* ClassToSpawn = GetClassToSpawn();
	const FString ClassToSpawnStr = ClassToSpawn ? ClassToSpawn->GetName() : TEXT("InvalidClass");

	OutNodeAttributes.Add(TKeyValuePair<FString, FString>(TEXT("Type"), TEXT("ActorPool_SpawnActor")));
	OutNodeAttributes.Add(TKeyValuePair<FString, FString>(TEXT("Class"), GetClass()->GetName()));
	OutNodeAttributes.Add(TKeyValuePair<FString, FString>(TEXT("Name"), GetName()));
	OutNodeAttributes.Add(TKeyValuePair<FString, FString>(TEXT("ActorClass"), ClassToSpawnStr));
}

FNodeHandlingFunctor* UK2Node_ActorPool_SpawnActor::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
	return new FNodeHandlingFunctor(CompilerContext);
}

void UK2Node_ActorPool_SpawnActor::OnClassPinChanged()
{
	// Remove all pins related to archetype variables
	TArray<UEdGraphPin*> OldPins = Pins;
	TArray<UEdGraphPin*> OldClassPins;
	
	for (UEdGraphPin* OldPin : OldPins)
	{
		if (IsSpawnVarPin(OldPin))
		{
			Pins.Remove(OldPin);
			OldClassPins.Add(OldPin);
		}
	}
	
	CachedNodeTitle.MarkDirty();
	
	TArray<UEdGraphPin*> NewClassPins;
	if (UClass* UseSpawnClass = GetClassToSpawn())
	{
		CreatePinsForClass(UseSpawnClass, &NewClassPins);
	}
	
	RestoreSplitPins(OldPins);
	
	UEdGraphPin* ResultPin = GetResultPin();
	// Cache all the pin connections to the ResultPin, we will attempt to recreate them
	TArray<UEdGraphPin*> ResultPinConnectionList = ResultPin->LinkedTo;
	// Because the archetype has changed, we break the output link as the output pin type will change
	ResultPin->BreakAllPinLinks(true);
	
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	
	// Recreate any pin links to the Result pin that are still valid
	for (UEdGraphPin* Connections : ResultPinConnectionList)
	{
		K2Schema->TryCreateConnection(ResultPin, Connections);
	}
	
	// Rewire the old pins to the new pins so connections are maintained if possible
	RewireOldPinsToNewPins(OldClassPins, Pins, nullptr);
	
	// Refresh the UI for the graph so the pin changes show up
	GetGraph()->NotifyGraphChanged();
	
	// Mark dirty
	FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
}

bool UK2Node_ActorPool_SpawnActor::UseWorldContext() const
{
	UBlueprint* BP = GetBlueprint();
	const UClass* ParentClass = BP ? BP->ParentClass : nullptr;
	return ParentClass ? ParentClass->HasMetaDataHierarchical(FBlueprintMetadata::MD_ShowWorldContextPin) != nullptr : false;
}

void UK2Node_ActorPool_SpawnActor::CreatePinsForClass(UClass* InClass, TArray<UEdGraphPin*>* OutClassPins)
{
	check(InClass);

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	const UObject* const ClassDefaultObject = InClass->GetDefaultObject(false);

	for (TFieldIterator<FProperty> PropertyIt(InClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;
		UClass* PropertyClass = CastChecked<UClass>(Property->GetOwner<UObject>());
		const bool bIsDelegate = Property->IsA(FMulticastDelegateProperty::StaticClass());
		const bool bIsExposedToSpawn = UEdGraphSchema_K2::IsPropertyExposedOnSpawn(Property);
		const bool bIsSettableExternally = !Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance);

		if (bIsExposedToSpawn &&
			!Property->HasAnyPropertyFlags(CPF_Parm) &&
			bIsSettableExternally &&
			Property->HasAllPropertyFlags(CPF_BlueprintVisible) &&
			!bIsDelegate &&
			(nullptr == FindPin(Property->GetFName())) &&
			FBlueprintEditorUtils::PropertyStillExists(Property))
		{
			if (UEdGraphPin* Pin = CreatePin(EGPD_Input, NAME_None, Property->GetFName()))
			{
				K2Schema->ConvertPropertyToPinType(Property, /*out*/ Pin->PinType);
				if (OutClassPins)
				{
					OutClassPins->Add(Pin);
				}

				if (ClassDefaultObject && K2Schema->PinDefaultValueIsEditable(*Pin))
				{
					FString DefaultValueAsString;
					const bool bDefaultValueSet = FBlueprintEditorUtils::PropertyValueToString(Property, reinterpret_cast<const uint8*>(ClassDefaultObject), DefaultValueAsString, this);
					check(bDefaultValueSet);
					K2Schema->SetPinAutogeneratedDefaultValue(Pin, DefaultValueAsString);
				}

				// Copy tooltip from the property.
				K2Schema->ConstructBasicPinTooltip(*Pin, Property->GetToolTipText(), Pin->PinToolTip);
			}
		}
	}

	// Change class of output pin
	UEdGraphPin* ResultPin = GetResultPin();
	ResultPin->PinType.PinSubCategoryObject = InClass->GetAuthoritativeClass();
}

bool UK2Node_ActorPool_SpawnActor::IsSpawnVarPin(UEdGraphPin* Pin) const
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	UEdGraphPin* ParentPin = Pin->ParentPin;
	while (ParentPin)
	{
		if (ParentPin->PinName == FK2Node_ActorPool_SpawnActor_Helper::SpawnTransformPinName)
		{
			return false;
		}

		ParentPin = ParentPin->ParentPin;
	}

	return (Pin->PinName != K2Schema->PN_Then &&
		Pin->PinName != K2Schema->PN_Execute &&
		Pin->PinName != K2Schema->PN_ReturnValue &&
		Pin->PinName != FK2Node_ActorPool_SpawnActor_Helper::WorldContextPinName &&
		Pin->PinName != FK2Node_ActorPool_SpawnActor_Helper::ClassPinName &&
		Pin->PinName != FK2Node_ActorPool_SpawnActor_Helper::IDPinName &&
		Pin->PinName != FK2Node_ActorPool_SpawnActor_Helper::SpawnTransformPinName &&
		Pin->PinName != FK2Node_ActorPool_SpawnActor_Helper::LifetimePinName &&
		Pin->PinName != FK2Node_ActorPool_SpawnActor_Helper::OwnerPinName &&		
		Pin->PinName != FK2Node_ActorPool_SpawnActor_Helper::CollisionHandlingPinName
	);
}

UEdGraphPin* UK2Node_ActorPool_SpawnActor::GetThenPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_Then);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UK2Node_ActorPool_SpawnActor::GetWorldContextPin() const
{
	UEdGraphPin* Pin = FindPin(FK2Node_ActorPool_SpawnActor_Helper::WorldContextPinName);
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_ActorPool_SpawnActor::GetClassPin(const TArray<UEdGraphPin*>* InPinsToSearch) const
{
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin* Pin = nullptr;
	for (UEdGraphPin* TestPin : *PinsToSearch)
	{
		if (TestPin && TestPin->PinName == FK2Node_ActorPool_SpawnActor_Helper::ClassPinName)
		{
			Pin = TestPin;
			break;
		}
	}
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_ActorPool_SpawnActor::GetIDPin() const
{
	UEdGraphPin* Pin = FindPin(FK2Node_ActorPool_SpawnActor_Helper::IDPinName);
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_ActorPool_SpawnActor::GetSpawnTransformPin() const
{
	UEdGraphPin* Pin = FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::SpawnTransformPinName);
	check(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_ActorPool_SpawnActor::GetLifetimePin() const
{
	UEdGraphPin* Pin = FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::LifetimePinName);
	check(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_ActorPool_SpawnActor::GetOwnerPin() const
{
	UEdGraphPin* Pin = FindPin(FK2Node_ActorPool_SpawnActor_Helper::OwnerPinName);
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_ActorPool_SpawnActor::GetCollisionHandlingPin() const
{
	UEdGraphPin* const Pin = FindPinChecked(FK2Node_ActorPool_SpawnActor_Helper::CollisionHandlingPinName);
	check(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_ActorPool_SpawnActor::GetResultPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UClass* UK2Node_ActorPool_SpawnActor::GetClassToSpawn(const TArray<UEdGraphPin*>* InPinsToSearch) const
{
	UClass* UseSpawnClass = nullptr;
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin* ClassPin = GetClassPin(PinsToSearch);
	if (ClassPin && ClassPin->DefaultObject && ClassPin->LinkedTo.Num() == 0)
	{
		UseSpawnClass = CastChecked<UClass>(ClassPin->DefaultObject);
	}
	else if (ClassPin && ClassPin->LinkedTo.Num())
	{
		UEdGraphPin* ClassSource = ClassPin->LinkedTo[0];
		UseSpawnClass = ClassSource ? Cast<UClass>(ClassSource->PinType.PinSubCategoryObject.Get()) : nullptr;
	}

	return UseSpawnClass;
}

void UK2Node_ActorPool_SpawnActor::MaybeUpdateCollisionPin(TArray<UEdGraphPin*>& OldPins)
{
	// see if there's a bNoCollisionFail pin
	for (UEdGraphPin* Pin : OldPins)
	{
		if (Pin->PinName == FK2Node_ActorPool_SpawnActor_Helper::NoCollisionFailPinName
			|| Pin->PinName == FK2Node_ActorPool_SpawnActor_Helper::SpawnEvenIfCollidingPinName)
		{
			bool bHadOldCollisionPin = true;
			if (Pin->LinkedTo.Num() == 0)
			{
				// no links, use the default value of the pin
				bool const bOldCollisionPinValue = (Pin->DefaultValue == FString(TEXT("true")));

				UEdGraphPin* const CollisionHandlingOverridePin = GetCollisionHandlingPin();
				if (CollisionHandlingOverridePin)
				{
					UEnum const* const MethodEnum = FindObjectChecked<UEnum>(nullptr, TEXT("/Script/Engine.ESpawnActorCollisionHandlingMethod"), true);
					CollisionHandlingOverridePin->DefaultValue =
						bOldCollisionPinValue
						? MethodEnum->GetNameStringByValue(static_cast<int>(ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
						: MethodEnum->GetNameStringByValue(static_cast<int>(ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding));
				}
			}
			else
			{
				// something was linked. we will just move the links to the new pin
				// #note: this will be an invalid linkage and the BP compiler will complain, and that's intentional
				// so that users will be able to see and fix issues
				UEdGraphPin* const CollisionHandlingOverridePin = GetCollisionHandlingPin();
				check(CollisionHandlingOverridePin);

				UEnum* const MethodEnum = FindObjectChecked<UEnum>(nullptr, TEXT("/Script/Engine.ESpawnActorCollisionHandlingMethod"), true);

				FGraphNodeCreator<UK2Node_EnumLiteral> AlwaysSpawnLiteralCreator(*GetGraph());
				UK2Node_EnumLiteral* const AlwaysSpawnLiteralNode = AlwaysSpawnLiteralCreator.CreateNode();
				AlwaysSpawnLiteralNode->Enum = MethodEnum;
				AlwaysSpawnLiteralNode->NodePosX = NodePosX;
				AlwaysSpawnLiteralNode->NodePosY = NodePosY;
				AlwaysSpawnLiteralCreator.Finalize();

				FGraphNodeCreator<UK2Node_EnumLiteral> AdjustIfNecessaryLiteralCreator(*GetGraph());
				UK2Node_EnumLiteral* const AdjustIfNecessaryLiteralNode = AdjustIfNecessaryLiteralCreator.CreateNode();
				AdjustIfNecessaryLiteralNode->Enum = MethodEnum;
				AdjustIfNecessaryLiteralNode->NodePosX = NodePosX;
				AdjustIfNecessaryLiteralNode->NodePosY = NodePosY;
				AdjustIfNecessaryLiteralCreator.Finalize();

				FGraphNodeCreator<UK2Node_Select> SelectCreator(*GetGraph());
				UK2Node_Select* const SelectNode = SelectCreator.CreateNode();
				SelectNode->NodePosX = NodePosX;
				SelectNode->NodePosY = NodePosY;
				SelectCreator.Finalize();

				// find pins we want to set and link up
				auto FindEnumInputPin = [](UK2Node_EnumLiteral const* Node)
				{
					for (UEdGraphPin* NodePin : Node->Pins)
					{
						if (NodePin->PinName == Node->GetEnumInputPinName())
						{
							return NodePin;
						}
					}
					return (UEdGraphPin*)nullptr;
				};

				UEdGraphPin* const AlwaysSpawnLiteralNodeInputPin = FindEnumInputPin(AlwaysSpawnLiteralNode);
				UEdGraphPin* const AdjustIfNecessaryLiteralInputPin = FindEnumInputPin(AdjustIfNecessaryLiteralNode);

				TArray<UEdGraphPin*> SelectOptionPins;
				SelectNode->GetOptionPins(SelectOptionPins);
				UEdGraphPin* const SelectIndexPin = SelectNode->GetIndexPin();

				auto FindResultPin = [](UK2Node const* Node)
				{
					for (UEdGraphPin* NodePin : Node->Pins)
					{
						if (EEdGraphPinDirection::EGPD_Output == NodePin->Direction)
						{
							return NodePin;
						}
					}
					return (UEdGraphPin*)nullptr;
				};
				UEdGraphPin* const AlwaysSpawnLiteralNodeResultPin = FindResultPin(AlwaysSpawnLiteralNode);
				check(AlwaysSpawnLiteralNodeResultPin);
				UEdGraphPin* const AdjustIfNecessaryLiteralResultPin = FindResultPin(AdjustIfNecessaryLiteralNode);
				check(AdjustIfNecessaryLiteralResultPin);

				UEdGraphPin* const OldBoolPin = Pin->LinkedTo[0];
				check(OldBoolPin);

				//
				// now set data and links that we want to set
				//

				AlwaysSpawnLiteralNodeInputPin->DefaultValue = MethodEnum->GetNameStringByValue(static_cast<int>(ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
				AdjustIfNecessaryLiteralInputPin->DefaultValue = MethodEnum->GetNameStringByValue(static_cast<int>(ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding));

				OldBoolPin->BreakLinkTo(Pin);
				OldBoolPin->MakeLinkTo(SelectIndexPin);

				AlwaysSpawnLiteralNodeResultPin->MakeLinkTo(SelectOptionPins[0]);
				AdjustIfNecessaryLiteralResultPin->MakeLinkTo(SelectOptionPins[1]);

				UEdGraphPin* const SelectOutputPin = SelectNode->GetReturnValuePin();
				check(SelectOutputPin);
				SelectOutputPin->MakeLinkTo(CollisionHandlingOverridePin);

				// tell select node to update its wildcard status
				SelectNode->NotifyPinConnectionListChanged(SelectIndexPin);
				SelectNode->NotifyPinConnectionListChanged(SelectOptionPins[0]);
				SelectNode->NotifyPinConnectionListChanged(SelectOptionPins[1]);
				SelectNode->NotifyPinConnectionListChanged(SelectOutputPin);

			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
