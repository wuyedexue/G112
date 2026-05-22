// PinballBall.h
// 弹球 - 带物理模拟的球体，核心游戏对象

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinballBall.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class PINBALLGAME_API APinballBall : public AActor
{
	GENERATED_BODY()

public:
	APinballBall();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 施加力（用于发射器） */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Ball")
	void LaunchBall(FVector Force);

	/** 施加冲量（用于碰撞反弹） */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Ball")
	void AddImpulse(FVector Impulse);

	/** 获取球速度 */
	UFUNCTION(BlueprintPure, Category = "Pinball|Ball")
	FVector GetBallVelocity() const;

	/** 获取球速度大小 */
	UFUNCTION(BlueprintPure, Category = "Pinball|Ball")
	float GetBallSpeed() const;

protected:
	// === 组件 ===

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BallMesh;

	// === 物理参数 ===

	/** 球的质量 (kg) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Physics")
	float BallMass = 0.08f;

	/** 弹性系数 (0-1) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Physics")
	float Restitution = 0.6f;

	/** 摩擦力 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Physics")
	float Friction = 0.3f;

	/** 最大速度限制 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Physics")
	float MaxSpeed = 3000.f;

	/** 最小速度（低于此值会被加速，防止球停下来） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Physics")
	float MinSpeed = 100.f;

	/** 球台倾斜角度对应的重力分量 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Physics")
	float TableTiltGravity = 200.f;

	/** 掉落检测Z坐标 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Physics")
	float DrainZThreshold = -100.f;

	/** 球的半径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Physics")
	float BallRadius = 3.0f;

	/** 标记球正在排水中，防止重复扣命 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Ball")
	void MarkAsDraining();

	/** 球是否正在排水 */
	UFUNCTION(BlueprintPure, Category = "Pinball|Ball")
	bool IsDraining() const { return bIsDraining; }

private:
	/** 碰撞响应 */
	UFUNCTION()
	void OnBallHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** 检查球是否掉落（安全网：如果触发器漏检） */
	void CheckDrain();

	/** 限制球速度 */
	void ClampVelocity();

	bool bIsDraining = false;
};
