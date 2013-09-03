#ifndef  messageReceiver_h
#define messageReceiver_h

#include <queue>
#include <atomic>

enum messageType { CREATE, DELETE, GAMEOVER };

struct Message
{
	messageType type;
	int id;
	std::string node_label;
};

/**
* \class MessageReceiver
* 
* \brief Allows primitive communication
* 
* This class offers a simple form of communication between engine modules via a message FIFO
* 
* \author Michael Becher
* 
* \date 15th August 2013
*/
class MessageReceiver
{
public:
	MessageReceiver() {}
	~MessageReceiver() {}

	void pushPlaceMessage();
	void pushRemoveMessage();
	void pushGameOverMessage();
	Message popMessage(); 

private:
	std::queue<Message> message_fifo;
	std::atomic<unsigned int> message_counter;
};

#endif
