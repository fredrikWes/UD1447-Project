#include "HelpFunctions.h"
#include <chrono>
#include <iostream>
#include <queue>

using namespace std;

MCallbackIdArray callbackIdArray;

MStatus status = MS::kSuccess;

double timeElapsed = 0;

SharedMemory memory;

queue<Message*> messages;

chrono::high_resolution_clock timer;
chrono::high_resolution_clock::time_point lastCameraUpdate;
chrono::high_resolution_clock::time_point lastTransformUpdate;
chrono::high_resolution_clock::time_point lastMeshUpdate;

int numSkipped = 0;
int numPassed = 0;

#undef SendMessage

bool SendMessage(Message* message)
{
	void* data;
	bool sent = false;

	switch (message->nodeType)
	{
		case NODETYPE::MESH:
		{
			if (message->messageType == MESSAGETYPE::ADDED || message->messageType == MESSAGETYPE::REMOVED)
			{
				data = message->Data();
				sent = memory.Send(data, message->Size());
				if (sent)
				{
					cout << "---SENT MESH ADDED MESSAGE---" << endl;
					cout << message->name << endl;
				}
				delete data;
			}

			else if (message->messageType == MESSAGETYPE::CHANGED)
			{
				auto meshChanged = static_cast<MeshChangedMessage*>(message);
				cout << meshChanged->Size() << " " << meshChanged->name << " " << meshChanged->vertices[0].Px << endl;
				data = meshChanged->Data();
				sent = memory.Send(data, meshChanged->Size());
				if (sent)
				{
					cout << "---SENT MESH CHANGED MESSAGE---" << endl;
					cout << meshChanged->name << endl;
					cout << meshChanged->numIndices << endl;
				}
					
				delete data;
			}

			break;
		}

		case NODETYPE::TRANSFORM:
		{
			auto transformChanged = static_cast<TransformChangedMessage*>(message);
			data = transformChanged->Data();
			sent = memory.Send(data, transformChanged->Size());
			if (sent)
			{
				cout << "---SENT TRANSFORM CHANGED MESSAGE---" << endl;
				cout << transformChanged->name << endl;
				cout << transformChanged->matrix[0] << endl;
			}
		}

		case NODETYPE::CAMERA:
		{
			if (message->messageType == MESSAGETYPE::CHANGED)
			{
				auto cameraChanged = static_cast<CameraChangedMessage*>(message);
				cout << cameraChanged->Size() << " " << cameraChanged->name << endl;
				data = cameraChanged->Data();
				sent = memory.Send(data, cameraChanged->Size());
				if (sent)
				{
					cout << "---SENT CAMERA CHANGED MESSAGE---" << endl;
					cout << cameraChanged->name << endl;
					cout << cameraChanged->orthoWidth << endl;
				}
					
				delete data;
			}
		}
	}

	delete message;
	return sent;
}

//MATERIAL CHANGED
void MaterialChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData)
{
	if (msg & MNodeMessage::AttributeMessage::kAttributeSet)
	{
		cout << "\n============================= MATERIAL CHANGED =============================" << endl;
		cout << plug.name() << endl;

		//MFnPhongShader shader(plug.node());
		//cout << shader.absoluteName() << endl;
	}

	if (msg & MNodeMessage::AttributeMessage::kConnectionMade || msg & MNodeMessage::AttributeMessage::kConnectionBroken)
	{
		cout << "\n============================= MATERIAL TEXTURE CONNECTION CHANGED =============================" << endl;
		cout << plug.name() << endl;

		//MFnPhongShader shader(plug.node());
		//cout << shader.absoluteName() << endl;
	}
}

void NotifyTransformChanged(const MObject& node)
{
	if (node.apiType() != MFn::Type::kTransform)
		return;

	MFnDagNode dagNode(node);
	MDagPath dagPath;

	status = dagNode.getPath(dagPath);
	if (status != MS::kSuccess)
		return;

	float matrixArr[16] = { 0 };

	MMatrix matrix = dagPath.inclusiveMatrix(&status);
	if (status != MS::kSuccess)
		return;

	UINT index = 0;
	for (UINT i = 0; i < 4; ++i)
	{
		for (UINT j = 0; j < 4; ++j)
		{
			matrixArr[index] = matrix[i][j];
			index++;
		}
	}

	Message* message = new TransformChangedMessage(dagNode.name().numChars(), (char*)dagNode.name().asChar(), matrixArr);
	messages.push(message);

	for (UINT i = 0; i < dagNode.childCount(); ++i)
		NotifyTransformChanged(dagNode.child(i));
}

void TransformChanged(MObject& node, MDagMessage::MatrixModifiedFlags& modified, void* clientData)
{
	//if (true)
	//{
	//	cout << "======================================" << endl;
	//	cout << "NUM PASSED: " << numPassed << endl;
	//	cout << "NUM SKIPPED: " << numSkipped << endl;
	//}

	//auto secondsSinceUpdate = chrono::duration_cast<chrono::milliseconds>(timer.now() - lastTransformUpdate).count();
	//if (secondsSinceUpdate < 17.0)
	//{
	//	numSkipped++;
	//	return;
	//}
		
	cout << "\n============================= TRANSFORM CHANGED =============================" << endl;
	cout << MFnDependencyNode(node, &status).name() << endl;
	NotifyTransformChanged(node);

	lastTransformUpdate = timer.now();
	numPassed++;
}

