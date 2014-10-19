/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DAVAENGINE_TYPE_HOLDERS_H__
#define __DAVAENGINE_TYPE_HOLDERS_H__

#include <new>
#include "TemplateHelpers.h"
#include "Base/BaseTypes.h"

namespace DAVA
{

class RefCounter
{
public:
	RefCounter() : refCount(1)
	{}

	void AddRef()
	{
		refCount++;
	}

	void RemRef()
	{
		refCount--;
		if (0 == refCount)
		{
			delete this;
		}
	}

private:
	uint32 refCount;
};

// ====================================================================================================================================================
// FunctionPointerHolder class 
// General class to store a pointer to function in common way.
// ====================================================================================================================================================
struct FunctionPointerHolder
{
	enum
	{
		FuncHolderMaxSize = (sizeof(void *) * 4)
	};

	unsigned char buf[FuncHolderMaxSize];

	FunctionPointerHolder()
	{
		memset(buf, 0, FuncHolderMaxSize);
	}

	~FunctionPointerHolder()
	{
		memset(buf, 0, FuncHolderMaxSize);
	}

	template<typename F>
	FunctionPointerHolder(F fn)
	{
		// Function pointer with size greater than FuncHolderMaxSize
		// can't be added to this holder. This will be checked on compile time.
		COMPILER_ASSERT(sizeof(F) <= FuncHolderMaxSize);

		memset(buf, 0, FuncHolderMaxSize);
		new(buf)F(fn);
	}

	template<typename F>
	inline F GetPointer() const
	{
		return *reinterpret_cast<F const *>(buf);
	}

	bool operator==(const FunctionPointerHolder &f)
	{
		return (0 == memcmp(buf, f.buf, FuncHolderMaxSize));
	}

	bool IsNull() const
	{
		static const unsigned char zero_block[FuncHolderMaxSize] = {0};
		return (0 == memcmp(zero_block, buf, FuncHolderMaxSize));
	}
};

// ====================================================================================================================================================
// ObjectPointerHolder class 
// Class to store a pointer to the object. It can delete the object if this object is RefCounter class
// ====================================================================================================================================================
struct ObjectPointerHolder
{
public:
	void *object;

	ObjectPointerHolder() 
		: object(NULL)
		, type(Holder_Regular)
	{}

	ObjectPointerHolder(void *obj) 
		: object(obj)
		, type(Holder_Regular)
	{}

	ObjectPointerHolder(RefCounter *obj) 
		: object(obj)
		, type(Holder_RefCounter)
	{}

	~ObjectPointerHolder()
	{
		RemRef();
	}

 	ObjectPointerHolder(const ObjectPointerHolder& holder)
 	{
 		type = holder.type;
 		object = holder.object;
 
 		AddRef();
 	}
 
 	ObjectPointerHolder& operator=(const ObjectPointerHolder& holder)
 	{
        if(this != &holder)
        {
            RemRef();
 
            type = holder.type;
            object = holder.object;
 
            AddRef();
        }
 
 		return *this;
 	}
 
protected:
	enum HoldedType
	{
		Holder_Regular,
		Holder_RefCounter
	};

	HoldedType type;

 	inline void RemRef()
 	{
		switch (type)
		{
		case Holder_RefCounter:
			if (NULL != object)
			{
				((RefCounter *)object)->RemRef();
				object = NULL;
			}
			break;
		default:
			break;
		}
	}
 
