// PinballFlipper.h
// 弹球挡板 - 玩家控制的左右挡板，物理旋转

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinballFlipper.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UPhysicsConstraintComponent;

UENUM(BlueprintType)
enum class EFlipperSide : uint8
{
	Left,
	Right
};

UCLASS()
class PINBALLGAME_API APinballFlipper : public AActor
{
	GENERATED_BODY()

public:
	APinballFlipper();

	virtual void Tick(float DeltaTime) override;

	/** 激活挡板（抬起） */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Flipper")
	void Activate();

	/** 释放挡板（放下） */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Flipper")
	void Deactivate();

	/** 挡板是否正在激活 */
	UFUNCTION(BlueprintPure, Category = "Pinball|Flipper")
	bool IsActivated() const { return bIsActivated; }

protected:
	virtual void BeginPlay() override;

	// === 组件 ===

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* FlipperMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* FlipperCollision;

	// === 配置 ===

	/** 挡板方向（左/右） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pinball|Flipper")
	EFlipperSide FlipperSide = EFlipperSide::Left;

	/** 挡板抬起角度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Flipper")
	float UpAngle = 45.f;

	/** 挡板落下角度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Flipper")
	float DownAngle = -25.f;

	/** 抬起速度（角速度 度/秒） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Flipper")
	float FlipUpSpeed = 1200.f;

	/** 落下速度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Flipper")
	float FlipDownSpeed = 600.f;

	/** 挡板击球力度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Flipper")
	float FlipForce = 2000.f;

private:
	bool bIsActivated = false;
	float CurrentAngle = 0.f;
	float TargetAngle = 0.f;

	/** 更新挡板旋转 */
	void UpdateRotation(float DeltaTime);
};