void CameraChanged(const MString& str, void* clientData)
{
	auto secondsSinceUpdate = chrono::duration_cast<chrono::milliseconds>(timer.now() - lastCameraUpdate).count();
	if (secondsSinceUpdate < 17.0)
		return;

	MString cmd = "getPanel -wf";
	MString activePanel = MGlobal::executeCommandStringResult(cmd);

	if (strcmp(str.asChar(), activePanel.asChar()) == 0)
	{
		M3dView currentView = M3dView::active3dView();
		MMatrix viewMatrix, perspectiveMatrix;

		currentView.updateViewingParameters();

		currentView.modelViewMatrix(viewMatrix);
		currentView.projectionMatrix(perspectiveMatrix);

		MDagPath cameraPath;

		currentView.getCamera(cameraPath);
		MFnCamera camera(cameraPath);
			
		MMatrix vMatrix = viewMatrix;
		MMatrix pMatrix = perspectiveMatrix;
		
		//ORTHO
		double orthoWidth = camera.orthoWidth();
		float nearZ = camera.unnormalizedNearClippingPlane();
		float farZ = camera.unnormalizedFarClippingPlane();
		float horFOV = camera.horizontalFieldOfView();
		float verFOV = camera.verticalFieldOfView();

		//VIEW
		float eyePos[4];
		camera.eyePoint(MSpace::kWorld).get(eyePos);
		float center[4];
		camera.centerOfInterestPoint(MSpace::kWorld).get(center);
		double up[3];
		camera.upDirection(MSpace::kWorld).get(up);

		int portWidth = currentView.portWidth();
		int portHeight = currentView.portHeight();

		Message* message = new CameraChangedMessage(camera.name().numChars(), (char*)camera.name().asChar(), orthoWidth, camera.isOrtho(), nearZ, farZ, horFOV, verFOV, eyePos, center, up, portWidth, portHeight);
		messages.push(message);

		lastCameraUpdate = timer.now();
	}
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

	MString nodeName = MFnDependencyNode(mesh.parent(0), &status).name();
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
		std::vector<int> indices;
		std::vector<Vertex> vertices;
		if (!ProcessMesh(mesh, indices, vertices))
			return;

		cout << "\n============================= GEOMETRY CHANGED =============================" << endl;
		cout << nodeName << endl;
		Message* message = new MeshChangedMessage(NODETYPE::MESH, MESSAGETYPE::CHANGED, nodeName.numChars(), (char*)nodeName.asChar(), indices.data(), indices.size(), vertices.data(), vertices.size(), vertexCache[mesh.name().asChar()].size());
		messages.push(message);
	}
}

//NODE ADDED
void NodeAdded(MObject& node, void* clientData)
{
	if (node.isNull())
		return;

	bool found = false;
	MString nodeName = MFnDependencyNode(node).name();

	switch (node.apiType())
	{
		case MFn::Type::kMesh:
		{
			found = true;
			nodeName = MFnDependencyNode(MFnDagNode(node).parent(0)).name();
			Message* message = new Message(NODETYPE::MESH, MESSAGETYPE::ADDED, nodeName.numChars(), (char*)nodeName.asChar());
			messages.push(message);
			callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(node, MeshChanged, NULL, &status));
			break;
		}
			
		case MFn::Type::kTransform:
		{
			found = true;
			MFnDagNode dagNode(node);
			MDagPath dagPath;
			dagNode.getPath(dagPath);
			callbackIdArray.append(MDagMessage::addMatrixModifiedCallback(dagPath, TransformChanged, NULL, &status));
			break;
		}

		case MFn::Type::kPhong:
		{
			found = true;
			callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(node, MaterialChanged, NULL, &status));
			break;
		}
	}

	if (!found)
		return;

	cout << "\n============================= NODE ADDED =============================" << endl;
	cout << "ADDED NODE: " << nodeName << endl;
}

//NODE REMOVED
void NodeRemoved(MObject& node, void* clientData)
{
	if (node.isNull())
		return;

	bool found = false;
	MString nodeName = MFnDependencyNode(node).name();

	switch (node.apiType())
	{
		case MFn::Type::kMesh:
		{
			nodeName = MFnDependencyNode(MFnDagNode(node).parent(0)).name();
			Message* message = new Message(NODETYPE::MESH, MESSAGETYPE::REMOVED, nodeName.numChars(), (char*)nodeName.asChar());
			messages.push(message);
			break;
		}
		

		case MFn::Type::kPhong:
			break;
	}

	cout << "\n============================= NODE REMOVED =============================" << endl;
	cout << "REMOVED NODE: " << nodeName << endl;
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

	callbackIdArray.append(MUiMessage::add3dViewPreRenderMsgCallback("modelPanel1", CameraChanged, &status));
	if (status != MS::kSuccess)
		return status;

	callbackIdArray.append(MUiMessage::add3dViewPreRenderMsgCallback("modelPanel2", CameraChanged, &status));
	if (status != MS::kSuccess)
		return status;

	callbackIdArray.append(MUiMessage::add3dViewPreRenderMsgCallback("modelPanel3", CameraChanged, &status));
	if (status != MS::kSuccess)
		return status;

	callbackIdArray.append(MUiMessage::add3dViewPreRenderMsgCallback("modelPanel4", CameraChanged, &status));
	if (status != MS::kSuccess)
		return status;

	callbackIdArray.append(MDGMessage::addNodeAddedCallback(NodeAdded, "dependNode", NULL, &status));
	if (status != MS::kSuccess)
		return status;

	callbackIdArray.append(MDGMessage::addNodeRemovedCallback(NodeRemoved, "dependNode", NULL, &status));
	if (status != MS::kSuccess)
		return status;

	callbackIdArray.append(MTimerMessage::addTimerCallback(0.001, TimerCallback, NULL, &status));
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