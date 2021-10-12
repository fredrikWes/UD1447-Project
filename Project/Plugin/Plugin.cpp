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

#undef SendMessage
bool SendMessage(Message* message)
{
	void* data;
	bool sent = false;

	auto nodeAdded = dynamic_cast<NodeAddedMessage*>(message);
	if (nodeAdded)
	{
		cout << "trying to send: " << nodeAdded->name << endl;

		data = nodeAdded->Data();
		sent = memory.Send(data, nodeAdded->Size());
		delete data;
	}

	return sent;
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
			Message* message = new NodeAddedMessage(NODETYPE::MESH, (char*)nodeName.asChar(), nodeName.numChars());
			messages.push(message);
			break;
		}
	}

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
	timeElapsed += elapsedTime;
	cout << "\n============================= > TIME: " << timeElapsed << " < =============================" << endl;

	cout << messages.size() << endl;
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

	callbackIdArray.append(MTimerMessage::addTimerCallback(0.1, TimerCallback, NULL, &status));
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