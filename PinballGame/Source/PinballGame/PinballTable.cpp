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
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));

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

	// 左墙可视网格
	LeftWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWallMesh"));
	LeftWallMesh->SetupAttachment(RootComponent);
	LeftWallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWallMesh->SetRelativeLocation(FVector(-TableWidth / 2.f - WallThickness, 0.f, WallHeight));
	if (CubeMesh.Succeeded())
	{
		LeftWallMesh->SetStaticMesh(CubeMesh.Object);
		// Cube 默认100x100x100，缩放到墙壁尺寸 (厚度x2 x 台面长度 x 墙高x2)
		LeftWallMesh->SetRelativeScale3D(FVector(WallThickness * 2.f / 100.f, TableLength / 100.f, WallHeight * 2.f / 100.f));
	}

	// 右墙
	RightWall = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWall"));
	RightWall->SetupAttachment(RootComponent);
	RightWall->SetBoxExtent(FVector(WallThickness, TableLength / 2.f, WallHeight));
	RightWall->SetRelativeLocation(FVector(TableWidth / 2.f + WallThickness, 0.f, WallHeight));
	RightWall->SetCollisionProfileName(TEXT("BlockAll"));
	RightWall->SetSimulatePhysics(false);

	// 右墙可视网格
	RightWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWallMesh"));
	RightWallMesh->SetupAttachment(RootComponent);
	RightWallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWallMesh->SetRelativeLocation(FVector(TableWidth / 2.f + WallThickness, 0.f, WallHeight));
	if (CubeMesh.Succeeded())
	{
		RightWallMesh->SetStaticMesh(CubeMesh.Object);
		RightWallMesh->SetRelativeScale3D(FVector(WallThickness * 2.f / 100.f, TableLength / 100.f, WallHeight * 2.f / 100.f));
	}

	// 顶墙
	TopWall = CreateDefaultSubobject<UBoxComponent>(TEXT("TopWall"));
	TopWall->SetupAttachment(RootComponent);
	TopWall->SetBoxExtent(FVector(TableWidth / 2.f, WallThickness, WallHeight));
	TopWall->SetRelativeLocation(FVector(0.f, TableLength / 2.f + WallThickness, WallHeight));
	TopWall->SetCollisionProfileName(TEXT("BlockAll"));
	TopWall->SetSimulatePhysics(false);

	// 顶墙可视网格
	TopWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopWallMesh"));
	TopWallMesh->SetupAttachment(RootComponent);
	TopWallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TopWallMesh->SetRelativeLocation(FVector(0.f, TableLength / 2.f + WallThickness, WallHeight));
	if (CubeMesh.Succeeded())
	{
		TopWallMesh->SetStaticMesh(CubeMesh.Object);
		TopWallMesh->SetRelativeScale3D(FVector(TableWidth / 100.f, WallThickness * 2.f / 100.f, WallHeight * 2.f / 100.f));
	}

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

	// 通过球的统一排水接口处理，防止与Ball自身的CheckDrain重复扣命
	Ball->MarkAsDraining();

	UE_LOG(LogTemp, Log, TEXT("Ball drained via trigger!"));
}
