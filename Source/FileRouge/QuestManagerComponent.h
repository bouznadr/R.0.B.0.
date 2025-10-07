// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "S_Quest.h"
#include "QuestManagerComponent.generated.h"

// ----- Délégations Blueprint -----
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestEvent, const FS_Quest&, QuestData);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FILEROUGE_API UQuestManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UQuestManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// ----- Données internes -----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	UDataTable* QuestDataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TMap<FName, FS_Quest> AllQuests;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TSet<FName> ActiveQuests;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TSet<FName> CompletedQuests;

public:
    // ----- Événements Blueprint -----
    UPROPERTY(BlueprintAssignable, Category="Quest")
    FOnQuestEvent OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category="Quest")
    FOnQuestEvent OnQuestCompleted;

    // ----- Fonctions principales -----
    UFUNCTION(BlueprintCallable, Category="Quest")
    void InitializeQuests();

    UFUNCTION(BlueprintCallable, Category="Quest")
    bool StartQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category="Quest")
    bool CompleteQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category="Quest")
    bool IsQuestCompleted(FName QuestID) const;

    UFUNCTION(BlueprintCallable, Category="Quest")
    bool IsQuestActive(FName QuestID) const;

private:
    bool AreRequirementsMet(const FS_Quest& Quest) const;
    void UnlockLinkedQuests(const FS_Quest& Quest);
};
