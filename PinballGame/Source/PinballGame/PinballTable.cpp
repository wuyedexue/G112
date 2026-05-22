// PinballTable.cpp

#include "PinballTable.h"
#include "PinballBall.h"
#include "PinballGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

APinballTable::APinballTable()
{
	PrimaryActorTick.bCanEverTick = false;

	// 台面底板 - 使用Cube而非Plane，保证双面碰撞且有厚度
	TableFloor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TableFloor"));
	RootComponent = TableFloor;
	TableFloor->SetCollisionProfileName(TEXT("BlockAll"));
	TableFloor->SetSimulatePhysics(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		TableFloor->SetStaticMesh(CubeMesh.Object);
		// Cube默认100x100x100, 缩放为台面大小（宽x长x薄）
		TableFloor->SetWorldScale3D(FVector(TableWidth / 100.f, TableLength / 100.f, 0.05f));
	}

	// 左墙 - 使用StaticMesh可视化
	LeftWall = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWall"));
	LeftWall->SetupAttachment(RootComponent);
	LeftWall->SetBoxExtent(FVector(WallThickness, TableLength / 2.f, WallHeight));
	LeftWall->SetRelativeLocation(FVector(-TableWidth / 2.f - WallThickness, 0.f, WallHeight));
	LeftWall->SetCollisionProfileName(TEXT("BlockAll"));
	LeftWall->SetSimulatePhysics(false);

	// 左墙可视网格
	LeftWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWallMesh"));
	LeftWallMesh->SetupAttachment(LeftWall);
	if (CubeMesh.Succeeded())
	{
		LeftWallMesh->SetStaticMesh(CubeMesh.Object);
		LeftWallMesh->SetRelativeScale3D(FVector(WallThickness / 50.f, TableLength / 100.f, WallHeight / 50.f));
	}
	LeftWallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 右墙
	RightWall = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWall"));
	RightWall->SetupAttachment(RootComponent);
	RightWall->SetBoxExtent(FVector(WallThickness, TableLength / 2.f, WallHeight));
	RightWall->SetRelativeLocation(FVector(TableWidth / 2.f + WallThickness, 0.f, WallHeight));
	RightWall->SetCollisionProfileName(TEXT("BlockAll"));
	RightWall->SetSimulatePhysics(false);

	// 右墙可视网格
	RightWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWallMesh"));
	RightWallMesh->SetupAttachment(RightWall);
	if (CubeMesh.Succeeded())
	{
		RightWallMesh->SetStaticMesh(CubeMesh.Object);
		RightWallMesh->SetRelativeScale3D(FVector(WallThickness / 50.f, TableLength / 100.f, WallHeight / 50.f));
	}
	RightWallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 顶墙
	TopWall = CreateDefaultSubobject<UBoxComponent>(TEXT("TopWall"));
	TopWall->SetupAttachment(RootComponent);
	TopWall->SetBoxExtent(FVector(TableWidth / 2.f + WallThickness * 2.f, WallThickness, WallHeight));
	TopWall->SetRelativeLocation(FVector(0.f, TableLength / 2.f + WallThickness, WallHeight));
	TopWall->SetCollisionProfileName(TEXT("BlockAll"));
	TopWall->SetSimulatePhysics(false);

	// 顶墙可视网格
	TopWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopWallMesh"));
	TopWallMesh->SetupAttachment(TopWall);
	if (CubeMesh.Succeeded())
	{
		TopWallMesh->SetStaticMesh(CubeMesh.Object);
		TopWallMesh->SetRelativeScale3D(FVector((TableWidth + WallThickness * 4.f) / 100.f, WallThickness / 50.f, WallHeight / 50.f));
	}
	TopWallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 底部左侧挡板引导墙（引导球进入排水口）
	BottomLeftWall = CreateDefaultSubobject<UBoxComponent>(TEXT("BottomLeftWall"));
	BottomLeftWall->SetupAttachment(RootComponent);
	float BottomWallWidth = (TableWidth / 2.f - DrainGapWidth / 2.f);
	BottomLeftWall->SetBoxExtent(FVector(BottomWallWidth / 2.f, WallThickness, WallHeight));
	BottomLeftWall->SetRelativeLocation(FVector(
		-(DrainGapWidth / 2.f + BottomWallWidth / 2.f),
		-TableLength / 2.f - WallThickness,
		WallHeight));
	BottomLeftWall->SetCollisionProfileName(TEXT("BlockAll"));
	BottomLeftWall->SetSimulatePhysics(false);

	// 底部右侧挡板引导墙
	BottomRightWall = CreateDefaultSubobject<UBoxComponent>(TEXT("BottomRightWall"));
	BottomRightWall->SetupAttachment(RootComponent);
	BottomRightWall->SetBoxExtent(FVector(BottomWallWidth / 2.f, WallThickness, WallHeight));
	BottomRightWall->SetRelativeLocation(FVector(
		(DrainGapWidth / 2.f + BottomWallWidth / 2.f),
		-TableLength / 2.f - WallThickness,
		WallHeight));
	BottomRightWall->SetCollisionProfileName(TEXT("BlockAll"));
	BottomRightWall->SetSimulatePhysics(false);

	// 排水口触发器（底部中央开口下方）
	DrainTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("DrainTrigger"));
	DrainTrigger->SetupAttachment(RootComponent);
	DrainTrigger->SetBoxExtent(FVector(DrainGapWidth / 2.f + 10.f, 30.f, 30.f));
	DrainTrigger->SetRelativeLocation(FVector(0.f, -TableLength / 2.f - 40.f, -20.f));
	DrainTrigger->SetCollisionProfileName(TEXT("OverlapAll"));
	DrainTrigger->SetGenerateOverlapEvents(true);

	// 发射通道右侧墙（将launcher通道与主台面隔开）
	// 通道宽度需容纳球(半径15)，放在X=60处，通道为X=65到X=105(40单位宽)
	LauncherChannelWall = CreateDefaultSubobject<UBoxComponent>(TEXT("LauncherChannelWall"));
	LauncherChannelWall->SetupAttachment(RootComponent);
	LauncherChannelWall->SetBoxExtent(FVector(WallThickness, TableLength / 3.f, WallHeight));
	LauncherChannelWall->SetRelativeLocation(FVector(60.f, -TableLength / 6.f, WallHeight));
	LauncherChannelWall->SetCollisionProfileName(TEXT("BlockAll"));
	LauncherChannelWall->SetSimulatePhysics(false);
}

