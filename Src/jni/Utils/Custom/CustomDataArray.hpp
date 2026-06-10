#pragma once
/*
Author: 0xPrince
*/

template <typename type>
struct C_Array
{
private:
	type* Data = NULL;
	size_t Size = NULL;
	size_t Capacity = NULL;

public:
	C_Array()
	{
		Data = NULL;
		Size = Capacity = NULL;
	}

	~C_Array()
	{
		//clear();/*Good?*/
	}
	constexpr size_t size()
	{
		return Size;
	}
	constexpr size_t capacity()
	{
		return Capacity;
	}
	type* data()
	{
		return Data;
	}
	inline type& operator[](size_t index)
	{
		//__ASSERT(index < Size, "C_Array index outside of range");
		__ASSERT(index < Capacity, "C_Array index outside of bounds");
		return Data[index];
	}
	void clear()
	{
		if (Data && Capacity > 0) free(Data);

		Data = NULL;
		Size = Capacity = NULL;
	}
	void reserve(size_t _NewCapacity)
	{

		if (Capacity >= _NewCapacity || _NewCapacity < 1) return;
		auto NewCapacityInBytes = _NewCapacity * sizeof(type);
		auto NewAlloc = (type*)malloc(NewCapacityInBytes);
		if (NewAlloc)
		{
			//ZeroMemory
			memset(NewAlloc, 0, NewCapacityInBytes);

			if (Size > Capacity) Size = Capacity;
			if (Data && Size > 0)
			{
				//Copy Old Buffer To New
				memcpy(NewAlloc, Data, Size * sizeof(type));

				free(Data); Data = NULL;
			}
		}

		Data = NewAlloc;
		Capacity = Data ? _NewCapacity : NULL;
	}
	void resize(size_t _size)
	{
		if (_size > Capacity)
			reserve(_size);

		Size = (size_t)_size;
	}
	void zero()
	{
		if (!Data|| Capacity <= 0) return;
		memset(Data, 0, Capacity * sizeof(type));
	}
	void insert(const type& _Val)
	{
		//better reserve first
		reserve(Size + 1);
		if (!this->Data) return;
		Data[Size++] = _Val;
	}
	void assign(type* _buff, size_t _size)
	{
		if (!_buff || _size <= 0) return;

		reserve(_size);
		
		if (!this->Data) return;

		memcpy(Data, _buff, _size * sizeof(type));
		this->Size = _size;

	}
	void append(type* _buff, size_t _size)
	{
		if (!_buff || _size <= 0) return;
		
		reserve(Size + _size);
		
		if (!this->Data) return;

		auto NwSize = _size * sizeof(type);
		auto CurrSizeb = this->Size * sizeof(type);

		memcpy(Data + CurrSizeb, _buff, NwSize);
		Size += _size;
	}
	int FindIndexOf(const type& _Val)
	{
		if (this->Data)
		{
			for (size_t i = 0; i < Size; i++)
			{
				if (Data[i] == _Val) return i;
			}
		}
		
		return -1;
	}
};