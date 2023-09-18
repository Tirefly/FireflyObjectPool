<a id="top"></a>

- [【必读】两种对象池](#必读两种对象池)
- [【必读】对象池Actor必须做的事情](#必读对象池actor必须做的事情)
- [一些通用的对象池Actor函数](#一些通用的对象池actor函数)
- [基于对象池生成Actor](#基于对象池生成actor)
- [将Actor回收到对象池中](#将actor回收到对象池中)
- [生成待命的Actor](#生成待命的actor)
- [从对象池中挪用休眠的Actor](#从对象池中挪用休眠的actor)
- [清理对象池](#清理对象池)
- [对象池的调试](#对象池的调试)

# 【必读】两种对象池

本插件基于开发者的游戏开发经验，提供了两种对象池，你可以基于自己游戏的需求，来自行决定使用哪种对象池：
+ 用FName，也就是 **ActorID** ，作为检索类型的对象池（ActorPoolOfID）
+ 用 **ActorClass** 作为检索类型的对象池（ActorPoolOfClass）

如果游戏的开发过程中，需要用到对象池的Actor的生成、初始化是基于数据驱动的方式（数据表、数据注册表、数据资产），强烈建议使用FName作为对象池的检索类型。

比如你的游戏中有很多种会生成投射物的远程法术。这些投射物都继承自同一个Actor类（假设叫BP_MagicalProjectile），都能对敌人造成伤害，但对敌人有不同的减益效果，且这些投射物的数据都存储在数据表中。那么强烈建议在这种情况下使用FName作为对象池的检索类型。

在接下来的文档中你会发现，大部分操作对象池的函数都有两个参数：```TSubclassOf<AActor> ActorClass```和```FName ActorID```。使用这些函数时，如果你只设置了ActorClass，那么你这次操作的对象池就是 **ActorPoolOfClass** ；如果你 **ActorClass** 和 **ActorID** 都设置了，那么你这次操作的对象池就是 **ActorPoolOfID** 。但如果你没有设置 **ActorClass** ，那么无论如何你这次操作都是没有意义的。

**[回到顶部](#top)**

# 【必读】对象池Actor必须做的事情

如果你希望你项目中的某个Actor能够通过对象池来循环利用，那么这个Actor的蓝图类或者C++类必须实现接口类 **FireflyPoolingActorInterface** 。 **FireflyPoolingActorInterface** 接口提供了如下5个函数：

*表格下方有蓝图演示和C++API*

|函数名|蓝图节点|用途|
|-|-|-|
|Pooling Begin Play|![PoolinigBeginPlay](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_PoolingBeginPlay.png)|通过函数 **ActorPool_SpawnActor** 从对象池生成的Actor都会执行的初始化操作。你可以实现这个函数来为Actor执行如下操作：<ul><li>开启物理</li><li>开启Tick</li><li>开启可见性</li><li>开启移动组件</li><li>开启AI大脑逻辑</li></ul>当然，本插件提供了一些对象池Actor通用的初始化函数，文档的后续内容会介绍到。|
|Pooling End Play|![PoolinigEndPlay](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_PoolingEndPlay.png)|通过函数 **ActorPool_ReleaseActor** 回收到对象池的Actor都会执行的回收重置操作。你可以实现这个函数来为Actor执行如下操作：<ul><li>关闭物理</li><li>关闭Tick</li><li>关闭可见性</li><li>关闭移动组件</li><li>停止AI大脑逻辑</li></ul>当然，本插件提供了一些对象池Actor通用的回收重置函数，文档的后续内容会介绍到。|
|Pooling Warm Up|![PoolinigWarmUp](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_PoolingWarmUp.png)|通过函数 **ActorPool_WarmUp** 生成的要被放进对象池待命的Actor都会执行的操作。因为Actor调用该函数时虽然刚刚被生成出来却仍然在对象池中处于休眠待命状态，所以该函数理论上并不应该执行Actor的初始化操作，大多数情况下该函数执行的操作应该和 **Pooling End Play** 相似。<br>当然，本插件提供了一些对象池Actor通用的生成后在对象池中待命的函数，文档的后续内容会介绍到。|
|Pooling Get Actor ID|![PoolinigGetActorID](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_PoolingGetActorID.png)|如果你希望对这个对象池Actor进行循环利用的操作是通过 **ActorID** 进行的，那么你应该让这个Actor类实现这个函数。<br>该函数应该返回Actor类中定义好的FName类型的存储 **ActorID** 的变量值，用于回收Actor时、挪用Actor时确认要操作的对象池是 **ActorPoolOfClass** 还是 **ActorPoolOfID** 。|
|Pooling Set Actor ID|![PoolinigSetActorID](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_PoolingSetActorID.png)|如果你希望对这个对象池Actor进行循环利用的操作是通过 **ActorID** 进行的，那么你应该让这个Actor类实现这个函数。<br>该函数应该用来设置Actor的ID值。函数 **ActorPool_SpawnActor** 会尝试通过该函数设置全新生成且尚未拥有ID的Actor的ID。|

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

**[回到顶部](#top)**

# 一些通用的对象池Actor函数

插件提供了一些通用函数，方便你对你项目中的Actor、Pawn、Character进行从对象池生成后的初始化、回收到对象池后的休眠处理等。这些函数的详解介绍如下：

*表格下方有蓝图演示和C++API*

|通用的Actor操作函数名|蓝图节点|用途|
|-|-|-|
|Universal Begin Play Actor|![UniversalBeginPlayActor](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_BeginPlayActor.png)|该函数执行了如下的一些Actor通用的初始化操作：<ul><li>开启碰撞</li><li>开启Tick</li><li>开启可见性</li><li>激活Actor的所有Niagara特效</li><li>激活Actor的所有Cascade特效</li><li>激活Actor的所有Primitive组件，并开启其可见性，重置其角速度和线速度</li><li>将Actor所有移动组件的UpdatedComponent设置为Actor的根组件，激活移动组件并清理移动组件的速度</li><li>如果移动组件是一个ProjectileMovement，为移动组件设置向前的初始速度</li><li>将Actor的所有其他组件都重新激活</li></ul>|
|Universal End Play Actor|![UniversalEndPlayActor](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_EndPlayActor.png)|该函数执行了如下的一些Actor通用的回收重置操作：<ul><li>关闭碰撞</li><li>关闭Tick</li><li>关闭可见性</li><li>关闭Actor的所有Niagara特效</li><li>关闭Actor的所有Cascade特效</li><li>关闭Actor的所有Primitive组件，并关闭其可见性，重置其角速度和线速度</li><li>将Actor所有移动组件的UpdatedComponent设置为空，关闭移动组件并清理移动组件的速度</li><li>关闭Actor的所有其他组件</li></ul>|
|Universal Warm Up Actor|![UniversalWarmUpActor](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_WarmUpActor.png)|该函数目前由于并未发现有任何需要特殊执行的操作，所以其内部逻辑和 **Universal End Play Actor** 一样|

|通用的Pawn操作函数名|蓝图节点|用途|
|-|-|-|
|Universal Begin Play Pawn|![UniversalBeginPlayPawn](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_BeginPlayPawn.png)|该函数执行了如下的一些Pawn通用的初始化操作：<ul><li>调用函数 **Universal Begin Play Actor** 来执行一些Actor通用的初始化操作</li><li>尝试为Pawn生成默认的Controller</li><li>若Pawn的Controller是一个AIController，开启AI的大脑组件逻辑</li></ul>|
|Universal End Play Pawn|![UniversalEndPlayPawn](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_EndPlayPawn.png)|该函数执行了如下的一些Pawn通用的回收重置操作：<ul><li>调用函数 **Universal End Play Actor** 来执行一些Actor通用的回收重置操作</li><li>若Pawn的Controller是一个AIController，清理并关闭AI的大脑组件逻辑</li></ul>|
|Universal Warm Up Pawn|![UniversalWarmUpPawn](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_WarmUpPawn.png)|该函数执行了如下的一些Pawn通用的生成后待命操作：<ul><li>调用函数 **Universal Warm Up Actor** 来执行一些Actor通用的生成后待命操作</li><li>尝试为Pawn生成默认的Controller</li><li>若Pawn的Controller是一个AIController，清理并关闭AI的大脑组件逻辑</li></ul>|

|通用的Character操作函数名|蓝图节点|用途|
|-|-|-|
|Universal Begin Play Character|![UniversalBeginPlayCharacter](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_BeginPlayCharacter.png)|该函数目前由于并未发现有任何需要特殊执行的操作，所以其内部逻辑和 **Universal Begin Play Pawn** 一样|
|Universal End Play Character|![UniversalEndPlayCharacter](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_EndPlayCharacter.png)|该函数目前由于并未发现有任何需要特殊执行的操作，所以其内部逻辑和 **Universal End Play Pawn** 一样|
|Universal Warm Up Character|![UniversalWarmUpCharacter](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_WarmUpCharacter.png)|该函数目前由于并未发现有任何需要特殊执行的操作，所以其内部逻辑和 **Universal Warm Up Pawn** 一样|

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

**[回到顶部](#top)**

# 基于对象池生成Actor

使用下面的函数，你可以从对象池中拿出一个Actor并将其初始化，如果无法根据指定的 **ActorID** 或者 **ActorClass** 从对象池中拿出一个Actor，该函数会直接生成并初始化一个指定Class的全新Actor实例，并把该全新生成的Actor的ID设置成指定的ID。如果Lifetime值大于0，则Actor会在从对象池生成后的一段时间后（即Lifetime的秒数）回到对象池进入休眠状态。

![ActorPool_SpawnActor](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_ActorPoolSpawnActor.png)

```c++
template<typename T>
T* ActorPool_SpawnActor(TSubclassOf<T> ActorClass, FName ActorID, const FTransform& Transform, float Lifetime
	, AActor* Owner, const ESpawnActorCollisionHandlingMethod CollisionHandling);
```

**[回到顶部](#top)**

# 将Actor回收到对象池中

使用下面的函数，你可以将Actor回收到对象池中并让Actor进入休眠状态。如果Actor的ID有效，则Actor会被回收到ActorPoolOfID，否则Actor会被回收到ActorPoolOfClass。

![ActorPool_ReleaseActor](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_ActorPoolReleaseActor.png)

```C++
// 把Actor回收到Actor池里，如果Actor有ID（并且Actor实现了IFireflyPoolingActorInterface::GetActorID）则回到对应ID的Actor池，否则回到Actor类的Actor池。
// Recycle the Actor back into the Actor pool. If the Actor has an ID (dn implements IFireflyPoolingActorInterface::GetActorID), return it to the ID-based Actor pool; otherwise, return it to the class-based Actor pool.
UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", Meta = (DisplayName = "Actor Pool Release Actor"))
static void ActorPool_ReleaseActor(AActor* Actor);
```

**[回到顶部](#top)**

# 生成待命的Actor

使用下面的函数，你可以为指定 **ActorClass** 的对象池或者指定 **ActorID** 的对象池生成一定数量的Actor。这些Actor在生成后不会初始化，而是进入休眠期，在对象池中待命。

![ActorPool_WarmUp](https://raw.githubusercontent.com/tzlFirefly/PersonalPictures/master/UEP001_ActorPoolWarmUp.png)

```C++
// 生成特定数量的指定类以及指定ID的Actor并放进Actor池中待命。
// Spawn a specific number of Actors of a specified class and a specified ID ,and place them in the Actor pool on standby.
UFUNCTION(BlueprintCallable, Category = "FireflyObjectPool", meta = (WorldContext = "WorldContextObject"))
static void ActorPool_WarmUp(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, FName ActorID, const FTransform& Transform, int32 Count = 16);
```

**[回到顶部](#top)**

# 从对象池中挪用休眠的Actor

使用下面的函数，你可以从 **ActorClass** 对象池或者 **ActorID** 对象池中取出一个（或一些）处于休眠期的Actor但不对其进行初始化和激活。如果你在使用这个（或这些）对象池Actor之前，希望对其进行某些定制化操作，那下面的函数能够帮到你。

**但是请注意**，在使用这个函数前，你最好确认你将要使用的对象池真的存在且该对象池中真的有能够被你挪用的Actor实例。至于说该如何确认对象池的存在与否以及对象池中是否存在可使用的Actor实例，请查阅 [对象池的调试](#对象池的调试) 。

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

**[回到顶部](#top)**

# 清理对象池

使用下面的函数，你可以在你想要的任何时间清理对象池。清理对象池会删除这个对象池，并销毁对象池中所有的Actor。总共有三种清理方式：
+ 清理所有对象池
+ 清理特定 **ActorClass** 的对象池
+ 清理特定 **ActorID** 的对象池

默认的，由于对象池的管理器是一个WorldSubsystem，所以当该管理器被销毁时，管理器中的对象池也都会被清理掉。

注意：清理对象池的操作只会销毁对象池中处于休眠待命状态的Actor，该操作不会清理归属于某个对象池，但处于激活状态的Actor。

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

**[回到顶部](#top)**

# 对象池的调试

使用下面的函数，你可以自行查看在游戏运行时， **ActorClass** 的对象池有哪些，**ActorID** 的对象池有哪些。你也可以查看在游戏运行时，某个 **ActorClass** 的对象池或者某个 **ActorID** 的对象池中有多少个休眠待命的Actor。

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

**[回到顶部](#top)**