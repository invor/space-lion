#ifndef MTQueue_hpp
#define MTQueue_hpp

#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace EngineCore
{
	namespace Utility
	{

		/**
		 * Basic multithreaded (i.e. thread-safe) queue.
		 */
		template <class T>
		class MTQueue
		{
		public:
			/** Default constructor */
			MTQueue() = default;
			/** (Guarded) destructor */
			~MTQueue() { std::lock_guard<std::mutex> lock(m_mutex); }

			/**
			 * Push new element to the queue.
			 * \param val Element to be inserted
			 */
			void push(const T& val)
			{
				std::lock_guard<std::mutex> lock(m_mutex);

				m_queue.push(val);
				m_cvar.notify_one();
			}

			void push(T&& val)
			{
				std::lock_guard<std::mutex> lock(m_mutex);

				m_queue.push(val);
				m_cvar.notify_one();
			}

			/**
			 * Pop element from the queue. Method call blocks if queue is empty until
			 * a new element is inserted.
			 * \param val Element extracted from queue
			 */
			void pop(T& val)
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_cvar.wait(lock, [this] {return !m_queue.empty(); });

				val = std::move(m_queue.front());
				m_queue.pop();
			}

			/**
			 * Variant of pop method using return value. For handling objects without default constructor.
			 * NEEDS A SECOND LOOK AND APPROVAL
			 */
			T pop()
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_cvar.wait(lock, [this] {return !m_queue.empty(); });

				T val = std::move(m_queue.front());
				m_queue.pop();

				return val;
			}

			/**
			 * Try to pop an element from the queue. Non-blocking but can wait for
			 * specified time for a new element to arrive if queue is empty.
			 * \param val Element extracted from the queue
			 * \param timeout Time that is waited for a new element in milliseconds
			 * (defaults to 0)
			 * \return Returns true if an element was succesfully extracted from queue.
			 * Returns false if queue remained empty.
			 */
			bool tryPop(T& val, std::chrono::microseconds timeout = std::chrono::microseconds(0))
			{
				std::unique_lock<std::mutex> lock(m_mutex);

				if (!m_cvar.wait_for(lock, timeout, [this] {return !m_queue.empty(); }))
					return false;

				val = std::move(m_queue.front()); // rethink usage of std::move
				m_queue.pop(); // still it should be fine, since the front element is deleted here anyway

				return true;
			}

			/**
			 * Check whether or not the queue is currently empty.
			 */
			bool empty() const // checking if the queue is empty shouldn't modify the queue
			{
				std::lock_guard<std::mutex> lock(m_mutex);

				return m_queue.empty();
			}

			size_t size() const
			{
				std::lock_guard<std::mutex> lock(m_mutex);

				return m_queue.size();
			}

		private:
			/** Underlying standard queue. */
			std::queue<T> m_queue;
			/** Mutex to protect queue operations. */
			mutable std::mutex m_mutex;
			/** Condition variable used for pop with timeout */
			std::condition_variable m_cvar;
		};

	}
}

#endif
