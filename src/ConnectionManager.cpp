#include "ConnectionManager.h"
#include "json.hpp"

std::thread ConnectionManager::m_recvThread;
std::mutex ConnectionManager::m_mutex;

std::queue<std::string> ConnectionManager::m_sendQueue;
std::queue<std::string> ConnectionManager::m_recvQueue;

std::string ConnectionManager::m_recvBuffer;

void ConnectionManager::Connect(const std::string& ip, int port) {
    // TODO: create a TCP socket
	// TODO: set the socket address to ip, port
	// TODO: make the socket non-blocking
	// TODO: connect to the server, don't forget to verify all return values for success!
	
    // create a thread that will run ConnectionManager::RecvLoop and detach it
	m_recvThread = std::thread(&ConnectionManager::RecvLoop);
	if (m_recvThread.joinable()) m_recvThread.detach();
}

void ConnectionManager::Disconnect() {
	if (!m_running) return;

	{
		std::lock_guard<std::mutex> lock(m_mutex);

		// TODO: flush anything still queued, then send a graceful DISCONNECT so the
		// server doesn't log us as a protocol violator for closing the socket
		// without notice. Sent synchronously (under the lock, so it can't race
		// the RecvLoop's own draining) to guarantee it goes out before we stop.

		// TODO: create a DISCONNECT message and send it

	}

	// stop the receive loop
	m_running = false;
}

void ConnectionManager::Send(const std::string& message) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_sendQueue.push(message);
}

std::optional<std::string> ConnectionManager::Poll() {
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_recvQueue.empty()) return std::nullopt;;
    // TODO: return the next message in the queue
	return std::nullopt;
}

void ConnectionManager::RecvLoop() {
	while (m_running) {
		// Send queued messages
		{
			std::lock_guard<std::mutex> lock(m_mutex);
            // TODO: send all messages in the sendQueue 
		}

		// TODO: Receive from the server, push to the recvQueue
		// Remember to handle Server Disconnects, WSAEWOULDBLOCK, and other failure cases
		
        // sleep briefly to avoid problems with loops 
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}