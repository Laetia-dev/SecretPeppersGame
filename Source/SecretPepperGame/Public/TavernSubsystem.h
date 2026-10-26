// TavernSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TavernLunarCalendarLibrary.h"
#include "TavernSubsystem.generated.h"

UENUM(BlueprintType)
enum class ETavernTimeOfDay : uint8
{
	Day UMETA(DisplayName = "Day"),
	Night UMETA(DisplayName = "Night"),
};

// The day's special sky alignment (if any).
UENUM(BlueprintType)
enum class ETavernCelestialEvent : uint8
{
	None UMETA(DisplayName = "None"),
	SolarEclipse UMETA(DisplayName = "Solar Eclipse"),
	MercuryTransit UMETA(DisplayName = "Mercury Transit"),
	VenusTransit UMETA(DisplayName = "Venus Transit")
};

// One random plan per 48-day game year. A day value of 0 means not scheduled.
// Dates are stored, not rerolled by reading the calendar.
USTRUCT(BlueprintType)
struct FTavernCelestialYearPlan
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Tavern|Celestial")
	int32 Year = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Tavern|Celestial")
	int32 SolarEclipseDay = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tavern|Celestial")
	int32 MercuryTransitDay = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tavern|Celestial")
	int32 VenusTransitDay = 0;
};

UENUM(BlueprintType)
enum class ETavernIngredient : uint8
{
	Generic UMETA(DisplayName = "Ingredient"),
	CatFloof UMETA(DisplayName = "Cat Floof"),
	Herb UMETA(DisplayName = "Herb"),
	Ash UMETA(DisplayName = "Ash"),
};

// An occupied grid slot refers to an item definition (for example, a DT_Loot row).
// Quantity 0 / ItemID None represents an empty slot.
USTRUCT(BlueprintType)
struct FTavernInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Tavern|Inventory|Grid")
	FName ItemID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Tavern|Inventory|Grid")
	int32 Quantity = 0;

	// Cards can use 1; stackable ingredients can use a larger limit.
	UPROPERTY(BlueprintReadOnly, Category = "Tavern|Inventory|Grid")
	int32 MaxStackSize = 99;
};

USTRUCT(BlueprintType)
struct FTavernState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Tavern")
	ETavernTimeOfDay TimeOfDay = ETavernTimeOfDay::Day;

	UPROPERTY(BlueprintReadOnly, Category = "Tavern")
	int32 DayNr = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Tavern")
	int32 Vibe = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tavern")
	int32 Cleanliness = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tavern")
	int32 Coins = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tavern")
	int32 PreparedFood = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTavernTimeChanged, ETavernTimeOfDay, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTavernVibeChanged, int32, NewVibe);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTavernCleanlinessChanged, int32, NewCleanliness);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTavernDayChanged, int32, NewDayNr);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTavernLunarDateChanged, FTavernLunarDate, NewLunarDate);

// Observers read current ingredient counts through GetIngredientCount.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTavernInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTavernCoinsChanged, int32, NewCoins);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTavernPreparedFoodChanged, int32, NewPreparedFood);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTavernRunReset);

