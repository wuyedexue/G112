// PinballSaveGame.h
// 存档 - 保存最高分和解锁状态

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PinballSaveGame.generated.h"

UCLASS()
class PINBALLGAME_API UPinballSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPinballSaveGame();

	/** 最高分 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int32 HighScore = 0;

	/** 总游戏次数 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int32 TotalGamesPlayed = 0;

	/** 累计总分 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int64 TotalScoreEarned = 0;

	/** 最高连击倍率 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int32 HighestMultiplier = 1;

	/** 存档槽位名称 */
	static const FString SaveSlotName;
};
