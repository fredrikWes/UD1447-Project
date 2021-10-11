#include "Messages.h"
#include <iostream>

using namespace std;

MCallbackIdArray callbackIdArray;

MStatus status = MS::kSuccess;

double timeElapsed = 0;

//map<string, MPointArray*> vertices;

SharedMemory memory;
//
////MESH CHANGED
//void OutMeshChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* clientData)
//{
//	MObject node = plug.node(&status);
//	if (status != MS::kSuccess)
//		return;
//
//	string nodeName = MFnDependencyNode(node, &status).name().asChar();
//	if (status != MS::kSuccess)
//		return;
//
//	string plugName = plug.info().asChar();
//
//	if (msg & MNodeMessage::kAttributeEval && plug.partialName() == "o")
//	{
//		MFnMesh newMesh(node, &status);
//		if (status != MS::kSuccess)
//			return;
//
//		MPointArray* oldVertices = vertices.at(nodeName);
//		MPointArray* newVertices = new MPointArray();
//
//		status = newMesh.getPoints(*newVertices);
//		if (status != MS::kSuccess)
//			return;
//
//		string outStr = "";
//		auto& n = *newVertices;
//		auto& o = *oldVertices;
//		for (UINT i = 0; i < newVertices->length(); ++i)
//		{
//			if (oldVertices->length() > i)
//			{
//				if (n[i] != o[i])
//					outStr += "\nCHANGED VERTEX [" + std::to_string(i) + "] >> " + std::to_string(n[i].x) + ", " + std::to_string(n[i].y) + ", " + std::to_string(n[i].z) + ", " + std::to_string(n[i].w) + "\n";
//			}
//
//			else
//				outStr += "\nCHANGED VERTEX [" + std::to_string(i) + "] >> " + std::to_string(n[i].x) + ", " + std::to_string(n[i].y) + ", " + std::to_string(n[i].z) + ", " + std::to_string(n[i].w) + "\n";
//		}
//
//		if (outStr.empty())
//			return;
//
//		cout << "\n============================= MESH CHANGE =============================" << endl;
//		cout << "TRIGGER NODE: " << nodeName << endl;
//
//		if (oldVertices->length() != newVertices->length())
//			cout << "LAST VERTEX COUNT: " << oldVertices->length() << " NEW VERTEX COUNT: " << newVertices->length() << endl;
//
//		cout << outStr;
//
//		delete vertices[nodeName];
//		vertices[nodeName] = nullptr;
//		vertices[nodeName] = newVertices;
//	}
//}
//
////TRANSFORM CHANGED
//void TransformChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* clientData)
//{
//	MObject node = plug.node(&status);
//	if (status != MS::kSuccess)
//		return;
//
//	string nodeName = MFnDependencyNode(node, &status).name().asChar();
//	if (status != MS::kSuccess)
//		return;
//
//	MFnTransform transform(node, &status);
//
//	if (status != MS::kSuccess)
//		return;
//
//	MTransformationMatrix localMatrix = transform.transformationMatrix(&status);
//	if (status != MS::kSuccess)
//		return;
//
//	MFnDagNode dagNode(node, &status);
//	if (status != MS::kSuccess)
//		return;
//
//	MDagPath dagPath;
//	dagNode.getPath(dagPath);
//
//	MMatrix worldMatrix = dagPath.inclusiveMatrix(&status);
//
//	if (status != MS::kSuccess)
//		return;
//
//	cout << "\n============================= TRANSFORM CHANGE =============================" << endl;
//	cout << "TRIGGER NODE: " << nodeName << " >> " << plug.info() << endl;
//	cout << "\nLOCAL:\n" << localMatrix.asMatrix() << endl;
//	cout << "\nGLOBAL:\n" << worldMatrix << endl;
//}
//
////LOCAL MATRIX CHANGED
//void LocalMatrixChanged(MObject& transformNode, MDagMessage::MatrixModifiedFlags& modified, void* clientData)
//{
//	MMatrix matrix = MFnDagNode(transformNode).transformationMatrix();
//	cout << "\n============================= LOCAL TRANSFORM CHANGE =============================" << endl;
//	cout << "TRIGGER NODE: " << MFnDependencyNode(transformNode).name() << endl;
//	cout << matrix << endl;
//}
//
////WORLD MATRIX CHANGED
//void GlobalMatrixChanged(MObject& transformNode, MDagMessage::MatrixModifiedFlags& modified, void* clientData)
//{
//	MDagPath dagPath;
//	MFnDagNode(transformNode).getPath(dagPath);
//
//	MMatrix matrix = MFnDagNode(transformNode).transformationMatrix(&status);
//
//	if (status != MS::kSuccess)
//		return;
//
//	cout << "\n============================= GLOBAL TRANSFORM CHANGE =============================" << endl;
//	cout << "TRIGGER NODE: " << MFnDependencyNode(transformNode).name() << endl;
//	cout << matrix << endl;
//}
//
////NAME CHANGED
//void NameChanged(MObject& node, const MString& str, void* clientData)
//{
//	string oldName = str.asChar();
//	string nodeName = MFnDependencyNode(node).name().asChar();
//
//
//	cout << "\n============================= NAME CHANGE =============================" << endl;
//	cout << "TRIGGER NODE: " << nodeName << endl;
//	cout << "NEW NAME: " << nodeName << "\nOLD NAME: " << str << endl;
//
//	if (vertices.find(oldName) != vertices.end())
//	{
//		vertices[nodeName] = vertices[oldName];
//		vertices[oldName] = nullptr;
//		vertices.erase(oldName);
//	}
//}