UCLASS(BlueprintType)
class SECRETPEPPERGAME_API UTavernSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Tavern")
	const FTavernState& GetState() const;

	UFUNCTION(BlueprintPure, Category = "Tavern")
	int32 GetDayNr() const;

	// Derived from the same DayNr used by FlipTimeOfDay / AdvanceDay.
	UFUNCTION(BlueprintPure, Category = "Tavern|Calendar")
	FTavernLunarDate GetCurrentLunarDate() const;

	// Prototype only: returns one multiline string for displaying every current
	// calendar property in a single UMG TextBlock. No gameplay side effects.
	UFUNCTION(BlueprintPure, Category = "Tavern|Calendar|Debug")
	FText GetCurrentCalendarDebugText() const;

	// Planned days for this game year; 0 means the event was not rolled.
	UFUNCTION(BlueprintPure, Category = "Tavern|Calendar|Celestial")
	FTavernCelestialYearPlan GetCurrentCelestialYearPlan() const;

	// A scheduled date even at night; for daytime visuals also check TimeOfDay.
	UFUNCTION(BlueprintPure, Category = "Tavern|Calendar|Celestial")
	ETavernCelestialEvent GetTodaysCelestialEvent() const;

	UFUNCTION(BlueprintCallable, Category = "Tavern")
	void SetDayNr(int32 NewDayNr);

	UFUNCTION(BlueprintCallable, Category = "Tavern")
	void AdvanceDay();

	UFUNCTION(BlueprintCallable, Category = "Tavern")
	void SetTimeOfDay(ETavernTimeOfDay NewTime);

	UFUNCTION(BlueprintCallable, Category = "Tavern")
	void FlipTimeOfDay();

	UFUNCTION(BlueprintCallable, Category = "Tavern")
	void SetVibe(int32 NewVibe);

	UFUNCTION(BlueprintCallable, Category = "Tavern")
	void AddVibe(int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "Tavern")
	void SetCleanliness(int32 NewCleanliness);

	UFUNCTION(BlueprintCallable, Category = "Tavern")
	void AddCleanliness(int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "Tavern|Inventory")
	int32 GetIngredientCount(ETavernIngredient Type) const;

	UFUNCTION(BlueprintCallable, Category = "Tavern|Inventory")
	bool HasIngredient(ETavernIngredient Type, int32 Amount) const;

	UFUNCTION(BlueprintCallable, Category = "Tavern|Inventory")
	void AddIngredient(ETavernIngredient Type, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Tavern|Inventory")
	bool ConsumeIngredient(ETavernIngredient Type, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Tavern|Inventory")
	void ClearInventory();

	// Slot-based inventory for the grid UI. The older enum ingredient counter
	// stays available while gameplay is migrated to the slot-based system.
	UFUNCTION(BlueprintPure, Category = "Tavern|Inventory|Grid")
	int32 GetInventorySlotCount() const;

	UFUNCTION(BlueprintPure, Category = "Tavern|Inventory|Grid")
	FTavernInventorySlot GetInventorySlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Tavern|Inventory|Grid")
	TArray<FTavernInventorySlot> GetInventorySlots() const;

	UFUNCTION(BlueprintPure, Category = "Tavern|Inventory|Grid")
	bool HasInventoryItem(FName ItemID, int32 Quantity = 1) const;

	// Returns false without changing inventory if there is insufficient room.
	// Use MaxStackSize = 1 for non-stackable Tarot cards.
	UFUNCTION(BlueprintCallable, Category = "Tavern|Inventory|Grid")
	bool AddInventoryItem(FName ItemID, int32 Quantity = 1, int32 MaxStackSize = 99);

	UFUNCTION(BlueprintCallable, Category = "Tavern|Inventory|Grid")
	bool RemoveInventoryItem(FName ItemID, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Tavern|Inventory|Grid")
	bool RemoveInventoryItemAtSlot(int32 SlotIndex, int32 Quantity = 1);

	// Drag-and-drop: move, swap, or merge two matching stacks.
	UFUNCTION(BlueprintCallable, Category = "Tavern|Inventory|Grid")
	bool MoveInventoryItem(int32 FromSlot, int32 ToSlot);

	UFUNCTION(BlueprintCallable, Category = "Tavern|Food")
	int32 GetPreparedFood() const;

	UFUNCTION(BlueprintCallable, Category = "Tavern|Food")
	void AddPreparedFood(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Tavern|Food")
	bool ConsumePreparedFood(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Tavern|Currency")
	int32 GetCoins() const;

	UFUNCTION(BlueprintCallable, Category = "Tavern|Currency")
	void AddCoins(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Tavern|Currency")
	bool SpendCoins(int32 Amount);

	// This is the one you call from your "Back to Main Menu" button.
	UFUNCTION(BlueprintCallable, Category = "Tavern")
	void ResetRun(bool bResetDayNr = true);

	UPROPERTY(BlueprintAssignable, Category = "Tavern|Events")
	FTavernTimeChanged OnTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tavern|Events")
	FTavernVibeChanged OnVibeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tavern|Events")
	FTavernCleanlinessChanged OnCleanlinessChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tavern|Events")
	FTavernDayChanged OnDayChanged;

	// Fired when DayNr changes, including Night -> Day via FlipTimeOfDay.
	UPROPERTY(BlueprintAssignable, Category = "Tavern|Calendar|Events")
	FTavernLunarDateChanged OnLunarDateChanged;

	// Broadcast after either ingredient counter or grid inventory changes, including ResetRun.
	UPROPERTY(BlueprintAssignable, Category = "Tavern|Events")
	FTavernInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tavern|Events")
	FTavernCoinsChanged OnCoinsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tavern|Events")
	FTavernPreparedFoodChanged OnPreparedFoodChanged;

	// Lets objective managers clear progress when this subsystem starts a new run.
	UPROPERTY(BlueprintAssignable, Category = "Tavern|Events")
	FTavernRunReset OnRunReset;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Tavern")
	FTavernState State;

	// Percent chance of each alignment occurring ONCE in a given game year.
	// Tune for future years. Changing a value does not reroll an existing year.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tavern|Celestial|Chances", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float SolarEclipseChancePercent = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tavern|Celestial|Chances", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float MercuryTransitChancePercent = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tavern|Celestial|Chances", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float VenusTransitChancePercent = 10.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Tavern|Inventory")
	TMap<ETavernIngredient, int32> Inventory;

	// Fixed-size 20-slot grid, maintained by the functions above.
	UPROPERTY(BlueprintReadOnly, Category = "Tavern|Inventory|Grid")
	TArray<FTavernInventorySlot> ItemSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Tavern|Tuning")
	int32 VibeMin = -5;

	UPROPERTY(EditDefaultsOnly, Category = "Tavern|Tuning")
	int32 VibeMax = 20;

	UPROPERTY(EditDefaultsOnly, Category = "Tavern|Tuning")
	int32 CleanlinessMin = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Tavern|Tuning")
	int32 CleanlinessMax = 10;

private:
	// Cache all years visited during this run so day jumps do not reroll past years.
	UPROPERTY(Transient)
	TMap<int32, FTavernCelestialYearPlan> CelestialYearPlans;

	void EnsureCelestialYearPlan(int32 Year);
	int32 ClampVibe(int32 InVibe) const;
	int32 ClampCleanliness(int32 InCleanliness) const;
};
