#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TavernLayoutSubsystem.generated.h"

class AActor;

// The deck is persistent within a run. The physical layout is temporary.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTavernDeckChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTavernLayoutGenerationRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTavernTemporaryRoomsCleared);

UCLASS(BlueprintType)
class SECRETPEPPERGAME_API UTavernLayoutSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// CardId is a Data Table row name, not a reference to a spawned room.
	// Slot indices belong to the wall UI, so no physical position is fixed here.
	UFUNCTION(BlueprintCallable, Category = "Tavern|Deck")
	bool PlaceCardInSlot(int32 SlotIndex, FName CardId);

	// Place an additional physical copy without moving an existing matching Arcana ID.
	// Safe only for an empty wall slot; intended for inventory-to-wall transfers.
	UFUNCTION(BlueprintCallable, Category = "Tavern|Deck")
	bool PlaceCardCopyInEmptySlot(int32 SlotIndex, FName CardId);

	// Move a specific wall-slot instance, not a card ID (duplicates are supported).
	// Swaps occupied slots rather than discarding either card.
	UFUNCTION(BlueprintCallable, Category = "Tavern|Deck")
	bool MoveCardBetweenSlots(int32 FromSlot, int32 ToSlot);

	UFUNCTION(BlueprintCallable, Category = "Tavern|Deck")
	bool RemoveCardFromSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Tavern|Deck")
	FName GetCardInSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Tavern|Deck")
	TMap<int32, FName> GetPlacedCards() const;

	// Called when it is time to manifest the next layout, normally at night.
	// Blueprint owns actual room selection, location, and door connections.
	UFUNCTION(BlueprintCallable, Category = "Tavern|Layout")
	void RequestLayoutGeneration();

	// Register each temporary room or independently spawned door/actor to be
	// removed when the player sleeps. Do not register permanent tavern actors.
	UFUNCTION(BlueprintCallable, Category = "Tavern|Layout")
	bool RegisterTemporaryActor(AActor* Actor);

	// Call only after the player has returned to a permanent, safe location.
	// This never clears the deck or advances the day.
	UFUNCTION(BlueprintCallable, Category = "Tavern|Layout")
	void ClearTemporaryRoomsForSleep();

	UFUNCTION(BlueprintPure, Category = "Tavern|Layout")
	int32 GetRegisteredTemporaryActorCount() const;

	UPROPERTY(BlueprintAssignable, Category = "Tavern|Deck|Events")
	FTavernDeckChanged OnDeckChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tavern|Layout|Events")
	FTavernLayoutGenerationRequested OnLayoutGenerationRequested;

	UPROPERTY(BlueprintAssignable, Category = "Tavern|Layout|Events")
	FTavernTemporaryRoomsCleared OnTemporaryRoomsCleared;

private:
	// Slot-to-card mapping stays unchanged when temporary rooms are cleared.
	UPROPERTY(Transient)
	TMap<int32, FName> PlacedCards;

	// Weak references do not keep level actors alive across level changes.
	TArray<TWeakObjectPtr<AActor>> TemporaryActors;
};
