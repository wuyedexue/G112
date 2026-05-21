// PinballTable.cpp

#include "PinballTable.h"
#include "PinballBall.h"
#include "PinballGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

APinballTable::APinballTable()
{
	PrimaryActorTick.bCanEverTick = false;

	// 台面底板
	TableFloor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TableFloor"));
	RootComponent = TableFloor;
	TableFloor->SetCollisionProfileName(TEXT("BlockAll"));
	TableFloor->SetSimulatePhysics(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded())
	{
		TableFloor->SetStaticMesh(PlaneMesh.Object);
		TableFloor->SetWorldScale3D(FVector(TableWidth / 100.f, TableLength / 100.f, 1.f));
	}

	// 左墙
	LeftWall = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWall"));
	LeftWall->SetupAttachment(RootComponent);
	LeftWall->SetBoxExtent(FVector(WallThickness, TableLength / 2.f, WallHeight));
	LeftWall->SetRelativeLocation(FVector(-TableWidth / 2.f - WallThickness, 0.f, WallHeight));
	LeftWall->SetCollisionProfileName(TEXT("BlockAll"));
	LeftWall->SetSimulatePhysics(false);

	// 右墙
	RightWall = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWall"));
	RightWall->SetupAttachment(RootComponent);
	RightWall->SetBoxExtent(FVector(WallThickness, TableLength / 2.f, WallHeight));
	RightWall->SetRelativeLocation(FVector(TableWidth / 2.f + WallThickness, 0.f, WallHeight));
	RightWall->SetCollisionProfileName(TEXT("BlockAll"));
	RightWall->SetSimulatePhysics(false);

	// 顶墙
	TopWall = CreateDefaultSubobject<UBoxComponent>(TEXT("TopWall"));
	TopWall->SetupAttachment(RootComponent);
	TopWall->SetBoxExtent(FVector(TableWidth / 2.f, WallThickness, WallHeight));
	TopWall->SetRelativeLocation(FVector(0.f, TableLength / 2.f + WallThickness, WallHeight));
	TopWall->SetCollisionProfileName(TEXT("BlockAll"));
	TopWall->SetSimulatePhysics(false);

	// 排水口触发器（底部开口区域）
	DrainTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("DrainTrigger"));
	DrainTrigger->SetupAttachment(RootComponent);
	DrainTrigger->SetBoxExtent(FVector(TableWidth / 2.f, 20.f, 20.f));
	DrainTrigger->SetRelativeLocation(FVector(0.f, -TableLength / 2.f - 30.f, -10.f));
	DrainTrigger->SetCollisionProfileName(TEXT("OverlapAll"));
	DrainTrigger->SetGenerateOverlapEvents(true);
}

void APinballTable::BeginPlay()
{
	Super::BeginPlay();

	// 应用台面倾斜
	FRotator TiltRotation = GetActorRotation();
	TiltRotation.Pitch = TableTiltAngle;
	SetActorRotation(TiltRotation);

	// 绑定排水口重叠事件
	DrainTrigger->OnComponentBeginOverlap.AddDynamic(this, &APinballTable::OnDrainOverlap);
}

void APinballTable::OnDrainOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APinballBall* Ball = Cast<APinballBall>(OtherActor);
	if (!Ball) return;

	// 球进入排水口 - 通知GameMode减命
	APinballGameMode* GameMode = Cast<APinballGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->LoseLife();
	}

	// 销毁球
	Ball->Destroy();

	UE_LOG(LogTemp, Log, TEXT("Ball drained!"));
}
