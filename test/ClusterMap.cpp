#include "../include/ClusterMap.h"
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
void print_cluster(sw::cluster<T, A> const& p_cluster)
{
	std::cout << "	Capacity " << p_cluster.capacity() << " : Length " << p_cluster.size() << " : ["; 
	for (auto & i : p_cluster)
	{
		std::cout << i << ", ";
	}
	std::cout << "]";
}

template < typename T, typename A >
void print_cluster_vector(sw::cluster_vector<T, A> const& p_cluster_vec)
{
	auto* cluster = p_cluster_vec.first_cluster();
	if (!cluster)
	{
		std::cout << "{}" << std::endl;
		return;
	}
	std::cout << "{" << std::endl;
	print_cluster(*cluster);
	std::cout << std::endl;

	while(!cluster->is_last_cluster())
	{
		cluster = cluster->next_cluster();
		print_cluster(*cluster);
		std::cout << std::endl;
	}

	std::cout << "}" << std::endl;
}

template < typename T, typename A >
void print_dense_cluster(sw::cluster<sw::cluster_map_dense_storage<T>, A> const& p_cluster)
{
	std::cout << "	Capacity " << p_cluster.capacity() << " : Length " << p_cluster.size() << " : ["; 
	for (auto & i : p_cluster)
	{
		std::cout << *reinterpret_cast<T*>(i.mData.mCharData) << ", ";
	}
	std::cout << "]";
}

template < typename T, typename A >
void print_dense_cluster_vector(sw::cluster_vector<T, A> const& p_cluster_vec)
{
	auto* cluster = p_cluster_vec.first_cluster();
	if (!cluster)
	{
		std::cout << "{}" << std::endl;
		return;
	}
	std::cout << "{" << std::endl;
	print_dense_cluster(*cluster);
	std::cout << std::endl;

	while(!cluster->is_last_cluster())
	{
		cluster = cluster->next_cluster();
		print_dense_cluster(*cluster);
		std::cout << std::endl;
	}

	std::cout << "}" << std::endl;
}

template < typename T, typename A >
void print_cluster_map(sw::cluster_map<T, A> const& p_cluster_map)
{
	std::cout << "{" << std::endl;
	std::cout << "dense_storage : ";
	print_dense_cluster_vector(p_cluster_map.dense_storage());

	std::cout << "sparse_indices : ";
	print_cluster_vector(p_cluster_map.sparse_indices());

	std::cout << "unoccupied_list : ";
	print_cluster_vector(p_cluster_map.unoccupied_list());
	std::cout << "}" << std::endl;
}

TEST(cluster_map_test, push_back_test)
{
	{
		using handle_type = sw::cluster_map<int, default_allocator>::handle_type;
		sw::cluster_map<int, default_allocator> sv(8);
		handle_type first = sv.insert(0);
		handle_type second = sv.insert(1);
		handle_type third = sv.insert(2);
		handle_type fourth = sv.insert(3);

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

		{
			auto i = sv.begin();
			EXPECT_EQ(sv.at(first) , 0);
			EXPECT_EQ(sv.at(second), 1);
			EXPECT_EQ(sv.at(third), 2);
			EXPECT_EQ(sv.at(fourth), 3);
		}
	}
}


TEST(cluster_map_test, empty_test)
{
	{
		sw::cluster_map<int, default_allocator> mapOfInt(8);
		sw::cluster_map<std::list<int>, default_allocator> mapOfListOfTO(8);
		EXPECT_TRUE(mapOfInt.empty());
		EXPECT_TRUE(mapOfListOfTO.empty());
	}
}

TEST(cluster_map_test, foreach_test)
{
	{
		std::vector<int> initValues{};
		uint32_t const numValues = 1024u;
		initValues.reserve(numValues);
		for (int i = 0; i < numValues; i++)
		{
			initValues.push_back(i);
		}

		std::vector<sw::cluster_map_handle<int>> handleVec{};
		sw::cluster_map<int, default_allocator> mapOfInt(4);
		EXPECT_EQ(mapOfInt.size(), 0);
		EXPECT_EQ(mapOfInt.empty() , true);
		for (int i : initValues)
		{
			handleVec.push_back(mapOfInt.insert(i));
		}

		EXPECT_EQ(mapOfInt.size() , numValues);
		EXPECT_EQ(mapOfInt.empty() , false);

		int j = 0;
		for (int i : mapOfInt)
		{
			EXPECT_EQ(i, initValues[j]);
			j++;
		}


		for (int l = 0; l < numValues; l++)
		{
			int k = 0;
			for (auto i : handleVec)
			{
				int lhs = mapOfInt.at(i);
				int rhs = initValues[l+k];
				EXPECT_EQ(mapOfInt.at(i), initValues[l+k]);
				k++;
			}
			mapOfInt.erase(handleVec.at(0));
			handleVec.erase(handleVec.begin());
		}	
	}
}

// Thanks to MarioTalevski for his simple GoL implementation
// https://github.com/MarioTalevski/game-of-life/blob/master/GameOfLife.cpp
constexpr size_t c_gridBounds = 26u;
constexpr size_t c_gridSize = 25u;

