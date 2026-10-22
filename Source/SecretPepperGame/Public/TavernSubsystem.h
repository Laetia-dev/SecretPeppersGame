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

UENUM(BlueprintType)
enum class ETavernIngredient : uint8
{
	Generic UMETA(DisplayName = "Ingredient"),
	CatFloof UMETA(DisplayName = "Cat Floof"),
	Herb UMETA(DisplayName = "Herb"),
	Ash UMETA(DisplayName = "Ash"),
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

	// Broadcast after an ingredient inventory change, or when ResetRun refreshes it.
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

	UPROPERTY(BlueprintReadOnly, Category = "Tavern|Inventory")
	TMap<ETavernIngredient, int32> Inventory;

	UPROPERTY(EditDefaultsOnly, Category = "Tavern|Tuning")
	int32 VibeMin = -5;

	UPROPERTY(EditDefaultsOnly, Category = "Tavern|Tuning")
	int32 VibeMax = 20;

	UPROPERTY(EditDefaultsOnly, Category = "Tavern|Tuning")
	int32 CleanlinessMin = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Tavern|Tuning")
	int32 CleanlinessMax = 10;

private:
	int32 ClampVibe(int32 InVibe) const;
	int32 ClampCleanliness(int32 InCleanliness) const;
};
