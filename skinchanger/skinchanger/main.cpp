#include "memory.h"

#include <thread>
#include <array>

namespace offset
{
	// Client
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDA746C;
	constexpr ::std::ptrdiff_t dwEntityList = 0x4DC178C;
	constexpr ::std::ptrdiff_t dwClientState = 0x589FCC;

	// Player
	constexpr ::std::ptrdiff_t m_hMyWeapons = 0x2E08;

	// Base attributable 
	constexpr ::std::ptrdiff_t m_flFallbackWear = 0x31E0;
	constexpr ::std::ptrdiff_t m_nFallbackPaintKit = 0x31D8;
	constexpr ::std::ptrdiff_t m_nFallbackSeed = 0X31DC;
	constexpr ::std::ptrdiff_t m_nFallbackStatTrack = 0x31E4;
	constexpr ::std::ptrdiff_t m_iItemDefinitionIndex = 0x2FBA;
	constexpr ::std::ptrdiff_t m_iItemIDHigh = 0x2FD0;
	constexpr ::std::ptrdiff_t m_iEntityQuality = 0x2FBC;
	constexpr ::std::ptrdiff_t m_iAccountID = 0x2FDB;
	constexpr ::std::ptrdiff_t m_OriginalOwnerXuidLow = 0x31D0;

}

// Set skin to apply here
constexpr const int GetWeaponPaint(const short& itemDefinition)
{
	switch (itemDefinition)
	{
	case 1: return 711; // deagle 
	case 4: return 38; // glock
	case 7: return 490; // ak-47
	case 9: return 344; // awp
	case 61: return 653; // usp
	default: return 0;
	}
}

int main()
{
	const auto memory = Memory{ "csgo.exe" };

	// Get our module addresses
	const auto client = memory.GetModuleAddress("client.dll");
	const auto engine = memory.GetModuleAddress("engine.dll");
	
	// Hack loop
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2));

		const auto& localPlayer = memory.Read<std::uintptr_t>(client + offset::dwLocalPlayer);
		const auto& weapons = memory.Read<std::array<unsigned long, 8>>(localPlayer + offset::m_hMyWeapons);

		// Local player weapon iteration
		for (const auto& handle : weapons)
		{
			const auto& weapon = memory.Read<std::uintptr_t>((client + offset::dwEntityList + (handle & 0xFFF) * 0x10) - 0x10);

			// Make sure weapon is valid
			if (!weapon)
				continue;

			// See if we want to apply a skin
			if (const auto paint = GetWeaponPaint(memory.Read<short>(weapon + offset::m_iItemDefinitionIndex)))
			{
				const bool shouldUpdate = memory.Read<std::int32_t>(weapon + offset::m_nFallbackPaintKit) != paint;

				// Force weapon to use fallback values
				memory.Write<std::int32_t>(weapon + offset::m_iItemIDHigh, -1);

				memory.Write<std::int32_t>(weapon + offset::m_nFallbackPaintKit, paint);
				memory.Write<float>(weapon + offset::m_flFallbackWear, 0.1f);

				memory.Write<std::int32_t>(weapon + offset::m_nFallbackSeed, 0);
				memory.Write<std::int32_t>(weapon + offset::m_nFallbackStatTrack, 1337);
				memory.Write<std::int32_t>(weapon + offset::m_iAccountID, memory.Read<std::int32_t>(weapon + offset::m_OriginalOwnerXuidLow));

				if (shouldUpdate)
					memory.Write<std::int32_t>(memory.Read<std::uintptr_t>(engine + offset::dwClientState) + 0x174, -1);
			}
		}
	}
	return 0;
}