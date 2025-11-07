// Fill out your copyright notice in the Description page of Project Settings.

#include "QuestManagerComponent.h"
#include "Engine/DataTable.h"

// Sets default values for this component's properties
UQuestManagerComponent::UQuestManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UQuestManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeQuests();
}

void UQuestManagerComponent::InitializeQuests()
{
    if (!QuestDataTable) return;

    // Charge toutes les quêtes de la DataTable
    static const FString Context(TEXT("QuestManager_Init"));
    TArray<FName> RowNames = QuestDataTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        if (const FS_Quest* Row = QuestDataTable->FindRow<FS_Quest>(RowName, Context))
        {
            AllQuests.Add(Row->QuestID, *Row);
        }
    }
}

bool UQuestManagerComponent::StartQuest(FName QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: QuestID '%s' not found."), *QuestID.ToString());
        return false;
    }

    if (ActiveQuests.Contains(QuestID) || CompletedQuests.Contains(QuestID))
    {
        // Déjà active ou terminée
        return false;
    }

    const FS_Quest& Quest = AllQuests[QuestID];

    // Vérifie si les prérequis sont remplis
    if (!AreRequirementsMet(Quest))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Requirements not met for quest '%s'."), *QuestID.ToString());
        return false;
    }

    ActiveQuests.Add(QuestID);
    UE_LOG(LogTemp, Log, TEXT("Quest started: %s"), *Quest.DisplayName.ToString());
    OnQuestStarted.Broadcast(Quest);

    return true;
}

bool UQuestManagerComponent::CompleteQuest(FName QuestID)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot complete '%s' (not active)."), *QuestID.ToString());
        return false;
    }

    ActiveQuests.Remove(QuestID);
    CompletedQuests.Add(QuestID);

    const FS_Quest& Quest = AllQuests[QuestID];
    UE_LOG(LogTemp, Log, TEXT("Quest completed: %s"), *Quest.DisplayName.ToString());
    OnQuestCompleted.Broadcast(Quest);

    // Débloque les quêtes liées
    UnlockLinkedQuests(Quest);

    return true;
}

bool UQuestManagerComponent::AreRequirementsMet(const FS_Quest& Quest) const
{
    for (const FName& ReqID : Quest.RequiredQuestIDs)
    {
        if (!CompletedQuests.Contains(ReqID))
        {
            return false;
        }
    }
    return true;
}

void UQuestManagerComponent::UnlockLinkedQuests(const FS_Quest& Quest)
{
    for (const FName& LinkedID : Quest.LinkedQuestIDs)
    {
        if (!AllQuests.Contains(LinkedID)) continue;

        const FS_Quest& LinkedQuest = AllQuests[LinkedID];
        if (AreRequirementsMet(LinkedQuest))
        {
            UE_LOG(LogTemp, Log, TEXT("Quest unlocked: %s"), *LinkedQuest.DisplayName.ToString());
            StartQuest(LinkedID); // Démarrage auto des quêtes narratives suivantes
        }
    }
}

bool UQuestManagerComponent::IsQuestCompleted(FName QuestID) const
{
    return CompletedQuests.Contains(QuestID);
}

bool UQuestManagerComponent::IsQuestActive(FName QuestID) const
{
    return ActiveQuests.Contains(QuestID);
}