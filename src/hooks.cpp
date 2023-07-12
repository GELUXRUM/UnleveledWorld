#include "hooks.h"

namespace HookLineAndSinker
{
	typedef int8_t(AddLeveledObjectSig)(RE::TESLeveledList*, uint16_t, int16_t, int8_t, RE::TESForm*, RE::ContainerItemExtra*);
	REL::Relocation<AddLeveledObjectSig> OriginalFunction;

	int8_t HookedAddLeveledObject(RE::TESLeveledList* a_leveledList, uint16_t a_level, int16_t a_count, int8_t a_chanceNone, RE::TESForm* a_item, RE::ContainerItemExtra* a_containerExta)
	{
		a_level = 1;
		return OriginalFunction(a_leveledList, a_level, a_count, a_chanceNone, a_item, a_containerExta);
	}

	void RegisterHook(F4SE::Trampoline& trampoline)
	{
		REL::Relocation<AddLeveledObjectSig> callLocation{ REL::ID(860553), 0x6C };
		OriginalFunction = trampoline.write_call<5>(callLocation.address(), &HookedAddLeveledObject);
	}
}
