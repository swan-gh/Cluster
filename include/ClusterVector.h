#pragma once

//-----------------------------------------------------------------------------
//	The contents of this file is modified versions of EASTL source --
//	https://github.com/electronicarts/EASTL
//-----------------------------------------------------------------------------
//	BSD 3-Clause License
//	
//	Copyright (c) 2019, Electronic Arts
//	All rights reserved.
//	
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are met:
//	
//	1. Redistributions of source code must retain the above copyright notice, this
//	list of conditions and the following disclaimer.
//	
//	2. Redistributions in binary form must reproduce the above copyright notice,
//	this list of conditions and the following disclaimer in the documentation
//	and/or other materials provided with the distribution.
//	
//	3. Neither the name of the copyright holder nor the names of its
//	contributors may be used to endorse or promote products derived from
//	this software without specific prior written permission.
//	
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "Common.h"

namespace sw
{

namespace detail
{

template<typename T>
struct cluster_helper
{
	static const uintptr_t	kIsLastCluster = 1 << 0;
	uintptr_t				mPrev;

	union
	{
		//If there is not another cluster, then mSize stores how many elements are in this container
		cluster_helper*		mNext;
		size_t				mSize;
	};
	//Data is allocated inline following this object
	T*						mDataEnd;
	T						mDummyData[2];
};

template<size_t t_base>
struct pow_table
{
	inline static constexpr size_t const_pow(size_t base, size_t exponent)
	{
		return exponent == 0u ? 1u : base * const_pow(base, exponent - 1u);
	}

	static constexpr size_t val[64] =
	{
		const_pow(t_base, 0),
		const_pow(t_base, 1),
		const_pow(t_base, 2),
		const_pow(t_base, 3),
		const_pow(t_base, 4),
		const_pow(t_base, 5),
		const_pow(t_base, 6),
		const_pow(t_base, 7),
		const_pow(t_base, 8),
		const_pow(t_base, 9),
		const_pow(t_base, 10),
		const_pow(t_base, 11),
		const_pow(t_base, 12),
		const_pow(t_base, 13),
		const_pow(t_base, 14),
		const_pow(t_base, 15),
		const_pow(t_base, 16),
		const_pow(t_base, 17),
		const_pow(t_base, 18),
		const_pow(t_base, 19),
		const_pow(t_base, 20),
		const_pow(t_base, 21),
		const_pow(t_base, 22),
		const_pow(t_base, 23),
		const_pow(t_base, 24),
		const_pow(t_base, 25),
		const_pow(t_base, 26),
		const_pow(t_base, 27),
		const_pow(t_base, 28),
		const_pow(t_base, 29),
		const_pow(t_base, 30),
		const_pow(t_base, 31),
		const_pow(t_base, 32),
		const_pow(t_base, 33),
		const_pow(t_base, 34),
		const_pow(t_base, 35),
		const_pow(t_base, 36),
		const_pow(t_base, 37),
		const_pow(t_base, 38),
		const_pow(t_base, 39),
		const_pow(t_base, 40),
		const_pow(t_base, 41),
		const_pow(t_base, 42),
		const_pow(t_base, 43),
		const_pow(t_base, 44),
		const_pow(t_base, 45),
		const_pow(t_base, 46),
		const_pow(t_base, 47),
		const_pow(t_base, 48),
		const_pow(t_base, 49),
		const_pow(t_base, 50),
		const_pow(t_base, 51),
		const_pow(t_base, 52),
		const_pow(t_base, 53),
		const_pow(t_base, 54),
		const_pow(t_base, 55),
		const_pow(t_base, 56),
		const_pow(t_base, 57),
		const_pow(t_base, 58),
		const_pow(t_base, 59),
		const_pow(t_base, 60),
		const_pow(t_base, 61),
		const_pow(t_base, 62),
		const_pow(t_base, 63),
	};
};

}

template<typename T>
class cluster
{
public:
	using size_type			= size_t;
	using this_type			= cluster<T>;
	using iterator			= T*;
	using const_iterator	= T const *;

