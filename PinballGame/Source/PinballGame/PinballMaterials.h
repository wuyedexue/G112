// PinballMaterials.h
// 运行时创建纯色材质的工具函数，无需Content资产依赖

#pragma once

#include "CoreMinimal.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialExpressionVectorParameter.h"

/**
 * 运行时创建彩色材质工具
 * 用于纯C++项目中给BasicShapes着色
 */
class FPinballMaterials
{
public:
	/**
	 * 获取共享的基础彩色材质（带Color向量参数）
	 * 首次调用时创建，后续调用返回缓存
	 */
	static UMaterial* GetColorBaseMaterial()
	{
		static TWeakObjectPtr<UMaterial> CachedMaterial;
		if (!CachedMaterial.IsValid())
		{
			CachedMaterial = CreateColorBaseMaterial();
		}
		return CachedMaterial.Get();
	}

	/**
	 * 创建指定颜色的动态材质实例
	 * @param Color - 线性颜色值
	 * @param Outer - 外部对象（通常传this）
	 */
	static UMaterialInstanceDynamic* CreateColoredMID(FLinearColor Color, UObject* Outer = nullptr)
	{
		UMaterial* BaseMat = GetColorBaseMaterial();
		if (!BaseMat) return nullptr;

		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, Outer);
		if (MID)
		{
			MID->SetVectorParameterValue(FName("Color"), Color);
		}
		return MID;
	}

	/**
	 * 将彩色材质应用到静态网格组件
	 */
	static void ApplyColor(UStaticMeshComponent* MeshComp, FLinearColor Color)
	{
		if (!MeshComp) return;

		UMaterialInstanceDynamic* MID = CreateColoredMID(Color, MeshComp->GetOwner());
		if (MID)
		{
			MeshComp->SetMaterial(0, MID);
		}
	}

	// === 预定义颜色常量（用户要求：球=红，地面=白，碰撞体=灰）===
	static FLinearColor TableColor()    { return FLinearColor(1.0f, 1.0f, 1.0f); }    // 白色台面
	static FLinearColor WallColor()     { return FLinearColor(0.5f, 0.5f, 0.5f); }    // 灰色墙壁
	static FLinearColor BallColor()     { return FLinearColor(1.0f, 0.0f, 0.0f); }    // 红色球
	static FLinearColor FlipperColor()  { return FLinearColor(0.5f, 0.5f, 0.5f); }    // 灰色挡板
	static FLinearColor BumperColor()   { return FLinearColor(0.5f, 0.5f, 0.5f); }    // 灰色弹射器
	static FLinearColor LauncherColor() { return FLinearColor(0.5f, 0.5f, 0.5f); }    // 灰色发射器

private:
	static UMaterial* CreateColorBaseMaterial()
	{
#if WITH_EDITOR
		// 创建一个带VectorParameter的基础材质
		UMaterial* Mat = NewObject<UMaterial>(GetTransientPackage(), FName("PinballColorBaseMat"), RF_Transient);

		// 使用Unlit着色模型 - 保证物体始终可见，不依赖场景光源
		Mat->SetShadingModel(MSM_Unlit);

		// 创建颜色向量参数节点
		UMaterialExpressionVectorParameter* ColorParam = NewObject<UMaterialExpressionVectorParameter>(Mat);
		ColorParam->ParameterName = FName("Color");
		ColorParam->DefaultValue = FLinearColor(0.5f, 0.5f, 0.5f);

		// 添加表达式并连接到EmissiveColor（Unlit模式下这是最终可见颜色）
		Mat->Expressions.Add(ColorParam);
		Mat->EmissiveColor.Expression = ColorParam;

		// 双面渲染
		Mat->TwoSided = true;

		// 编译材质着色器
		Mat->PreEditChange(nullptr);
		Mat->PostEditChange();

		UE_LOG(LogTemp, Log, TEXT("PinballMaterials: Color base material created and compiled"));
		return Mat;
#else
		// 非编辑器构建：回退到默认材质
		return UMaterial::GetDefaultMaterial(MD_Surface);
#endif
	}
};
