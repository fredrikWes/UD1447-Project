#pragma once
#include "MayaIncludes.h"
#include "SharedMemory.h"
#include <vector>

std::map<std::string, std::vector<Vertex>> vertexCache;

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
		position.z, position.y, position.x,
		normal.x, normal.y, normal.z,
		u, 1 - v
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
	const UINT IDs[] = { 0, 3, 2, 0, 2, 1 };

	for (auto& ID : IDs)
		vertices.emplace_back(ProcessVertex(mesh, polygonID, ID, vertexList));
}

inline bool ProcessMesh(MFnMesh& mesh, std::vector<int>& indices, std::vector<Vertex>& vertices)
{
	MStatus status;
	const char* meshName = mesh.name().asChar();

	for (UINT i = 0; i < mesh.numPolygons(); ++i)
	{
		MIntArray vertexList;

		status = mesh.getPolygonVertices(i, vertexList);
		if (status != MS::kSuccess)
			return false;

		UINT numVertices = vertexList.length();

		if (numVertices == 3)
			ProcessTriangle(mesh, i, vertexList, numVertices, vertices);

		else if (numVertices == 4)
			ProcessQuad(mesh, i, vertexList, numVertices, vertices);

		else
			cout << ">>N-GONS NOT SUPPORTED<<\n";
	}

	if (vertices.empty())
		return false;

	//FIRST INITIALIZE OF MESH
	if (vertexCache.find(meshName) == vertexCache.end())
	{
		cout << "INIT MESH" << endl;
		vertexCache[meshName] = vertices;
		for (UINT i = 0; i < vertices.size(); ++i)
			indices.emplace_back(i);
		return true;
	}
	
	//MESH HAS BEEN INITIALIZED AND THEN CHANGED
	auto oldVertices = vertexCache.at(meshName);
	vertexCache[meshName] = vertices;

	//SEND ONLY THE CHANGED VERTICES AND THE INDICES

	cout << "OLD SIZE: " << oldVertices.size() << endl;
	cout << "NEW SIZE: " << vertices.size() << endl;

	for (UINT i = vertices.size(); i > 0; --i)
	{
		UINT index = i - 1;
		cout << "VERTEX: " << index << endl;

		if (i > oldVertices.size())
			indices.emplace_back(index);
		else
		{
			if (vertices[index] == oldVertices[index])
				vertices.erase(vertices.begin() + index);
			else
				indices.emplace_back(index);
		}
	}

	/*
	for (UINT i = 0; i < vertices.size(); ++i)
	{
		if (currentIndex < oldVertices.size())
		{
			if (vertices[i] == oldVertices[currentIndex])
			{
				cout << "VERTEX INDEX " << currentIndex << " WAS THE SAME AS NEW " << i << endl;
				vertices.erase(vertices.begin() + i);
				++currentIndex;
				--i;
			}

			else
			{
				cout << "CHANGED VERTEX INDEX: " << currentIndex << endl;
				indices.emplace_back(currentIndex);
			}
		}

		else
		{
			cout << "CHANGED VERTEX INDEX: " << i << endl;
			indices.emplace_back(i);
		}
	}
	*/

	if (indices.empty())
		return false;

	cout << "CHANGED " << indices.size() << " VERTICES";

	return true;
}