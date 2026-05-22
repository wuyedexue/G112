// PinballLauncher.h
// 弹球发射器 - 弹簧发射机制

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinballLauncher.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class PINBALLGAME_API APinballLauncher : public AActor
{
	GENERATED_BODY()

public:
	APinballLauncher();

	/** 开始蓄力 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Launcher")
	void StartCharging();

	/** 释放发射 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Launcher")
	void Release();

	/** 获取当前蓄力百分比 (0-1) */
	UFUNCTION(BlueprintPure, Category = "Pinball|Launcher")
	float GetChargePercent() const { return CurrentCharge / MaxCharge; }

	/** 是否正在蓄力 */
	UFUNCTION(BlueprintPure, Category = "Pinball|Launcher")
	bool IsCharging() const { return bIsCharging; }

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// === 组件 ===

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* LauncherCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PlungerMesh;

	// === 配置 ===

	/** 最大蓄力值 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Launcher")
	float MaxCharge = 1.0f;

	/** 蓄力速度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Launcher")
	float ChargeRate = 2.0f;

	/** 最小发射力 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Launcher")
	float MinLaunchForce = 1200.f;

	/** 最大发射力 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Launcher")
	float MaxLaunchForce = 6000.f;

	/** 发射方向 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pinball|Launcher")
	FVector LaunchDirection = FVector(0.f, 1.f, 0.2f);

	/** 弹簧回缩距离 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Launcher")
	float PlungerRetractDistance = 15.f;

	/** 发射音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Launcher")
	USoundBase* LaunchSound;

private:
	bool bIsCharging = false;
	float CurrentCharge = 0.f;
	FVector PlungerInitialLocation;
};
