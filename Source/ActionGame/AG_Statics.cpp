
#include "AG_Statics.h"

const UItemStaticData* UAG_Statics::GetItemStaticData(TSubclassOf<UItemStaticData> ItemDataClass)
{

	if (IsValid(ItemDataClass))
	{
		//������ ��������� ������ �� ���������, �����������
		return GetDefault<UItemStaticData>(ItemDataClass);
	}

	return nullptr;
}
