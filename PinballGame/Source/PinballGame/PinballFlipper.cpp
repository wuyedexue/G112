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

	// 记录旋转前的角度
	float PreviousAngle = CurrentAngle;

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

	// 计算新的旋转目标
	FRotator NewRotation = GetActorRotation();
	if (FlipperSide == EFlipperSide::Right)
	{
		NewRotation.Roll = -CurrentAngle; // 右侧镜像
	}
	else
	{
		NewRotation.Roll = CurrentAngle;
	}

	// 使用Sweep方式设置旋转，确保能推动物理对象
	FHitResult SweepHit;
	FlipperCollision->MoveComponent(
		FVector::ZeroVector,
		NewRotation,
		true,  // bSweep = true，检测并推动物体
		&SweepHit
	);

	// 如果Sweep命中了球并且正在快速抬起，施加额外冲量
	if (bIsActivated && SweepHit.bBlockingHit)
	{
		APinballBall* Ball = Cast<APinballBall>(SweepHit.GetActor());
		if (Ball)
		{
			// 计算基于角速度的击球力度
			float AngularVelocity = FMath::Abs(CurrentAngle - PreviousAngle) / DeltaTime;
			float ImpulseStrength = FMath::Clamp(AngularVelocity / FlipUpSpeed, 0.3f, 1.0f) * FlipForce;

			FVector FlipDirection = (Ball->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			FlipDirection.Z = FMath::Max(FlipDirection.Z, 0.3f);
			FlipDirection.Normalize();

			Ball->AddImpulse(FlipDirection * ImpulseStrength);
		}
	}
}
