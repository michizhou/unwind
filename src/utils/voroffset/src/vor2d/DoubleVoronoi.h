#pragma once

////////////////////////////////////////////////////////////////////////////////
#include "vor2d/Common.h"
#include "vor2d/Image.h"
#include <iostream>
#include <vector>
#include <array>
#include <set>
////////////////////////////////////////////////////////////////////////////////

namespace voroffset
{

	struct VoronoiMorphoF 
	{
		////////////////
		// Data types //
		// A horizontal segment
		struct SegmentF 
		{
			double x, y1, y2;

			SegmentF(double _x, double _y1, double _y2)
				: x(_x), y1(_y1), y2(_y2)
			{ }

			inline PointF left()  const { return PointF(x, y1); }
			inline PointF right() const { return PointF(x, y2); }
			inline PointF any()   const { return PointF(x, y1); }

			bool operator <(const SegmentF &s) const 
			{
				return (y1 + y2 < s.y1 + s.y2) || (y1 + y2 == s.y1 + s.y2 && x < s.x);
			}

			inline bool contains(const SegmentF &o) const 
			{
				return y1 <= o.y1 && o.y2 <= y2;
			}

			inline bool isPoint() const
			{
				return y1 == y2;
			}
		};
		////////////////
		//////////////////
		// Data members //
		//////////////////

		// Constant parameters for the current dilation
		const int m_XMax, m_YMax;
		const double m_Radius;

		// Seeds above sorted by ascending y of their midpoint
		std::set<SegmentF> m_S;

		// Candidate Voronoi vertices sorted by ascending x
		std::vector<std::vector<SegmentF> > m_Q;

		// Typedefs
		typedef std::set<SegmentF>::const_iterator S_const_iter;

		/////////////
		// Methods //
		/////////////

		// Assumes that y_p < y_q < y_r
		double rayIntersect(PointF p, PointF q, PointF r) const;

		// Assumes that y_lp < y_ab < y_qr
		double treatSegments(SegmentF lp, SegmentF ab, SegmentF qr) const;

		// Assumes that it != m_S.end()
		void exploreLeft(S_const_iter it, SegmentF qr, int i);

		// Assumes that it != m_S.end()
		void exploreRight(S_const_iter it, SegmentF lp, int i);

		// Insert a new seed segment [(i,j1), (i,j2)]
		void insertSegment(int i, double j1, double j2);

		// Remove seeds that are not contributing anymore to the current sweep line (y==i)
		void removeInactiveSegments(int i);

		// Extract the result for the current line
		void getLine(int i, std::vector<double> &nl, int ysize);

		VoronoiMorphoF(int _xmax, int _ymax, double _radius)
			: m_XMax(_xmax)
			, m_YMax(_ymax)
			, m_Radius(_radius)
			, m_Q(m_XMax)
		{ }
	};

	////////////////////////////////////////////////////////////////////////////////

	// Forward sweep for dilation operation
	template<typename Iterator>
	void voronoiF_half_dilate(
		int xsize, int ysize, double radius,
		Iterator it, std::vector<std::vector<double> > &result)
	{
		VoronoiMorphoF voronoi(xsize, ysize, radius);
		result.assign(xsize, std::vector<double>());
		for (int i = 0; i < xsize; ++i) 
		{
			voronoi.removeInactiveSegments(i);
			vor_assert(it[i].size() % 2 == 0);
			for (int k = (int)it[i].size() - 2; k >= 0; k -= 2)
			{
				voronoi.insertSegment(i, it[i][k], it[i][k + 1]);
			}
			voronoi.getLine(i, result[i], ysize);
		}
	}

	// -----------------------------------------------------------------------------

	// Forward sweep for erosion operation
	template<typename Iterator>
	void voronoiF_half_erode(
		int xsize, int ysize, double radius,
		Iterator it, std::vector<std::vector<double> > &result)
	{
		VoronoiMorphoF voronoi(xsize, ysize + 1, radius);
		result.assign(xsize, std::vector<double>());
		if (xsize == 0) { return; }
		// Extra segment before first row
		voronoi.insertSegment(-1, -1, ysize);
		for (int i = 0; i < xsize; ++i) 
		{
			voronoi.removeInactiveSegments(i);
			vor_assert(it[i].size() % 2 == 0);
			// Flip segments + insert an extra one at the extremities
			double j2 = ysize*1.0f;
			for (int k = (int)it[i].size() - 2; k >= 0; k -= 2)
			{
				voronoi.insertSegment(i, it[i][k + 1], j2);
				j2 = it[i][k];
			}
			voronoi.insertSegment(i, -1, j2);
			// Retrieve result
			voronoi.getLine(i, result[i], ysize);
		}
	}

	////////////////////////////////////////////////////////////////////////////////

} // namespace voroffset

