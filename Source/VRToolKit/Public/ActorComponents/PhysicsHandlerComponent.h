// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Online/PhysicsReplicationComponent.h"
#include "Utility/PIDControllers.h"
#include "Utility/PhysicsSprings.h"
#include "PhysicsHandlerComponentAsync.h"
#include "PhysicsHandlerComponent.generated.h"

class AController;

/* Who has control over the final say over this physics object
* @None means that all clients will their own say over the object and wont be sync through networking at all
* @Client authority will only work when the component is also owned by a client
*/
UENUM(BlueprintType)
enum EAuthorityType
{
	EAT_Server	UMETA(MetaTag1="Server Authority"),
	EAT_Client	UMETA(MetaTag1="Client Authority"),
	EAT_None	UMETA(MetaTag1="No Authority")
};

UENUM(BlueprintType)
enum ETargetType
{
	ETT_NoTarget					UMETA(MetaTag1="NoTarget"),
	ETT_SetLocationAndRotation		UMETA(MetaTag1="SetVector"),
	ETT_SetObject					UMETA(MetaTag1="SetObject"),
};

/*
* Tick function used to process input data to the async physics thread before simulation
*/
USTRUCT()
struct FPhysicsHandlerPrePhysicsTickFunction : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

	/*class that is the target of the tick*/
	class UPhysicsHandlerComponent* Target;

	/**
	 * Abstract function actually execute the tick.
	 * @param DeltaTime - frame time to advance, in seconds
	 * @param TickType - kind of tick for this frame
	 * @param CurrentThread - thread we are executing on, useful to pass along as new tasks are created
	 * @param MyCompletionGraphEvent - completion event for this task. Useful for holding the completion of this task until certain child tasks are complete.
	 **/
	virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;

	/** Abstract function to describe this tick. Used to print messages about illegal cycles in the dependency graph **/
	virtual FString DiagnosticMessage() override;
	/** Function used to describe this tick for active tick reporting. **/
	virtual FName DiagnosticContext(bool bDetailed) override;
};

template<>
struct TStructOpsTypeTraits<FPhysicsHandlerPrePhysicsTickFunction> : public TStructOpsTypeTraitsBase2<FPhysicsHandlerPrePhysicsTickFunction>
{
	enum
	{
		WithCopy = false
	};
};

USTRUCT(BlueprintType)
struct FPhysicsNetSnapShot
{
	GENERATED_BODY()

	UPROPERTY()
		FVector _Location = FVector::ZeroVector;

	UPROPERTY()
		FQuat _Rotation = FQuat(1);

	UPROPERTY()
		FVector _LinearVelocity = FVector::ZeroVector;

	UPROPERTY()
		FVector _AngularVelocity = FVector::ZeroVector;

	/*bool NetSerialize(FArchive& AR, class UPackageMap* Map, bool& bOutSuccess)
	{

	}*/
};

USTRUCT()
struct FPhysicsHandlerPostPhysicsTickFunction : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

		/*class that is the target of the tick*/
		class UPhysicsHandlerComponent* Target;

	/**
	 * Abstract function actually execute the tick.
	 * @param DeltaTime - frame time to advance, in seconds
	 * @param TickType - kind of tick for this frame
	 * @param CurrentThread - thread we are executing on, useful to pass along as new tasks are created
	 * @param MyCompletionGraphEvent - completion event for this task. Useful for holding the completion of this task until certain child tasks are complete.
	 **/
	virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;

	/** Abstract function to describe this tick. Used to print messages about illegal cycles in the dependency graph **/
	virtual FString DiagnosticMessage() override;
	/** Function used to describe this tick for active tick reporting. **/
	virtual FName DiagnosticContext(bool bDetailed) override;
};

template<>
struct TStructOpsTypeTraits<FPhysicsHandlerPostPhysicsTickFunction> : public TStructOpsTypeTraitsBase2<FPhysicsHandlerPostPhysicsTickFunction>
{
	enum
	{
		WithCopy = false
	};
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTOOLKIT_API UPhysicsHandlerComponent : public UActorComponent /*public UPhysicsReplicationComponent*/
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPhysicsHandlerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual void RegisterComponentTickFunctions(bool bRegister) override;

	/** Pre-physics tick function for this character */
	struct FPhysicsHandlerPrePhysicsTickFunction PrePhysicsTickFunction;

	/** Tick function called before physics */
	virtual void PrePhysicsTickComponent(float DeltaTime, FPhysicsHandlerPrePhysicsTickFunction& ThisTickFunction);

	/** Post-physics tick function for this character */
	UPROPERTY()
		struct FPhysicsHandlerPostPhysicsTickFunction PostPhysicsTickFunction;

	/** Tick function called after physics (sync scene) has finished simulation, before cloth */
	virtual void PostPhysicsTickComponent(float DeltaTime, FPhysicsHandlerPostPhysicsTickFunction& ThisTickFunction);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Set the authority type for this physics component only applys for networked games and when matching a target */
	UFUNCTION(BlueprintCallable)
	void SetMatchTargetAuthorityType(EAuthorityType MatchTargetAuthorityType);

	/* Sets the target type we want to try and match, or set it to none to have the default physics */
	UFUNCTION(BlueprintCallable)
	void SetTargetType(ETargetType TargetType);

	/* if match target is true then this is the object we will be trying to match */
	UFUNCTION(BlueprintCallable)
	void SetPhysicsObject(UPrimitiveComponent* PComp);

