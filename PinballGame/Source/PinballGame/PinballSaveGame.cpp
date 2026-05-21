// PinballSaveGame.cpp

#include "PinballSaveGame.h"

const FString UPinballSaveGame::SaveSlotName = TEXT("PinballSave");

UPinballSaveGame::UPinballSaveGame()
{
	HighScore = 0;
	TotalGamesPlayed = 0;
	TotalScoreEarned = 0;
	HighestMultiplier = 1;
}
