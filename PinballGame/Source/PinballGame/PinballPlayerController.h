// PinballPlayerController.h
// 弹球玩家控制器 - 处理输入（挡板和发射器）

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PinballPlayerController.generated.h"

class APinballFlipper;
class APinballLauncher;

UCLASS()
class PINBALLGAME_API APinballPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APinballPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	// === 输入绑定 ===

	/** 左挡板按下 */
	void OnLeftFlipperPressed();
	/** 左挡板释放 */
	void OnLeftFlipperReleased();
	/** 右挡板按下 */
	void OnRightFlipperPressed();
	/** 右挡板释放 */
	void OnRightFlipperReleased();
	/** 发射器按下 */
	void OnLauncherPressed();
	/** 发射器释放 */
	void OnLauncherReleased();
	/** 左推台面 */
	void OnNudgeLeft();
	/** 右推台面 */
	void OnNudgeRight();
	/** 重新开始 */
	void OnRestart();

	// === 引用 ===

	/** 左挡板引用（在关卡中设置） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinball|References")
	APinballFlipper* LeftFlipper;

	/** 右挡板引用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinball|References")
	APinballFlipper* RightFlipper;

	/** 发射器引用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinball|References")
	APinballLauncher* Launcher;

	// === 推台面配置 ===

	/** 推台面力度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Nudge")
	float NudgeForce = 300.f;

	/** 推台面冷却时间 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Nudge")
	float NudgeCooldown = 0.5f;

private:
	/** 查找场景中的挡板和发射器 */
	void FindPinballActors();

	float LastNudgeTime = 0.f;
};
