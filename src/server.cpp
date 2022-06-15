#include <evpp/udp/udp_server.h>
#include <evpp/udp/udp_message.h>

#include <vector>

const uint32_t MAX_CLIENTS = 128;

static const uint16_t InvalidIdentifier = UINT16_MAX;

struct ClientInfo
{
public:
	uint16_t Identifier;

public:
	ClientInfo()
		: Identifier(InvalidIdentifier)
	{
	}

	ClientInfo(uint16_t identifier)
		: Identifier(identifier)
	{}

	bool IsConnected() const
	{
		return Identifier != InvalidIdentifier;
	}
};

uint16_t incrementalIdentifier = 0;

#include <unordered_map>
// http://www.blog.matejzavrsnik.com/map_vs_unordered_map_performance.html
std::array<sockaddr, MAX_CLIENTS> ClientAddress;
std::array<bool, MAX_CLIENTS> ClientConnected;

uint16_t FindFreeClientIdentifier()
{
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (!ClientConnected[i])
		{
			return i;
		}
	}

	return InvalidIdentifier;
}

uint16_t FindClientIdentifier(const sockaddr* address)
{
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (ClientConnected[i] && ClientAddress[i] == *address)
		{
			return i;
		}
	}

	return InvalidIdentifier;
}

bool IsClientConnected(uint16_t clientIdentifier)
{
	return ClientConnected[clientIdentifier];
}

/// 

ClientInfo ClientInfos[MAX_CLIENTS];
int main(int argc, char** argv)
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	int error = WSAStartup(wVersionRequested, &wsaData);
	if (0 != error) { return 0; }

	///


	///

	std::vector<int> ports = { 1053, 5353 };
	evpp::udp::Server server;

	server.SetMessageHandler([](evpp::EventLoop* loop, evpp::udp::MessagePtr& msg) {
		// In order of frequency

		uint8_t mode = msg->ReadByte();

		uint16_t clientIdentifier = FindClientIdentifier(msg->remote_addr());

		// if unreliable
		if (mode == 0)
		{
			std::cout << "handling unreliable package" << std::endl; 
			// Forward to packet handlers

			if (clientIdentifier == InvalidIdentifier)
			{
				std::cout << "client was not connected" << std::endl;
				return;
			}
		}
		// else if reliable
		else if (mode == 1)
		{
			std::cout << "handling reliable package" << std::endl;


			if (clientIdentifier == InvalidIdentifier)
			{
				std::cout << "client was not connected" << std::endl;
				return;
			}
			// Forward to packet handlers
		}
		// else if connection type
		else if (mode == 2)
		{
			/*
			If the sender corresponds to the address of a client that is already connected, also reply with connection accepted.
			This is necessary because the first response packet may not have gotten through due to packet loss. If we don’t resend this response,
			the client gets stuck in the connecting state until it times out.
			*/
			if (clientIdentifier == InvalidIdentifier)
			{
				std::cout << "client already connected" << std::endl;
				// TODO: send connection accepted
				return;
			}

			// TODO challenge

			/*
			If the connection request is from a new client and we have a slot free,
			assign the client to a free slot and respond with connection accepted.
			*/
			uint16_t freeIdentifier = FindFreeClientIdentifier();
			if (freeIdentifier != InvalidIdentifier)
			{
				ClientAddress[freeIdentifier] = *msg->remote_addr();
				ClientConnected[freeIdentifier] = true;

				// Send connection accepted
			}
			
			/*
			If the server is full, reply with connection denied.
			*/
			else
			{
				// Server is full, send connection denied
			}
		}
		else
		{
			std::cout << "handling unknown package type" << std::endl;
		}
		// std::cout << "MSG:" << msg->NextAllString() << " - fd: " << msg->sockfd() << '\n';
		// evpp::udp::SendMessage(msg);
		});

	server.Init(ports);
	server.Start();

	while (!server.IsStopped()) { usleep(1); }

	return 0;
}