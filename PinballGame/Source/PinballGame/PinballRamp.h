// PinballRamp.h
// 弹球坡道/轨道 - 球通过时加分，增加游戏丰富度

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinballRamp.generated.h"

class USplineComponent;
class UBoxComponent;

UCLASS()
class PINBALLGAME_API APinballRamp : public AActor
{
	GENERATED_BODY()

public:
	APinballRamp();

protected:
	virtual void BeginPlay() override;

	// === 组件 ===

	/** 轨道入口触发器 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* EntryTrigger;

	/** 轨道出口触发器 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* ExitTrigger;

	/** 轨道路径（用于视觉效果） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* RampSpline;

	// === 配置 ===

	/** 通过轨道得分 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pinball|Ramp")
	int32 RampScore = 2000;

	/** 连续通过加成倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pinball|Ramp")
	int32 ConsecutiveBonus = 500;

	/** 轨道名称（用于UI显示） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pinball|Ramp")
	FString RampName = TEXT("Ramp");

	/** 通过音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Ramp")
	USoundBase* RampCompleteSound;

private:
	UFUNCTION()
	void OnEntryOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnExitOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 追踪已进入轨道但未退出的球 */
	TSet<AActor*> BallsInRamp;

	/** 连续通过次数 */
	int32 ConsecutiveCount = 0;
};
