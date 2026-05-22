// PinballHUD.h
// 弹球游戏HUD - 使用Canvas直接绘制，无需Widget Blueprint

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PinballHUD.generated.h"

UCLASS()
class PINBALLGAME_API APinballHUD : public AHUD
{
	GENERATED_BODY()

public:
	APinballHUD();

	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

private:
	// 缓存的游戏数据
	int32 CachedScore = 0;
	int32 CachedLives = 3;
	int32 CachedMultiplier = 1;
	int32 CachedHighScore = 0;
	bool bShowGameOver = false;

	// 事件回调
	UFUNCTION()
	void OnScoreChanged(int32 NewScore);
	UFUNCTION()
	void OnLivesChanged(int32 NewLives);
	UFUNCTION()
	void OnMultiplierChanged(int32 NewMultiplier);
	UFUNCTION()
	void OnGameOver();

	// 绘制工具函数
	void DrawTextCentered(const FString& Text, float X, float Y, FLinearColor Color, float Scale = 1.0f);
};
