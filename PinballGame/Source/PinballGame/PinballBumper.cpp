// PinballBumper.cpp

#include "PinballBumper.h"
#include "PinballBall.h"
#include "PinballGameMode.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

APinballBumper::APinballBumper()
{
	PrimaryActorTick.bCanEverTick = false;

	// 碰撞体
	BumperCollision = CreateDefaultSubobject<USphereComponent>(TEXT("BumperCollision"));
	BumperCollision->InitSphereRadius(CollisionRadius);
	BumperCollision->SetCollisionProfileName(TEXT("PhysicsActor"));
	BumperCollision->SetSimulatePhysics(false);
	BumperCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	BumperCollision->SetNotifyRigidBodyCollision(true);
	RootComponent = BumperCollision;

	// 网格
	BumperMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BumperMesh"));
	BumperMesh->SetupAttachment(RootComponent);
	BumperMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 使用圆柱体作为默认外观
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		BumperMesh->SetStaticMesh(CylinderMesh.Object);
		BumperMesh->SetWorldScale3D(FVector(0.4f, 0.4f, 0.2f));
	}

	// 击中指示灯
	HitLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("HitLight"));
	HitLight->SetupAttachment(RootComponent);
	HitLight->SetIntensity(0.f);
	HitLight->SetLightColor(FLinearColor::Red);
	HitLight->SetAttenuationRadius(100.f);
}

void APinballBumper::BeginPlay()
{
	Super::BeginPlay();

	// 绑定碰撞
	BumperCollision->OnComponentHit.AddDynamic(this, &APinballBumper::OnBumperHit);

	// 根据类型设置参数
	switch (BumperType)
	{
	case EBumperType::RoundBumper:
		BounceForce = 1200.f;
		ScoreValue = 100;
		break;
	case EBumperType::TriangleBumper:
		BounceForce = 800.f;
		ScoreValue = 50;
		break;
	case EBumperType::MushroomBumper:
		BounceForce = 400.f;
		ScoreValue = 500;
		break;
	case EBumperType::DropTarget:
		BounceForce = 200.f;
		ScoreValue = 1000;
		break;
	}

	// 设置红色材质让bumper清晰可见
	if (BumperMesh && BumperMesh->GetMaterial(0))
	{
		UMaterialInstanceDynamic* BumperMat = UMaterialInstanceDynamic::Create(
			BumperMesh->GetMaterial(0), this);
		if (BumperMat)
		{
			BumperMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.9f, 0.1f, 0.1f));
			BumperMesh->SetMaterial(0, BumperMat);
		}
	}
}

void APinballBumper::OnBumperHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bIsActive) return;

	APinballBall* Ball = Cast<APinballBall>(OtherActor);
	if (!Ball) return;

	// === 弹射球 ===
	FVector BounceDirection = (Ball->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	// 添加向上分量，防止球被弹到桌面下方
	BounceDirection.Z = FMath::Max(BounceDirection.Z, 0.2f);
	BounceDirection.Normalize();

	Ball->AddImpulse(BounceDirection * BounceForce);

	// === 加分 ===
	APinballGameMode* GameMode = Cast<APinballGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->AddScore(ScoreValue);

		if (bIncrementsMultiplier)
		{
			GameMode->IncrementMultiplier();
		}
	}

	// === 视觉反馈 ===
	// 闪灯
	HitLight->SetIntensity(5000.f);
	GetWorld()->GetTimerManager().SetTimer(LightTimerHandle, this, &APinballBumper::TurnOffLight, LightFlashDuration, false);

	// 播放音效
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation(), 1.0f);
	}

	// === 特殊处理 ===
	if (BumperType == EBumperType::DropTarget)
	{
		HandleDropTarget();
	}

	UE_LOG(LogTemp, Log, TEXT("Bumper hit! Type: %d, Score: +%d"), (int32)BumperType, ScoreValue);
}

void APinballBumper::TurnOffLight()
{
	HitLight->SetIntensity(0.f);
}

void APinballBumper::HandleDropTarget()
{
	// Drop Target: 击中后隐藏自己
	bIsActive = false;
	BumperMesh->SetVisibility(false);
	BumperCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 一段时间后重新激活
	FTimerHandle ResetTimer;
	GetWorld()->GetTimerManager().SetTimer(ResetTimer, [this]()
	{
		bIsActive = true;
		BumperMesh->SetVisibility(true);
		BumperCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}, 10.f, false);
}
