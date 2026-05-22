// PinballGameMode.h
// 弹球游戏模式 - 管理游戏流程、生命值、分数

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PinballGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLivesChanged, int32, RemainingLives);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMultiplierChanged, int32, NewMultiplier);

UCLASS()
class PINBALLGAME_API APinballGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APinballGameMode();

	virtual void BeginPlay() override;

	// === 分数系统 ===

	/** 增加分数 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Score")
	void AddScore(int32 Points);

	/** 获取当前分数 */
	UFUNCTION(BlueprintPure, Category = "Pinball|Score")
	int32 GetCurrentScore() const { return CurrentScore; }

	/** 获取最高分 */
	UFUNCTION(BlueprintPure, Category = "Pinball|Score")
	int32 GetHighScore() const { return HighScore; }

	// === 生命值系统 ===

	/** 失去一条命（球掉落） */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Lives")
	void LoseLife();

	/** 获取剩余生命 */
	UFUNCTION(BlueprintPure, Category = "Pinball|Lives")
	int32 GetRemainingLives() const { return RemainingLives; }

	// === 连击/倍率系统 ===

	/** 增加连击倍率 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Combo")
	void IncrementMultiplier();

	/** 重置连击倍率 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Combo")
	void ResetMultiplier();

	/** 获取当前倍率 */
	UFUNCTION(BlueprintPure, Category = "Pinball|Combo")
	int32 GetMultiplier() const { return ScoreMultiplier; }

	// === 游戏流程 ===

	/** 重新开始游戏 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Game")
	void RestartGame();

	/** 发射新球 */
	UFUNCTION(BlueprintCallable, Category = "Pinball|Game")
	void SpawnNewBall();

	/** 游戏是否结束 */
	UFUNCTION(BlueprintPure, Category = "Pinball|Game")
	bool IsGameOver() const { return bIsGameOver; }

	// === 事件委托 ===

	UPROPERTY(BlueprintAssignable, Category = "Pinball|Events")
	FOnScoreChanged OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "Pinball|Events")
	FOnLivesChanged OnLivesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Pinball|Events")
	FOnGameOver OnGameOver;

	UPROPERTY(BlueprintAssignable, Category = "Pinball|Events")
	FOnMultiplierChanged OnMultiplierChanged;

protected:
	// === 数值配置 ===

	/** 初始生命数 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Config")
	int32 InitialLives = 3;

	/** 最大连击倍率 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Config")
	int32 MaxMultiplier = 10;

	/** 额外奖命所需分数 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Config")
	int32 ExtraLifeScoreThreshold = 50000;

	/** 球生成位置 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Config")
	FVector BallSpawnLocation = FVector(0.f, 0.f, 200.f);

	/** 球类引用 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinball|Config")
	TSubclassOf<AActor> BallClass;

private:
	int32 CurrentScore = 0;
	int32 HighScore = 0;
	int32 RemainingLives = 3;
	int32 ScoreMultiplier = 1;
	int32 NextExtraLifeScore = 50000;
	bool bIsGameOver = false;

	/** 保存最高分 */
	void SaveHighScore();
	/** 加载最高分 */
	void LoadHighScore();

	/** 程序化生成整个弹球台场景 */
	void SpawnPinballScene();
	/** 创建HUD */
	void CreateHUD();
	/** 设置俯视摄像机 */
	void SetupCamera();
};
