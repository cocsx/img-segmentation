#pragma once

#include <vector>
#include <unordered_map>

#include "Image.h"

struct Point
{
	int x, y;
	Point(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	bool operator==(const Point &p) const
	{
		return x == p.x && y == p.y;
	}
};

namespace std {
	template <>
	struct hash<Point>
	{
		std::size_t operator()(const Point& p) const
		{
			using std::size_t;
			using std::hash;
			return (hash<int>()(p.x)^ (hash<int>()(p.y)));
		}
	};
}

static int DistanceBetweenTwoPixelsColor(const Pixel& p1, const Pixel& p2)
{
	return (p1.r-p2.r)*(p1.r-p2.r) + (p1.g-p2.g)*(p1.g-p2.g) + (p1.b-p2.b)*(p1.b-p2.b);
}


class KMeans
{
public:
	explicit KMeans(Image& img);
	KMeans(Image& img, int maxIter);

	void Partition();

private:
	void Init();

	void MakeClusters();
	void CalculateCentroids();
private:
	Image& m_image;

	static const int m_numOfClusters = 10;
	int m_maxIterations = 50;

	using ClusterMap = std::unordered_map<Point, std::vector<Point>>;

	ClusterMap m_clusterMap;
	int m_centroids[m_numOfClusters][2];
};