	const this_type*		next_cluster() const;
	this_type*				next_cluster();

	const_iterator			begin() const;
	iterator				begin();

	const_iterator			end() const;
	iterator				end();

	size_type				capacity() const;
	size_type				size() const;

	static size_type		allocation_size(size_type num_elements);
	bool					is_last_cluster() const;

	static const uintptr_t	kIsLastCluster = 1 << 0;
	uintptr_t				mPrev;

	union
	{
		//If there is not another cluster, then mSize stores how many elements are in this container
		this_type*			mNext;
		size_type			mSize;
	};
	//Data is allocated inline following this object
	T*						mDataEnd;
	template<typename, typename, size_t> friend class cluster_vector;
	template<typename> friend struct cluster_vector_iterator;
};


template <typename T>
struct cluster_vector_iterator
{
public:
	using this_type			= cluster_vector_iterator<T>;
	using cluster_type		= cluster<T>;

	T*						operator->() const;
	T&						operator*() const;

	this_type&				operator++();
	this_type				operator++(int);

public:
	T*						mCurrent;
	T*						mEnd;
	cluster_type*			mCluster;
};


template <typename T, typename Allocator, size_t tStepSize = 2u>
class cluster_vector
{
public:
	template <typename U, typename OtherAllocator, size_t tOtherStepSize>
	friend class cluster_map;

	using size_type				= size_t;
	using this_type				= cluster_vector<T, Allocator, tStepSize>;
	using cluster_type			= cluster<T>;
	using cluster_helper_type	= typename detail::cluster_helper<T>;
	using allocator_type		= Allocator;
	using iterator				= cluster_vector_iterator<T>;
	using const_iterator		= cluster_vector_iterator<const T>;

	using value_type			= T;

	cluster_vector(size_type initialClusterCapacity, const Allocator& allocator = Allocator());
	cluster_vector() : cluster_vector(64u) {}
	~cluster_vector();

	cluster_vector(cluster_vector &&) = default;
	cluster_vector& operator=(cluster_vector &&) = default;

	cluster_vector(cluster_vector const &) = delete;
	cluster_vector& operator=(cluster_vector const &) = delete;

	allocator_type&			get_allocator();

	const cluster_type*		first_cluster() const;
	cluster_type*			first_cluster();
	const_iterator			begin() const;
	iterator				begin();

	const_iterator			end() const;
	iterator				end();

	size_type				size() const;
	size_type				cluster_count() const;
	T&						front();
	T&						back();

	bool					empty() const;
	void					clear();

	iterator				push_back();
	iterator				push_back(const T& value);
	iterator				push_back_uninitialized();

	void					pop_back();

	void					erase_unsorted(cluster_type& cluster, typename cluster_type::iterator it);
	iterator				erase_unsorted(const iterator& i);

	void					swap(this_type& other);

protected:
	cluster_type*			DoAlloccluster(cluster_type* prevcluster, size_t numElements);
	iterator				DoPushBack();

