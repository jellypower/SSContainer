#pragma once
#include "SSEngineDefault/SSEngineDefault.h"
#include "SSEngineDefault/SSContainer/SSUtilityContainer.h"
#include "SSEngineDefault/SSContainer/PooledList.h"
#include "BasicHashFunctionCollection.h"

namespace SS
{
	template<typename KeyType, typename ValueType>
	class HashMap
	{
#pragma region HashMapInternal
		template<typename HMNKeyType, typename HMNValueType>
		class HashMapBucket
		{
		public:
			SS::pair<HMNKeyType, HMNValueType> _pair;
			HashMapBucket* _next = nullptr;

		public:
			HashMapBucket(const HMNKeyType& Key, const HMNValueType& Value) :
				_pair(Key, Value), _next(nullptr)
			{

			}

			HashMapBucket(HMNKeyType&& Key, HMNValueType&& Value) :
				_pair(Key, Value), _next(nullptr)
			{
					
			}
		};
		typedef HashMapBucket<KeyType, ValueType> HashMapBucket_INL;

		template<typename HMNHKeyType, typename HMNHValueType>
		class HashMapBucketHead
		{
		public:
			HashMapBucketHead(int32 inIdxInArray, int32 inHashValue) :
				_idxInArray(inIdxInArray), _hashValue(inHashValue)
			{ }

			HashMapBucket_INL* _next = nullptr;
			int32 _idxInArray = INVALID_IDX;
			int32 _hashValue = INVALID_IDX;
		};
		typedef HashMapBucketHead<KeyType, ValueType> HashMapBucketHead_INL;
#pragma endregion

	private:
		SS::PooledList<HashMapBucketHead_INL> _bucketHeadList;
		SS::PooledList<int32> _hashValueToBucketHeadListIdx;
		int32 _hashMapCapacity = 0;
		int32 _cnt = 0;
		// _itemPool은 Key와 Value를 모두 알고있기 때문에 현재 Item이 _hashValueToItemPoolIdx의 어디에 위치했는지 알 수 있다.
		// _hashValueToItemPoolIdx은 HashValue를 기준으로 Item을 찾을 수 있다.

	public:
		explicit HashMap(int32 InHashMapCapacity, int32 InBucketCapacity = 0)
		{
			if(InHashMapCapacity <= 0)
			{
				SS_INTERRUPT();
			}
			_hashMapCapacity = InHashMapCapacity;
			

			if (InBucketCapacity > 0)
			{
				_bucketHeadList.Reserve(InBucketCapacity);
			}

			_hashValueToBucketHeadListIdx.Reserve(_hashMapCapacity);
			_hashValueToBucketHeadListIdx.SetSizeDirectly(_hashMapCapacity);
			for (int32 i = 0; i < _hashMapCapacity; i++)
			{
				_hashValueToBucketHeadListIdx[i] = INVALID_IDX;
			}
		}

		~HashMap()
		{
			Clear();
		}

		int32 GetCnt() const { return _cnt; }

		void Reserve(int32 poolSize)
		{
			_bucketHeadList.Reserve(poolSize);
		}

		void Add(const KeyType& Key, const ValueType& Value)
		{
			const int32 hashValue = HashValue(Key) % _hashMapCapacity;
			int32 itemPoolIdx = _hashValueToBucketHeadListIdx[hashValue];
			if (itemPoolIdx == INVALID_IDX)
			{
				uint32 itemIdx = _bucketHeadList.GetSize();
				_hashValueToBucketHeadListIdx[hashValue] = itemIdx;

				_bucketHeadList.PushBack(HashMapBucketHead_INL(itemIdx, hashValue));
				
				HashMapBucket_INL* newHashMapNode = DBG_NEW HashMapBucket_INL(Key, Value);
				_bucketHeadList[itemIdx]._next = newHashMapNode;
				_cnt++;
			}
			else
			{
				HashMapBucket_INL* hashMapNodeItem = _bucketHeadList[itemPoolIdx]._next;
				while(hashMapNodeItem->_next != nullptr)
				{
					if(hashMapNodeItem->_pair.first == Key)
					{
						hashMapNodeItem->_pair.second = Value;
						return;
					}

					hashMapNodeItem = hashMapNodeItem->_next;
				}

				HashMapBucket_INL* newHashMapNode = DBG_NEW HashMapBucket_INL(Key, Value);
				hashMapNodeItem->_next = newHashMapNode;
				_cnt++;
			}
		}

