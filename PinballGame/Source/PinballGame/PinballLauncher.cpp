// PinballLauncher.cpp

#include "PinballLauncher.h"
#include "PinballBall.h"
#include "PinballMaterials.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

APinballLauncher::APinballLauncher()
{
	PrimaryActorTick.bCanEverTick = true;

	// 发射器碰撞区域（足够大以包含球）
	LauncherCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LauncherCollision"));
	LauncherCollision->SetBoxExtent(FVector(25.f, 30.f, 30.f));
	LauncherCollision->SetCollisionProfileName(TEXT("OverlapAll"));
	LauncherCollision->SetSimulatePhysics(false);
	RootComponent = LauncherCollision;

	// 弹簧柱塞网格
	PlungerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlungerMesh"));
	PlungerMesh->SetupAttachment(RootComponent);
	PlungerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		PlungerMesh->SetStaticMesh(CylinderMesh.Object);
		PlungerMesh->SetWorldScale3D(FVector(0.15f, 0.15f, 0.3f));
	}
}

void APinballLauncher::BeginPlay()
{
	Super::BeginPlay();
	PlungerInitialLocation = PlungerMesh->GetRelativeLocation();

	// 设置深红色材质
	if (PlungerMesh && PlungerMesh->GetMaterial(0))
	{
		UMaterialInstanceDynamic* LauncherMat = UMaterialInstanceDynamic::Create(
			PlungerMesh->GetMaterial(0), this);
		if (LauncherMat)
		{
			LauncherMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.6f, 0.05f, 0.05f));
			PlungerMesh->SetMaterial(0, LauncherMat);
		}
	}
}

void APinballLauncher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsCharging)
	{
		// 蓄力
		CurrentCharge = FMath::Min(CurrentCharge + ChargeRate * DeltaTime, MaxCharge);

		// 视觉效果 - 弹簧后缩
		FVector RetractOffset = -LaunchDirection.GetSafeNormal() * PlungerRetractDistance * GetChargePercent();
		PlungerMesh->SetRelativeLocation(PlungerInitialLocation + RetractOffset);
	}
}

void APinballLauncher::StartCharging()
{
	bIsCharging = true;
	CurrentCharge = 0.f;
	UE_LOG(LogTemp, Log, TEXT("Launcher: Charging started"));
}

void APinballLauncher::Release()
{
	if (!bIsCharging) return;

	bIsCharging = false;

	// 计算发射力
	float LaunchForce = FMath::Lerp(MinLaunchForce, MaxLaunchForce, GetChargePercent());
	FVector Force = LaunchDirection.GetSafeNormal() * LaunchForce;

	// 找到发射器区域内的球
	TArray<AActor*> OverlappingActors;
	LauncherCollision->GetOverlappingActors(OverlappingActors, APinballBall::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		APinballBall* Ball = Cast<APinballBall>(Actor);
		if (Ball)
		{
			Ball->LaunchBall(Force);
			UE_LOG(LogTemp, Log, TEXT("Launcher: Ball launched with force %.0f (charge: %.1f%%)"), LaunchForce, GetChargePercent() * 100.f);
		}
	}

	// 播放音效
	if (LaunchSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LaunchSound, GetActorLocation(), GetChargePercent());
	}

	// 重置弹簧位置（带动画）
	PlungerMesh->SetRelativeLocation(PlungerInitialLocation);
	CurrentCharge = 0.f;
}
