#include "HelpFunctions.h"
#include <iostream>
#include <queue>

using namespace std;

MCallbackIdArray callbackIdArray;

MStatus status = MS::kSuccess;

double timeElapsed = 0;

SharedMemory memory;

queue<Message*> messages;

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
				delete data;
			}

			else if (message->messageType == MESSAGETYPE::CHANGED)
			{
				auto meshChanged = static_cast<MeshChangedMessage*>(message);
				cout << "---TRYING TO SEND MESH CHANGED MESSAGE---" << endl;
				cout << meshChanged->Size() << " " << meshChanged->name << " " << meshChanged->vertices[0].Px << endl;
				data = meshChanged->Data();
				sent = memory.Send(data, meshChanged->Size());
				if (sent)
					cout << "---SENT MESH CHANGED MESSAGE---" << endl;
				delete data;
			}

			break;
		}

		case NODETYPE::CAMERA:
		{
			if (message->messageType == MESSAGETYPE::CHANGED)
			{
				auto cameraChanged = static_cast<CameraChangedMessage*>(message);
				cout << "---TRYING TO SEND CAMERA CHANGED MESSAGE---" << endl;
				cout << cameraChanged->Size() << " " << cameraChanged->name << endl;
				data = cameraChanged->Data();
				sent = memory.Send(data, cameraChanged->Size());
				if (sent)
					cout << "---SENT CAMERA CHANGED MESSAGE---" << endl;
				delete data;
			}
		}
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

void TransformChanged(/*ARGS*/)
{

}

void CameraChanged(const MString& str, void* clientData)
{
	MString cmd = "getPanel -wf"; //wf = with focus (current panel)
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

		double orthoWidth = camera.orthoWidth();

		float viewMatrixArr[16] = {};
		float perspMatrixArr[16] = {};

		viewMatrixArr[0] = viewMatrix[0][0];
		viewMatrixArr[1] = viewMatrix[1][0];
		viewMatrixArr[2] = viewMatrix[2][0];
		viewMatrixArr[3] = viewMatrix[3][0];

		viewMatrixArr[4] = viewMatrix[0][1];
		viewMatrixArr[5] = viewMatrix[1][1];
		viewMatrixArr[6] = viewMatrix[2][1];
		viewMatrixArr[7] = viewMatrix[3][1];

		viewMatrixArr[8] = viewMatrix[0][2];
		viewMatrixArr[9] = viewMatrix[1][2];
		viewMatrixArr[10] = viewMatrix[2][2];
		viewMatrixArr[11] = viewMatrix[3][2];

		viewMatrixArr[12] = viewMatrix[0][3];
		viewMatrixArr[13] = viewMatrix[1][3];
		viewMatrixArr[14] = viewMatrix[2][3];
		viewMatrixArr[15] = viewMatrix[3][3];

		
		perspMatrixArr[0] = perspectiveMatrix[0][0];
		perspMatrixArr[1] = perspectiveMatrix[1][0];
		perspMatrixArr[2] = perspectiveMatrix[2][0];
		perspMatrixArr[3] = perspectiveMatrix[3][0];

		perspMatrixArr[4] = perspectiveMatrix[0][1];
		perspMatrixArr[5] = perspectiveMatrix[1][1];
		perspMatrixArr[6] = perspectiveMatrix[2][1];
		perspMatrixArr[7] = perspectiveMatrix[3][1];

		perspMatrixArr[8] = perspectiveMatrix[0][2];
		perspMatrixArr[9] = perspectiveMatrix[1][2];
		perspMatrixArr[10] = perspectiveMatrix[2][2];
		perspMatrixArr[11] = perspectiveMatrix[3][2];

		perspMatrixArr[12] = perspectiveMatrix[0][3];
		perspMatrixArr[13] = perspectiveMatrix[1][3];
		perspMatrixArr[14] = perspectiveMatrix[2][3];
		perspMatrixArr[15] = perspectiveMatrix[3][3];



		//UINT index = 0;
		//for (UINT i = 0; i < 4; ++i)
		//{
		//	for (UINT j = 0; j < 4; j++)
		//	{
		//		viewMatrixArr[index] = vMatrix.matrix[j][i];
		//		cout << vMatrix.matrix[j][i] << endl;

		//		index++;
		//	}
		//}

		//index = 0;
		//for (UINT k = 0; k < 4; ++k)
		//{
		//	for (UINT l = 0; l < 4; l++)
		//	{
		//		perspMatrixArr[index] = pMatrix.matrix[k][l];
		//		cout << pMatrix.matrix[k][l] << endl;

		//		index++;
		//	}
		//}
		//dex = 0;

		Message* message = new CameraChangedMessage(camera.name().numChars(), (char*)camera.name().asChar(), viewMatrixArr, perspMatrixArr, orthoWidth, camera.isOrtho());
		messages.push(message);
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
		std::vector<int> indices;
		std::vector<Vertex> vertices;
		if (!ProcessMesh(mesh, indices, vertices))
			return;

		cout << "\n============================= GEOMETRY CHANGED =============================" << endl;
		MeshChangedMessage* message = new MeshChangedMessage(NODETYPE::MESH, MESSAGETYPE::CHANGED, MFnDependencyNode(node, &status).name().numChars(), (char*)MFnDependencyNode(node, &status).name().asChar(), indices.data(), indices.size(), vertices.data(), vertices.size(), vertexCache[nodeName].size());
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

		case MFn::Type::kCamera:
			found = true;
			//callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(node, CameraChanged, NULL, &status));
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
	if (node.isNull())
		return;

	bool found = false;
	MString nodeName = MFnDependencyNode(node).name();
	cout << node.apiTypeStr() << endl;

	switch (node.apiType())
	{
	case MFn::Type::kMesh:
		found = true;
		messages.push(new Message(NODETYPE::MESH, MESSAGETYPE::REMOVED, nodeName.numChars(), (char*)nodeName.asChar()));
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

	cout << "\n============================= NODE REMOVED =============================" << endl;
	cout << "ADDED NODE: " << MFnDependencyNode(node).name() << endl;
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