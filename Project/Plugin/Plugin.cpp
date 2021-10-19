#include "HelpFunctions.h"
#include <iostream>
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
		cout << "Shader Color values: " << shader.color().r << " " << shader.color().g << " " << shader.color().b << endl;
	}

	if (msg & MNodeMessage::AttributeMessage::kConnectionMade || msg & MNodeMessage::AttributeMessage::kConnectionBroken)
	{
		cout << "\n============================= MATERIAL TEXTURE CONNECTION CHANGED =============================" << endl;
		cout << plug.name() << endl;


		//MFnPhongShader test(plug.node());
		//MPlugArray plugArr;
		//test.getConnections(plugArr);
		//
		//for (int i = 0; i < plugArr.length(); i++)
		//{
		//	cout << plugArr[i].name() << endl;
		//}

		//MPlug testName = test.findPlug((MString)"fileTextureName", false, &status);
		//MString filePath;
		//testName.getValue(filePath);
	

	/*	MFnDependencyNode texture(plug.node());
		MPlug textureName = texture.findPlug((MString)"fileTextureName", false, &status);
		MString filePath;
		textureName.getValue(filePath);
		cout << filePath << endl;*/

		
		MItDependencyGraph dtIt()

		



		if (plug.node().hasFn(MFn::kFileTexture))
		{
	

		

		}

		MFnPhongShader shader(plug.node());
		cout << shader.absoluteName() << endl;
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
		M3dView currentView = M3dView();
		MMatrix viewMatrix, perspectiveMatrix;

		currentView.updateViewingParameters();

		currentView.modelViewMatrix(viewMatrix);
		currentView.projectionMatrix(perspectiveMatrix);

		MMatrix matrix = viewMatrix * perspectiveMatrix;
		float matrixArr[16] = {};

		UINT index = 0;
		for (UINT i = 0; i < 4; ++i)
		{
			for (UINT j = 0; j < 4; j++)
			{
				matrixArr[index] = matrix.matrix[i][j];
				index++;
			}
		}

		Message* message = new CameraChangedMessage(activePanel.numChars(), (char*)activePanel.asChar(), matrixArr);
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
		cout << "\n============================= GEOMETRY CHANGED =============================" << endl;

		std::vector<Vertex> vertices;
		ProcessMesh(mesh, vertices);

		MeshChangedMessage* message = new MeshChangedMessage(NODETYPE::MESH, MESSAGETYPE::CHANGED, MFnDependencyNode(node, &status).name().numChars(), (char*)MFnDependencyNode(node, &status).name().asChar(), vertices.data(), vertices.size());
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
	//cout << node.apiTypeStr() << endl;

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

	//MUiMessage::add3dViewPreRenderMsgCallback("modelPanel1", CameraChanged);
	//MUiMessage::add3dViewPreRenderMsgCallback("modelPanel2", CameraChanged);
	//MUiMessage::add3dViewPreRenderMsgCallback("modelPanel3", CameraChanged);
	//MUiMessage::add3dViewPreRenderMsgCallback("modelPanel4", CameraChanged);

	callbackIdArray.append(MDGMessage::addNodeAddedCallback(NodeAdded, "dependNode", NULL, &status));
	if (status != MS::kSuccess)
		return status;

	callbackIdArray.append(MDGMessage::addNodeRemovedCallback(NodeRemoved, "dependNode", NULL, &status));
	if (status != MS::kSuccess)
		return status;

	callbackIdArray.append(MTimerMessage::addTimerCallback(0.001f, TimerCallback, NULL, &status));
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