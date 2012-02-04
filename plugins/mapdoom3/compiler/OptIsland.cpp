#include "OptIsland.h"

#include "itextstream.h"
#include "OptUtils.h"

namespace map
{

OptIsland::OptIsland(ProcOptimizeGroup& group, 
					 std::vector<OptVertex>& vertices, 
					 std::vector<OptEdge>& edges) :
	_group(group),
	_verts(NULL),
	_edges(NULL),
	_tris(NULL),
	_optVerts(vertices),
	_optEdges(edges)
{
	linkVerts();
	linkEdges();
}

void OptIsland::linkVerts()
{
	// link everything together
	for (std::size_t i = 0; i < _optVerts.size(); ++i)
	{
		_optVerts[i].islandLink = _verts;
		_verts = &_optVerts[i];
	}
}

void OptIsland::linkEdges()
{
	for (std::size_t i = 0; i < _optEdges.size(); ++i)
	{
		_optEdges[i].islandLink = _edges;
		_edges = &_optEdges[i];
	}
}

bool OptIsland::pointInTri(const Vector3& p, const ProcTri& tri)
{
	// the normal[2] == 0 case is not uncommon when a square is triangulated in
	// the opposite manner to the original

	Vector3 d1 = tri.optVert[0]->pv - p;
	Vector3 d2 = tri.optVert[1]->pv - p;

	Vector3 normal = d1.crossProduct(d2);

	if (normal[2] < 0)
	{
		return false;
	}

	d1 = tri.optVert[1]->pv - p;
	d2 = tri.optVert[2]->pv - p;
	normal = d1.crossProduct(d2);

	if (normal[2] < 0)
	{
		return false;
	}

	d1 = tri.optVert[2]->pv - p;
	d2 = tri.optVert[0]->pv - p;
	normal = d1.crossProduct(d2);

	if (normal[2] < 0)
	{
		return false;
	}

	return true;
}

void OptIsland::linkTriToEdge(OptTri& optTri, OptEdge& edge)
{
	if ((edge.v1 == optTri.v[0] && edge.v2 == optTri.v[1]) || 
		(edge.v1 == optTri.v[1] && edge.v2 == optTri.v[2]) || 
		(edge.v1 == optTri.v[2] && edge.v2 == optTri.v[0]))
	{
		if (edge.backTri)
		{
			globalOutputStream() << "Warning: linkTriToEdge: already in use" << std::endl;
			return;
		}

		edge.backTri = &optTri;
		return;
	}

	if ((edge.v1 == optTri.v[1] && edge.v2 == optTri.v[0]) || 
		(edge.v1 == optTri.v[2] && edge.v2 == optTri.v[1]) || 
		(edge.v1 == optTri.v[0] && edge.v2 == optTri.v[2]))
	{
		if (edge.frontTri)
		{
			globalOutputStream() << "Warning: linkTriToEdge: already in use" << std::endl;
			return;
		}

		edge.frontTri = &optTri;
		return;
	}

	globalErrorStream() << "linkTriToEdge: edge not found on tri" << std::endl;
}

void OptIsland::createOptTri(OptVertex* first, OptEdge* e1, OptEdge* e2)
{
	OptVertex* second = NULL;
	OptVertex* third = NULL;

	if (e1->v1 == first)
	{
		second = e1->v2;
	}
	else if (e1->v2 == first)
	{
		second = e1->v1;
	}
	else
	{
		globalErrorStream() << "createOptTri: mislinked edge" << std::endl;
		return;
	}

	if (e2->v1 == first)
	{
		third = e2->v2;
	} 
	else if (e2->v2 == first)
	{
		third = e2->v1;
	} 
	else 
	{
		globalErrorStream() << "createOptTri: mislinked edge" << std::endl;
		return;
	}

	if (!OptUtils::IsTriangleValid(first, second, third))
	{
		globalErrorStream() << "createOptTri: invalid" << std::endl;
		return;
	}

#if 0
//DrawEdges( island );

		// identify the third edge
	if ( dmapGlobals.drawflag ) {
		qglColor3f(1,1,0);
		qglBegin( GL_LINES );
		qglVertex3fv( e1->v1->pv.ToFloatPtr() );
		qglVertex3fv( e1->v2->pv.ToFloatPtr() );
		qglEnd();
		qglFlush();
		qglColor3f(0,1,1);
		qglBegin( GL_LINES );
		qglVertex3fv( e2->v1->pv.ToFloatPtr() );
		qglVertex3fv( e2->v2->pv.ToFloatPtr() );
		qglEnd();
		qglFlush();
	}
#endif

	OptEdge* opposite = NULL;

	for (opposite = second->edges; opposite; )
	{
		if (opposite != e1 && (opposite->v1 == third || opposite->v2 == third))
		{
			break;
		}

		if (opposite->v1 == second)
		{
			opposite = opposite->v1link;
		}
		else if (opposite->v2 == second)
		{
			opposite = opposite->v2link;
		}
		else
		{
			globalErrorStream() << "createOptTri: invalid" << std::endl;
			return;
		}
	}

	if (!opposite)
	{
		globalErrorStream() << "Warning: createOptTri: couldn't locate opposite" << std::endl;
		return;
	}

#if 0
	if ( dmapGlobals.drawflag ) {
		qglColor3f(1,0,1);
		qglBegin( GL_LINES );
		qglVertex3fv( opposite->v1->pv.ToFloatPtr() );
		qglVertex3fv( opposite->v2->pv.ToFloatPtr() );
		qglEnd();
		qglFlush();
	}
#endif

	// create new triangle
	_tris.push_back(OptTriPtr(new OptTri));
	OptTri& optTri = *_tris.back();

	optTri.v[0] = first;
	optTri.v[1] = second;
	optTri.v[2] = third;
	optTri.midpoint = (optTri.v[0]->pv + optTri.v[1]->pv + optTri.v[2]->pv ) * ( 1.0f / 3.0f );

#if 0
	if ( dmapGlobals.drawflag ) {
		qglColor3f( 1, 1, 1 );
		qglPointSize( 4 );
		qglBegin( GL_POINTS );
		qglVertex3fv( optTri->midpoint.ToFloatPtr() );
		qglEnd();
		qglFlush();
	}
#endif

	// find the midpoint, and scan through all the original triangles to
	// see if it is inside any of them
	ProcTris::const_iterator tri;
	for (tri = _group.triList.begin(); tri != _group.triList.end(); ++tri)
	{
		if (pointInTri(optTri.midpoint, *tri))
		{
			break;
		}
	}

	if (tri != _group.triList.end())
	{
		optTri.filled = true;
	}
	else
	{
		optTri.filled = false;
	}

#if 0
	if ( dmapGlobals.drawflag ) {
		if ( optTri->filled ) {
			qglColor3f( ( 128 + orandom.RandomInt( 127 ) )/ 255.0, 0, 0 );
		} else {
			qglColor3f( 0, ( 128 + orandom.RandomInt( 127 ) ) / 255.0, 0 );
		}
		qglBegin( GL_TRIANGLES );
		qglVertex3fv( optTri->v[0]->pv.ToFloatPtr() );
		qglVertex3fv( optTri->v[1]->pv.ToFloatPtr() );
		qglVertex3fv( optTri->v[2]->pv.ToFloatPtr() );
		qglEnd();
		qglColor3f( 1, 1, 1 );
		qglBegin( GL_LINE_LOOP );
		qglVertex3fv( optTri->v[0]->pv.ToFloatPtr() );
		qglVertex3fv( optTri->v[1]->pv.ToFloatPtr() );
		qglVertex3fv( optTri->v[2]->pv.ToFloatPtr() );
		qglEnd();
		qglFlush();
	}
#endif

	// link the triangle to its edges
	linkTriToEdge(optTri, *e1);
	linkTriToEdge(optTri, *e2);
	linkTriToEdge(optTri, *opposite);
}

void OptIsland::buildOptTriangles()
{
	// free them
	_tris.clear();

	// clear the vertex emitted flags
	for (OptVertex* ov = _verts; ov; ov = ov->islandLink)
	{
		ov->emitted = false;
	}

	// clear the edge triangle links
	for (OptEdge* check = _edges; check; check = check->islandLink)
	{
		check->frontTri = check->backTri = NULL;
	}

	// check all possible triangle made up out of the
	// edges coming off the vertex
	for (OptVertex* ov = _verts; ov; ov = ov->islandLink)
	{
		if (!ov->edges) continue;

#if 0
		if ( dmapGlobals.drawflag && ov == (optVertex_t *)0x1845a60 ) {
			for ( e1 = ov->edges ; e1 ; e1 = e1Next ) {
				qglBegin( GL_LINES );
				qglColor3f( 0,1,0 );
				qglVertex3fv( e1->v1->pv.ToFloatPtr() );
				qglVertex3fv( e1->v2->pv.ToFloatPtr() );
				qglEnd();
				qglFlush();
				if ( e1->v1 == ov ) {
					e1Next = e1->v1link;
				} else if ( e1->v2 == ov ) {
					e1Next = e1->v2link;
				}
			}
		}
#endif

		OptEdge* e1Next = NULL;

		for (OptEdge* e1 = ov->edges; e1; e1 = e1Next)
		{
			OptVertex* second = NULL;

			if (e1->v1 == ov)
			{
				second = e1->v2;
				e1Next = e1->v1link;
			}
			else if (e1->v2 == ov)
			{
				second = e1->v1;
				e1Next = e1->v2link;
			} 
			else
			{
				globalErrorStream() << "buildOptTriangles: mislinked edge" << std::endl;
				return;
			}

			// if the vertex has already been used, it can't be used again
			if (second->emitted) continue;

			OptEdge* e2Next = NULL;

			for (OptEdge* e2 = ov->edges; e2; e2 = e2Next)
			{
				OptVertex* third = NULL;

				if (e2->v1 == ov) 
				{
					third = e2->v2;
					e2Next = e2->v1link;
				} 
				else if (e2->v2 == ov)
				{
					third = e2->v1;
					e2Next = e2->v2link;
				} 
				else
				{
					globalErrorStream() << "buildOptTriangles: mislinked edge" << std::endl;
					return;
				}

				if (e2 == e1) continue;

				// if the vertex has already been used, it can't be used again
				if (third->emitted) continue;

				// if the triangle is backwards or degenerate, don't use it
				if (!OptUtils::IsTriangleValid(ov, second, third))
				{
					continue;
				}

				// see if any other edge bisects these two, which means
				// this triangle shouldn't be used
				OptEdge* checkNext = NULL;
				OptEdge* check = NULL;

				for (check = ov->edges; check; check = checkNext)
				{
					OptVertex* middle = NULL;

					if (check->v1 == ov)
					{
						middle = check->v2;
						checkNext = check->v1link;
					} 
					else if (check->v2 == ov)
					{
						middle = check->v1;
						checkNext = check->v2link;
					} 
					else
					{
						globalErrorStream() << "buildOptTriangles: mislinked edge" << std::endl;
						return;
					}

					if (check == e1 || check == e2)
					{
						continue;
					}

					if (OptUtils::IsTriangleValid(ov, second, middle) && 
						OptUtils::IsTriangleValid(ov, middle, third))
					{
						break;	// should use the subdivided ones
					}
				}

				if (check)
				{
					continue;	// don't use it
				}

				// the triangle is valid
				createOptTri(ov, e1, e2);
			}
		}

		// later vertexes will not emit triangles that use an
		// edge that this vert has already used
		ov->emitted = true;
	}
}

void OptIsland::optimise()
{
	// add space-filling fake edges so we have a complete
	// triangulation of a convex hull before optimization
	addInteriorEdges();
	
	// determine all the possible triangles, and decide if
	// the are filled or empty
	buildOptTriangles();

	/*// remove interior vertexes that have filled triangles
	// between all their edges
	RemoveInteriorEdges( island );
	DrawEdges( island );

	ValidateEdgeCounts( island );

	// remove vertexes that only have two colinear edges
	CombineColinearEdges( island );
	CullUnusedVerts( island );
	DrawEdges( island );

	// add new internal edges between the remaining exterior edges
	// to give us a full triangulation again
	AddInteriorEdges( island );
	DrawEdges( island );

	// determine all the possible triangles, and decide if
	// the are filled or empty
	BuildOptTriangles( island );

	// make mapTri_t out of the filled optTri_t
	RegenerateTriangles( island );*/
}

int OptIsland::LengthSort(const void *a, const void *b)
{
	const EdgeLength* ea = reinterpret_cast<const EdgeLength*>(a);
	const EdgeLength* eb = reinterpret_cast<const EdgeLength*>(b);

	if (ea->length < eb->length)
	{
		return -1;
	}

	if (ea->length > eb->length)
	{
		return 1;
	}

	return 0;
}

bool OptIsland::tryAddNewEdge(OptVertex* v1, OptVertex* v2)
{
	// if the new edge crosses any other edges, don't add it
	for (OptEdge* e = _edges; e; e = e->islandLink)
	{
		if (OptUtils::EdgesCross(e->v1, e->v2, v1, v2))
		{
			return false;
		}
	}

#if 0
	if ( dmapGlobals.drawflag ) {
		qglBegin( GL_LINES );
		qglColor3f( 0, ( 128 + orandom.RandomInt( 127 ) )/ 255.0, 0 );
		qglVertex3fv( v1->pv.ToFloatPtr() );
		qglVertex3fv( v2->pv.ToFloatPtr() );
		qglEnd();
		qglFlush();
	}
#endif

	// add it
	_optEdges.push_back(OptEdge());

	OptEdge& e = _optEdges.back();

	e.islandLink = _edges;
	_edges = &e;
	e.v1 = v1;
	e.v2 = v2;

	e.created = true;

	// link the edge to its verts
	e.linkToVertices();

	return true;
}

void OptIsland::addInteriorEdges()
{
	// count the verts
	std::size_t vertCount = 0;

	for (OptVertex* vert = _verts; vert; vert = vert->islandLink)
	{
		if (!vert->edges) continue;

		vertCount++;
	}

	// allocate space for all the lengths
	std::vector<EdgeLength> lengths(vertCount * vertCount / 2);

	std::size_t numLengths = 0;

	for (OptVertex* vert = _verts; vert; vert = vert->islandLink)
	{
		if (!vert->edges) continue;

		for (OptVertex* vert2 = vert->islandLink; vert2; vert2 = vert2->islandLink)
		{
			if (!vert2->edges) continue;

			lengths[numLengths].v1 = vert;
			lengths[numLengths].v2 = vert2;

			Vector3	dir = vert->pv - vert2->pv;

			lengths[numLengths].length = dir.getLength();
			numLengths++;
		}
	}

	// sort by length, shortest first
	qsort(&lengths[0], numLengths, sizeof(lengths[0]), LengthSort);

	// try to create them in that order
	std::size_t addedEdges = 0;

	for (std::size_t i = 0; i < numLengths; ++i)
	{
		if (tryAddNewEdge(lengths[i].v1, lengths[i].v2))
		{
			addedEdges++;
		}
	}

	if (false/* dmapGlobals.verbose */)
	{
		globalOutputStream() << (boost::format("%6i tested segments") % numLengths).str() << std::endl;
		globalOutputStream() << (boost::format("%6i added interior edges") % addedEdges).str() << std::endl;
	}
}

} // namespace
