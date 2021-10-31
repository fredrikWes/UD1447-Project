#include "HelpFunctions.h"
#include <chrono>
#include <iostream>
#include <queue>

using namespace std;

MCallbackIdArray callbackIdArray;

MStatus status = MS::kSuccess;

double timeElapsed = 0;

SharedMemory memory;

#undef SendMessage

void SendMessage(Message* message)
{
	void* data;

	switch (message->nodeType)
	{
		case NODETYPE::MESH:
		{
			if (message->messageType == MESSAGETYPE::ADDED || message->messageType == MESSAGETYPE::REMOVED)
			{
				data = message->Data();
				memory.Send(data, message->Size());
				delete data;
			}

			else if (message->messageType == MESSAGETYPE::CHANGED)
			{
				auto meshChanged = static_cast<MeshChangedMessage*>(message);
				data = meshChanged->Data();
				memory.Send(data, meshChanged->Size());
					
				delete data;
			}

			break;
		}

		case NODETYPE::TRANSFORM:
		{
			auto transformChanged = static_cast<TransformChangedMessage*>(message);
			data = transformChanged->Data();
			memory.Send(data, transformChanged->Size());
			break;
		}

		case NODETYPE::CAMERA:
		{
			if (message->messageType == MESSAGETYPE::CHANGED)
			{
				auto cameraChanged = static_cast<CameraChangedMessage*>(message);
				data = cameraChanged->Data();
				memory.Send(data, cameraChanged->Size());		
				delete data;
			}

			break;
		}

		case NODETYPE::MATERIAL:
		{
			if (message->messageType == MESSAGETYPE::CHANGED)
			{
				auto materialChanged = static_cast<MaterialChangedMessage*>(message);
				data = materialChanged->Data();
				memory.Send(data, materialChanged->Size());
				delete data;
			}

			else if (message->messageType == MESSAGETYPE::ADDED || message->messageType == MESSAGETYPE::REMOVED)
			{
				data = message->Data();
				memory.Send(data, message->Size());
				delete data;
			}

			break;
		}

		case NODETYPE::MATERIALCONNECTION:
		{
			auto materialConnection = static_cast<MaterialConnectionMessage*>(message);
			data = materialConnection->Data();
			memory.Send(data, materialConnection->Size());
			delete data;
			break;
		}

		case NODETYPE::NAMECHANGE:
		{
			auto nameChange = static_cast<NameChangedMessage*>(message);
			data = nameChange->Data();
			memory.Send(data, nameChange->Size());
			delete data;
			break;
		}
	}

	delete message;
}

//MATERIAL CHANGED
void MaterialChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData)
{
	if (plug.node().apiType() == MFn::Type::kLambert && string(plug.info().asChar()).find("color") != string::npos)
	{
		MFnLambertShader shader(plug.node(), &status);
		if (status != MS::kSuccess)
			return;

		MFnDependencyNode depNode(plug.node(), &status);
		if (status != MS::kSuccess)
			return;

		MPlug colorPlug = shader.findPlug("color", true, &status);
		if (status != MS::kSuccess)
			return;

		if (!colorPlug.isNull(&status))
		{
			if (status != MS::kSuccess)
				return;
		}

		if (status != MS::kSuccess)
			return;

		colorPlug = depNode.findPlug("color", true, &status);
		if (status != MS::kSuccess)
			return;

		MPlugArray colorConnections;
		colorPlug.connectedTo(colorConnections, true, false, &status);
		if (status != MS::kSuccess)
			return;

		if (colorConnections.length() != 0)
			return;

		float color[3];
		shader.color().get(color);
		SendMessage(new MaterialChangedMessage(shader.name().numChars(), (char*)shader.name().asChar(), 0, nullptr, color));
	}

	if (plug.node().apiType() == MFn::Type::kFileTexture && 
		msg & MNodeMessage::AttributeMessage::kAttributeSet && 
		string(plug.info().asChar()).find("fileTextureName") != string::npos)
	{
		MPlug outColorPlug = MFnDependencyNode(plug.node()).findPlug("outColor", true);

		//FIND CONNECTED SHADERS
		MPlugArray connections;
		outColorPlug.connectedTo(connections, false, true);

		for (UINT i = 0; i < connections.length(); ++i)
		{
			string fullName = connections[i].name().asChar();
			MString shader = fullName.substr(0, fullName.find_first_of('.')).c_str();
			float color[4] = { 0,0,0,0 };
			SendMessage(new MaterialChangedMessage(shader.numChars(), (char*)shader.asChar(), plug.asString().numChars(), (char*)plug.asString().asChar(), color));
		}
	}
}

//NOTIFY THE AFFECTED NODES (CHILDREN)
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

	SendMessage(new TransformChangedMessage(dagNode.name().numChars(), (char*)dagNode.name().asChar(), matrixArr));

	for (UINT i = 0; i < dagNode.childCount(); ++i)
		NotifyTransformChanged(dagNode.child(i));
}

