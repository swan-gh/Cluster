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

#pragma once 

#include "Common.h"

#include "ClusterVector.h"

#include <type_traits>

namespace sw
{

template <typename T>
struct cluster_map_dense_storage;

template <typename T>
struct cluster_map_dense_storage
{
	cluster_map_dense_storage<T>**					mSparseIndexPtr;
	sw::aligned_storage_t<sizeof(T), alignof(T)>	mData;
};

template <typename T>
struct cluster_map_handle
{
	cluster_map_dense_storage<T>**		mSparseIndexPtr;
	cluster_map_dense_storage<T>*		mElementPtr;
};

template <typename T> inline void		validate(cluster_map_handle<T>& handle);
template <typename T> inline T&			at(cluster_map_handle<T>& handle);
template <typename T> inline T const&	at_c(cluster_map_handle<T>& handle);
template <typename T> inline bool		is_null(cluster_map_handle<T> handle);

template <typename T>
void validate(cluster_map_handle<T>& handle)
{
	cluster_map_dense_storage<T>* index = handle.mElementPtr;
	if (index->mSparseIndexPtr != handle.mSparseIndexPtr)
	{
		//Update the element ptr
		handle.mElementPtr = *(handle.mSparseIndexPtr);
	}
}

template <typename T>
T& at(cluster_map_handle<T>& handle)
{
	validate(handle);
	return *reinterpret_cast<T*>(handle.mElementPtr->mData.mCharData);
}

template <typename T>
T const& at_c(cluster_map_handle<T>& handle)
{
	validate(handle);
	return *reinterpret_cast<T const*>(handle.mElementPtr->mData.mCharData);
}

template <typename T> 
bool is_null(cluster_map_handle<T> handle)
{
	return handle.mElementPtr == nullptr;
}

template <typename T>
struct cluster_type_helper
{
	using const_ver = typename std::conditional< std::is_const<T>::value, T, const T>::type;
	using constless = typename std::conditional< std::is_const<T>::value, typename std::remove_const<T>::type, T>::type;
	//using constless = T;
	using inner_type = cluster_map_dense_storage<constless>;
};


template <typename T>
struct cluster_map_dense_storage_iterator
{
public:
	typedef cluster_map_dense_storage_iterator<T>										this_type;
	typedef cluster_vector_iterator< typename cluster_type_helper<T>::inner_type>		vec_itr_type;

	cluster_map_dense_storage_iterator() = default;
	cluster_map_dense_storage_iterator(vec_itr_type const & currentElement, vec_itr_type const & lastElement);
	
	this_type&						operator=(vec_itr_type const &);

	T*								operator->() const;
	T&								operator*() const;

	this_type&						operator++();
	this_type						operator++(int);

	vec_itr_type					mCurrentElement;
	vec_itr_type					mLastElement;
};

template <typename T>
inline typename cluster_map_handle<T> itr_to_handle(cluster_map_dense_storage_iterator<T> const * itr);

template <typename T>
typename cluster_map_handle<T> itr_to_handle(cluster_map_dense_storage_iterator<T> const * itr)
{
	using handle_type = cluster_map_handle<T>;
	using itr_type = cluster_map_dense_storage_iterator<T>;
	handle_type han{};
	if (!itr->mCurrentElement.mCurrent)
	{
		return han;
	}
	han.mElementPtr = itr->mCurrentElement.mCurrent;
	han.mSparseIndexPtr = han.mElementPtr->mSparseIndexPtr;
	return han;
}

template <typename T, typename Allocator, size_t tStepSize = 2u>
class cluster_map
{
public:

	using this_type				= cluster_map<T, Allocator, tStepSize>;
	using allocator_type		= Allocator;

	using size_type				= size_t;
	using storage_type			= cluster_map_dense_storage<T>;
	using index_type			= cluster_map_dense_storage<T>*;
	using handle_type			= cluster_map_handle<T>;

	template <typename U>
	using cluster_vector_type	= cluster_vector<U, Allocator, tStepSize>;

	using storage_vector_type	= cluster_vector_type<storage_type>;
	using storage_cluster_type	= typename storage_vector_type::cluster_type;
	using const_iterator		= cluster_map_dense_storage_iterator<const T>;
	using iterator				= cluster_map_dense_storage_iterator<T>;

