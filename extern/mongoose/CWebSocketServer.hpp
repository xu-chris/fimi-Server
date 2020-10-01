#pragma once



#include <iostream>
#include <cstring>
#include <string>
#include <mutex>

#include "mongoose.h" // Should be in extern

//! See http://stackoverflow.com/questions/11709859/how-to-have-static-data-members-in-a-header-only-library
#define CWEBSOCKETS_STATIC_DEFINITIONS					\
	std::string Common::CWebSocketServer::m_ServerBroadcastBuffer = ""; \
struct mg_connection * Common::CWebSocketServer::m_WSConnection = 0; \
	std::mutex * Common::CWebSocketServer::m_sMutex = new std::mutex;

namespace Common
{
	/** CWebSocketServer

		The CWebSocketServer class is a WebSocket server for broadcasting data on the network.
		It uses the lightweight mongoose WebSocket implementation. Any kind of string stream
		can be broadcast on the server. Clients can then connect to the server to pull the data.

		TODO: Server probably can't request data from clients.

		Requires mongoose includes and libs.
		*/
	class CWebSocketServer
	{
	protected:
		//! Mongoose related members
		struct mg_context * m_Ctx;
		struct mg_callbacks m_Callbacks;
		std::string m_BroadcastPorts;
		std::string m_ServerRoot;

		//! Server state. True is up. False is down.
		bool m_ServerState;

		//! Check initialization
		bool m_isInitialized;

		//! Controlling server state externally
		bool m_StartServer;
		bool m_StopServer;

		//! Class mutex
		std::mutex m_Mutex;
		static std::mutex * m_sMutex;

		//! The connection
		static struct mg_connection * m_WSConnection;

	public:
		//! See http://stackoverflow.com/questions/11709859/how-to-have-static-data-members-in-a-header-only-library
		static std::string m_ServerBroadcastBuffer;

		//! Constructor
		CWebSocketServer(void) {};

		//! Destructor
		virtual ~CWebSocketServer(void) {};

		//! Check if server is initialized
		bool isServerInitialized(void) { return m_isInitialized; };

		//! Check if server is up
		bool isServerStarted(void) { return m_ServerState; };

		//! Ask server to send data
		bool SendData(const std::string& Message)
		{
			int Sent;
			if (m_WSConnection != NULL)
			{
				m_Mutex.lock();
				Sent = mg_websocket_write(m_WSConnection, WEBSOCKET_OPCODE_TEXT, Message.c_str(), std::strlen(Message.c_str()));
				m_Mutex.unlock();
			}
			else
				return false;

			// Return:
			//  0   when the connection has been closed
			//  -1  on error
			//  >0  number of bytes written on success
			if (Sent < 1)
				return false;

			return true;
		};

		//! Ready handler
		static void ReadyHandler(struct mg_connection * conn)
		{
			m_WSConnection = conn;
			std::string Message = "CWebSocketServer - Server is ready.";
			mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, Message.c_str(), std::strlen(Message.c_str()));

			std::cout << "[ INFO ]: " << "Client handshake is complete. Ready for data transmission." << std::endl;
		};

		//! Data transmission
		static int DataTransmitter(struct mg_connection *conn, int flags, char *data, size_t data_len)
		{
			// cutios::Logx() << data_len;
			// // Print data incoming from client
			// std::string Test(data, data+data_len);
			// cutios::Logx() << Test;
			// std::stringstream ss;
			// m_sMutex->lock();
			// ss << m_ServerBroadcastBuffer;
			// m_sMutex->unlock();
			// mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, ss.str().c_str(), std::strlen(ss.str().c_str()));

			// // Clear the buffer for the next data
			// m_ServerBroadcastBuffer = "";

			// Returning zero means stopping websocket conversation.
			return 1;
		};

		//! Client connect handler
		static int ConnectHandler(const struct mg_connection *conn)
		{
			std::cout << "[ INFO ]: " << "Client connected to server" << std::endl;
			//std::string Message = "Welcome client!";
			//mg_websocket_write((struct mg_connection *)conn, WEBSOCKET_OPCODE_TEXT, Message.c_str(), std::strlen(Message.c_str()));

			// Returning zero means mongoose proceeds with callback
			return 0;
		};

		//! Initialize server
		void Initialize(void)
		{
			m_ServerState = false; // Is down initially
			m_isInitialized = false;

			memset(&m_Callbacks, 0, sizeof(m_Callbacks));
			m_Callbacks.websocket_ready = CWebSocketServer::ReadyHandler;
			m_Callbacks.websocket_data = CWebSocketServer::DataTransmitter;
			//m_Callbacks.websocket_connect = CWebSocketServer::ConnectHandler;

			m_isInitialized = true;
		};

		//! Start the server
		void StartServer(const std::string Port = "", const std::string Root = "")
		{
			m_BroadcastPorts = Port;
			m_ServerRoot = Root;
			if (m_BroadcastPorts.empty() == true)
				m_BroadcastPorts = "8080"; // Default to 8080
			if (m_ServerRoot.empty() == true)
				m_ServerRoot = "."; // Default to pwd

			std::cout << "[ INFO ]: " << "Starting data server on port " << m_BroadcastPorts << " and root directory " << m_ServerRoot << " ." << std::endl;

			const char * options[] =
			{
				"listening_ports", m_BroadcastPorts.c_str(),
				"document_root", m_ServerRoot.c_str(),
				NULL
			};

			m_Ctx = mg_start(&m_Callbacks, NULL, options);
			m_ServerState = true;

			std::cout << "[ INFO ]: " << "Done starting server.";
		};

		//! Stop the server
		void StopServer(void)
		{
			m_Mutex.lock();
			std::cout << "[ INFO ]: " << "Stopping data server... ";
			mg_stop(m_Ctx);
			m_ServerState = false;
			std::cout << "[ INFO ]: " << "Done stopping server.";
			m_Mutex.unlock();
		};
	};
} // namespace Common


