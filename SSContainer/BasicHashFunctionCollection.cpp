#include "BasicHashFunctionCollection.h"

int32 HashValue(int32 inValue)
{
	return inValue;
}

int32 HashValue(int64 inValue)
{
	union {
		struct {
			int32 valueH; // 해쉬 상위 32비트
			int32 valueL; // 해쉬 하위 32비트
		};
		int64 valueX; // 해쉬 64비트 전체값
	};

	valueX = inValue;

	return valueH ^ valueL;
}

int32 HashValue(uint64 inValue)
{
	union {
		struct {
			int32 valueH; // 해쉬 상위 32비트
			int32 valueL; // 해쉬 하위 32비트
		};
		uint64 valueX; // 해쉬 64비트 전체값
	};

	valueX = inValue;

	return valueH ^ valueL;
}

int32 HashValue(const SS::SHasherW& inValue)
{
	int32 HashedValue = HashValue(inValue.GetDirectValue());

	return HashedValue;
}