	allocator_type			mAllocator;
	cluster_type*			mFirstcluster;
	cluster_type*			mLastcluster;
	size_type				mClusterCount;
	size_type const			mInitialClusterCapacity;
};


template<typename T>
inline const cluster<T>*
cluster<T>::next_cluster() const
{
	if (mPrev & kIsLastCluster)
	{
		return nullptr;
	}
	else
	{
		return mNext;
	}
}

template<typename T>
inline cluster<T>*
cluster<T>::next_cluster()
{
	if (mPrev & kIsLastCluster)
	{
		return nullptr;
	}
	else
	{
		return mNext;
	}
}

template<typename T>
inline typename cluster<T>::const_iterator
cluster<T>::begin() const
{
	char const * dataOffset = reinterpret_cast<char const *>(this) + CLUSTER_OFFSETOF(detail::cluster_helper<T>, mDummyData);
	return (cluster<T>::iterator)(dataOffset);
}

template<typename T>
inline typename cluster<T>::iterator
cluster<T>::begin()
{
	char* dataOffset = reinterpret_cast<char*>(this) + CLUSTER_OFFSETOF(detail::cluster_helper<T>, mDummyData);
	return (cluster<T>::iterator)(dataOffset);
}

template<typename T>
inline typename cluster<T>::const_iterator
cluster<T>::end() const
{
	if (mPrev & this_type::kIsLastCluster)
	{
		return begin() + mSize;
	}
	else
	{
		return mDataEnd;
	}
}

template<typename T>
inline typename cluster<T>::iterator
cluster<T>::end()
{
	if (mPrev & kIsLastCluster)
	{
		return begin() + mSize;
	}
	else
	{
		return mDataEnd;
	}
}

template<typename T>
inline typename cluster<T>::size_type 
cluster<T>::capacity() const
{
	return mDataEnd - begin();
}

template<typename T>
inline typename cluster<T>::size_type
cluster<T>::size() const
{
	if (mPrev & this_type::kIsLastCluster)
	{
		return mSize;
	}
	else
	{
		return capacity();
	}
}

template<typename T>
inline typename cluster<T>::size_type 
cluster<T>::allocation_size(size_type num_elements)
{
	return sizeof(detail::cluster_helper<T>) + (sizeof(T) * num_elements) - (sizeof(T) * 2u);
}

template<typename T>
inline bool cluster<T>::is_last_cluster() const
{
	return mPrev & this_type::kIsLastCluster;
}

template<typename T>
T*
cluster_vector_iterator<T>::operator->() const
{
	return mCurrent;
}

template<typename T>
T&
cluster_vector_iterator<T>::operator*() const
{
	return *mCurrent;
}

template<typename T>
cluster_vector_iterator<T>&
cluster_vector_iterator<T>::operator++()
{
	++mCurrent;
	if(CLUSTER_UNLIKELY(mCurrent == mEnd))
	{
		if (!(mCluster->mPrev & cluster_type::kIsLastCluster))
		{
			mCluster = mCluster->mNext;
			mCurrent = mCluster->begin();
			mEnd = mCluster->end();
		}
		else
		{
			mCurrent = 0;
		}
	}
	return *this;
}

template<typename T>
cluster_vector_iterator<T>
cluster_vector_iterator<T>::operator++(int)
{
	this_type i(*this);
	operator++();
	return i;
}


template <typename T,  typename Allocator, size_t tStepSize>
inline cluster_vector<T, Allocator, tStepSize>::cluster_vector(size_type initialClusterCapacity, const Allocator& allocator)
	:	mAllocator(allocator)
	,	mFirstcluster(nullptr)
	,	mLastcluster(nullptr)
	,	mClusterCount(0)
	,	mInitialClusterCapacity(initialClusterCapacity)
{
}

template <typename T,  typename Allocator, size_t tStepSize>
inline cluster_vector<T, Allocator, tStepSize>::~cluster_vector()
{
	clear();
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::allocator_type&
cluster_vector<T, Allocator, tStepSize>::get_allocator()
{
	return mAllocator;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline const typename cluster_vector<T, Allocator, tStepSize>::cluster_type*
cluster_vector<T, Allocator, tStepSize>::first_cluster() const
{
	return mFirstcluster;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::cluster_type*
cluster_vector<T, Allocator, tStepSize>::first_cluster()
{
	return mFirstcluster;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::const_iterator
cluster_vector<T, Allocator, tStepSize>::begin() const
{
	iterator i;
	i.mCluster = mFirstcluster;
	if (mFirstcluster)
	{
		i.mCurrent = mFirstcluster->begin();
		i.mEnd = mFirstcluster->end();
	}
	else
	{
		i.mCurrent = 0;
	}
	return (const_iterator&)i;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::iterator
cluster_vector<T, Allocator, tStepSize>::begin()
{
	iterator i;
	i.mCluster = mFirstcluster;
	if (mFirstcluster)
	{
		i.mCurrent = mFirstcluster->begin();
		i.mEnd = mFirstcluster->end();
	}
	else
	{
		i.mCurrent = 0;
	}
	return i;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::const_iterator
cluster_vector<T, Allocator, tStepSize>::end() const
{
	iterator i;
	i.mCurrent = 0;
	return (const_iterator&)i;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::iterator
cluster_vector<T, Allocator, tStepSize>::end()
{
	iterator i;
	i.mCurrent = 0;
	return i;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::size_type
cluster_vector<T, Allocator, tStepSize>::size() const
{
	if (cluster_type* cluster = mLastcluster)
	{
		//Geometric Series formula: Sn = a1(1-r^n) / (1-r)
		size_t series_sum = detail::pow_table<tStepSize>::val[mClusterCount - 1u];
		size_t numerator = (mInitialClusterCapacity * (series_sum - 1));
		size_t denominator = (tStepSize - 1);
		size_t result = (numerator / denominator) + cluster->mSize;
		return result;
	}
	return 0;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::size_type
cluster_vector<T, Allocator, tStepSize>::cluster_count() const
{
	return mClusterCount;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline T&
cluster_vector<T, Allocator, tStepSize>::front()
{
	return *mFirstcluster->begin();
}

template <typename T,  typename Allocator, size_t tStepSize>
inline T&
cluster_vector<T, Allocator, tStepSize>::back()
{
	cluster_type* lastcluster = mLastcluster;
	return *(lastcluster->end() - 1u);
}

template <typename T,  typename Allocator, size_t tStepSize>
inline bool
cluster_vector<T, Allocator, tStepSize>::empty() const
{
	return mFirstcluster == 0;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline void
cluster_vector<T, Allocator, tStepSize>::clear()
{
	if (cluster_type* clust = mFirstcluster)
	{
		while (clust != mLastcluster)
		{
			cluster_type* nextcluster = clust->mNext;
			clust->~cluster_type();
			CLUSTERFree(mAllocator, clust, cluster_type::allocation_size(clust->capacity()));
			clust = nextcluster;
		}
		for (T* i = clust->begin(), *e = clust->begin() + clust->mSize; i!=e; ++i)
		{
			i->~T();
		}
		CLUSTERFree(mAllocator, clust, cluster_type::allocation_size(clust->capacity()));
		mFirstcluster = 0;
		mLastcluster = 0;
		mClusterCount = 0;
	}
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::iterator
cluster_vector<T, Allocator, tStepSize>::push_back()
{
	iterator itr = DoPushBack();
	new (itr.mCurrent) T();
	return itr;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::iterator
cluster_vector<T, Allocator, tStepSize>::push_back(const T& value)
{
	iterator itr = DoPushBack();
	new (itr.mCurrent) T(value);
	return itr;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::iterator
cluster_vector<T, Allocator, tStepSize>::push_back_uninitialized()
{
	return DoPushBack();
}

template <typename T,  typename Allocator, size_t tStepSize>
inline void
cluster_vector<T, Allocator, tStepSize>::pop_back()
{
	cluster_type* lastcluster = mLastcluster;
#if CLUSTER_ASSERT_ENABLED
	if(CLUSTER_UNLIKELY(!lastcluster))
		CLUSTER_ASSERT("cluster_vector::pop_back -- clustered vector is empty");
#endif
	--lastcluster->mSize;
	(lastcluster->begin() + lastcluster->mSize)->T::~T();

	if (!lastcluster->mSize)
	{
		--mClusterCount;
		mLastcluster = (cluster_type*)(lastcluster->mPrev & (~cluster_type::kIsLastCluster));
		CLUSTERFree(mAllocator, lastcluster, cluster_type::allocation_size(lastcluster->capacity()));
		if (mLastcluster)
		{
			mLastcluster->mPrev |= cluster_type::kIsLastCluster;
			mLastcluster->mSize = mLastcluster->capacity();
		}
		else
		{
			mFirstcluster = 0;
		}
	}
}

template <typename T,  typename Allocator, size_t tStepSize>
inline void
cluster_vector<T, Allocator, tStepSize>::erase_unsorted(cluster_type& cluster, typename cluster_type::iterator it)
{
	EA_UNUSED(cluster);

	*it = back();
	pop_back();
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::iterator
cluster_vector<T, Allocator, tStepSize>::erase_unsorted(const iterator& i)
{
	iterator ret(i);
	*i = back();
	if (i.mCluster == mLastcluster && mLastcluster->mSize == 1)
		ret.mCurrent = 0;
	pop_back();
	return ret;
}

template <typename T,  typename Allocator, size_t tStepSize>
void
cluster_vector<T, Allocator, tStepSize>::swap(this_type& other)
{
	allocator_type tempAllocator(mAllocator);
	cluster_type* tempFirstcluster = mFirstcluster;
	cluster_type* tempLastcluster = mLastcluster;
	size_type tempclusterCount = mClusterCount;

	mAllocator = other.mAllocator;
	mFirstcluster = other.mFirstcluster;
	mLastcluster = other.mLastcluster;
	mClusterCount = other.mClusterCount;

	other.mAllocator = tempAllocator;
	other.mFirstcluster = tempFirstcluster;
	other.mLastcluster = tempLastcluster;
	other.mClusterCount = tempclusterCount;
}

template <typename T,  typename Allocator, size_t tStepSize>
cluster<T>*
cluster_vector<T, Allocator, tStepSize>::DoAlloccluster(cluster_type* prevcluster, size_t numElements)
{
	++mClusterCount;

	size_t allocationSize = cluster_type::allocation_size(numElements);
	cluster_type* cluster = (cluster_type*)sw_allocate_memory(mAllocator, allocationSize, CLUSTER_ALIGN_OF(cluster_helper_type), 0);
	cluster->mPrev = uintptr_t(prevcluster) | cluster_type::kIsLastCluster;
	cluster->mSize = 1;
	cluster->mDataEnd = (T*)((char*)cluster + allocationSize);
	return cluster;
}

template <typename T,  typename Allocator, size_t tStepSize>
inline typename cluster_vector<T, Allocator, tStepSize>::iterator
cluster_vector<T, Allocator, tStepSize>::DoPushBack()
{
	iterator itr{};
	if (cluster_type* cluster = mLastcluster)
	{
		size_type size = cluster->mSize;
		if (size < cluster->capacity())
		{
			++cluster->mSize;
		}
		else
		{
			cluster_type* lastcluster = mLastcluster;
			cluster_type* newcluster = mLastcluster = DoAlloccluster(mLastcluster, lastcluster->capacity() * 2u);
			lastcluster->mPrev &= ~cluster_type::kIsLastCluster;
			lastcluster->mNext = newcluster;
		}
	}
	else
	{
		cluster = mFirstcluster = mLastcluster = DoAlloccluster(0, mInitialClusterCapacity);
	}

	itr.mCurrent = mLastcluster->begin() + mLastcluster->mSize - 1u;
	itr.mCluster = mLastcluster;
	itr.mEnd = mLastcluster->end();

	return itr;
}

template<typename T>
inline bool operator==(const cluster_vector_iterator<T>& a, const cluster_vector_iterator<T>& b)
{
	return a.mCurrent == b.mCurrent;
}


template<typename T>
inline bool operator!=(const cluster_vector_iterator<T>& a, const cluster_vector_iterator<T>& b)
{
	return a.mCurrent != b.mCurrent;
}

}

