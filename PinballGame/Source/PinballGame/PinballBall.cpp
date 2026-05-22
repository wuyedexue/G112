// PinballBall.cpp

#include "PinballBall.h"
#include "PinballGameMode.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

APinballBall::APinballBall()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建碰撞球体
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->InitSphereRadius(BallRadius);
	SphereCollision->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereCollision->SetSimulatePhysics(true);
	SphereCollision->SetEnableGravity(true);
	SphereCollision->SetNotifyRigidBodyCollision(true);
	SphereCollision->BodyInstance.bUseCCD = true; // 连续碰撞检测，防止穿墙
	SphereCollision->BodyInstance.SetMassOverride(BallMass);
	RootComponent = SphereCollision;

	// 创建球体网格
	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(RootComponent);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 设置默认球体 Mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		BallMesh->SetStaticMesh(SphereMesh.Object);
		// 默认Sphere半径为50单位，缩放到BallRadius
		BallMesh->SetWorldScale3D(FVector(BallRadius / 50.f));
	}
}

void APinballBall::BeginPlay()
{
	Super::BeginPlay();

	// 设置物理材质
	SphereCollision->SetPhysMaterialOverride(nullptr);
	
	// 绑定碰撞事件
	SphereCollision->OnComponentHit.AddDynamic(this, &APinballBall::OnBallHit);

	// 设置物理属性
	if (SphereCollision->GetBodyInstance())
	{
		SphereCollision->GetBodyInstance()->SetMassOverride(BallMass);
		SphereCollision->SetLinearDamping(0.1f);
		SphereCollision->SetAngularDamping(0.5f);
	}

	// 设置红色材质，确保球清晰可见（用户要求球为红色）
	if (BallMesh && BallMesh->GetMaterial(0))
	{
		UMaterialInstanceDynamic* BallMat = UMaterialInstanceDynamic::Create(
			BallMesh->GetMaterial(0), this);
		if (BallMat)
		{
			// 红色球体 - 在白色地面上高度可见
			BallMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
			BallMesh->SetMaterial(0, BallMat);
		}
	}
}

void APinballBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 只有在球已被发射后才施加倾斜力和检查排水
	if (bHasBeenLaunched)
	{
		// 施加球台倾斜重力（模拟弹球台的倾斜）
		FVector TiltForce = FVector(0.f, -TableTiltGravity * BallMass, 0.f);
		SphereCollision->AddForce(TiltForce);

		// 检查是否掉入排水口
		CheckDrain();

		// 限制速度
		ClampVelocity();
	}
}

void APinballBall::LaunchBall(FVector Force)
{
	bHasBeenLaunched = true;
	SphereCollision->AddImpulse(Force, NAME_None, true);
	UE_LOG(LogTemp, Log, TEXT("Ball launched with force: %s"), *Force.ToString());
}

void APinballBall::AddImpulse(FVector Impulse)
{
	SphereCollision->AddImpulse(Impulse);
}

FVector APinballBall::GetBallVelocity() const
{
	return SphereCollision->GetPhysicsLinearVelocity();
}

float APinballBall::GetBallSpeed() const
{
	return SphereCollision->GetPhysicsLinearVelocity().Size();
}

void APinballBall::OnBallHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 碰撞音效和粒子效果可在这里触发
	// 通知被撞击的对象（如果是 Bumper 等）
	UE_LOG(LogTemp, Verbose, TEXT("Ball hit: %s"), *OtherActor->GetName());
}

void APinballBall::CheckDrain()
{
	// 安全网：只有当Table的DrainTrigger没有检测到时才作为备用
	// 如果已经被标记为draining，不再重复扣命
	if (bIsDraining) return;

	if (GetActorLocation().Z < DrainZThreshold)
	{
		MarkAsDraining();
	}
}

void APinballBall::MarkAsDraining()
{
	if (bIsDraining) return;
	bIsDraining = true;

	// 球掉落 - 通知 GameMode
	APinballGameMode* GameMode = Cast<APinballGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->LoseLife();
	}

	// 销毁球
	Destroy();
}

void APinballBall::ClampVelocity()
{
	FVector Velocity = SphereCollision->GetPhysicsLinearVelocity();
	float Speed = Velocity.Size();

	// 限制最大速度
	if (Speed > MaxSpeed)
	{
		SphereCollision->SetPhysicsLinearVelocity(Velocity.GetSafeNormal() * MaxSpeed);
	}
	// 如果速度太低且球没有静止在发射器上，施加微小力保持运动
	else if (Speed < MinSpeed && Speed > 1.f)
	{
		FVector Boost = Velocity.GetSafeNormal() * MinSpeed * 0.1f;
		SphereCollision->AddForce(Boost);
	}
}
