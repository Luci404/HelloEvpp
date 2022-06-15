#include <evpp/udp/udp_server.h>
#include <evpp/udp/udp_message.h>

#include <vector>

#define MAX_CLIENTS 64

struct ClientInfo
{
public:
	static const uint16_t InvalidIdentifier = UINT16_MAX;

public:
	uint16_t Identifier;

public:
	ClientInfo(uint16_t identifier)
		: Identifier(identifier)
	{}
};

ClientInfo ClientInfos[MAX_CLIENTS];
int main(int argc, char** argv)
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	int error = WSAStartup(wVersionRequested, &wsaData);
	if (0 != error) { return 0; }

	///

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		ClientInfos[i] = ClientInfo(ClientInfo::InvalidIdentifier);
	}

	///

	std::vector<int> ports = { 1053, 5353 };
	evpp::udp::Server server;

	server.SetMessageHandler([](evpp::EventLoop* loop, evpp::udp::MessagePtr& msg) {
		// In order of frequency

		// if unreliable

		// else if reliable

		// else if connection type
		{
			for (int i = 0; i < MAX_CLIENTS; ++i)
		{
			// find unused ClientInfo
			// Check if ip is connected
				// Send connection refused packet

			if (ClientInfos[i].Identifier == ClientInfo::InvalidIdentifier)
			{
				// Init client
				ClientInfos[i] = ClientInfo(i);
				// Send connection accepted with client info
				break;
			}

			// Failed to find an invalid identifier, server is full
			if (i == MAX_CLIENTS - 1)
			{
				// Send connection refused packet
			}
		}

		// std::cout << "MSG:" << msg->NextAllString() << " - fd: " << msg->sockfd() << '\n';
		// evpp::udp::SendMessage(msg);
	});

	server.Init(ports);
	server.Start();

	while (!server.IsStopped()) { usleep(1); }

	return 0;
}