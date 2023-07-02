# Short Description

A user-friendly, blueprint and C++ compatible, data-driven, non-intrusive object pool plugin. This plugin is designed to help you recycle actors such as bullets, magic effects, and pawns running AI logic like minions in your game.

# Long Description

A user-friendly, blueprint and C++ compatible, data-driven, non-intrusive object pool plugin. This plugin is designed to help you recycle actors such as bullets, magic effects, and pawns running AI logic like minions in your game.

Documentation: [中文](https://github.com/tzlFirefly/BusinessFiles/blob/main/Plugins/UEP001_FireflyObjectPool/%E4%B8%AD%E6%96%87%E6%96%87%E6%A1%A3.md) / [English](https://github.com/tzlFirefly/BusinessFiles/blob/main/Plugins/UEP001_FireflyObjectPool/EnglishDocumentation.md)

简单易用的，蓝图和C++兼容的，支持数据驱动的，非侵入式程序设计的对象池插件。该插件能够帮助你在游戏中循环利用如子弹、魔法领域等Actor，以及喽啰小兵等运行AI逻辑的Pawn。

Features: 
+ Easy to use in both C++ and BP
+ Supports **Data-Driven** way to reuse Actor (Use FName as Actor ID, for DataTable and DataAsset)
+ Non-intrusive code framework (you don't need to modify any existing code framework in your project)
+ Blueprint Functions supports **Automatic Type Cast** for Actor to reuse by ObjectPool
+ Several C++ Function templates for Actor to reuse by ObjectPool
+ Universal **BeginPlay** and **EndPlay** functions for **Actor**, **Pawn** and **Character** to reuse by ObjectPool

特点：
+ 易于在C++和BP中使用
+ 支持**数据驱动**的Actor重用方式(使用FName作为Actor ID，对于DataTable和DataAsset)
+ 非侵入式代码框架（你不需要更改你项目中原有的任何代码框架）
+ Blueprint函数支持**自动类型转换**供Actor由对象池循环利用
+ 几个C++函数模板，供Actor由对象池循环利用
+ 通用**BeginPlay**和**Endplay**函数，供 **Actor** 、 **Pawn** 和 **Character** 由对象池循环利用

# Techinical Information

Features: 
+ Easy to use in both C++ and BP
+ Supports **Data-Driven** way to reuse Actor (Use FName as Actor ID, for DataTable and DataAsset)
+ Non-intrusive code framework (you don't need to modify any existing code framework in your project)
+ Blueprint Functions supports **Automatic Type Cast** for Actor to reuse by ObjectPool
+ Several C++ Function templates for Actor to reuse by ObjectPool
+ Universal **BeginPlay** and **EndPlay** functions for **Actor**, **Pawn** and **Character** to reuse by ObjectPool

Code Modules:
+ FireflyObjectPool (Runtime)
+ FireflyObjectPoolDeveloper (UncookedOnly)

Number of Blueprints: 0
Number of C++ Classes: 6
Network Replicated: No
Supported Development Platforms: Win32, Win64
Supported Target Build Platforms: Win32, Win64
Documentation: [中文](https://github.com/tzlFirefly/BusinessFiles/blob/main/Plugins/UEP001_FireflyObjectPool/%E4%B8%AD%E6%96%87%E6%96%87%E6%A1%A3.md) / [English](https://github.com/tzlFirefly/BusinessFiles/blob/main/Plugins/UEP001_FireflyObjectPool/EnglishDocumentation.md)