//MESH TRANSFORMED
void TransformChanged(MObject& node, MDagMessage::MatrixModifiedFlags& modified, void* clientData)
{	
	NotifyTransformChanged(node);
}

//CAMERA UPDATED
void CameraChanged(const MString& str, void* clientData)
{
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

		SendMessage(new CameraChangedMessage(camera.name().numChars(), (char*)camera.name().asChar(), orthoWidth, camera.isOrtho(), nearZ, farZ, horFOV, verFOV, eyePos, center, up, portWidth, portHeight));
	}
}

//MATERIAL CONNECTION OR VERTEX MANIPULATION
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
		MDagPath path;
		status = mesh.getPath(path);
		if (status != MS::kSuccess)
			return;

		int inst = 0;
		if (path.isInstanced())
			inst = path.instanceNumber(&status);
		if (status != MS::kSuccess)
			return;

		MObjectArray shaders;
		MIntArray indices;

		status = mesh.getConnectedShaders(inst, shaders, indices);
		if (status != MS::kSuccess)
			return;

		if (indices.length() > 0)
		{
			if (indices[0] != -1)
			{
				MPlug shaderPlug = MFnDependencyNode(shaders[0]).findPlug("surfaceShader", true, &status);
				if (status != MS::kSuccess)
					return;

				MPlugArray plugs;
				shaderPlug.connectedTo(plugs, true, false, &status);
				if (status != MS::kSuccess)
					return;

				MFnDependencyNode surfaceShader(plugs[0].node(), &status);
				if (status != MS::kSuccess)
					return;

				SendMessage(new MaterialConnectionMessage(nodeName.numChars(), (char*)nodeName.asChar(), surfaceShader.name().numChars(), (char*)surfaceShader.name().asChar()));
			}
		}
	}

	if (msg & MNodeMessage::AttributeMessage::kAttributeEval && plugInfo.find(".outMesh") != std::string::npos)
	{
		std::vector<int> indices;
		std::vector<Vertex> vertices;
		if (!ProcessMesh(mesh, indices, vertices))
			return;

		SendMessage(new MeshChangedMessage(NODETYPE::MESH, MESSAGETYPE::CHANGED, nodeName.numChars(), (char*)nodeName.asChar(), indices.data(), indices.size(), vertices.data(), vertices.size()));
	}
}

//NAME CHANGE
void NodeNameChange(MObject& node, const MString& str, void* clientData)
{
	MString newName = MFnDependencyNode(node).name().asChar();
	SendMessage(new NameChangedMessage(str.numChars(), (char*)str.asChar(), newName.numChars(), (char*)newName.asChar()));
}

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
			nodeName = MFnDependencyNode(MFnDagNode(node).parent(0)).name();
			SendMessage(new Message(NODETYPE::MESH, MESSAGETYPE::ADDED, nodeName.numChars(), (char*)nodeName.asChar()));
			callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(node, MeshChanged, NULL, &status));
			break;
		}
			
		case MFn::Type::kTransform:
		{
			MFnDagNode dagNode(node);
			MDagPath dagPath;
			dagNode.getPath(dagPath);
			callbackIdArray.append(MDagMessage::addMatrixModifiedCallback(dagPath, TransformChanged, NULL, &status));
			callbackIdArray.append(MNodeMessage::addNameChangedCallback(node, NodeNameChange, NULL, &status));
			break;
		}

		case MFn::Type::kLambert:
		{
			SendMessage(new Message(NODETYPE::MATERIAL, MESSAGETYPE::ADDED, nodeName.numChars(), (char*)nodeName.asChar()));
			callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(node, MaterialChanged, NULL, &status));
			callbackIdArray.append(MNodeMessage::addNameChangedCallback(node, NodeNameChange, NULL, &status));
			break;
		}

		case MFn::Type::kFileTexture:
		{
			callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(node, MaterialChanged, NULL, &status));
			break;
		}
	}
}

//NODE REMOVED
void NodeRemoved(MObject& node, void* clientData)
{
	if (node.isNull())
		return;

	MString nodeName = MFnDependencyNode(node).name();

	switch (node.apiType())
	{
		case MFn::Type::kTransform: // ONLY TRASNFORM FOR NAMING REASONS
		{
			SendMessage(new Message(NODETYPE::MESH, MESSAGETYPE::REMOVED, nodeName.numChars(), (char*)nodeName.asChar()));
			break;
		}

		case MFn::Type::kLambert:
		{
			SendMessage(new Message(NODETYPE::MATERIAL, MESSAGETYPE::REMOVED, nodeName.numChars(), (char*)nodeName.asChar()));
			break;
		}
			
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