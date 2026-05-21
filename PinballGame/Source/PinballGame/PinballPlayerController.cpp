// PinballPlayerController.cpp

#include "PinballPlayerController.h"
#include "PinballFlipper.h"
#include "PinballLauncher.h"
#include "PinballBall.h"
#include "PinballGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/World.h"

APinballPlayerController::APinballPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
}

void APinballPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 自动查找场景中的弹球组件
	FindPinballActors();

	// 设置摄像机视角（俯视弹球台）
	// 实际项目中应在关卡中放置摄像机
}

void APinballPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 绑定输入 (UE4.26 Legacy Input System)
	InputComponent->BindAction("LeftFlipper", IE_Pressed, this, &APinballPlayerController::OnLeftFlipperPressed);
	InputComponent->BindAction("LeftFlipper", IE_Released, this, &APinballPlayerController::OnLeftFlipperReleased);
	InputComponent->BindAction("RightFlipper", IE_Pressed, this, &APinballPlayerController::OnRightFlipperPressed);
	InputComponent->BindAction("RightFlipper", IE_Released, this, &APinballPlayerController::OnRightFlipperReleased);
	InputComponent->BindAction("Launch", IE_Pressed, this, &APinballPlayerController::OnLauncherPressed);
	InputComponent->BindAction("Launch", IE_Released, this, &APinballPlayerController::OnLauncherReleased);
	InputComponent->BindAction("NudgeLeft", IE_Pressed, this, &APinballPlayerController::OnNudgeLeft);
	InputComponent->BindAction("NudgeRight", IE_Pressed, this, &APinballPlayerController::OnNudgeRight);
	InputComponent->BindAction("Restart", IE_Pressed, this, &APinballPlayerController::OnRestart);
}

void APinballPlayerController::OnLeftFlipperPressed()
{
	if (LeftFlipper)
	{
		LeftFlipper->Activate();
	}
}

void APinballPlayerController::OnLeftFlipperReleased()
{
	if (LeftFlipper)
	{
		LeftFlipper->Deactivate();
	}
}

void APinballPlayerController::OnRightFlipperPressed()
{
	if (RightFlipper)
	{
		RightFlipper->Activate();
	}
}

void APinballPlayerController::OnRightFlipperReleased()
{
	if (RightFlipper)
	{
		RightFlipper->Deactivate();
	}
}

void APinballPlayerController::OnLauncherPressed()
{
	if (Launcher)
	{
		Launcher->StartCharging();
	}
}

void APinballPlayerController::OnLauncherReleased()
{
	if (Launcher)
	{
		Launcher->Release();
	}
}

void APinballPlayerController::OnNudgeLeft()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastNudgeTime < NudgeCooldown) return;
	LastNudgeTime = CurrentTime;

	// 对场景中所有球施加力
	for (TActorIterator<APinballBall> It(GetWorld()); It; ++It)
	{
		APinballBall* Ball = *It;
		Ball->AddImpulse(FVector(-NudgeForce, 0.f, 0.f));
	}

	UE_LOG(LogTemp, Log, TEXT("Nudge Left!"));
}

void APinballPlayerController::OnNudgeRight()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastNudgeTime < NudgeCooldown) return;
	LastNudgeTime = CurrentTime;

	for (TActorIterator<APinballBall> It(GetWorld()); It; ++It)
	{
		APinballBall* Ball = *It;
		Ball->AddImpulse(FVector(NudgeForce, 0.f, 0.f));
	}

	UE_LOG(LogTemp, Log, TEXT("Nudge Right!"));
}

void APinballPlayerController::OnRestart()
{
	APinballGameMode* GameMode = Cast<APinballGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode && GameMode->IsGameOver())
	{
		GameMode->RestartGame();
	}
}

void APinballPlayerController::FindPinballActors()
{
	// 查找左右挡板
	for (TActorIterator<APinballFlipper> It(GetWorld()); It; ++It)
	{
		APinballFlipper* Flipper = *It;
		// 通过标签或属性区分左右
		if (Flipper->ActorHasTag(TEXT("LeftFlipper")) || !LeftFlipper)
		{
			LeftFlipper = Flipper;
		}
		if (Flipper->ActorHasTag(TEXT("RightFlipper")))
		{
			RightFlipper = Flipper;
		}
	}

	// 查找发射器
	for (TActorIterator<APinballLauncher> It(GetWorld()); It; ++It)
	{
		Launcher = *It;
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("Found - LeftFlipper: %s, RightFlipper: %s, Launcher: %s"),
		LeftFlipper ? TEXT("Yes") : TEXT("No"),
		RightFlipper ? TEXT("Yes") : TEXT("No"),
		Launcher ? TEXT("Yes") : TEXT("No"));
}
