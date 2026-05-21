// PinballFlipper.cpp

#include "PinballFlipper.h"
#include "PinballBall.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

APinballFlipper::APinballFlipper()
{
	PrimaryActorTick.bCanEverTick = true;

	// 碰撞盒
	FlipperCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("FlipperCollision"));
	FlipperCollision->SetBoxExtent(FVector(40.f, 5.f, 5.f)); // 长条形
	FlipperCollision->SetCollisionProfileName(TEXT("PhysicsActor"));
	FlipperCollision->SetSimulatePhysics(false); // 使用Kinematic运动
	FlipperCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	RootComponent = FlipperCollision;

	// 网格
	FlipperMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlipperMesh"));
	FlipperMesh->SetupAttachment(RootComponent);
	FlipperMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 使用默认盒体 Mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (BoxMesh.Succeeded())
	{
		FlipperMesh->SetStaticMesh(BoxMesh.Object);
		FlipperMesh->SetWorldScale3D(FVector(0.8f, 0.1f, 0.1f)); // 扁平长条
	}
}

void APinballFlipper::BeginPlay()
{
	Super::BeginPlay();

	// 初始化角度
	CurrentAngle = DownAngle;
	TargetAngle = DownAngle;

	// 根据左右侧调整初始旋转
	FRotator InitialRotation = GetActorRotation();
	if (FlipperSide == EFlipperSide::Right)
	{
		// 右侧挡板镜像角度
		InitialRotation.Roll = -DownAngle;
	}
	else
	{
		InitialRotation.Roll = DownAngle;
	}
	SetActorRotation(InitialRotation);
}

void APinballFlipper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateRotation(DeltaTime);
}

void APinballFlipper::Activate()
{
	bIsActivated = true;
	TargetAngle = UpAngle;
}

void APinballFlipper::Deactivate()
{
	bIsActivated = false;
	TargetAngle = DownAngle;
}

void APinballFlipper::UpdateRotation(float DeltaTime)
{
	if (FMath::IsNearlyEqual(CurrentAngle, TargetAngle, 0.5f))
	{
		CurrentAngle = TargetAngle;
		return;
	}

	// 确定旋转速度
	float Speed = bIsActivated ? FlipUpSpeed : FlipDownSpeed;

	// 根据左右侧调整方向
	float Direction = (TargetAngle > CurrentAngle) ? 1.f : -1.f;

	// 插值旋转
	float DeltaAngle = Speed * DeltaTime * Direction;
	CurrentAngle += DeltaAngle;

	// 钳制到目标
	if (Direction > 0.f)
	{
		CurrentAngle = FMath::Min(CurrentAngle, TargetAngle);
	}
	else
	{
		CurrentAngle = FMath::Max(CurrentAngle, TargetAngle);
	}

	// 应用旋转
	FRotator NewRotation = GetActorRotation();
	if (FlipperSide == EFlipperSide::Right)
	{
		NewRotation.Roll = -CurrentAngle; // 右侧镜像
	}
	else
	{
		NewRotation.Roll = CurrentAngle;
	}
	SetActorRotation(NewRotation);

	// 如果正在快速抬起且碰到球，施加额外力
	if (bIsActivated && FMath::Abs(DeltaAngle) > 5.f)
	{
		// 通过 Sweep 检测是否有球在附近
		TArray<FHitResult> HitResults;
		FVector Start = GetActorLocation();
		FVector End = Start + GetActorForwardVector() * 50.f;
		FCollisionShape Shape = FCollisionShape::MakeBox(FVector(40.f, 5.f, 5.f));

		if (GetWorld()->SweepMultiByChannel(HitResults, Start, End, GetActorQuat(), ECC_PhysicsBody, Shape))
		{
			for (const FHitResult& Hit : HitResults)
			{
				APinballBall* Ball = Cast<APinballBall>(Hit.GetActor());
				if (Ball)
				{
					// 计算击球方向 - 向上和远离挡板
					FVector FlipDirection = (FVector::UpVector + GetActorForwardVector()).GetSafeNormal();
					Ball->AddImpulse(FlipDirection * FlipForce);
				}
			}
		}
	}
}
