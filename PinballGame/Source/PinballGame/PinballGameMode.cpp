// PinballGameMode.cpp

#include "PinballGameMode.h"
#include "PinballBall.h"
#include "PinballSaveGame.h"
#include "PinballPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "Engine/World.h"

APinballGameMode::APinballGameMode()
{
	// 设置默认 PlayerController
	PlayerControllerClass = APinballPlayerController::StaticClass();

	// 默认 Pawn 为空（弹球不需要玩家角色）
	DefaultPawnClass = nullptr;
}

void APinballGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 初始化游戏状态
	RemainingLives = InitialLives;
	CurrentScore = 0;
	ScoreMultiplier = 1;
	bIsGameOver = false;
	NextExtraLifeScore = ExtraLifeScoreThreshold;

	// 加载最高分
	LoadHighScore();

	// 通知 UI 初始状态
	OnScoreChanged.Broadcast(CurrentScore);
	OnLivesChanged.Broadcast(RemainingLives);
	OnMultiplierChanged.Broadcast(ScoreMultiplier);

	// 生成第一个球
	SpawnNewBall();
}

void APinballGameMode::AddScore(int32 Points)
{
	if (bIsGameOver) return;

	// 应用倍率
	int32 ActualPoints = Points * ScoreMultiplier;
	CurrentScore += ActualPoints;

	// 检查是否达到奖命分数
	if (CurrentScore >= NextExtraLifeScore)
	{
		RemainingLives++;
		NextExtraLifeScore += ExtraLifeScoreThreshold;
		OnLivesChanged.Broadcast(RemainingLives);

		UE_LOG(LogTemp, Log, TEXT("Extra life! Lives: %d, Next at: %d"), RemainingLives, NextExtraLifeScore);
	}

	// 更新最高分
	if (CurrentScore > HighScore)
	{
		HighScore = CurrentScore;
		SaveHighScore();
	}

	OnScoreChanged.Broadcast(CurrentScore);

	UE_LOG(LogTemp, Log, TEXT("Score: %d (+%d x%d) | High: %d"), CurrentScore, Points, ScoreMultiplier, HighScore);
}

void APinballGameMode::LoseLife()
{
	if (bIsGameOver) return;

	RemainingLives--;
	ResetMultiplier();

	OnLivesChanged.Broadcast(RemainingLives);

	if (RemainingLives <= 0)
	{
		// 游戏结束
		bIsGameOver = true;
		OnGameOver.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("GAME OVER! Final Score: %d"), CurrentScore);
	}
	else
	{
		// 延迟生成新球
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APinballGameMode::SpawnNewBall, 2.0f, false);
		UE_LOG(LogTemp, Log, TEXT("Ball lost! Lives remaining: %d"), RemainingLives);
	}
}

void APinballGameMode::IncrementMultiplier()
{
	if (ScoreMultiplier < MaxMultiplier)
	{
		ScoreMultiplier++;
		OnMultiplierChanged.Broadcast(ScoreMultiplier);
	}
}

void APinballGameMode::ResetMultiplier()
{
	ScoreMultiplier = 1;
	OnMultiplierChanged.Broadcast(ScoreMultiplier);
}

void APinballGameMode::RestartGame()
{
	CurrentScore = 0;
	RemainingLives = InitialLives;
	ScoreMultiplier = 1;
	bIsGameOver = false;
	NextExtraLifeScore = ExtraLifeScoreThreshold;

	OnScoreChanged.Broadcast(CurrentScore);
	OnLivesChanged.Broadcast(RemainingLives);
	OnMultiplierChanged.Broadcast(ScoreMultiplier);

	SpawnNewBall();

	UE_LOG(LogTemp, Log, TEXT("Game Restarted!"));
}

void APinballGameMode::SpawnNewBall()
{
	if (bIsGameOver) return;
	if (!BallClass) 
	{
		UE_LOG(LogTemp, Warning, TEXT("BallClass not set in GameMode!"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<AActor>(BallClass, BallSpawnLocation, FRotator::ZeroRotator, SpawnParams);
}

void APinballGameMode::SaveHighScore()
{
	UPinballSaveGame* SaveGameInstance = Cast<UPinballSaveGame>(
		UGameplayStatics::LoadGameFromSlot(UPinballSaveGame::SaveSlotName, 0));

	if (!SaveGameInstance)
	{
		SaveGameInstance = Cast<UPinballSaveGame>(
			UGameplayStatics::CreateSaveGameObject(UPinballSaveGame::StaticClass()));
	}

	if (SaveGameInstance)
	{
		SaveGameInstance->HighScore = HighScore;
		SaveGameInstance->TotalGamesPlayed++;
		SaveGameInstance->TotalScoreEarned += CurrentScore;
		if (ScoreMultiplier > SaveGameInstance->HighestMultiplier)
		{
			SaveGameInstance->HighestMultiplier = ScoreMultiplier;
		}

		UGameplayStatics::SaveGameToSlot(SaveGameInstance, UPinballSaveGame::SaveSlotName, 0);
		UE_LOG(LogTemp, Log, TEXT("High Score Saved: %d"), HighScore);
	}
}

void APinballGameMode::LoadHighScore()
{
	UPinballSaveGame* SaveGameInstance = Cast<UPinballSaveGame>(
		UGameplayStatics::LoadGameFromSlot(UPinballSaveGame::SaveSlotName, 0));

	if (SaveGameInstance)
	{
		HighScore = SaveGameInstance->HighScore;
		UE_LOG(LogTemp, Log, TEXT("High Score Loaded: %d"), HighScore);
	}
	else
	{
		HighScore = 0;
		UE_LOG(LogTemp, Log, TEXT("No save found, starting fresh."));
	}
}