void copyGrid(
	bool(&rhs)[c_gridBounds][c_gridBounds],
	bool(&lhs)[c_gridBounds][c_gridBounds]
)
{
	memcpy(lhs, rhs, sizeof(bool) * c_gridBounds * c_gridBounds);
}

void advanceConwayRegular(bool(&gridOne)[c_gridBounds][c_gridBounds])
{
	bool gridTwo[c_gridBounds][c_gridBounds] = {};
	copyGrid(gridOne, gridTwo);

	for(int a = 1; a < c_gridSize; a++)
	{
		for(int b = 1; b < c_gridSize; b++)
		{
			int alive = 0;
			for(int c = -1; c < 2; c++)
			{
				for(int d = -1; d < 2; d++)
				{
					if(!(c == 0 && d == 0))
					{
						if(gridTwo[a+c][b+d])
						{
							++alive;
						}
					}
				}
			}
			if(alive < 2)
			{
				gridOne[a][b] = false;
			}
			else if(alive == 3)
			{
				gridOne[a][b] = true;
			}
			else if(alive > 3)
			{
				gridOne[a][b] = false;
			}
		}
	}
}

void copyClusterGrid(
	sw::cluster_map<bool, default_allocator>& rhsGrid,
	sw::cluster_map_handle<bool>(&rhsHandles)[c_gridBounds][c_gridBounds],
	sw::cluster_map<bool, default_allocator>& lhsGrid,
	sw::cluster_map_handle<bool>(&lhsHandles)[c_gridBounds][c_gridBounds]
)
{
	for(int a =0; a < c_gridSize; a++)
	{
		for(int b = 0; b < c_gridSize; b++)
		{
			if(rhsHandles[a][b].mSparseIndexPtr)
			{
				lhsHandles[a][b] = lhsGrid.insert(true);
			}
		}
	}
}

void advanceConwayCluster(
	sw::cluster_map<bool, default_allocator>& gridOne,
	sw::cluster_map_handle<bool> (&gridOneHandles)[c_gridBounds][c_gridBounds]
	)
{
	sw::cluster_map<bool, default_allocator> gridTwo(4);
	sw::cluster_map_handle<bool> gridTwoHandles[c_gridBounds][c_gridBounds]{};
	copyClusterGrid(gridOne, gridOneHandles, gridTwo, gridTwoHandles);

	for(int a = 1; a < c_gridSize; a++)
	{
		for(int b = 1; b < c_gridSize; b++)
		{
			int alive = 0;
			for(int c = -1; c < 2; c++)
			{
				for(int d = -1; d < 2; d++)
				{
					if(!(c == 0 && d == 0))
					{
						if(gridTwoHandles[a+c][b+d].mElementPtr && gridTwo.at(gridTwoHandles[a+c][b+d]))
						{
							++alive;
						}
					}
				}
			}
			if(alive == 3)
			{
				if(!gridOneHandles[a][b].mElementPtr)
				{
					gridOneHandles[a][b] = gridOne.insert(true);
				}
			}
			else if (alive < 2 || alive > 3)
			{
				if(gridOneHandles[a][b].mElementPtr)
				{
					gridOne.erase(gridOneHandles[a][b]);
					gridOneHandles[a][b] = {};
				}
			}
		}
	}
}

void compareMethods(
	bool gridOne[c_gridBounds][c_gridBounds],
	sw::cluster_map_handle<bool>(&gridOneHandles)[c_gridBounds][c_gridBounds],
	sw::cluster_map<bool, default_allocator> &clustermap
)
{
	for(int a =0; a < c_gridSize; a++)
	{
		for(int b = 0; b < c_gridSize; b++)
		{
			bool gridState = gridOne[a][b];
			EXPECT_EQ(gridState, gridOneHandles[a][b].mElementPtr != nullptr);
			if (gridState)
			{
				EXPECT_EQ(gridState, clustermap.at(gridOneHandles[a][b]));
			}
		}
	}
}

TEST(cluster_map_test, conway_gol_test)
{
	{
		sw::cluster_map<bool, default_allocator> cluster_conway_first(4);
		sw::cluster_map_handle<bool> cluster_conway_handles_first[c_gridBounds][c_gridBounds]{};
	
		bool array_conway[c_gridBounds][c_gridBounds]{};

		bool initial_state[c_gridBounds][c_gridBounds] =
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0,
			0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		};
		
		compareMethods(array_conway, cluster_conway_handles_first, cluster_conway_first);

		//initialise
		for(int a =0; a < c_gridSize; a++)
		{
			for(int b = 0; b < c_gridSize; b++)
			{
				bool gridState = initial_state[a][b];
				
				if(gridState)
				{
					cluster_conway_handles_first[a][b] = cluster_conway_first.insert(true);
					array_conway[a][b] = true;
				}
			}
		}

		for(int i = 0; i < 50; i++)
		{
			advanceConwayRegular(array_conway);
			advanceConwayCluster(cluster_conway_first, cluster_conway_handles_first);
			compareMethods(array_conway, cluster_conway_handles_first, cluster_conway_first);
		}
	}
}
