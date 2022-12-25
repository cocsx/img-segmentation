#pragma GCC optimize("O3","unroll-loops","omit-frame-pointer","inline") //Optimization flags
#pragma GCC option("arch=native","tune=native","no-zero-upper") //Enable AVX
#pragma GCC target("avx")

#include "KMeans.h"

#include <random>
#include <limits.h>
#include <algorithm>
#include <execution>

KMeans::KMeans(Image& img) : m_image(img)
{
	Init();
}

KMeans::KMeans(Image& img, int maxIter) : m_image(img), m_maxIterations(maxIter)
{
	Init();
}

void KMeans::Partition()
{
	for (int k = 0; k < m_maxIterations; k++)
	{
		MakeClusters();
		CalculateCentroids();
		if (k < m_maxIterations - 1)
		{
			m_clusterMap.clear();
		}
	}

	std::for_each(std::execution::par, m_clusterMap.begin(), m_clusterMap.end(), [this, p](const auto& cp){
		for (auto p : cp.second)
		{
			m_image.m_pixels[p.x + m_image.m_width*p.y] = m_image.m_pixels[cp.first.x + m_image.m_width*cp.first.y];
		}
	});
}

void KMeans::Init()
{
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<int> w(0, m_image.m_width - 1);
	std::uniform_int_distribution<int> h(0, m_image.m_height - 1);

	for (int i = 0; i < m_numOfClusters; i++)
	{
		m_centroids[i][0] = w(rng);
		m_centroids[i][1] = h(rng);
	}
}

void KMeans::MakeClusters()
{
	for (int x = 0; x < m_image.m_width; x++)
	{
		for (int y = 0; y < m_image.m_height; y++)
		{
			Pixel p1 = m_image.m_pixels[x + m_image.m_width*y];
			int i = -1, j = -1;
			int min_dist = INT_MAX;
			for (int p = 0; p < m_numOfClusters; ++p)
			{
				auto c = m_centroids[p];
				Pixel p2 = m_image.m_pixels[c[0] + m_image.m_width*c[1]];
				int dist = DistanceBetweenTwoPixelsColor(p1, p2);

				if (dist >= min_dist) continue;
				min_dist = dist;
				i = c[0];
				j = c[1];
			}
			m_clusterMap[{i,j}].emplace_back(x, y);
		}
	}
}

void KMeans::CalculateCentroids()
{
	int k = 0;
	for (const auto& cp : m_clusterMap)
	{
		int mx = -1, my = -1, n = 0;
		for (auto p : cp.second)
		{
			mx += p.x;
			my += p.y;
			n++;
		}
		m_centroids[k][0] = mx / n;
		m_centroids[k][1] = my / n;
		k++;
	}
}

