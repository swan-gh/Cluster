#include "../include/ClusterArray.h"
#include <gtest/gtest.h>

#include <list>
#include <stdio.h>

class default_allocator
{
public:

	void* allocate(size_t n)
	{
		return _aligned_malloc(n, 8);
	}

	void* allocate(size_t n, size_t alignment, size_t alignmentOffset)
	{
		if ((alignmentOffset % alignment) == 0)
		{
			return _aligned_malloc(n, alignment);
		}

		return NULL;
	}

	void deallocate(void* p, size_t n)
	{
		_aligned_free(p);
	}
};

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

template < typename T, typename A >
void print_cluster(sw::cluster<T, A> const& p_cluster, int cluster_num)
{
	std::cout << "Cluster " << cluster_num << " : Capacity " << p_cluster.capacity() << " : Length " << p_cluster.size() << " : ["; 
	for (int i : p_cluster)
	{
		std::cout << i << ", ";
	}
	std::cout << "]";
}

template < typename T, typename A >
void print_cluster_vector(sw::cluster_vector<T, A> const& p_vec)
{
	int cluster_num = 1;
	auto* cluster = p_vec.first_cluster();
	print_cluster(*cluster, cluster_num);
	std::cout << std::endl;

	while(!cluster->is_last_cluster())
	{
		cluster = cluster->next_cluster();
		cluster_num++;
		print_cluster(*cluster, cluster_num);
		std::cout << std::endl;
	}
}

TEST(cluster_vector_test, push_back_test)
{
	{
		sw::cluster_vector<int, default_allocator> sv(8);
		sv.push_back(0);
		sv.push_back(1);
		sv.push_back(2);
		sv.push_back(3);

		{
			auto i = sv.begin();
			EXPECT_EQ(*i  , 0);
			EXPECT_EQ(*i++ , 0);
			EXPECT_EQ(*i++ , 1);
			EXPECT_EQ(*i++ , 2);
			EXPECT_EQ(*i++ , 3);
		}

		{
			auto i = sv.begin();
			EXPECT_EQ(*i , 0);
			EXPECT_EQ(*(++i) , 1);
			EXPECT_EQ(*(++i) , 2);
			EXPECT_EQ(*(++i) , 3);
		}
	}
}

TEST(cluster_vector_test, empty_test)
{
	{
		sw::cluster_vector<int, default_allocator> vectorOfInt(8);
		sw::cluster_vector<std::list<int>, default_allocator> vectorOfListOfTO(8);
		EXPECT_TRUE(vectorOfInt.empty());
		EXPECT_TRUE(vectorOfListOfTO.empty());
	}
}

TEST(cluster_vector_test, cluster_count_test)
{
	{
		sw::cluster_vector<int, default_allocator> vectorOfInt(4);
		vectorOfInt.push_back(42);
		EXPECT_EQ(vectorOfInt.size() , 1);
		EXPECT_EQ(vectorOfInt.cluster_count() , 1);
		EXPECT_EQ(vectorOfInt.empty() , false);

		vectorOfInt.push_back(43);
		vectorOfInt.push_back(44);
		vectorOfInt.push_back(45);
		vectorOfInt.push_back(46);
		EXPECT_EQ(vectorOfInt.size() , 5);
		EXPECT_EQ(vectorOfInt.cluster_count() , 2);

		EXPECT_EQ(vectorOfInt.front() , 42);
		EXPECT_EQ(vectorOfInt.back() , 46);

		vectorOfInt.pop_back();
		EXPECT_EQ(vectorOfInt.size() , 4);
		EXPECT_EQ(vectorOfInt.cluster_count(), 1);

		vectorOfInt.clear();
		EXPECT_TRUE(vectorOfInt.empty());
		EXPECT_EQ(vectorOfInt.size() , 0);
		EXPECT_EQ(vectorOfInt.cluster_count() , 0);
	}
}

TEST(cluster_vector_test, foreach_test)
{
	{
		std::vector<int> initValues{};
		uint32_t const numValues = 2048u;
		initValues.reserve(numValues);
		for (int i = 0; i < numValues; i++)
		{
			initValues.push_back(i);
		}

		sw::cluster_vector<int, default_allocator> vectorOfInt(4);
		EXPECT_EQ(vectorOfInt.size(), 0);
		EXPECT_EQ(vectorOfInt.cluster_count() , 0);
		EXPECT_EQ(vectorOfInt.empty() , true);
		for (int i : initValues)
		{
			vectorOfInt.push_back(i);
		}

		EXPECT_EQ(vectorOfInt.size() , 2048u);
		EXPECT_EQ(vectorOfInt.cluster_count() , 10);
		EXPECT_EQ(vectorOfInt.empty() , false);

		int j = 0;
		for (int i : vectorOfInt)
		{
			EXPECT_EQ(i, initValues[j]);
			j++;
		}

		
		for (int l : initValues)
		{
			int k = 0;
			for (int i : vectorOfInt)
			{
				EXPECT_EQ(i, initValues[k]);
				k++;
			}
			vectorOfInt.pop_back();
		}	
	}
}