//NODE ADDED
void NodeAdded(MObject& node, void* clientData)
{
	if (node.isNull())
		return;

	MString nodeName = MFnDependencyNode(node).name();

	switch (node.apiType())
	{
		case MFn::Type::kMesh:
		{
			NodeAddedMessage message(NODETYPE::MESH, nodeName.asChar(), nodeName.length());
			memory.Send(&message, message.Size());
			break;
		}
	}

	//NAME CHANGE
	//callbackIdArray.append(MNodeMessage::addNameChangedCallback(node, NameChanged, NULL, &status));

	//switch (node.apiType())
	//{
	//	case MFn::Type::kMesh:
	//	{
	//		MFnMesh mesh(node);
	//		MPointArray* v = new MPointArray();
	//		mesh.getPoints(*v);
	//		vertices.emplace(MFnDependencyNode(node).name().asChar(), v);

	//		MCallbackId id = MNodeMessage::addAttributeChangedCallback(node, OutMeshChanged, NULL, &status);
	//		if (status == MS::kSuccess)
	//			callbackIdArray.append(id);

	//		break;
	//	}

	//	case MFn::Type::kTransform:
	//	{
	//		MDagPath dagPath;
	//		MFnDagNode(node).getPath(dagPath);

	//		MCallbackId id = MDagMessage::addMatrixModifiedCallback(dagPath, LocalMatrixChanged, NULL, &status);
	//		if (status == MS::kSuccess)
	//			callbackIdArray.append(id);

	//		id = MDagMessage::addWorldMatrixModifiedCallback(dagPath, GlobalMatrixChanged, NULL, &status);
	//		if (status == MS::kSuccess)
	//			callbackIdArray.append(id);

	//		/*	MCallbackId id = MNodeMessage::addAttributeChangedCallback(node, TransformChanged, NULL, &status);
	//			if (status == MS::kSuccess)
	//				callbackIdArray.append(id);*/
	//		break;
	//	}
	//}

	cout << "\n============================= NODE ADDED =============================" << endl;
	cout << "ADDED NODE: " << MFnDependencyNode(node).name() << endl;
}

////NODE REMOVED
//void NodeRemoved(MObject& node, void* clientData)
//{
//	cout << "\n============================= NODE REMOVED =============================" << endl;
//	cout << "REMOVED NODE: " << MFnDependencyNode(node).name() << endl;
//}

//TIMER
void TimerCallback(float elapsedTime, float lastTime, void* clientData)
{
	timeElapsed += elapsedTime;
	cout << "\n============================= > TIME: " << timeElapsed << " < =============================" << endl;
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

	//callbackIdArray.append(MDGMessage::addNodeRemovedCallback(NodeRemoved, "dependNode", NULL, &status));
	//if (status != MS::kSuccess)
	//	return status;

	//callbackIdArray.append(MTimerMessage::addTimerCallback(5.0, TimerCallback, NULL, &status));
	//if (status != MS::kSuccess)
	//	return status;

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