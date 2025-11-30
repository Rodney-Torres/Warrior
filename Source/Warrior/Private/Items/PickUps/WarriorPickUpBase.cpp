// Rodney Torres All Rights Reserved


#include "Items/PickUps/WarriorPickUpBase.h"
//For the type USphereComponent
#include "Components/SphereComponent.h"

AWarriorPickUpBase::AWarriorPickUpBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	//We construct the sphere component. We create, name it, set it as the root component, set its radius, and bind the overlap event to a callbackfunction
	PickUpCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickUpCollisionSphere"));
	SetRootComponent(PickUpCollisionSphere);
	PickUpCollisionSphere->InitSphereRadius(50.f);
	PickUpCollisionSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnPickUpCollisionSphereBeginOverlap);
}

void AWarriorPickUpBase::OnPickUpCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}