	using index_vector_type		= cluster_vector_type<index_type>;
	using index_ptr_vector_type	= cluster_vector_type<index_type*>;

	using value_type			= T;

								cluster_map(size_type initialClusterCapacity, const Allocator& allocator = Allocator());
								cluster_map() : cluster_map(64u) {}
	void						swap(this_type& other);

	allocator_type&				get_allocator() {return mDenseStorage.get_allocator();}

	const_iterator				begin() const
	{
		typename storage_vector_type::const_iterator itr = mDenseStorage.begin();	
		typename storage_vector_type::iterator * unconstitr = (typename storage_vector_type::iterator *)(void*)&itr;
		const_iterator i(*unconstitr, mDenseEnd);
		return i;
	}
	iterator					begin() { iterator i(mDenseStorage.begin(), mDenseEnd); return i; }

	const_iterator				end() const;
	iterator					end();

	size_type					size() const;
	handle_type					front();
	handle_type					back();

	bool						empty() const {return begin() == end();}
	void						clear();

	void						swap_pos(iterator lhs, iterator rhs);
	void						swap_pos(handle_type& lhs, handle_type& rhs);

	template<typename... Args>
	handle_type					insert(Args&&... args);

	void						erase(handle_type& handle);
	void						erase(iterator itr);

	storage_vector_type const & dense_storage() const { return mDenseStorage; }
	index_vector_type const &	sparse_indices() const { return mSparseIndices; }
	index_ptr_vector_type const & unoccupied_list() const { return mUnoccupiedElements; }

protected:

