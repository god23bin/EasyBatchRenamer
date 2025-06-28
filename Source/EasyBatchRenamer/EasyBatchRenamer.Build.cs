// Copyright Devil Dev, 2025.

using UnrealBuildTool;

public class EasyBatchRenamer : ModuleRules
{
	public EasyBatchRenamer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "Slate",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",			// 提供项目相关功能，例如访问项目设置和资源
				"InputCore",		// 核心输入模块，支持键盘、鼠标等输入设备的处理
				"EditorFramework",	// 编辑器框架模块，提供编辑器基础功能和服务
				"UnrealEd",			// Unreal 编辑器模块，包含编辑器核心功能和工具
				"ToolMenus",		// 工具菜单模块，用于创建和管理编辑器中的菜单项
				"CoreUObject",		// 核心 UObject 模块，提供基本的对象系统和序列化功能
				"Engine",			// 引擎模块，包含游戏运行所需的核心引擎功能
				"Slate",			// Slate UI 框架模块，用于构建编辑器用户界面，提供各种框架和控件
				"SlateCore",		// Slate 核心模块，提供底层渲染和输入处理功能
				// ... add private dependencies that you statically link with here ...	
				"AssetRegistry",    // 提供资产注册与查找功能，支持在运行时访问项目中的资源信息
				"AssetTools",       // 包含用于创建、操作和管理资源的工具类
				"ContentBrowser",   // 实现内容浏览器相关功能，用于浏览和管理项目资源
				"EditorStyle",      // 提供编辑器样式资源（如图标、颜色等）以保持UI风格一致
				"EditorWidgets",    // 提供可在自定义编辑器界面中使用的通用UI控件库
				"LevelEditor",      // 包含关卡编辑器的核心功能，支持对场景、Actor等进行编辑
				"Localization",     // 支持本地化功能，包括多语言资源加载与处理
				"WorkspaceMenuStructure", // 用于构建编辑器工作区菜单结构，支持插件菜单集成到编辑器界面
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
