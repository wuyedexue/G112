// PinballHUD.cpp

#include "PinballHUD.h"
#include "PinballGameMode.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UPinballHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化UI
	if (GameOverPanel)
	{
		GameOverPanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UPinballHUD::OnRestartClicked);
	}

	if (LauncherChargeBar)
	{
		LauncherChargeBar->SetPercent(0.f);
	}

	// 绑定 GameMode 事件
	APinballGameMode* GameMode = Cast<APinballGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->OnScoreChanged.AddDynamic(this, &UPinballHUD::UpdateScore);
		GameMode->OnLivesChanged.AddDynamic(this, &UPinballHUD::UpdateLives);
		GameMode->OnMultiplierChanged.AddDynamic(this, &UPinballHUD::UpdateMultiplier);
	}

	// 初始显示
	UpdateScore(0);
	UpdateLives(3);
	UpdateMultiplier(1);
}

void UPinballHUD::UpdateScore(int32 NewScore)
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewScore)));
	}
}

void UPinballHUD::UpdateLives(int32 RemainingLives)
{
	if (LivesText)
	{
		// 用球形图标表示生命
		FString LivesStr;
		for (int32 i = 0; i < RemainingLives; i++)
		{
			LivesStr += TEXT("● ");
		}
		LivesText->SetText(FText::FromString(LivesStr));
	}
}

void UPinballHUD::UpdateMultiplier(int32 NewMultiplier)
{
	if (MultiplierText)
	{
		if (NewMultiplier > 1)
		{
			MultiplierText->SetText(FText::FromString(FString::Printf(TEXT("x%d"), NewMultiplier)));
			MultiplierText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			MultiplierText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UPinballHUD::ShowGameOver(int32 FinalScore, int32 HighScore)
{
	if (GameOverPanel)
	{
		GameOverPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (GameOverScoreText)
	{
		GameOverScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), FinalScore)));
	}

	if (GameOverHighScoreText)
	{
		GameOverHighScoreText->SetText(FText::FromString(FString::Printf(TEXT("Best: %d"), HighScore)));
	}
}

void UPinballHUD::HideGameOver()
{
	if (GameOverPanel)
	{
		GameOverPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPinballHUD::UpdateLauncherCharge(float Percent)
{
	if (LauncherChargeBar)
	{
		LauncherChargeBar->SetPercent(Percent);
	}
}

void UPinballHUD::OnRestartClicked()
{
	APinballGameMode* GameMode = Cast<APinballGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->RestartGame();
		HideGameOver();
	}
}
