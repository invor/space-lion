#ifndef  messageReceiver_h
#define messageReceiver_h

#include <queue>
#include <atomic>

struct Message
{
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

	void pushMessage(Message msg);
	Message popMessage(); 

private:
	std::queue<Message> message_fifo;
	std::atomic<unsigned int> message_counter;
};

#endif
