#include <iostream>
#include "SharedMemory.h"
#include "MayaIncludes.h"
#include <queue>

using namespace std;

MCallbackIdArray callbackIdArray;

MStatus status = MS::kSuccess;

double timeElapsed = 0;

SharedMemory memory;

queue<Message*> messages;

std::map<string, MPointArray*> vertexCache;

#undef SendMessage
bool SendMessage(Message* message)
{
	void* data;
	bool sent = false;

	auto nodeAdded = dynamic_cast<Message*>(message);
	if (nodeAdded)
	{
		data = nodeAdded->Data();
		sent = memory.Send(data, nodeAdded->Size());
		delete data;
	}

	return sent;
}

//MATERIAL CHANGED
void MaterialChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData)
{
	if (msg & MNodeMessage::AttributeMessage::kAttributeSet)
	{
		cout << "\n============================= MATERIAL CHANGED =============================" << endl;
		cout << plug.name() << endl;

		MFnPhongShader shader(plug.node());
		cout << shader.absoluteName() << endl;
	}

	if (msg & MNodeMessage::AttributeMessage::kConnectionMade || msg & MNodeMessage::AttributeMessage::kConnectionBroken)
	{
		cout << "\n============================= MATERIAL TEXTURE CONNECTION CHANGED =============================" << endl;
		cout << plug.name() << endl;

		MFnPhongShader shader(plug.node());
		cout << shader.absoluteName() << endl;
	}
}

void TransformChanged(/*ARGS*/)
{

}

void MeshChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* clientData)
{
	std::string plugInfo(plug.info().asChar());

	MObject node = plug.node(&status);
	if (status != MS::kSuccess)
		return;

	MFnMesh mesh(node, &status);
	if (status != MS::kSuccess)
		return;

	string nodeName = MFnDependencyNode(node, &status).name().asChar();
	if (status != MS::kSuccess)
		return;

	if (plugInfo.find(".instObjGroups") != std::string::npos)
	{
		cout << "\n============================= MATERIAL CONNECTION CHANGED =============================" << endl;
		MDagPath path;
		status = mesh.getPath(path);
		if (status != MS::kSuccess)
			return;

		int inst = 0;
		if (path.isInstanced())
			inst = path.instanceNumber();

		MObjectArray shaders;
		MIntArray indices;
	}

	if (msg & MNodeMessage::AttributeMessage::kAttributeEval && plugInfo.find(".outMesh") != std::string::npos)
	{
		cout << "\n============================= GEOMETRY CHANGED =============================" << endl;

		MIntArray localIndex;

		MIntArray triangles;
		MIntArray triangleVertices;
		status = mesh.getTriangles(triangles, triangleVertices);
		if (status != MS::kSuccess)
			return;

		MIntArray vertexCount;
		MIntArray vertexList;
		status = mesh.getVertices(vertexCount, vertexList);
		if (status != MS::kSuccess)
			return;

		MFloatVectorArray normals;
		status = mesh.getNormals(normals);
		if (status != MS::kSuccess)
			return;

		MFloatArray us;
		MFloatArray vs;
		status = mesh.getUVs(us, vs);
		if (status != MS::kSuccess)
			return;

		cout << normals.length() << endl;
		cout << normals;
		cout << us.length() << endl;
		cout << us;
		cout << vs.length() << endl;
		cout << vs;

		MPointArray vertices;
		mesh.getPoints(vertices);

		for (size_t localVertTri = 0; localVertTri < triangleVertices.length(); ++localVertTri)
		{
			for (size_t localVertPoly = 0; localVertPoly < vertexList.length(); ++localVertPoly)
			{
				if (triangleVertices[localVertTri] == vertexList[localVertPoly])
				{
					localIndex.append(localVertPoly);
					break;
				}
			}
		}

		for (UINT i = 0; i < mesh.numVertices(); ++i)
		{
			Vertex vertex = {};
			
			vertex.Px = vertices[i].x;
			vertex.Py = vertices[i].y;
			vertex.Pz = vertices[i].z;

			vertex.nX = normals[i].x;
			vertex.nY = normals[i].y;
			vertex.nZ = normals[i].z;

			vertex.u = us[i];
			vertex.v = vs[i];
		}

		//Message* message = new OutMeshChangedMessage(NODETYPE::MESH, MESSAGETYPE::CHANGED, nodeName.length(), (char*)nodeName.c_str(), ATTRIBUTETYPE::GEOMETRY, nullptr, 0, nullptr, 0);
		//messages.push(message);
		//cout << vertices << endl;
		//cout << localIndex << endl;
	}
}

//NODE ADDED
void NodeAdded(MObject& node, void* clientData)
{
	if (node.isNull())
		return;

	bool found = false;
	MString nodeName = MFnDependencyNode(node).name();
	cout << node.apiTypeStr() << endl;

	switch (node.apiType())
	{
		case MFn::Type::kMesh:
			found = true;
			messages.push(new Message(NODETYPE::MESH, MESSAGETYPE::ADDED, nodeName.numChars(), (char*)nodeName.asChar()));
			callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(node, MeshChanged, NULL, &status));
			break;

		case MFn::Type::kTransform:
			found = true;
			//ADD TRANSFORM ADDED MESSAGE
			break;

		case MFn::Type::kPhong: //ONLY ACCEPTS PHONG AS MATERIAL
			found = true;
			cout << node.apiTypeStr() << endl;
			callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(node, MaterialChanged, NULL, &status));
			break;
	}

	if (!found)
		return;

	cout << "\n============================= NODE ADDED =============================" << endl;
	cout << "ADDED NODE: " << MFnDependencyNode(node).name() << endl;
}

//NODE REMOVED
void NodeRemoved(MObject& node, void* clientData)
{
	cout << "\n============================= NODE REMOVED =============================" << endl;
	cout << "REMOVED NODE: " << MFnDependencyNode(node).name() << endl;
}

//TIMER
void TimerCallback(float elapsedTime, float lastTime, void* clientData)
{
	if (!messages.empty())
	{
		bool sent = SendMessage(messages.front());
		if (sent)
			messages.pop();
	}
}

//INITIALIZE
EXPORT MStatus initializePlugin(MObject obj)
{
	MStatus res = MS::kSuccess;

	MFnPlugin myPlugin(obj, "level editor", "1.0", "Any", &res);

	if (MFAIL(res))
	{
		CHECK_MSTATUS(res);
		return res;
	}

	std::cout.set_rdbuf(MStreamUtils::stdOutStream().rdbuf());
	std::cerr.set_rdbuf(MStreamUtils::stdErrorStream().rdbuf());

	cout << "============================= >>>> PLUGIN LOADED <<<< =============================" << endl;

	callbackIdArray.append(MDGMessage::addNodeAddedCallback(NodeAdded, "dependNode", NULL, &status));
	if (status != MS::kSuccess)
		return status;

	/*callbackIdArray.append(MDGMessage::addNodeRemovedCallback(NodeRemoved, "dependNode", NULL, &status));
	if (status != MS::kSuccess)
		return status;*/

	callbackIdArray.append(MTimerMessage::addTimerCallback(1.0, TimerCallback, NULL, &status));
	if (status != MS::kSuccess)
		return status;

	return res;
}

//UNINITIALIZE
EXPORT MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	cout << "============================= >>>> PLUGIN UNLOADED <<<< =============================" << endl;

	MMessage::removeCallbacks(callbackIdArray);

	return MS::kSuccess;
}