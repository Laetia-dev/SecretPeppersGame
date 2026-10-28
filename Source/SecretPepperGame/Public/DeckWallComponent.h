#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeckWallComponent.generated.h"

class UDataTable;
class UTavernLayoutSubsystem;
class UTavernSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDeckWallChanged);

// Attach to an actor in the permanent tavern, not a temporary generated room.
// The component validates wall selections; the GameInstance subsystem stores them.
UCLASS(ClassGroup=(Tavern), meta=(BlueprintSpawnableComponent))
class SECRETPEPPERGAME_API UDeckWallComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDeckWallComponent();

	// Set to /Game/_GameContent/Blueprints/Systems/CardWalls/DT_MajorArcana
	// in the owning Blueprint's Details panel.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tavern|Deck Wall")
	TObjectPtr<UDataTable> MajorArcanaTable = nullptr;

	// The designer may change the slot count without changing the C++ code.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tavern|Deck Wall", meta=(ClampMin="1", UIMin="1"))
	int32 SlotCount = 22;

	// These are defined card rows, not necessarily cards unlocked by the player.
	UFUNCTION(BlueprintPure, Category="Tavern|Deck Wall")
	TArray<FName> GetDefinedMajorArcanaIds() const;

	UFUNCTION(BlueprintPure, Category="Tavern|Deck Wall")
	bool IsValidSlot(int32 SlotIndex) const;

	// Returning false means the slot, card ID, or table configuration is invalid.
	UFUNCTION(BlueprintCallable, Category="Tavern|Deck Wall")
	bool PlaceMajorArcanaInSlot(int32 SlotIndex, FName CardId);

	// Inventory-to-wall: consumes exactly one item from the given grid slot.
	// Fails without consuming anything if the wall slot is occupied or the card is invalid.
	UFUNCTION(BlueprintCallable, Category="Tavern|Deck Wall|Inventory")
	bool PlaceInventoryCardInWallSlot(int32 FromInventorySlot, int32 ToWallSlot);

	// Wall-to-inventory: returns one card copy; fails if inventory has no space.
	UFUNCTION(BlueprintCallable, Category="Tavern|Deck Wall|Inventory")
	bool ReturnWallCardToInventory(int32 WallSlot);

	// For dragging a card already on the wall; swap with the destination if needed.
	UFUNCTION(BlueprintCallable, Category="Tavern|Deck Wall")
	bool MoveWallCardBetweenSlots(int32 FromWallSlot, int32 ToWallSlot);

	UFUNCTION(BlueprintCallable, Category="Tavern|Deck Wall")
	bool RemoveCardFromWallSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category="Tavern|Deck Wall")
	FName GetCardInWallSlot(int32 SlotIndex) const;

	// Returns one entry per slot in ascending slot order, NAME_None for empty slots.
	UFUNCTION(BlueprintPure, Category="Tavern|Deck Wall")
	TArray<FName> GetWallCardsInSlotOrder() const;

	// Fires when card placement changes, including changes made from another wall UI.
	UPROPERTY(BlueprintAssignable, Category="Tavern|Deck Wall|Events")
	FDeckWallChanged OnWallChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTavernLayoutSubsystem> LayoutSubsystem = nullptr;

	UFUNCTION()
	void HandleDeckChanged();
};
