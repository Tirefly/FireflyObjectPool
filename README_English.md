<a id="top"></a>

- [【Must Read】Two Types of Object Pools](#must-readtwo-types-of-object-pools)
- [【Must Read】Things object pooling Actor must do](#must-readthings-object-pooling-actor-must-do)
- [Some Universal Pooling Actor Functions](#some-universal-pooling-actor-functions)
- [Spawn Actor based on ObjectPool](#spawn-actor-based-on-objectpool)
- [Recycle Actor into ObjectPool](#recycle-actor-into-objectpool)
- [Spawn standby Actor](#spawn-standby-actor)
- [Misappropriate dormant Actor from object pool](#misappropriate-dormant-actor-from-object-pool)
- [Clear Object Pool](#clear-object-pool)
- [Debug Object Pool](#debug-object-pool)

# 【Must Read】Two Types of Object Pools

This plugin provides two types of object pools based on the developer's game development experience. You can decide which type of object pool to use based on your game's requirements:
+ Object pool with FName, aka **ActorID**, as the retrieval type (ActorPoolOfID)
+ Object pool with **ActorClass** as the retrieval type (ActorPoolOfClass)

If the generation and initialization of pooled actors in the game are driven by data (such as data tables, registries, or assets), it is strongly recommended to use FName as the retrieval type for the object pool.

For example, if your game includes various remote spells that generate projectiles. These projectiles inherit from the same actor class (let's say it's called BP_MagicalProjectile) and can cause damage to enemies with different debuff effects. The data for these projectiles is stored in a data table. In this case, it is strongly recommended to use FName as the retrieval type for the object pool.

In the following documentation, you will notice that most functions related to object pools have two parameters: ```TSubclassOf<AActor> ActorClass``` and ```FName ActorID```. When using these functions, if you only set the **ActorClass** parameter, the object pool you are operating on is the **ActorPoolOfClass** . If you set both **ActorClass** and **ActorID** , the object pool you are operating on is the **ActorPoolOfID** . However, if you don't set ActorClass, then your operation in this case is meaningless regardless of other settings.

**[Back to Top](#top)**

# 【Must Read】Things object pooling Actor must do

If you want a specific actor in your project to be reused through an object pool, the blueprint or C++ class of that actor must implement the interface class **FireflyPoolingActorInterface** . The **FireflyPoolingActorInterface** interface provides the following 5 functions:

*Below the table, there are blueprint demonstrations and C++ API.*

|Function Name|Blueprint Node|Purpose|
|-|-|-|
|Pooling Begin Play|![PoolinigBeginPlay](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_PoolingBeginPlay.png)|Initialization operation executed by all actors spawned from the object pool using the function ActorPool_SpawnActor. You can implement this function to perform the following operations on actors:<ul><li>Enable physics</li><li>Enable ticking</li><li>Enable visibility</li><li>Enable movement components</li><li>Enable AI logic</li></ul>This plugin also provides some common initialization functions for object pool actors, which will be covered in the subsequent documentation.|
|Pooling End Play|![PoolinigEndPlay](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_PoolingEndPlay.png)|Cleanup and reset operation executed by actors returned to the object pool using the function ActorPool_ReleaseActor. You can implement this function to perform the following operations on actors:<ul><li>Disable physics</li><li>Disable ticking</li><li>Disable visibility</li><li>Disable movement components</li><li>Disable AI logic</li></ul>This plugin also provides some common reset functions for object pool actors, which will be covered in the subsequent documentation.|
|Pooling Warm Up|![PoolinigWarmUp](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_PoolingWarmUp.png)|Operation executed by actors spawned from the function ActorPool_WarmUp and placed in the object pool for standby. Since actors calling this function are newly spawned but still in a dormant state within the object pool, theoretically, this function should not perform actor initialization operations. In most cases, the operations performed by this function should be similar to Pooling End Play. <br>This plugin also provides some common functions for spawning actors and placing them in standby within the object pool, which will be covered in the subsequent documentation.|
|Pooling Get Actor ID|![PoolinigGetActorID](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_PoolingGetActorID.png)|If you want to reuse this object pool actor based on its ActorID, you should implement this function in the actor class.<br>This function should return the value of the FName variable that stores the ActorID defined in the actor class. It is used to determine whether to operate on the ActorPoolOfClass or ActorPoolOfID when recycling or reusing the actor.|
|Pooling Set Actor ID|![PoolinigSetActorID](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_PoolingSetActorID.png)|If you want to reuse this object pool actor based on its ActorID, you should implement this function in the actor class.<br>This function should be used to set the ID value of the actor.<br>The function **ActorPool_SpawnActor** will attempt to set the ID of a newly spawned actor that doesn't have an ID yet, using this function.|

![ActorPool_PoolingActorInterface](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_ActorPoolInterface.png)

```C++
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
```

**[Back to Top](#top)**

# Some Universal Pooling Actor Functions

The plugin provides some universal functions that facilitate the initialization of Actors, Pawns, and Characters after they are spawned from the object pool, as well as handling their dormant state after being reclaimed by the object pool. The detailed explanation of these functions is as follows:

*Below the table, there are blueprint demonstrations and C++ API.*

|Universal Actor Operating Functions|Blueprint Node|Purpose|
|-|-|-|
|Universal Begin Play Actor|![UniversalBeginPlayActor](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_BeginPlayActor.png)|The function performs the following common initialization operations for Actors:<ul><li>Enable collision</li><li>Enable tick</li><li>Enable visibility</li><li>Activate all Niagara effects of the Actor</li><li>Activate all Cascade effects of the Actor</li><li>Activate all Primitive components of the Actor, enable their visibility, and reset their angular and linear velocities</li><li>Set the UpdatedComponent of all movement components of the Actor to the root component, activate the movement components, and clear their velocities</li><li>If a movement component is a ProjectileMovement, set the initial forward velocity for the movement component</li><li>Re-activate all other components of the Actor</li></ul>|
|Universal End Play Actor|![UniversalEndPlayActor](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_EndPlayActor.png)|The function performs the following common reset operations for an Actor: <ul><li>Disables collision.</li><li>Disables ticking.</li><li>Hides the Actor.</li><li>Deactivates all Niagara effects attached to the Actor.</li><li>Deactivates all Cascade effects attached to the Actor.</li><li>Disables and hides all Primitive components of the Actor, resetting their angular velocity and linear velocity.</li><li>Sets the UpdatedComponent of all movement components of the Actor to null, disables the movement components, and clears their velocities.</li><li>Disables all other components of the Actor.</li></ul>|
|Universal Warm Up Actor|![UniversalWarmUpActor](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_WarmUpActor.png)|The function currently doesn't contain any special operations, so its internal logic is the same as the **Universal End Play Actor** function.|

|Universal Pawn Operating Functions|Blueprint Node|Purpose|
|-|-|-|
|Universal Begin Play Pawn|![UniversalBeginPlayPawn](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_BeginPlayPawn.png)|The function performs the following common initialization operations for Pawns: <ul><li>Calls the <strong>Universal Begin Play Actor</strong> function to execute some common initialization operations for Actors.</li><li>Attempts to generate a default Controller for the Pawn.</li><li>If the Pawn's Controller is an AIController, activates the AI's brain component logic.</li></ul>|
|Universal End Play Pawn|![UniversalEndPlayPawn](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_EndPlayPawn.png)|The function performs the following common reset operations for Pawns: <ul><li>Calls the <strong>Universal End Play Actor</strong> function to execute some common reset operations for Actors.</li><li>If the Pawn's Controller is an AIController, cleans up and deactivates the AI's brain component logic.</li></ul>|
|Universal Warm Up Pawn|![UniversalWarmUpPawn](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_WarmUpPawn.png)|The function performs the following common post-spawn operations for Pawns: <ul><li>Calls the <strong>Universal Warm Up Actor</strong> function to execute some common post-spawn operations for Actors.</li><li>Attempts to generate a default Controller for the Pawn.</li><li>If the Pawn's Controller is an AIController, cleans up and deactivates the AI's brain component logic.</li></ul>|

|Universal Character Operating Functions|Blueprint Node|Purpose|
|-|-|-|
|Universal Begin Play Character|![UniversalBeginPlayCharacter](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_BeginPlayCharacter.png)|The function currently doesn't contain any special operations, so its internal logic is the same as the **Universal Begin Play Pawn** function.|
|Universal End Play Character|![UniversalEndPlayCharacter](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_EndPlayCharacter.png)|The function currently doesn't contain any special operations, so its internal logic is the same as the **Universal End Play Pawn** function.|
|Universal Warm Up Character|![UniversalWarmUpCharacter](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_WarmUpCharacter.png)|The function currently doesn't contain any special operations, so its internal logic is the same as the **Universal Warm Up Pawn** function.|

![ActorPool_UniversalFunctions](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_ActorPoolUniversalFunctions.png)

```C++
#pragma region Actor_Universal_Pool_Operation

	// Actor通用的从对象池中取出后进行初始化的操作。
	// Common operation for an Actor to initialize after being taken out from the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (WorldContext = "WorldContextObject"))
	static void UniversalBeginPlay_Actor(const UObject* WorldContextObject, AActor* Actor);

	// Actor通用的回到对象池后进入冻结状态的操作。
	// Common operation for an Actor to enter a frozen state after returning to the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (WorldContext = "WorldContextObject"))
	static void UniversalEndPlay_Actor(const UObject* WorldContextObject, AActor* Actor);

	// Actor通用的在对象池中生成后进入待命状态的操作。
	// Common operation for an Actor to enter a standby state after being generated in the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (WorldContext = "WorldContextObject"))
	static void UniversalWarmUp_Actor(const UObject* WorldContextObject, AActor* Actor);

#pragma endregion


#pragma region Pawn_Universal_Pool_Operation

	// Pawn通用的从对象池中取出后进行初始化的操作。
	// Common operation for an Pawn to initialize after being taken out from the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (WorldContext = "WorldContextObject"))
	static void UniversalBeginPlay_Pawn(const UObject* WorldContextObject, APawn* Pawn);

	// Pawn通用的回到对象池后进入冻结状态的操作。
	// Common operation for an Pawn to enter a frozen state after returning to the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (WorldContext = "WorldContextObject"))
	static void UniversalEndPlay_Pawn(const UObject* WorldContextObject, APawn* Pawn);

	// Pawn通用的在对象池中生成后进入待命状态的操作。
	// Common operation for an Pawn to enter a standby state after being generated in the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (WorldContext = "WorldContextObject"))
	static void UniversalWarmUp_Pawn(const UObject* WorldContextObject, APawn* Pawn);

#pragma endregion


#pragma region Character_Universal_Pool_Operation

	// Character通用的从对象池中取出后进行初始化的操作。
	// Common operation for an Character to initialize after being taken out from the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (WorldContext = "WorldContextObject"))
	static void UniversalBeginPlay_Character(const UObject* WorldContextObject, ACharacter* Character);

	// Character通用的回到对象池后进入冻结状态的操作。
	// Common operation for an Character to enter a frozen state after returning to the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (WorldContext = "WorldContextObject"))
	static void UniversalEndPlay_Character(const UObject* WorldContextObject, ACharacter* Character);

	// Character通用的在对象池中生成后进入待命状态的操作。
	// Common operation for an Character to enter a standby state after being generated in the object pool.
	UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (WorldContext = "WorldContextObject"))
	static void UniversalWarmUp_Character(const UObject* WorldContextObject, ACharacter* Character);

#pragma endregion
```

# Spawn Actor based on ObjectPool

Using the following function, you can take an Actor from the object pool and initialize it. If you cannot get an Actor from the object pool according to the specified **ActorID** or **ActorClass** , the function will directly spawn and initialize a new Actor instance of the specified Class and set the ID of the newly spawned Actor to the specified ID. If the Lifetime value is greater than 0, the Actor will return to the object pool and go to sleep after a period of time (that is, the number of seconds of Lifetime) after it was spawned from the object pool.

![ActorPool_SpawnActor](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_ActorPoolSpawnActor.png)

```c++
template<typename T>
T* ActorPool_SpawnActor(TSubclassOf<T> ActorClass, FName ActorID, const FTransform& Transform, float Lifetime
	, AActor* Owner, const ESpawnActorCollisionHandlingMethod CollisionHandling);
```

**[Back to Top](#top)**

# Recycle Actor into ObjectPool

Using the following function, you can recycle Actor into the object pool and put Actor into a dormant state. If the ID of Actor is valid, Actor will be recycled to ActorPoolOfID, otherwise Actor will be recycled to ActorPoolOfClass.

![ActorPool_ReleaseActor](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_ActorPoolReleaseActor.png)

```C++
// 把Actor回收到Actor池里，如果Actor有ID（并且Actor实现了IFireflyPoolingActorInterface::GetActorID）则回到对应ID的Actor池，否则回到Actor类的Actor池。
// Recycle the Actor back into the Actor pool. If the Actor has an ID (dn implements IFireflyPoolingActorInterface::GetActorID), return it to the ID-based Actor pool; otherwise, return it to the class-based Actor pool.
UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (DisplayName = "Actor Pool Release Actor"))
static void ActorPool_ReleaseActor(AActor* Actor);
```

**[Back to Top](#top)**

# Spawn standby Actor

Using the following function, you can spawn a certain number of Actor for the object pool of the specified **ActorClass** or the object pool of the specified **ActorID**. These Actor are not initialized after they are spawned, but go into dormancy and stand by in the object pool.

![ActorPool_WarmUp](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_ActorPoolWarmUp.png)

```C++
// 生成特定数量的指定类以及指定ID的Actor并放进Actor池中待命。
// Spawn a specific number of Actors of a specified class and a specified ID ,and place them in the Actor pool on standby.
UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", meta = (WorldContext = "WorldContextObject"))
static void ActorPool_WarmUp(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, FName ActorID, const FTransform& Transform, int32 Count = 16);
```

**[Back to Top](#top)**

# Misappropriate dormant Actor from object pool

Using the following function, you can take one (or a number of) dormant Actor from the **ActorClass** object pool or the **ActorID** object pool without initializing and activating it. If you want to customize this (or these) object pooling Actor before using it, the following functions can help you. 

**But please note**, before using these functions, you must confirm that the object pool you are going to use really exists and that there are Actor instances in the object pool that can be misappropriated by you. As for how to confirm the existence of object pool and whether there are Actor instances available in object pool, please refer to [Debug Object Pool](#debug-object-pool) .

![ActorPool_FetchActor](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_ActorPoolFetchActor.png)

```C++
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
```

**[Back to Top](#top)**

# Clear Object Pool

Using the following functions, you can clean up object pools at any desired time. Cleaning an object pool will delete the object pool itself and destroy all the Actors within it. There are three ways to clean an object pool:
+ Clean all object pools.
+ Clean object pools with a specific **ActorClass** .
+ Clean object pools with a specific **ActorID** .

By default, since the object pool manager is a WorldSubsystem, when the manager is destroyed, all the object pools within it are also cleaned.

Note: Cleaning an object pool will only destroy the Actors in a dormant state within the object pool. It does not clean the active Actors that belong to a specific object pool.

![ActorPool_ClearPool](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_ActorPoolClearPool.png)

```C++
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
```

**[Back to Top](#top)**

# Debug Object Pool

Using the following functions, you can examine the object pools of specific **ActorClass** and **ActorID** during runtime. You can also determine the number of dormant Actors in a specific **ActorClass** object pool or **ActorID** object pool while the game is running.

![ActorPool_Debug](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_ActorPoolDebug.png)

```C++
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
```

**[Back to Top](#top)**