		void Add(KeyType&& Key, ValueType&& Value)
		{
			const int32 hashValue = HashValue(Key) % _hashMapCapacity;
			int32 itemPoolIdx = _hashValueToBucketHeadListIdx[hashValue];
			if (itemPoolIdx == INVALID_IDX)
			{
				uint32 itemIdx = _bucketHeadList.GetSize();
				_hashValueToBucketHeadListIdx[hashValue] = itemIdx;

				_bucketHeadList.PushBack(HashMapBucketHead_INL(itemIdx, hashValue));

				HashMapBucket_INL* newHashMapNode = DBG_NEW HashMapBucket_INL(std::move(Key), std::move(Value));
				_bucketHeadList[itemIdx]._next = newHashMapNode;
				_cnt++;
			}
			else
			{
				HashMapBucket_INL* hashMapNodeItem = _bucketHeadList[itemPoolIdx]._next;
				while (hashMapNodeItem->_next != nullptr)
				{
					if (hashMapNodeItem->_pair.first == Key)
					{
						hashMapNodeItem->_pair.second = Value;
						return;
					}

					hashMapNodeItem = hashMapNodeItem->_next;
				}

				HashMapBucket_INL* newHashMapNode = DBG_NEW HashMapBucket_INL(Key, Value);
				hashMapNodeItem->_next = newHashMapNode;
				_cnt++;
			}
		}


		void Remove(const KeyType& Key)
		{
			const int32 hashValue = HashValue(Key) % _hashMapCapacity;
			const int32 itemPoolIdx = _hashValueToBucketHeadListIdx[hashValue];
			if (itemPoolIdx == INVALID_IDX)
			{
				return;
			}


			HashMapBucket_INL* firstHashMapNode = _bucketHeadList[itemPoolIdx]._next;
			HashMapBucket_INL* PrevNode = nullptr;
			HashMapBucket_INL* CurNode = nullptr;

			if(firstHashMapNode->_pair.first == Key)
			{
				_bucketHeadList[itemPoolIdx]._next = firstHashMapNode->_next;
				delete firstHashMapNode;
				_cnt--;
				goto lb_CleanupEmptyHashMapNodeHead;
			}


			PrevNode = firstHashMapNode;
			CurNode = PrevNode->_next;

			while (CurNode != nullptr)
			{
				if(CurNode->_pair.first == Key)
				{
					PrevNode->_next = CurNode->_next;
					delete CurNode;
					_cnt--;
					return;
				}

				PrevNode = PrevNode->_next;
				CurNode = CurNode->_next;
			}
			

lb_CleanupEmptyHashMapNodeHead:
			if (_bucketHeadList[itemPoolIdx]._next == nullptr)
			{
				_bucketHeadList.RemoveAtAndFillLast(itemPoolIdx);
				_hashValueToBucketHeadListIdx[hashValue] = INVALID_IDX;

				if (_bucketHeadList.GetSize() == 0)
				{
					return;
				}

				if (_bucketHeadList.IsValidIndex(itemPoolIdx)) {
					HashMapBucketHead_INL& replacedHead = _bucketHeadList[itemPoolIdx];
					_hashValueToBucketHeadListIdx[replacedHead._hashValue] = itemPoolIdx;
					replacedHead._idxInArray = itemPoolIdx;
				}
			}

		}

		ValueType* Find(const KeyType& Key)
		{
			const int32 hashValue = HashValue(Key) % _hashMapCapacity;
			int32 itemPoolIdx = _hashValueToBucketHeadListIdx[hashValue];
			if (itemPoolIdx == INVALID_IDX)
			{
				return nullptr;
			}

			HashMapBucket_INL* hashMapNode = _bucketHeadList[itemPoolIdx]._next;
			do
			{
				if (hashMapNode->_pair.first == Key)
				{
					return &hashMapNode->_pair.second;
				}

				hashMapNode = hashMapNode->_next;
			} while (hashMapNode != nullptr);

			return nullptr;
		}
		
