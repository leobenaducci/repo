#pragma once

template<typename T>
class vector
{
public:
	vector() : Size(0), Capacity(0), Data(nullptr) {}
	~vector() { delete[] Data; }

	void push_back(const T& i)
	{
		if (Size + 1 >= Capacity)
			resize((Size + 1) * 2);

		Data[Size] = i;
		Size++;
	}

	void resize(size_t NewCapacity)
	{
		T* NewData = new T[NewCapacity];

		if (Data != nullptr && Size > 0)
		{
			memcpy(NewData, Data, std::min(NewCapacity, Size));
			delete[] Data;
		}

		Capacity = NewCapacity;
		Data = NewData;
	}

	size_t size() const { return Size; }

	T* data() { return Data; }
	const T* data() const { return Data; }

	T& operator[](size_t idx) { return Data[idx]; }
	const T& operator[](size_t idx) const { return Data[idx]; }

protected:
	size_t Size;
	size_t Capacity;
	T* Data;
};
