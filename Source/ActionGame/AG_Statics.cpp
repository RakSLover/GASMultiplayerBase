
#include "AG_Statics.h"

const UItemStaticData* UAG_Statics::GetItemStaticData(TSubclassOf<UItemStaticData> ItemDataClass)
{

	if (IsValid(ItemDataClass))
	{
		//вернем дефолтный объект по умолчанию, посстоянный
		return GetDefault<UItemStaticData>(ItemDataClass);
	}

	return nullptr;
}
