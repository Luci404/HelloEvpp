
#define GOOGLE_STRIP_LOG 0
#include <evpp/udp/udp_server.h>
#include <evpp/udp/udp_message.h>

#include <vector>

int main(int argc, char** argv)
{
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (0 != err) {
        return 0;
    }

    std::vector<int> ports = { 1053, 5353 };
    evpp::udp::Server server;

    server.SetMessageHandler([](evpp::EventLoop* loop, evpp::udp::MessagePtr& msg) {
        evpp::udp::SendMessage(msg);
        });
    server.Init(ports);
    server.Start();

    while (!server.IsStopped()) {
        usleep(1);
    }
	
    return 0;
}