	storage_vector_type				mDenseStorage;			//Store our data without any gaps or null elements, addresses are not stable. Intrusively stores a ptr back to the sparse Indices array.
	index_vector_type				mSparseIndices;			//Store stable ptrs to the dense storage associated with this index. Null entries are nullptrs.
	index_ptr_vector_type			mUnoccupiedElements;	//Store a list of removed sparse indices so that we have constant-time insertion
	typename iterator::vec_itr_type	mDenseEnd;				//Itr to the last dense element + cluster
};

template<typename T>
inline cluster_map_dense_storage_iterator<T>::cluster_map_dense_storage_iterator(vec_itr_type const& currentElement, vec_itr_type const & lastElement)
	: mCurrentElement(currentElement)
	, mLastElement(lastElement)
{
}

template<typename T>
inline typename cluster_map_dense_storage_iterator<T>::this_type&
cluster_map_dense_storage_iterator<T>::operator=
(
	typename cluster_map_dense_storage_iterator<T>::vec_itr_type const& itr
)
{
	mCurrentElement = itr;
	return *this;
}

template<typename T>
T*
cluster_map_dense_storage_iterator<T>::operator->() const
{
	return reinterpret_cast<T*>(mCurrentElement->mData.mCharData);
}

template<typename T>
T&
cluster_map_dense_storage_iterator<T>::operator*() const
{
	return *reinterpret_cast<T*>(mCurrentElement->mData.mCharData);
}

template<typename T>
cluster_map_dense_storage_iterator<T>&
cluster_map_dense_storage_iterator<T>::operator++()
{
	if (CLUSTER_UNLIKELY(mCurrentElement.mCurrent + 1u == mLastElement.mCurrent))
	{
		mCurrentElement = mLastElement;
	}
	else
	{
		++mCurrentElement;
	}
	return *this;
}

template<typename T>
cluster_map_dense_storage_iterator<T>
cluster_map_dense_storage_iterator<T>::operator++(int)
{
	this_type i(*this);
	operator++();
	return i;
}

template<typename T, typename Allocator, size_t tStepSize>
inline cluster_map<T, Allocator, tStepSize>::cluster_map(size_type initialClusterCapacity, const Allocator& allocator) :
	mDenseStorage(initialClusterCapacity, allocator)
	,mSparseIndices(initialClusterCapacity, allocator)
	,mUnoccupiedElements(initialClusterCapacity, allocator)
	,mDenseEnd{}
{}

template<typename T, typename Allocator, size_t tStepSize>
inline void cluster_map<T, Allocator, tStepSize>::swap(this_type & other)
{
	mDenseStorage.swap(other.mDenseStorage);
	mSparseIndices.swap(other.mSparseIndices);
	mUnoccupiedElements.swap(other.mUnoccupiedElements);
	mDenseEnd.swap(other.mDenseEnd);
}

template<typename T, typename Allocator, size_t tStepSize>
inline typename cluster_map<T, Allocator, tStepSize>::iterator
cluster_map<T, Allocator, tStepSize>::end()
{
	return iterator(mDenseEnd, mDenseEnd);
}


template<typename T, typename Allocator, size_t tStepSize>
inline typename cluster_map<T, Allocator, tStepSize>::const_iterator
cluster_map<T, Allocator, tStepSize>::end() const
{	
	return const_iterator(mDenseEnd, mDenseEnd);
}

template<typename T, typename Allocator, size_t tStepSize>
inline typename cluster_map<T, Allocator, tStepSize>::size_type
cluster_map<T, Allocator, tStepSize>::size() const
{
	if (storage_cluster_type* cluster = mDenseEnd.mCluster)
	{

		size_t lastCapacity = cluster->capacity();
		size_t powerDifference = lastCapacity / mDenseStorage.mInitialClusterCapacity;
		uint32_t numDenseClusters = static_cast<uint32_t>(64 - CLUSTER_COUNT_LEADING_ZEROES(powerDifference));
		size_t lastSize = mDenseEnd.mCurrent - cluster->begin();

		//Geometric Series formula: Sn = a1(1-r^n) / (1-r)
		size_t series_sum = detail::pow_table<tStepSize>::val[numDenseClusters - 1u];
		size_t numerator = (mDenseStorage.mInitialClusterCapacity * (series_sum - 1));
		size_t denominator = (tStepSize - 1);
		size_t result = (numerator / denominator) + lastSize;
		return result;
	}
	return 0;
}

template<typename T, typename Allocator, size_t tStepSize>
inline typename cluster_map<T, Allocator, tStepSize>::handle_type
cluster_map<T, Allocator, tStepSize>::front()
{
	iterator first = begin();
	return handle_type
	{
		reinterpret_cast<cluster_map_dense_storage<T>**>(first.mCurrentElement->mSparseIndexPtr),
		reinterpret_cast<cluster_map_dense_storage<T>*>(first.operator->())
	};
}

template<typename T, typename Allocator, size_t tStepSize>
inline typename cluster_map<T, Allocator, tStepSize>::handle_type
cluster_map<T, Allocator, tStepSize>::back()
{
	storage_type* last = mDenseEnd.mCurrent - 1u;
	return handle_type
	{
		reinterpret_cast<cluster_map_dense_storage<T>**>(last->mSparseIndexPtr),
		reinterpret_cast<cluster_map_dense_storage<T>*>(last->mData.mCharData)
	};
}

template<typename T, typename Allocator, size_t tStepSize>
inline void cluster_map<T, Allocator, tStepSize>::clear()
{
	mDenseStorage.clear();
	mSparseIndices.clear();
	mUnoccupiedElements.clear();
	mDenseEnd = mDenseStorage.end();
}

template<typename T, typename Allocator, size_t tStepSize>
inline void cluster_map<T, Allocator, tStepSize>::swap_pos(iterator lhs, iterator rhs)
{
	storage_type& lh = *reinterpret_cast<storage_type*>(lhs.mCurrentElement.mCurrent);
	storage_type& rh = *reinterpret_cast<storage_type*>(rhs.mCurrentElement.mCurrent);
	index_type* lh_index_ptr = lh.mSparseIndexPtr;
	index_type* rh_index_ptr = rh.mSparseIndexPtr;
	std::swap(lh, rh);

	//Patch up indexes for swapped elements
	*lh.mSparseIndexPtr = &lh;
	*rh.mSparseIndexPtr = &rh;
}

template<typename T, typename Allocator, size_t tStepSize>
inline void cluster_map<T, Allocator, tStepSize>::swap_pos(handle_type& lhs, handle_type& rhs)
{
	validate(lhs);
	validate(rhs);
	storage_type& lh = *lhs.mElementPtr;
	storage_type& rh = *rhs.mElementPtr;
	index_type* lh_index_ptr = lh.mSparseIndexPtr;
	index_type* rh_index_ptr = rh.mSparseIndexPtr;
	std::swap(lh, rh);

	//Patch up indexes for swapped elements
	*lh.mSparseIndexPtr = &lh;
	*rh.mSparseIndexPtr = &rh;
}

template<typename T, typename Allocator, size_t tStepSize>
template<typename ...Args>
inline typename cluster_map<T, Allocator, tStepSize>::handle_type
cluster_map<T, Allocator, tStepSize>::insert(Args && ...args)
{
	index_type* index_ptr{};
	index_type index{};

	if(mUnoccupiedElements.empty())
	{		
		//No free space in our dense storage
		typename storage_vector_type::iterator iter = mDenseStorage.push_back();
		index = iter.mCurrent;
		index_ptr = mSparseIndices.push_back(index).mCurrent;
		//Points to the element
		mDenseEnd = iter;
		//Increment past the end
		mDenseEnd.mCurrent++;
	}
	else
	{
		//Free space to be reused
		index_ptr = mUnoccupiedElements.back();
		mUnoccupiedElements.pop_back();
		//We know there must be space after DenseEnd as there are unoccupied elements

		//We increment like this to account for iterating between clusters
		mDenseEnd.mCurrent--;
		mDenseEnd++;
		mDenseEnd.mCurrent++;

		index = mDenseEnd.mCurrent - 1u;
		*index_ptr = index;
	}

	index->mSparseIndexPtr = index_ptr;
	new (&(index->mData)) T(std::forward<Args>(args)...);

	return handle_type{index_ptr, index};
}

template<typename T, typename Allocator, size_t tStepSize>
inline void cluster_map<T, Allocator, tStepSize>::erase(handle_type& handle)
{
	validate(handle);
	//Swap
	storage_type& target = *handle.mElementPtr;
	storage_type& back = *(mDenseEnd.mCurrent - 1u);
	std::swap(target, back);

	//Destruct
	reinterpret_cast<T*>(back.mData.mCharData)->~T();
	index_type* index_ptr = back.mSparseIndexPtr;

	//Patch up index for swapped live element
	mUnoccupiedElements.push_back(index_ptr);
	index_type* swapped_index_ptr = target.mSparseIndexPtr;
	*swapped_index_ptr = &target;

	//Pop
	back.mSparseIndexPtr = nullptr;

	//Decrement mDenseEnd
	mDenseEnd.mCurrent--;
	if (mDenseEnd.mCluster->begin() == mDenseEnd.mCurrent)
	{
		mDenseEnd.mCluster = (storage_cluster_type*)(mDenseEnd.mCluster->mPrev & (~storage_cluster_type::kIsLastCluster));
		if (mDenseEnd.mCluster)
		{
			mDenseEnd.mCurrent = mDenseEnd.mEnd = mDenseEnd.mCluster->end();
		}
		else
		{
			mDenseEnd.mCurrent = mDenseEnd.mEnd = nullptr;
		}
	}
}

template<typename T, typename Allocator, size_t tStepSize>
inline void cluster_map<T, Allocator, tStepSize>::erase(iterator itr)
{
	erase(handle_type{itr.mCurrent->mSparseIndexPtr, itr.operator->()});
}

template<typename T>
inline bool operator==(const cluster_map_dense_storage_iterator<const T>& a, const cluster_map_dense_storage_iterator<const T>& b)
{
	return a.mCurrentElement == b.mCurrentElement;
}


template<typename T>
inline bool operator!=(const cluster_map_dense_storage_iterator<const T>& a, const cluster_map_dense_storage_iterator<const T>& b)
{
	return a.mCurrentElement != b.mCurrentElement;
}

template<typename T>
inline bool operator==(const cluster_map_dense_storage_iterator<T>& a, const cluster_map_dense_storage_iterator<T>& b)
{
	return a.mCurrentElement == b.mCurrentElement;
}


template<typename T>
inline bool operator!=(const cluster_map_dense_storage_iterator<T>& a, const cluster_map_dense_storage_iterator<T>& b)
{
	return a.mCurrentElement != b.mCurrentElement;
}

template<typename T>
inline bool operator==(const cluster_map_dense_storage_iterator<T>& a, const cluster_vector_iterator<cluster_map_dense_storage<T>>& b)
{
	return a.mCurrentElement == b.mCurrent;
}

template<typename T>
inline bool operator!=(const cluster_vector_iterator<cluster_map_dense_storage<T>>& a, const cluster_map_dense_storage_iterator<T>& b)
{
	return a.mCurrent != b.mCurrentElement;
}

}

