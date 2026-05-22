// PinballHUD.h
// 弹球游戏HUD - 显示分数、生命、倍率和游戏结束界面

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinballHUD.generated.h"

class UTextBlock;
class UProgressBar;
class UVerticalBox;
class UButton;

UCLASS()
class PINBALLGAME_API UPinballHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 更新分数显示 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|HUD")
	void UpdateScore(int32 NewScore);

	/** 更新生命显示 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|HUD")
	void UpdateLives(int32 RemainingLives);

	/** 更新倍率显示 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|HUD")
	void UpdateMultiplier(int32 NewMultiplier);

	/** 显示游戏结束界面 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|HUD")
	void ShowGameOver(int32 FinalScore, int32 HighScore);

	/** 隐藏游戏结束界面 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|HUD")
	void HideGameOver();

	/** 更新发射器蓄力条 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|HUD")
	void UpdateLauncherCharge(float Percent);

protected:
	virtual void NativeConstruct() override;

	// === UI 绑定 ===
	// 这些需要在 UMG 蓝图中绑定对应的 Widget

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HighScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LivesText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MultiplierText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* LauncherChargeBar;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* GameOverPanel;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameOverScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameOverHighScoreText;

	UPROPERTY(meta = (BindWidget))
	UButton* RestartButton;

private:
	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnGameOverTriggered();
};
