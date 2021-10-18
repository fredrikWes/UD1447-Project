#pragma once
#include "MayaIncludes.h"
#include "SharedMemory.h"
#include <vector>

inline Vertex ProcessVertex(MFnMesh& mesh, UINT polygonID, UINT vertexID, MIntArray vertexList)
{
	float u, v;
	MVector normal;
	MPoint position;

	mesh.getPolygonUV(polygonID, vertexID, u, v);
	mesh.getPolygonNormal(polygonID, normal);

	UINT meshRelativeIndex = vertexList[vertexID];
	mesh.getPoint(meshRelativeIndex, position);

	Vertex vertex
	{
		position.x, position.y, position.z,
		normal.x, normal.y, normal.z,
		u, v
	};

	return vertex;
}

inline void ProcessTriangle(MFnMesh& mesh, UINT polygonID, MIntArray vertexList, UINT numVertices, std::vector<Vertex>& vertices)
{
	for (UINT j = numVertices; j > 0; --j)
		vertices.emplace_back(ProcessVertex(mesh, polygonID, j - 1, vertexList));
}

inline void ProcessQuad(MFnMesh& mesh, UINT polygonID, MIntArray vertexList, UINT numVertices, std::vector<Vertex>& vertices)
{
	const UINT IDs[] = { 2, 3, 0, 3, 1, 0 };

	for (auto& ID : IDs)
		vertices.emplace_back(ProcessVertex(mesh, polygonID, ID, vertexList));
}

inline void ProcessMesh(MFnMesh& mesh, std::vector<Vertex>& vertices)
{
	for (UINT i = 0; i < mesh.numPolygons(); ++i)
	{
		MIntArray vertexList;

		mesh.getPolygonVertices(i, vertexList);

		UINT numVertices = vertexList.length();

		if (numVertices == 3)
			ProcessTriangle(mesh, i, vertexList, numVertices, vertices);

		else if (numVertices == 4)
			ProcessQuad(mesh, i, vertexList, numVertices, vertices);

		else
			cout << ">>N-GONS NOT SUPPORTED<<\n";
	}
}