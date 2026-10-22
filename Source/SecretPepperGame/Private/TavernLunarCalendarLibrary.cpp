#include "TavernLunarCalendarLibrary.h"

FTavernLunarDate UTavernLunarCalendarLibrary::GetLunarCalendarDate(int32 DayNr)
{
    // Four days per lunar month, twelve zodiac months per year.
    constexpr int32 DaysPerMonth = 4;
    constexpr int32 MonthsPerYear = 12;
    constexpr int32 DaysPerYear = DaysPerMonth * MonthsPerYear;

    FTavernLunarDate Date;
    Date.DayNr = FMath::Max(1, DayNr);

    // Convert to zero-based indices; Day 1 is New Moon in Aries.
    const int32 DayIndex = Date.DayNr - 1;
    Date.Year = DayIndex / DaysPerYear + 1;
    Date.Month = (DayIndex % DaysPerYear) / DaysPerMonth + 1;
    Date.DayOfMonth = (DayIndex % DaysPerMonth) + 1;

    const int32 SunIndex = Date.Month - 1;
    const int32 PhaseIndex = Date.DayOfMonth - 1;

    // Moon is in the Sun's sign on New Moon, and the opposite sign on Full Moon.
    // The four fantasy phases use offsets of 0, 3, 6, and 9 zodiac signs.
    const int32 MoonIndex = (SunIndex + PhaseIndex * 3) % MonthsPerYear;

    Date.MoonPhase = static_cast<ETavernMoonPhase>(PhaseIndex);
    Date.SunSign = static_cast<ETavernZodiacSign>(SunIndex);
    Date.MoonSign = static_cast<ETavernZodiacSign>(MoonIndex);

    // Zodiac elements repeat in Fire, Earth, Air, Water order:
    // Aries Fire, Taurus Earth, Gemini Air, Cancer Water, etc.
    // Sun is the stable month-long influence; Moon is the daily influence.
    Date.SunElement = static_cast<ETavernLunarElement>(SunIndex % 4);
    Date.MoonElement = static_cast<ETavernLunarElement>(MoonIndex % 4);
    return Date;
}