		const ValueType* Find(const KeyType& Key) const
		{
			const int32 hashValue = HashValue(Key) % _hashMapCapacity;
			int32 itemPoolIdx = _hashValueToBucketHeadListIdx[hashValue];
			if (itemPoolIdx == INVALID_IDX)
			{
				return nullptr;
			}

			HashMapBucket_INL* hashMapNode = _bucketHeadList[itemPoolIdx]._next;
			do
			{
				if(hashMapNode->_pair.first == Key)
				{
					return &hashMapNode->_pair.second;
				}

				hashMapNode = hashMapNode->_next;
			} while (hashMapNode != nullptr);

			return nullptr;
		}

		void Clear()
		{
			int32 curBucketHeadIdx = -1;
			HashMapBucket_INL* curBucketNode = nullptr;


			while(true)
			{
				if(curBucketNode == nullptr)
				{
					curBucketHeadIdx++;
					if (_bucketHeadList.IsValidIndex(curBucketHeadIdx) == false)
					{
						break;
					}
					curBucketNode = _bucketHeadList[curBucketHeadIdx]._next;
				}

				HashMapBucket_INL* nextBucketNode = curBucketNode->_next;
				delete curBucketNode;
				curBucketNode = nextBucketNode;
			}


			_bucketHeadList.SetSizeDirectly(0);
			for(int32 i=0;i<_hashValueToBucketHeadListIdx.GetSize();i++)
			{
				_hashValueToBucketHeadListIdx[i] = INVALID_IDX;
			}

			_cnt = 0;
		}



#pragma region iterator
	public:
		class iterator {
			friend class HashMap;
		private:
			int32 _bucketIdx;
			HashMapBucket_INL* _bucketNode;
			const HashMap& _hashMap;

		public:
			iterator& operator++() {

				_bucketNode = _bucketNode->_next;

				if(_bucketNode == nullptr)
				{
					_bucketIdx++;
					if(_bucketIdx >= _hashMap._bucketHeadList.GetSize())
					{
						return *this;
					}

					_bucketNode = _hashMap._bucketHeadList[_bucketIdx]._next;
				}

				return *this;
			}
			
			bool operator==(const iterator rhs) const { return _bucketIdx == rhs._bucketIdx && _bucketNode == rhs._bucketNode; }
			bool operator!=(const iterator rhs) const { return _bucketIdx != rhs._bucketIdx || _bucketNode != rhs._bucketNode; }
			SS::pair<KeyType, ValueType>& operator*() { return _bucketNode->_pair; }

		private:
			iterator(int32 bucketIdx, HashMapBucket_INL* bucketNode, HashMap& hashMap) :
			_bucketIdx(bucketIdx), _bucketNode(bucketNode), _hashMap(hashMap) { }
		};
		FORCEINLINE iterator begin()
		{
			if(_bucketHeadList.GetSize() <= 0)
			{
				return end();
			}
			return iterator(0, _bucketHeadList[0]._next, *this);
		}
		FORCEINLINE iterator end() { return iterator(_bucketHeadList.GetSize(), nullptr, *this); }


		class const_iterator {
			friend class HashMap;
		private:
			int32 _bucketIdx;
			HashMapBucket_INL* _bucketNode;
			const HashMap& _hashMap;

		public:
			const_iterator& operator++() {

				_bucketNode = _bucketNode->_next;

				if (_bucketNode == nullptr)
				{
					_bucketIdx++;
					if (_bucketIdx >= _hashMap._bucketHeadList.GetSize())
					{
						return *this;
					}

					_bucketNode = _hashMap._bucketHeadList[_bucketIdx]._next;
				}

				return *this;
			}


			bool operator==(const const_iterator rhs) const { return _bucketIdx == rhs._bucketIdx && _bucketNode == rhs._bucketNode; }
			bool operator!=(const const_iterator rhs) const { return _bucketIdx != rhs._bucketIdx || _bucketNode != rhs._bucketNode; }
			const SS::pair<KeyType, ValueType>& operator*() { return _bucketNode->_pair; }

		private:
			const_iterator(int32 bucketIdx, HashMapBucket_INL* bucketNode, const HashMap& hashMap) :
				_bucketIdx(bucketIdx), _bucketNode(bucketNode), _hashMap(hashMap) { }
		};
		FORCEINLINE const_iterator begin() const
		{
			if (_bucketHeadList.GetSize() <= 0)
			{
				return end();
			}
			return const_iterator(0, _bucketHeadList[0]._next, *this);
		}
		FORCEINLINE const_iterator end() const { return const_iterator(_bucketHeadList.GetSize(), nullptr, *this); }
	};
#pragma endregion
}