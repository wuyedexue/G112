// PinballHUD.cpp
// Canvas-based HUD - 无需Widget Blueprint，程序化绘制

#include "PinballHUD.h"
#include "PinballGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

APinballHUD::APinballHUD()
{
}

void APinballHUD::BeginPlay()
{
	Super::BeginPlay();

	// 绑定 GameMode 事件
	APinballGameMode* GameMode = Cast<APinballGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->OnScoreChanged.AddDynamic(this, &APinballHUD::OnScoreChanged);
		GameMode->OnLivesChanged.AddDynamic(this, &APinballHUD::OnLivesChanged);
		GameMode->OnMultiplierChanged.AddDynamic(this, &APinballHUD::OnMultiplierChanged);
		GameMode->OnGameOver.AddDynamic(this, &APinballHUD::OnGameOver);

		CachedHighScore = GameMode->GetHighScore();
	}
}

void APinballHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas) return;

	const float ScreenW = Canvas->SizeX;
	const float ScreenH = Canvas->SizeY;

	// === 分数 (左上角) ===
	FString ScoreStr = FString::Printf(TEXT("SCORE: %d"), CachedScore);
	DrawText(ScoreStr, FLinearColor::White, 20.f, 20.f, nullptr, 1.5f);

	// === 最高分 (右上角) ===
	FString HighScoreStr = FString::Printf(TEXT("BEST: %d"), CachedHighScore);
	DrawText(HighScoreStr, FLinearColor::Yellow, ScreenW - 200.f, 20.f, nullptr, 1.2f);

	// === 生命值 (左上角下方) ===
	FString LivesStr = FString::Printf(TEXT("LIVES: %d"), CachedLives);
	DrawText(LivesStr, FLinearColor::Green, 20.f, 55.f, nullptr, 1.2f);

	// === 倍率 (分数下方) ===
	if (CachedMultiplier > 1)
	{
		FString MultStr = FString::Printf(TEXT("x%d COMBO"), CachedMultiplier);
		DrawText(MultStr, FLinearColor(1.f, 0.5f, 0.f, 1.f), 20.f, 85.f, nullptr, 1.3f);
	}

	// === 操作提示 (底部) ===
	DrawText(TEXT("A/D: Flippers | SPACE: Launch | R: Restart"), FLinearColor(0.7f, 0.7f, 0.7f, 1.f), 20.f, ScreenH - 30.f, nullptr, 0.9f);

	// === Game Over 界面 ===
	if (bShowGameOver)
	{
		// 半透明背景
		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.7f), 0.f, 0.f, ScreenW, ScreenH);

		// Game Over 文字
		float CenterX = ScreenW / 2.f - 100.f;
		float CenterY = ScreenH / 2.f - 60.f;

		DrawText(TEXT("GAME OVER"), FLinearColor::Red, CenterX, CenterY, nullptr, 2.5f);

		FString FinalScore = FString::Printf(TEXT("Final Score: %d"), CachedScore);
		DrawText(FinalScore, FLinearColor::White, CenterX, CenterY + 50.f, nullptr, 1.5f);

		FString BestScore = FString::Printf(TEXT("Best: %d"), CachedHighScore);
		DrawText(BestScore, FLinearColor::Yellow, CenterX, CenterY + 80.f, nullptr, 1.3f);

		DrawText(TEXT("Press R to Restart"), FLinearColor::Green, CenterX, CenterY + 120.f, nullptr, 1.2f);
	}
}

void APinballHUD::OnScoreChanged(int32 NewScore)
{
	CachedScore = NewScore;

	APinballGameMode* GameMode = Cast<APinballGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		CachedHighScore = GameMode->GetHighScore();
		// 如果分数归零且游戏没有结束，说明是重新开始
		if (NewScore == 0 && !GameMode->IsGameOver())
		{
			bShowGameOver = false;
		}
	}
}

void APinballHUD::OnLivesChanged(int32 NewLives)
{
	CachedLives = NewLives;
}

void APinballHUD::OnMultiplierChanged(int32 NewMultiplier)
{
	CachedMultiplier = NewMultiplier;
}

void APinballHUD::OnGameOver()
{
	bShowGameOver = true;

	APinballGameMode* GameMode = Cast<APinballGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		CachedHighScore = GameMode->GetHighScore();
	}
}

void APinballHUD::DrawTextCentered(const FString& Text, float X, float Y, FLinearColor Color, float Scale)
{
	DrawText(Text, Color, X, Y, nullptr, Scale);
}
