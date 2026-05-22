// PinballGameMode.cpp

#include "PinballGameMode.h"
#include "PinballBall.h"
#include "PinballTable.h"
#include "PinballFlipper.h"
#include "PinballBumper.h"
#include "PinballLauncher.h"
#include "PinballHUD.h"
#include "PinballSaveGame.h"
#include "PinballPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "Camera/CameraActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"

APinballGameMode::APinballGameMode()
{
	// 设置默认 PlayerController
	PlayerControllerClass = APinballPlayerController::StaticClass();

	// 默认 Pawn 为空（弹球不需要玩家角色）
	DefaultPawnClass = nullptr;

	// 设置 HUD 类
	HUDClass = APinballHUD::StaticClass();

	// 设置默认 BallClass
	BallClass = APinballBall::StaticClass();
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

	// 程序化生成场景
	SpawnPinballScene();

	// 设置摄像机
	SetupCamera();

	// 通知 UI 初始状态
	OnScoreChanged.Broadcast(CurrentScore);
	OnLivesChanged.Broadcast(RemainingLives);
	OnMultiplierChanged.Broadcast(ScoreMultiplier);

	// 生成第一个球（延迟1秒等待场景就绪）
	FTimerHandle SpawnTimer;
	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &APinballGameMode::SpawnNewBall, 1.0f, false);
}

void APinballGameMode::SpawnPinballScene()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// === 生成弹球台 ===
	APinballTable* Table = World->SpawnActor<APinballTable>(APinballTable::StaticClass(), FVector(0.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);

	// === 生成左挡板（位置按台面2.5倍放大） ===
	APinballFlipper* LFlipper = World->SpawnActor<APinballFlipper>(APinballFlipper::StaticClass(), FVector(-150.f, -425.f, 30.f), FRotator::ZeroRotator, SpawnParams);
	if (LFlipper)
	{
		LFlipper->Tags.Add(TEXT("LeftFlipper"));
	}

	// === 生成右挡板 ===
	APinballFlipper* RFlipper = World->SpawnActor<APinballFlipper>(APinballFlipper::StaticClass(), FVector(150.f, -425.f, 30.f), FRotator::ZeroRotator, SpawnParams);
	if (RFlipper)
	{
		RFlipper->Tags.Add(TEXT("RightFlipper"));
	}

	// === 生成发射器（右侧通道底部） ===
	World->SpawnActor<APinballLauncher>(APinballLauncher::StaticClass(), FVector(205.f, -400.f, 20.f), FRotator::ZeroRotator, SpawnParams);

	// === 生成 Bumpers ===
	// 上方区域放置6个弹射器（位置按比例放大）
	TArray<FVector> BumperLocations = {
		FVector(-100.f, 200.f, 30.f),
		FVector(100.f, 200.f, 30.f),
		FVector(0.f, 300.f, 30.f),
		FVector(-150.f, 75.f, 30.f),
		FVector(150.f, 75.f, 30.f),
		FVector(0.f, 125.f, 30.f)
	};

	for (const FVector& Loc : BumperLocations)
	{
		World->SpawnActor<APinballBumper>(APinballBumper::StaticClass(), Loc, FRotator::ZeroRotator, SpawnParams);
	}

	// 设置球生成位置（发射通道中心）
	BallSpawnLocation = FVector(205.f, -400.f, 40.f);

	UE_LOG(LogTemp, Log, TEXT("PinballScene spawned: Table, 2 Flippers, 1 Launcher, 6 Bumpers"));
}

void APinballGameMode::SetupCamera()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 俯视摄像机（从上方看弹球台，提高高度以覆盖更大的台面）
	ACameraActor* Camera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FVector(0.f, -50.f, 1200.f), FRotator(-90.f, 0.f, 0.f), SpawnParams);

	// 将玩家视角切换到这个摄像机
	APlayerController* PC = World->GetFirstPlayerController();
	if (PC && Camera)
	{
		PC->SetViewTarget(Camera);
	}

	// === 场景照明 ===

	// 主方向光（从上方偏斜射入）
	ADirectionalLight* MainLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0.f, 0.f, 400.f), FRotator(-60.f, 30.f, 0.f), SpawnParams);
	if (MainLight)
	{
		ULightComponent* LightComp = MainLight->GetLightComponent();
		if (LightComp)
		{
			LightComp->SetIntensity(4.0f);
		}
	}

	// 补光方向光（从另一侧补光，减少暗面）
	ADirectionalLight* FillLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0.f, 0.f, 300.f), FRotator(-45.f, -150.f, 0.f), SpawnParams);
	if (FillLight)
	{
		ULightComponent* FillComp = FillLight->GetLightComponent();
		if (FillComp)
		{
			FillComp->SetIntensity(2.0f);
		}
	}

	// 中央点光源（环境补充）
	APointLight* PointLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), FVector(0.f, 0.f, 350.f), FRotator::ZeroRotator, SpawnParams);
	if (PointLight)
	{
		UPointLightComponent* PLComp = PointLight->GetPointLightComponent();
		if (PLComp)
		{
			PLComp->SetIntensity(3000.f);
			PLComp->SetAttenuationRadius(800.f);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Scene lighting setup complete: 2 directional + 1 point light"));
}

void APinballGameMode::CreateHUD()
{
	// HUD通过Canvas绘制方式由PlayerController直接处理
	// 避免BindWidget依赖Widget Blueprint
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
		// 默认使用 APinballBall
		BallClass = APinballBall::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<AActor>(BallClass, BallSpawnLocation, FRotator::ZeroRotator, SpawnParams);
	UE_LOG(LogTemp, Log, TEXT("New ball spawned at %s"), *BallSpawnLocation.ToString());
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