	/* if match target is true then this is the object we will be trying to match */
	UFUNCTION(BlueprintCallable)
	void SetTargetObject(USceneComponent* Comp);

	/* Set the target location we want to goto */
	UFUNCTION(BlueprintCallable)
	void SetTargetLocation(FVector Location);

	/* Set the target location we want to goto */
	UFUNCTION(BlueprintCallable)
	void SetTargetRotation(FQuat Rotation);

	/* Set the target location and rotation we want to goto */
	UFUNCTION(BlueprintCallable)
	void SetTargetLocationAndRotation(FVector Location, FQuat Rotation);

	/* Get the location and rotation from the Transform and use it to set the target location and rotation we want to goto */
	UFUNCTION(BlueprintCallable)
	void SetTargetTransform(FTransform Transform);

	/* Set the Player Controller that owns this object */
	UFUNCTION(BlueprintCallable)
	void SetPlayerControllerOwner(AController* PC);

	/* wether to enable or disable this component */
	UFUNCTION(BlueprintCallable)
	void EnableComponent(bool bEnabled);

	//set an offset from the target location that we want to use
	UFUNCTION(BlueprintCallable)
	void SetTargetLocationOffset(FVector Offset);

	//set an offset from the target rotation that we want to use
	UFUNCTION(BlueprintCallable)
	void SetTargetRotationOffset(FQuat Offset);

	//set the desired velocity that we want to match
	UFUNCTION(BlueprintCallable)
	void SetDesiredVelocity(FVector DesiredVel) { _DesiredVelocity = DesiredVel; }
protected:
	/* Who has default authority over this object when we are matching a target
	* @Only applies to match target functions
	* @When using the defualt physics system, the server has full control to sync up objects properly
	*/
	UPROPERTY(EditAnywhere, Category = "Replication Settings")
		TEnumAsByte<EAuthorityType> _MatchTargetAuthorityType;

	UPROPERTY(EditAnywhere, Category = "Replication Settings")
		TEnumAsByte<ETargetType> _MatchTargetType = ETargetType::ETT_NoTarget;

	UPROPERTY(EditAnywhere, Category = "PhysicsTuning")
		bool _bHardSetLinearVelocity = false;

	UPROPERTY(EditAnywhere, Category = "PhysicsTuning")
		FPIDVectorController _LocPD;

	UPROPERTY(EditAnywhere, Category = "PhysicsTuning")
		bool _bHardSetAngularVelocity = false;

	UPROPERTY(EditAnywhere, Category = "PhysicsTuning")
		FQuatPDController _RotPD;

	
	/* should we match the specified target */
	bool _bMatchTarget = false;

//async callback setup
private:
	PhysicsHandlerComponentAsync* _AsyncCallback;

	void RegisterAsyncCallback();

	bool IsAsyncCallbackRegistered() { return _AsyncCallback != nullptr; }

	void BuildAsyncInput();

	void ProccessAsyncOuput();

public:
	void AsyncTick(float DeltaTime);

private:
	/*void UpdateToNextPhysicsFrame();

	void PhysicsTick_DefaultPhysics(float SubsetDeltaTime);

	void PhysicsTick_MatchTarget(float SubsetDeltaTime, FBodyInstance* BodyInstance);

	FCalculateCustomPhysics _OnCalculateCustomPhysics;

	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);*/
	
	UPROPERTY()
	USceneComponent* _TargetObject = NULL;
	
	UPROPERTY()
	UPrimitiveComponent* _PhysicsObject = NULL;

	UPROPERTY()
	AController* _PCOwner;

	FTransform _TransformToMatch = FTransform();

	bool _bComponentEnabled = true;

	//FVector _Known_Goto_Location;
	//FQuat _Known_Goto_Rotation;

	//float _DefaultTickTimer = 1;
	//float _TickTimer = 1;

	FVector _TargetLocationOffset = FVector::ZeroVector;

	FQuat _TargetRotationOffset = FQuat(FVector::ForwardVector, 0);

	FTransform GetSubstepComponentWorldTransform(const USceneComponent* SceneComp, FBodyInstance* BI);
	FTransform GetBoneRefTransformInComponentSpace(const USkeletalMeshComponent* SK, const FName& BoneName);
	void GetBoneTrasnformRecursive(FTransform& Transform, const USkeletalMeshComponent* SK, const FName& BoneName);

	//float _CheckTimer = 1;
	//int _StepsRan = 0;

	//FVector _LocLastStep = FVector::ZeroVector;
	//FVector _TargetLastLoc = FVector::ZeroVector;
	//Async Tick test
	//float _AsyncTimer = 1;
	//int _AsyncTicksRan = 0;

	FVector _DesiredVelocity = FVector::ZeroVector;

	//async apply physics
	void APT_AddForce(UPrimitiveComponent* Comp, FVector Force, bool bHardSetVelocity);
	void APT_AddTorque(UPrimitiveComponent* Comp, FVector Torque, bool bHardSetVelocity);
	void APT_MatchTarget(float DeltaTime);
	void APT_DefaultPhysics(float DeltaTime);


/*Networking Functions */
public:
	UFUNCTION(NetMulticast, Reliable)
	void NF_NetMulticast_SendPhysicsSnapShot(const FPhysicsNetSnapShot& SnapShot);

	UFUNCTION(Server, Reliable)
	void NF_Server_SendPhysicsSnapShot(const FPhysicsNetSnapShot& SnapShot);

	float _TimeBeforeSending = 0;
};
