#include "NavMesh.h"

#include <math.h>
#include <string.h>

#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "DetourCrowd.h"

#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/MeshWorldObjectInstance.h"

namespace Ganymede
{
	NavMesh::NavMesh()
	{
		m_Agents.resize(1000);
		m_cfg = new rcConfig();
	}

	bool NavMesh::Generate(const std::vector<MeshWorldObjectInstance*> instances)
	{
		if (instances.size() == 0)
		{
			return false;
		}

		float* rc_verts;
		unsigned int rc_nverts;
		int* rc_tris;
		unsigned int rc_ntris;
		float rc_bmin[3];
		float rc_bmax[3];

		rc_bmin[0] = 0;
		rc_bmin[1] = 0;
		rc_bmin[2] = 0;
		rc_bmax[0] = 0;
		rc_bmax[1] = 0;
		rc_bmax[2] = 0;


		std::vector<glm::vec3> vertices;
		std::vector<glm::ivec3> faces;

		unsigned int vertexIndexOffset = 0;
		unsigned int vertexIndexBase = 0;

		for (const MeshWorldObjectInstance* instance : instances)
		{
			if (instance->GetMobility() != WorldObjectInstance::Mobility::Static)
			{
				// Only static geometry will be used to create the navigation mesh
				continue;
			}
			const MeshWorldObject::Mesh& mesh = *instance->GetMeshWorldObject()->m_Meshes[0];

			const auto& meshVertices = mesh.m_Vertices;
			const auto& meshIndices = mesh.m_VertexIndicies;
			const unsigned int numFaces = mesh.m_VertexIndicies.size() / 3;

			const glm::mat4 localTransform = instance->GetLocalTransform();

			for (const MeshWorldObject::Mesh::Vertex meshVertex : meshVertices)
			{
				glm::vec4 vertex = { meshVertex.m_Position.x, meshVertex.m_Position.y, meshVertex.m_Position.z, 1 };
				vertex = localTransform * vertex;

				vertices.push_back({ vertex.x, vertex.y, vertex.z });

				rc_bmin[0] = std::min(rc_bmin[0], vertex.x);
				rc_bmin[1] = std::min(rc_bmin[1], vertex.y);
				rc_bmin[2] = std::min(rc_bmin[2], vertex.z);

				rc_bmax[0] = std::max(rc_bmax[0], vertex.x);
				rc_bmax[1] = std::max(rc_bmax[1], vertex.y);
				rc_bmax[2] = std::max(rc_bmax[2], vertex.z);
			}

			for (unsigned int i = 0; i < numFaces; ++i)
			{
				const unsigned int idxX = i * 3;
				const unsigned int idxY = i * 3 + 1;
				const unsigned int idxZ = i * 3 + 2;

				faces.push_back({ meshIndices[idxX] + vertexIndexOffset, meshIndices[idxY] + vertexIndexOffset, meshIndices[idxZ] + vertexIndexOffset });
			}

			vertexIndexOffset += meshVertices.size();
		}

		rc_verts = &vertices[0].x;
		rc_tris = &faces[0].x;
		rc_nverts = vertices.size();
		rc_ntris = faces.size();

		//CleanUp();
		m_ctx = new rcContext(true);
		m_ctx->enableLog(true);
		//
		// Step 1. Initialize build config.
		//

		m_agentHeight = 2.f;
		m_agentRadius = .5f;
		m_regionMinSize = 50;
		m_regionMergeSize = 20;
		m_vertsPerPoly = 6;
		m_detailSampleDist = 12;
		m_detailSampleMaxError = 1;
		m_keepInterResults = false;

		// Init build configuration from GUI
		//memset(&m_cfg, 0, sizeof(rcConfig));
		memset(m_cfg, 0, sizeof(rcConfig));
		m_cfg->cs = m_agentRadius / 3.0f;
		m_cfg->ch = m_cfg->cs / 2.0f;
		m_cfg->walkableSlopeAngle = 45;
		m_cfg->walkableHeight = (int)ceilf(m_agentHeight / m_cfg->ch);
		m_cfg->walkableClimb = (int)ceilf((m_agentHeight * .2f) / m_cfg->ch);
		m_cfg->walkableRadius = (int)ceilf(m_agentRadius / m_cfg->cs);
		m_cfg->maxEdgeLen = (int)(m_cfg->walkableRadius * 8);
		m_cfg->maxSimplificationError = 1.3f;
		m_cfg->minRegionArea = (int)rcSqr(m_regionMinSize);		// Note: area = size*size
		m_cfg->mergeRegionArea = (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
		m_cfg->maxVertsPerPoly = (int)m_vertsPerPoly;
		m_cfg->detailSampleDist = m_detailSampleDist < 0.9f ? 0 : m_cfg->cs * m_detailSampleDist;
		m_cfg->detailSampleMaxError = m_cfg->ch * m_detailSampleMaxError;

		// Set the area where the navigation will be build.
		// Here the bounds of the input mesh are used, but the
		// area could be specified by an user defined box, etc.
		rcVcopy(m_cfg->bmin, rc_bmin);
		rcVcopy(m_cfg->bmax, rc_bmax);
		rcCalcGridSize(m_cfg->bmin, m_cfg->bmax, m_cfg->cs, &m_cfg->width, &m_cfg->height);

		// Reset build times gathering.
		m_ctx->resetTimers();

		// Start the build process.	
		m_ctx->startTimer(RC_TIMER_TOTAL);

		m_ctx->log(RC_LOG_PROGRESS, "Building navigation:");
		m_ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg->width, m_cfg->height);
		m_ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", rc_nverts / 1000.0f, rc_ntris / 1000.0f);

		//
		// Step 2. Rasterize input polygon soup.
		//

		// Allocate voxel heightfield where we rasterize our input data to.
		m_solid = rcAllocHeightfield();
		if (!m_solid)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
			return false;
		}
		if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg->width, m_cfg->height, m_cfg->bmin, m_cfg->bmax, m_cfg->cs, m_cfg->ch))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
			return false;
		}

		// Allocate array that can hold triangle area types.
		// If you have multiple meshes you need to process, allocate
		// and array which can hold the max number of triangles you need to process.
		m_triareas = new unsigned char[rc_ntris];
		if (!m_triareas)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", rc_ntris);
			return false;
		}

		// Find triangles which are walkable based on their slope and rasterize them.
		// If your input data is multiple meshes, you can transform them here, calculate
		// the are type for each of the meshes and rasterize them.
		memset(m_triareas, 0, rc_ntris * sizeof(unsigned char));
		rcMarkWalkableTriangles(m_ctx, m_cfg->walkableSlopeAngle, rc_verts, rc_nverts, rc_tris, rc_ntris, m_triareas);
		if (!rcRasterizeTriangles(m_ctx, rc_verts, rc_nverts, rc_tris, m_triareas, rc_ntris, *m_solid, m_cfg->walkableClimb))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
			return false;
		}

		if (!m_keepInterResults)
		{
			delete[] m_triareas;
			m_triareas = 0;
		}

		//
		// Step 3. Filter walkables surfaces.
		//

		// Once all geoemtry is rasterized, we do initial pass of filtering to
		// remove unwanted overhangs caused by the conservative rasterization
		// as well as filter spans where the character cannot possibly stand.
		if (m_filterLowHangingObstacles)
			rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg->walkableClimb, *m_solid);
		if (m_filterLedgeSpans)
			rcFilterLedgeSpans(m_ctx, m_cfg->walkableHeight, m_cfg->walkableClimb, *m_solid);
		if (m_filterWalkableLowHeightSpans)
			rcFilterWalkableLowHeightSpans(m_ctx, m_cfg->walkableHeight, *m_solid);


		//
		// Step 4. Partition walkable surface to simple regions.
		//

		// Compact the heightfield so that it is faster to handle from now on.
		// This will result more cache coherent data as well as the neighbours
		// between walkable cells will be calculated.
		m_chf = rcAllocCompactHeightfield();
		if (!m_chf)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
			return false;
		}
		if (!rcBuildCompactHeightfield(m_ctx, m_cfg->walkableHeight, m_cfg->walkableClimb, *m_solid, *m_chf))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
			return false;
		}

		if (!m_keepInterResults)
		{
			rcFreeHeightField(m_solid);
			m_solid = 0;
		}

		// Erode the walkable area by agent radius.
		if (!rcErodeWalkableArea(m_ctx, m_cfg->walkableRadius, *m_chf))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
			return false;
		}

		/*
		// (Optional) Mark areas.
		const ConvexVolume* vols = m_geom->getConvexVolumes();
		for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
			rcMarkConvexPolyArea(m_ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);
		*/

		// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
		// There are 3 martitioning methods, each with some pros and cons:
		// 1) Watershed partitioning
		//   - the classic Recast partitioning
		//   - creates the nicest tessellation
		//   - usually slowest
		//   - partitions the heightfield into nice regions without holes or overlaps
		//   - the are some corner cases where this method creates produces holes and overlaps
		//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
		//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
		//   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
		// 2) Monotone partioning
		//   - fastest
		//   - partitions the heightfield into regions without holes and overlaps (guaranteed)
		//   - creates long thin polygons, which sometimes causes paths with detours
		//   * use this if you want fast navmesh generation
		// 3) Layer partitoining
		//   - quite fast
		//   - partitions the heighfield into non-overlapping regions
		//   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
		//   - produces better triangles than monotone partitioning
		//   - does not have the corner cases of watershed partitioning
		//   - can be slow and create a bit ugly tessellation (still better than monotone)
		//     if you have large open areas with small obstacles (not a problem if you use tiles)
		//   * good choice to use for tiled navmesh with medium and small sized tiles

		m_partitionType = SAMPLE_PARTITION_WATERSHED;
		if (m_partitionType == SAMPLE_PARTITION_WATERSHED)
		{
			// Prepare for region partitioning, by calculating distance field along the walkable surface.
			if (!rcBuildDistanceField(m_ctx, *m_chf))
			{
				m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
				return false;
			}

			// Partition the walkable surface into simple regions without holes.
			if (!rcBuildRegions(m_ctx, *m_chf, 0, m_cfg->minRegionArea, m_cfg->mergeRegionArea))
			{
				m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
				return false;
			}
		}
		else if (m_partitionType == SAMPLE_PARTITION_MONOTONE)
		{
			// Partition the walkable surface into simple regions without holes.
			// Monotone partitioning does not need distancefield.
			if (!rcBuildRegionsMonotone(m_ctx, *m_chf, 0, m_cfg->minRegionArea, m_cfg->mergeRegionArea))
			{
				m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
				return false;
			}
		}
		else // SAMPLE_PARTITION_LAYERS
		{
			// Partition the walkable surface into simple regions without holes.
			if (!rcBuildLayerRegions(m_ctx, *m_chf, 0, m_cfg->minRegionArea))
			{
				m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
				return false;
			}
		}

		//
		// Step 5. Trace and simplify region contours.
		//

		// Create contours.
		m_cset = rcAllocContourSet();
		if (!m_cset)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
			return false;
		}
		if (!rcBuildContours(m_ctx, *m_chf, m_cfg->maxSimplificationError, m_cfg->maxEdgeLen, *m_cset))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
			return false;
		}

		//
		// Step 6. Build polygons mesh from contours.
		//

		// Build polygon navmesh from the contours.
		m_pmesh = rcAllocPolyMesh();
		if (!m_pmesh)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
			return false;
		}
		if (!rcBuildPolyMesh(m_ctx, *m_cset, m_cfg->maxVertsPerPoly, *m_pmesh))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
			return false;
		}

		//
		// Step 7. Create detail mesh which allows to access approximate height on each polygon.
		//

		m_dmesh = rcAllocPolyMeshDetail();
		if (!m_dmesh)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
			return false;
		}

		if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf, m_cfg->detailSampleDist, m_cfg->detailSampleMaxError, *m_dmesh))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
			return false;
		}

		if (!m_keepInterResults)
		{
			rcFreeCompactHeightfield(m_chf);
			m_chf = 0;
			rcFreeContourSet(m_cset);
			m_cset = 0;
		}

		// At this point the navigation mesh data is ready, you can access it from m_pmesh.
		// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.

		//
		// (Optional) Step 8. Create Detour data from Recast poly mesh.
		//

		// The GUI may allow more max points per polygon than Detour can handle.
		// Only build the detour navmesh if we do not exceed the limit.
		if (m_cfg->maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
		{
			unsigned char* navData = 0;
			int navDataSize = 0;

			// Update poly flags from areas.
			for (int i = 0; i < m_pmesh->npolys; ++i)
			{
				if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
				{
					m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
				}
			}


			dtNavMeshCreateParams params;
			memset(&params, 0, sizeof(params));

			params.verts = m_pmesh->verts;
			params.vertCount = m_pmesh->nverts;
			params.polys = m_pmesh->polys;
			params.polyAreas = m_pmesh->areas;
			params.polyFlags = m_pmesh->flags;
			params.polyCount = m_pmesh->npolys;
			params.nvp = m_pmesh->nvp;
			params.detailMeshes = m_dmesh->meshes;
			params.detailVerts = m_dmesh->verts;
			params.detailVertsCount = m_dmesh->nverts;
			params.detailTris = m_dmesh->tris;
			params.detailTriCount = m_dmesh->ntris;

			// no off mesh connections yet
			m_offMeshConCount = 0;
			params.offMeshConVerts = m_offMeshConVerts;
			params.offMeshConRad = m_offMeshConRads;
			params.offMeshConDir = m_offMeshConDirs;
			params.offMeshConAreas = m_offMeshConAreas;
			params.offMeshConFlags = m_offMeshConFlags;
			params.offMeshConUserID = m_offMeshConId;
			params.offMeshConCount = m_offMeshConCount;

			params.walkableHeight = m_agentHeight;
			params.walkableRadius = m_agentRadius;
			params.walkableClimb = m_agentMaxClimb;
			rcVcopy(params.bmin, m_pmesh->bmin);
			rcVcopy(params.bmax, m_pmesh->bmax);
			params.cs = m_cfg->cs;
			params.ch = m_cfg->ch;
			params.buildBvTree = true;

			if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
			{
				m_ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
				return false;
			}

			m_navMesh = dtAllocNavMesh();
			if (!m_navMesh)
			{
				dtFree(navData);
				m_ctx->log(RC_LOG_ERROR, "Could not create Detour navmesh");
				return false;
			}

			dtStatus status;

			status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
			if (dtStatusFailed(status))
			{
				dtFree(navData);
				m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh");
				return false;
			}

			m_navQuery = dtAllocNavMeshQuery();
			status = m_navQuery->init(m_navMesh, 2048);
			if (dtStatusFailed(status))
			{
				m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
				return false;
			}

			MeshWorldObject::Mesh dMesh;
			//Renderer::DebugDrawMesh* ddMesh = new Renderer::DebugDrawMesh();

			const int numPolys = m_pmesh->npolys;
			const int nvp = m_pmesh->nvp;
			unsigned int vertidx = 0;

			for (unsigned int i = 0; i < numPolys; ++i)
			{
				const unsigned short* poly = &m_pmesh->polys[i * 2 * nvp];

				unsigned short vi[3];
				for (int j = 2; j < nvp; ++j) // go through all verts in the polygon
				{
					if (poly[j] == RC_MESH_NULL_IDX) break;
					vi[0] = poly[0];
					vi[1] = poly[j - 1];
					vi[2] = poly[j];
					for (int k = 0; k < 3; ++k) // create a 3-vert triangle for each 3 verts in the polygon.
					{
						const unsigned short* v = &m_pmesh->verts[vi[k] * 3];
						const float x = m_pmesh->bmin[0] + v[0] * m_pmesh->cs;
						const float y = m_pmesh->bmin[1] + (v[1] + 1) * m_pmesh->ch;
						const float z = m_pmesh->bmin[2] + v[2] * m_pmesh->cs;

						MeshWorldObject::Mesh::Vertex vertex;
						vertex.m_Position = { x, y, z };
						dMesh.m_Vertices.push_back(vertex);

						dMesh.m_VertexIndicies.push_back(vertidx);

						//ddMesh->m_VertexPositions.push_back({x, y, z});
						//ddMesh->m_VertexIndices.push_back(vertidx);
						++vertidx;
					}
				}


			}

			m_Crowd = dtAllocCrowd();
			m_Crowd->init(m_Agents.size(), 2.f, m_navMesh);
		}

		m_ctx->stopTimer(RC_TIMER_TOTAL);
	}

	// 0 for the slot to save the path too, and 0 as the "target", to remember what the path was for.
	// will return a negative number if the search for a path failed.
	int NavMesh::FindPath(float* pStartPos, float* pEndPos, int nPathSlot, int nTarget, std::vector<glm::vec3>& pathOut)
	{
		std::scoped_lock lock(m_Mutex);

		pathOut.clear();

		dtStatus status;
		float pExtents[3] = { 32.0f, 32.0f, 32.0f }; // size of box around start/end points to look for nav polygons
		dtPolyRef StartPoly;
		float StartNearest[3];
		dtPolyRef EndPoly;
		float EndNearest[3];
		dtPolyRef PolyPath[MAX_PATHPOLY];
		int nPathCount = 0;
		float StraightPath[MAX_PATHVERT * 3];
		int nVertCount = 0;


		// setup the filter
		dtQueryFilter Filter;
		Filter.setIncludeFlags(0xFFFF);
		Filter.setExcludeFlags(0);
		Filter.setAreaCost(SAMPLE_POLYAREA_GROUND, 1.0f);

		// find the start polygon
		status = m_navQuery->findNearestPoly(pStartPos, pExtents, &Filter, &StartPoly, StartNearest);
		if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -1; // couldn't find a polygon

		// find the end polygon
		status = m_navQuery->findNearestPoly(pEndPos, pExtents, &Filter, &EndPoly, EndNearest);
		if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -2; // couldn't find a polygon

		status = m_navQuery->findPath(StartPoly, EndPoly, StartNearest, EndNearest, &Filter, PolyPath, &nPathCount, MAX_PATHPOLY);
		if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -3; // couldn't create a path
		if (nPathCount == 0) return -4; // couldn't find a path

		status = m_navQuery->findStraightPath(StartNearest, EndNearest, PolyPath, nPathCount, StraightPath, NULL, NULL, &nVertCount, MAX_PATHVERT);
		if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -5; // couldn't create a path
		if (nVertCount == 0) return -6; // couldn't find a path

		// At this point we have our path.  Copy it to the path store
		int nIndex = 0;
		for (int nVert = 0; nVert < nVertCount; nVert++)
		{
			m_PathStore[nPathSlot].PosX[nVert] = StraightPath[nIndex++];
			m_PathStore[nPathSlot].PosY[nVert] = StraightPath[nIndex++];
			m_PathStore[nPathSlot].PosZ[nVert] = StraightPath[nIndex++];

			//sprintf(m_chBug, "Path Vert %i, %f %f %f", nVert, m_PathStore[nPathSlot].PosX[nVert], m_PathStore[nPathSlot].PosY[nVert], m_PathStore[nPathSlot].PosZ[nVert]) ;
			//m_pLog->logMessage(m_chBug);
		}
		m_PathStore[nPathSlot].MaxVertex = nVertCount;
		m_PathStore[nPathSlot].Target = nTarget;

		for (int i = 0; i < nVertCount; ++i)
		{
			const glm::vec3 pos = { m_PathStore->PosX[i],
			m_PathStore->PosY[i],
			m_PathStore->PosZ[i] };

			pathOut.push_back(pos);
		}

		return nVertCount;
	}

	void NavMesh::NavigateAgentToDestination(int agentID, glm::vec3 to)
	{
		std::scoped_lock lock(m_Mutex);


		dtPolyRef target_poly;

		dtStatus status;
		float pExtents[3] = { 32.0f, 32.0f, 32.0f }; // size of box around start/end points to look for nav polygons
		dtPolyRef DestPoly;
		float DestNearest[3];


		// setup the filter
		dtQueryFilter Filter;
		Filter.setIncludeFlags(0xFFFF);
		Filter.setExcludeFlags(0);
		Filter.setAreaCost(SAMPLE_POLYAREA_GROUND, 1.0f);

		//const float* currentAgenetPos = m_Crowd->getAgent(agentID)->npos;

		// find the start polygon
		status = m_navQuery->findNearestPoly(&to.x, pExtents, &Filter, &DestPoly, DestNearest);
		if (!dtStatusSucceed(status))
			return;

		m_Crowd->requestMoveTarget(agentID, DestPoly, &to.x);


	}

	namespace NavMesh_Private
	{
		float GetRandomSeed()
		{
			return 0.0f;
		}
	}

	bool NavMesh::GetRandomPointOnNavMesh(glm::vec3& pointOut, glm::vec3 center, float distance)
	{
		std::scoped_lock lock(m_Mutex);
		using namespace NavMesh_Private;

		if (m_navQuery == nullptr)
			return false;

		//const dtQueryFilter filter; // Use default query filter
		const dtQueryFilter filter; // Use default query filter
		float randPoint[3];

		dtPolyRef randPoly;
		const dtStatus status = m_navQuery->findRandomPoint(&filter, &GetRandomSeed, &randPoly, randPoint);

		if (status & DT_FAILURE)
			return false;

		pointOut.x = randPoint[0];
		pointOut.y = randPoint[1];
		pointOut.z = randPoint[2];

		return true;
	}

	void NavMesh::UpdateCrowd(float deltaTime)
	{
		m_Crowd->update(deltaTime, 0);
	}

	void NavMesh::CleanUp()
	{
		delete[] m_triareas;
		m_triareas = 0;
		rcFreeHeightField(m_solid);
		m_solid = 0;
		rcFreeCompactHeightfield(m_chf);
		m_chf = 0;
		rcFreeContourSet(m_cset);
		m_cset = 0;
		rcFreePolyMesh(m_pmesh);
		m_pmesh = 0;
		rcFreePolyMeshDetail(m_dmesh);
		m_dmesh = 0;
		dtFreeNavMesh(m_navMesh);
		m_navMesh = 0;
	}



	int NavMesh::TryRegisterAgent(const glm::vec3& startPosition)
	{
		std::scoped_lock lock(m_Mutex);

		for (int agentID = 0; agentID < m_Agents.size(); ++agentID)
		{
			if (!m_Agents[agentID].m_InUse)
			{
				m_Agents[agentID].m_InUse = true;
				m_Agents[agentID].m_Position = startPosition;
				dtCrowdAgentParams params;
				params.radius = .7f;
				params.height = 2.f;
				params.maxAcceleration = 5.f;
				params.maxSpeed = 2.5f;
				params.collisionQueryRange = params.radius * 2.f;
				params.pathOptimizationRange = params.radius * 10.0f;
				params.separationWeight = 20.0f;
				params.obstacleAvoidanceType = 2;
				params.updateFlags = DT_CROWD_ANTICIPATE_TURNS | DT_CROWD_OPTIMIZE_VIS | DT_CROWD_OPTIMIZE_TOPO | DT_CROWD_OBSTACLE_AVOIDANCE;

				m_Crowd->addAgent(&startPosition.x, &params);

				return agentID;
			}
		}
		return -1;
	};

	bool NavMesh::TryUnregisterAgent(int agentID)
	{
		if (agentID > -1 && agentID < m_Agents.size())
		{
			m_Agents[agentID].Reset();
			m_Crowd->removeAgent(agentID);
			return true;
		}

		return false;
	}
}