 	inline void AddRef()
 	{
		switch (type)
		{
		case Holder_RefCounter:
			if (NULL != object)
			{
				((RefCounter *)object)->AddRef();
			}
			break;
		default:
			break;
		}
 	}
};

// ====================================================================================================================================================
// Placeholder class 
// ====================================================================================================================================================
template<int>
struct Placeholder
{
    Placeholder() {};
};

template<typename U> struct IsPlaceholder { enum { result = false }; };
template<> struct IsPlaceholder< Placeholder<1> > { enum { result = true }; };
template<> struct IsPlaceholder< Placeholder<2> > { enum { result = true }; };
template<> struct IsPlaceholder< Placeholder<3> > { enum { result = true }; };
template<> struct IsPlaceholder< Placeholder<4> > { enum { result = true }; };
template<> struct IsPlaceholder< Placeholder<5> > { enum { result = true }; };
template<> struct IsPlaceholder< Placeholder<6> > { enum { result = true }; };
template<> struct IsPlaceholder< Placeholder<7> > { enum { result = true }; };
template<> struct IsPlaceholder< Placeholder<8> > { enum { result = true }; };

template<typename U> struct PlaceholderIndex { enum { value = 0 }; };
template<> struct PlaceholderIndex< Placeholder<1> > { enum { value = 1 }; };
template<> struct PlaceholderIndex< Placeholder<2> > { enum { value = 2 }; };
template<> struct PlaceholderIndex< Placeholder<3> > { enum { value = 3 }; };
template<> struct PlaceholderIndex< Placeholder<4> > { enum { value = 4 }; };
template<> struct PlaceholderIndex< Placeholder<5> > { enum { value = 5 }; };
template<> struct PlaceholderIndex< Placeholder<6> > { enum { value = 6 }; };
template<> struct PlaceholderIndex< Placeholder<7> > { enum { value = 7 }; };
template<> struct PlaceholderIndex< Placeholder<8> > { enum { value = 8 }; };

namespace
{
	static const DAVA::Placeholder<1> _1;
	static const DAVA::Placeholder<2> _2;
	static const DAVA::Placeholder<3> _3;
	static const DAVA::Placeholder<4> _4;
	static const DAVA::Placeholder<5> _5;
	static const DAVA::Placeholder<6> _6;
	static const DAVA::Placeholder<7> _7;
	static const DAVA::Placeholder<8> _8;
};

// ====================================================================================================================================================
// ParamHolder class
// ====================================================================================================================================================
template<typename T>
struct ParamHolder
{
	typedef typename TypeTraits<T>::NonRefType ParamType;

	ParamHolder(const T& p) : param(p)
	{}

	const T& GetParameter() const
	{
		return param;
	}

	template <typename P1>
	const T& GetParameter(const P1&) const
	{
		return param;
	}

	template <typename P1, typename P2>
	const T& GetParameter(const P1&, const P2&) const
	{
		return param;
	}

	template <typename P1, typename P2, typename P3>
	const T& GetParameter(const P1&, const P2&, const P3&) const
	{
		return param;
	}

	template <typename P1, typename P2, typename P3, typename P4>
	const T& GetParameter(const P1&, const P2&, const P3&, const P4&) const
	{
		return param;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	const T& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5&) const
	{
		return param;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	const T& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5&, const P6&) const
	{
		return param;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	const T& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5&, const P6&, const P7&) const
	{
		return param;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	const T& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5&, const P6&, const P7&, const P8&) const
	{
		return param;
	}

protected:
	ParamType param;
};

template<>
struct ParamHolder< Placeholder<1> >
{
	ParamHolder(const Placeholder<1> &)
	{}

	template <typename P1>
	const P1& GetParameter(const P1 &p1) const
	{
		return p1;
	}

	template <typename P1, typename P2>
	const P1 & GetParameter(const P1& p1, const P2&) const
	{
		return p1;
	}

	template <typename P1, typename P2, typename P3>
	const P1& GetParameter(const P1& p1, const P2&, const P3&) const
	{
		return p1;
	}

