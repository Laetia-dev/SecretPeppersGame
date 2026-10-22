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
    Date.DayOfYear = DayIndex % DaysPerYear + 1;
    Date.Month = (Date.DayOfYear - 1) / DaysPerMonth + 1;
    Date.DayOfMonth = (DayIndex % DaysPerMonth) + 1;

    const int32 SunIndex = Date.Month - 1;

    // Each season lasts three zodiac months, beginning with Aries,
    // Cancer, Libra, or Capricorn respectively.
    Date.Season = static_cast<ETavernSeason>(SunIndex / 3);

    // Solar turning points: days 1, 13, 25, 37.
    // Cross-quarter festivals: days 7, 19, 31, 43.
    // Even six-day spacing is a deliberate compressed-calendar convention.
    switch (Date.DayOfYear)
    {
    case 1:  Date.Festival = ETavernYearFestival::Ostara; break;
    case 7:  Date.Festival = ETavernYearFestival::Beltane; break;
    case 13: Date.Festival = ETavernYearFestival::Litha; break;
    case 19: Date.Festival = ETavernYearFestival::Lughnasadh; break;
    case 25: Date.Festival = ETavernYearFestival::Mabon; break;
    case 31: Date.Festival = ETavernYearFestival::Samhain; break;
    case 37: Date.Festival = ETavernYearFestival::Yule; break;
    case 43: Date.Festival = ETavernYearFestival::Imbolc; break;
    default: Date.Festival = ETavernYearFestival::None; break;
    }
    Date.bIsFestival = Date.Festival != ETavernYearFestival::None;
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
