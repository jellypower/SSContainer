#pragma once
#include "SSEngineDefault/SSEngineInlineSettings.h"
#include "SSEngineDefault/SSEngineDefault.h"


constexpr utf16 HASHER_EMPTY[] = L"EMPTY";

namespace SS {
	class SHasherW;

	class SHashPoolNode
	{
		friend class SHasherW;
	private:
		utf16* _str = nullptr;
		uint32 _strLen = 0;
		SHashPoolNode* _next = nullptr;

	public:
		SHashPoolNode();
		SHashPoolNode(const utf16* str, uint32 inStrLen);
	};
}

namespace SS {

	// String Hasher -> �̸� �ؽ̵� String���� ���Ͽ� ��Ʈ�� �� ȿ���� �����ִ� Ŭ����
	class SHasherW
	{
	public:
		static const SHasherW Empty;
	private:
		static SHashPoolNode* g_SHasherPool;
		static uint32 g_sHasherPoolCnt;

	private:

		union {
			struct {
				uint32 _hashH; // �ؽ� ���� 32��Ʈ
				uint32 _hashL; // �ؽ� ���� 32��Ʈ
			};
			uint64 _hashX; // �ؽ� 64��Ʈ ��ü��
		};


	public:
		SHasherW();
		SHasherW(const char* inStr);
		SHasherW(const utf16* str);
		SHasherW(const SS::SHasherW& rhs);
		SHasherW& operator=(SHasherW rhs);
		bool operator==(SHasherW rhs) const;

		bool IsEmpty() const;
		const utf16* C_Str(uint32* const outStrLen = nullptr) const;
		uint64 GetDirectValue() const { return _hashX; }

	private:
		static void InitHashPool(uint32 hashPoolSize = SHASHER_DEFAULT_POOL_SIZE);
	public:
		static void ClearHashPool();
	};

};