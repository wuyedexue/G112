// PinballBumper.h
// 弹球保险杠 - 碰撞时弹开球并加分

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinballBumper.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UENUM(BlueprintType)
enum class EBumperType : uint8
{
	/** 圆形弹射器 - 大力弹射 */
	RoundBumper,
	/** 三角形挡板 - 中等弹射 */
	TriangleBumper,
	/** 蘑菇头 - 轻微弹射+高分 */
	MushroomBumper,
	/** 目标靶 - 击中一次后消失 */
	DropTarget
};

UCLASS()
class PINBALLGAME_API APinballBumper : public AActor
{
	GENERATED_BODY()

public:
	APinballBumper();

	virtual void BeginPlay() override;

protected:
	// === 组件 ===

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* BumperCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BumperMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPointLightComponent* HitLight;

	// === 配置 ===

	/** 保险杠类型 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pinball|Bumper")
	EBumperType BumperType = EBumperType::RoundBumper;

	/** 弹射力度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Bumper")
	float BounceForce = 1000.f;

	/** 击中得分 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pinball|Bumper")
	int32 ScoreValue = 100;

	/** 是否增加连击倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pinball|Bumper")
	bool bIncrementsMultiplier = true;

	/** 灯光闪烁时间 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Bumper")
	float LightFlashDuration = 0.2f;

	/** 击中音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Bumper")
	USoundBase* HitSound;

	/** 碰撞半径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Bumper")
	float CollisionRadius = 20.f;

private:
	/** 碰撞回调 */
	UFUNCTION()
	void OnBumperHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** 关闭闪烁灯光 */
	void TurnOffLight();

	/** Drop Target 被击中 */
	void HandleDropTarget();

	bool bIsActive = true;
	FTimerHandle LightTimerHandle;
};
