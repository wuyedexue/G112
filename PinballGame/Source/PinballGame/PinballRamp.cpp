// PinballRamp.cpp

#include "PinballRamp.h"
#include "PinballBall.h"
#include "PinballGameMode.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"

APinballRamp::APinballRamp()
{
	PrimaryActorTick.bCanEverTick = false;

	// 轨道路径
	RampSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RampSpline"));
	RootComponent = RampSpline;

	// 入口
	EntryTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryTrigger"));
	EntryTrigger->SetupAttachment(RootComponent);
	EntryTrigger->SetBoxExtent(FVector(15.f, 15.f, 15.f));
	EntryTrigger->SetCollisionProfileName(TEXT("OverlapAll"));
	EntryTrigger->SetGenerateOverlapEvents(true);

	// 出口
	ExitTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitTrigger"));
	ExitTrigger->SetupAttachment(RootComponent);
	ExitTrigger->SetBoxExtent(FVector(15.f, 15.f, 15.f));
	ExitTrigger->SetCollisionProfileName(TEXT("OverlapAll"));
	ExitTrigger->SetGenerateOverlapEvents(true);
	ExitTrigger->SetRelativeLocation(FVector(0.f, 100.f, 50.f)); // 出口在更高位置
}

void APinballRamp::BeginPlay()
{
	Super::BeginPlay();

	EntryTrigger->OnComponentBeginOverlap.AddDynamic(this, &APinballRamp::OnEntryOverlap);
	ExitTrigger->OnComponentBeginOverlap.AddDynamic(this, &APinballRamp::OnExitOverlap);
}

void APinballRamp::OnEntryOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APinballBall* Ball = Cast<APinballBall>(OtherActor);
	if (!Ball) return;

	// 记录球进入轨道
	BallsInRamp.Add(Ball);
	UE_LOG(LogTemp, Log, TEXT("Ball entered ramp: %s"), *RampName);
}

void APinballRamp::OnExitOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APinballBall* Ball = Cast<APinballBall>(OtherActor);
	if (!Ball) return;

	// 只有从入口进来的球才算通过
	if (!BallsInRamp.Contains(Ball)) return;
	BallsInRamp.Remove(Ball);

	// 加分
	ConsecutiveCount++;
	int32 TotalScore = RampScore + (ConsecutiveCount - 1) * ConsecutiveBonus;

	APinballGameMode* GameMode = Cast<APinballGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->AddScore(TotalScore);
		GameMode->IncrementMultiplier();
	}

	// 音效
	if (RampCompleteSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RampCompleteSound, GetActorLocation());
	}

	UE_LOG(LogTemp, Log, TEXT("Ramp %s complete! Score: +%d (x%d consecutive)"), *RampName, TotalScore, ConsecutiveCount);
}
