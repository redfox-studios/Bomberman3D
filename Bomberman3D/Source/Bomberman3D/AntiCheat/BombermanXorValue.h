#pragma once

#include "CoreMinimal.h"

template <typename T>
struct TXorValue
{
  public:
	TXorValue()
	{
		Key = FMath::Rand();
		Set(T{});
	}

	TXorValue(T InValue)
	{
		Key = FMath::Rand();
		Set(InValue);
	}

	void Set(T InValue)
	{
		Encrypted = Xor(InValue);
	}

	T Get() const
	{
		return Xor(Encrypted);
	}

	TXorValue& operator=(T InValue)
	{
		Set(InValue);
		return *this;
	}

	operator T() const
	{
		return Get();
	}

  private:
	int32 Encrypted = 0;
	int32 Key = 0;

	T Xor(T InValue) const
	{
		T Result;
		const uint8* Src = reinterpret_cast<const uint8*>(&InValue);
		const uint8* KeyBytes = reinterpret_cast<const uint8*>(&Key);
		uint8* Dst = reinterpret_cast<uint8*>(&Result);

		for (SIZE_T i = 0; i < sizeof(T); i++)
			Dst[i] = Src[i] ^ KeyBytes[i % sizeof(int32)];

		return Result;
	}
};
