// PinballTable.h
// 弹球台面 - 包含墙壁、轨道、坡道等静态结构

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinballTable.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

/**
 * 弹球台面基础Actor
 * 包含台面的物理边界和基本结构
 * 在编辑器中放置并调整形状
 */
UCLASS()
class PINBALLGAME_API APinballTable : public AActor
{
	GENERATED_BODY()

public:
	APinballTable();

protected:
	virtual void BeginPlay() override;

	// === 组件 ===

	/** 台面底板 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TableFloor;

	/** 左边墙 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* LeftWall;

	/** 右边墙 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* RightWall;

	/** 顶部墙 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TopWall;

	/** 排水口触发器（球掉落检测） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* DrainTrigger;

	/** 左墙可视网格 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* LeftWallMesh;

	/** 右墙可视网格 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* RightWallMesh;

	/** 顶墙可视网格 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TopWallMesh;

	// === 配置 ===

	/** 台面宽度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Table")
	float TableWidth = 200.f;

	/** 台面长度（高度方向） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Table")
	float TableLength = 400.f;

	/** 台面倾斜角度 (度) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pinball|Table")
	float TableTiltAngle = 6.5f;

	/** 墙壁高度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Table")
	float WallHeight = 15.f;

	/** 墙壁厚度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Table")
	float WallThickness = 5.f;

private:
	UFUNCTION()
	void OnDrainOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
