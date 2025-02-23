#include "BasicHashFunctionCollection.h"

int32 HashValue(int32 inValue)
{
	return inValue;
}

int32 HashValue(int64 inValue)
{
	union {
		struct {
			int32 valueH; // �ؽ� ���� 32��Ʈ
			int32 valueL; // �ؽ� ���� 32��Ʈ
		};
		int64 valueX; // �ؽ� 64��Ʈ ��ü��
	};

	valueX = inValue;

	return valueH ^ valueL;
}

int32 HashValue(uint64 inValue)
{
	union {
		struct {
			int32 valueH; // �ؽ� ���� 32��Ʈ
			int32 valueL; // �ؽ� ���� 32��Ʈ
		};
		uint64 valueX; // �ؽ� 64��Ʈ ��ü��
	};

	valueX = inValue;

	return valueH ^ valueL;
}

int32 HashValue(const SS::SHasherW& inValue)
{
	int32 HashedValue = HashValue(inValue.GetDirectValue());

	return HashedValue;
}
