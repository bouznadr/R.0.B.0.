// Fill out your copyright notice in the Description page of Project Settings.

#include "QuestManagerComponent.h"
#include "Engine/DataTable.h"

// Sets default values for this component's properties
UQuestManagerComponent::UQuestManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UQuestManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeQuests();
}

// Charge toutes les quêtes de la DataTable
void UQuestManagerComponent::InitializeQuests()
{
    if (!QuestDataTable) return;

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

// Démarre une quête si les prérequis sont remplis
bool UQuestManagerComponent::StartQuest(FName QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: QuestID '%s' not found."), *QuestID.ToString());
        return false;
    }

    if (ActiveQuests.Contains(QuestID) || CompletedQuests.Contains(QuestID))
    {
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

// Complète une quête active
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

    UnlockLinkedQuests(Quest);

    return true;
}

// Vérifie si les prérequis d'une quête sont remplis
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

// Débloque les quêtes liées après la complétion d'une quête
void UQuestManagerComponent::UnlockLinkedQuests(const FS_Quest& Quest)
{
    for (const FName& LinkedID : Quest.LinkedQuestIDs)
    {
        if (!AllQuests.Contains(LinkedID)) continue;

        const FS_Quest& LinkedQuest = AllQuests[LinkedID];
        if (AreRequirementsMet(LinkedQuest))
        {
            UE_LOG(LogTemp, Log, TEXT("Quest unlocked: %s"), *LinkedQuest.DisplayName.ToString());
            StartQuest(LinkedID);
        }
    }
}

// Vérifie si une quête est complétée
bool UQuestManagerComponent::IsQuestCompleted(FName QuestID) const
{
    return CompletedQuests.Contains(QuestID);
}

// Vérifie si une quête est active
bool UQuestManagerComponent::IsQuestActive(FName QuestID) const
{
    return ActiveQuests.Contains(QuestID);
}