	template <typename P1, typename P2, typename P3, typename P4>
	const P1& GetParameter(const P1& p1, const P2&, const P3&, const P4&) const
	{
		return p1;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	const P1& GetParameter(const P1& p1, const P2&, const P3&, const P4&, const P5&) const
	{
		return p1;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	const P1& GetParameter(const P1& p1, const P2&, const P3&, const P4&, const P5&, const P6&) const
	{
		return p1;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	const P1& GetParameter(const P1& p1, const P2&, const P3&, const P4&, const P5&, const P6&, const P7&) const
	{
		return p1;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	const P1& GetParameter(const P1& p1, const P2&, const P3&, const P4&, const P5&, const P6&, const P7&, const P8&) const
	{
		return p1;
	}
};

template<>
struct ParamHolder< Placeholder<2> >
{
	ParamHolder(const Placeholder<2> &)
	{}

	template <typename P1, typename P2>
	const P2& GetParameter(const P1&, const P2& p2) const
	{
		return p2;
	}

	template <typename P1, typename P2, typename P3>
	const P2& GetParameter(const P1&, const P2& p2, const P3&) const
	{
		return p2;
	}

	template <typename P1, typename P2, typename P3, typename P4>
	const P2& GetParameter(const P1&, const P2& p2, const P3&, const P4&) const
	{
		return p2;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	const P2& GetParameter(const P1&, const P2& p2, const P3&, const P4&, const P5&) const
	{
		return p2;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	const P2& GetParameter(const P1&, const P2& p2, const P3&, const P4&, const P5&, const P6&) const
	{
		return p2;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	const P2& GetParameter(const P1&, const P2& p2, const P3&, const P4&, const P5&, const P6&, const P7&) const
	{
		return p2;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	const P2& GetParameter(const P1&, const P2& p2, const P3&, const P4&, const P5&, const P6&, const P7&, const P8&) const
	{
		return p2;
	}
};


template<>
struct ParamHolder< Placeholder<3> >
{
	ParamHolder(const Placeholder<3> &)
	{}

	template <typename P1, typename P2, typename P3>
	const P3& GetParameter(const P1&, const P2&, const P3& p3) const
	{
		return p3;
	}

	template <typename P1, typename P2, typename P3, typename P4>
	const P3& GetParameter(const P1&, const P2&, const P3& p3, const P4&) const
	{
		return p3;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	const P3& GetParameter(const P1&, const P2&, const P3& p3, const P4&, const P5&) const
	{
		return p3;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	const P3& GetParameter(const P1&, const P2&, const P3& p3, const P4&, const P5&, const P6&) const
	{
		return p3;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	const P3& GetParameter(const P1&, const P2&, const P3& p3, const P4&, const P5&, const P6&, const P7&) const
	{
		return p3;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	const P3& GetParameter(const P1&, const P2&, const P3& p3, const P4&, const P5&, const P6&, const P7&, const P8&) const
	{
		return p3;
	}
};

template<>
struct ParamHolder< Placeholder<4> >
{
	ParamHolder(const Placeholder<4> &)
	{}

	template <typename P1, typename P2, typename P3, typename P4>
	const P4& GetParameter(const P1&, const P2&, const P3&, const P4& p4) const
	{
		return p4;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	const P4& GetParameter(const P1&, const P2&, const P3&, const P4& p4, const P5&) const
	{
		return p4;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	const P4& GetParameter(const P1&, const P2&, const P3&, const P4& p4, const P5&, const P6&) const
	{
		return p4;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	const P4& GetParameter(const P1&, const P2&, const P3&, const P4& p4, const P5&, const P6&, const P7&) const
	{
		return p4;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	const P4& GetParameter(const P1&, const P2&, const P3&, const P4& p4, const P5&, const P6&, const P7&, const P8&) const
	{
		return p4;
	}
};

template<>
struct ParamHolder< Placeholder<5> >
{
	ParamHolder(const Placeholder<5> &)
	{}

	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	const P5& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5& p5) const
	{
		return p5;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	const P5& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5& p5, const P6&) const
	{
		return p5;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	const P5& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5& p5, const P6&, const P7&) const
	{
		return p5;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	const P5& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5& p5, const P6&, const P7&, const P8&) const
	{
		return p5;
	}
};

template<>
struct ParamHolder< Placeholder<6> >
{
	ParamHolder(const Placeholder<6> &)
	{}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	const P6& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5&, const P6& p6) const
	{
		return p6;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	const P6& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5&, const P6& p6, const P7&) const
	{
		return p6;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	const P6& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5&, const P6& p6, const P7&, const P8&) const
	{
		return p6;
	}
};

template<>
struct ParamHolder< Placeholder<7> >
{
	ParamHolder(const Placeholder<7> &)
	{}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	const P7& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5&, const P6&, const P7& p7) const
	{
		return p7;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	const P7& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5&, const P6&, const P7& p7, const P8&) const
	{
		return p7;
	}
};

template<>
struct ParamHolder< Placeholder<8> >
{
	ParamHolder(const Placeholder<8> &)
	{}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	const P8& GetParameter(const P1&, const P2&, const P3&, const P4&, const P5&, const P6&, const P7&, const P8& p8) const
	{
		return p8;
	}
};

}

#endif // __DAVAENGINE_TYPE_HOLDERS_H__