void APinballTable::BeginPlay()
{
	Super::BeginPlay();

	// 不再物理倾斜台面——倾斜效果通过Ball的TiltForce模拟
	// 这样避免了spawn坐标与倾斜后的碰撞面不匹配的问题

	// 绑定排水口重叠事件
	DrainTrigger->OnComponentBeginOverlap.AddDynamic(this, &APinballTable::OnDrainOverlap);

	// 应用材质颜色
	ApplyMaterials();
}

void APinballTable::ApplyMaterials()
{
	// 深绿色台面
	if (TableFloor && TableFloor->GetMaterial(0))
	{
		UMaterialInstanceDynamic* FloorMat = UMaterialInstanceDynamic::Create(
			TableFloor->GetMaterial(0), this);
		if (FloorMat)
		{
			FloorMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.05f, 0.3f, 0.05f));
			TableFloor->SetMaterial(0, FloorMat);
		}
	}

	// 棕色墙壁
	TArray<UStaticMeshComponent*> WallMeshes = { LeftWallMesh, RightWallMesh, TopWallMesh };
	for (UStaticMeshComponent* WMesh : WallMeshes)
	{
		if (WMesh && WMesh->GetMaterial(0))
		{
			UMaterialInstanceDynamic* WallMat = UMaterialInstanceDynamic::Create(
				WMesh->GetMaterial(0), this);
			if (WallMat)
			{
				WallMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.45f, 0.25f, 0.1f));
				WMesh->SetMaterial(0, WallMat);
			}
		}
	}
}

void APinballTable::OnDrainOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APinballBall* Ball = Cast<APinballBall>(OtherActor);
	if (!Ball) return;

	// 通过球的统一排水接口处理，防止与Ball自身的CheckDrain重复扣命
	Ball->MarkAsDraining();

	UE_LOG(LogTemp, Log, TEXT("Ball drained via trigger!"));
}
