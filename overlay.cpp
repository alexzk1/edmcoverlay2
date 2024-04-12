//this file was heavy simplified by alexzkhr@gmail.com in 2021

#include <iostream>
#include <stdlib.h>
#include <csignal>

#include "socket.hh"
#include "json_message.hh"
#include "drawables.h"
#include "xoverlayoutput.h"

constexpr unsigned short port = 5010;


static void sighandler(int signum)
{
    std::cout << "edmcoverlay2: got signal " << signum << std::endl;
    if ((signum == SIGINT) || (signum == SIGTERM))
    {
        std::cout << "edmcoverlay2: SIGINT/SIGTERM, exiting" << std::endl;
        exit(0);
    }
}

//FYI: test string to send over "telnet 127.0.0.1 5010"
//111#{"id": "test1", "text": "You are low on fuel!", "size": "normal", "color": "red", "x": 200, "y": 100, "ttl": 8}
int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cerr << "Usage: overlay X Y W H" << std::endl;
        return 1;
    }
    auto& drawer = XOverlayOutput::get(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

    //std::cout << "edmcoverlay2: overlay starting up..." << std::endl;
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    std::shared_ptr<tcp_server_t> server(new tcp_server_t(port), [](tcp_server_t *p)
    {
        if (p)
        {
            //have no idea why it cannot be called from server destructor, but ok
            //let's do wrapper
            p->close();
            delete p;
        }
    });

    drawer.cleanFrame();
    drawer.showVersionString("Binary is awaiting connection(s) from EDMC's plugins...", "green");
    drawer.flushFrame();
    //std::cout << "edmcoverlay2: overlay ready." << std::endl;

    while (true)
    {
        const static std::string stop_cmd = "NEED_TO_STOP";

        auto socket = server->accept_autoclose();
        const std::string request = read_response(*socket);

        if (request == stop_cmd)
        {
            break;
        }

        //std::cout << "edmcoverlay2: overlay got request: \"" << request << "\"" <<std::endl;

        drawer.cleanFrame();
        //drawer.showVersionString("edmcoverlay2 running", "white");

        draw_task::draw_list draws;
        try
        {
            draws = draw_task::parseJsonString(request);
        }
        catch (std::exception& e)
        {
            std::cerr << "Json parse failed with message: " << e.what() << std::endl;
            draws.clear();
        }
        catch (...)
        {
            std::cerr << "Json parse failed with uknnown reason." << std::endl;
            draws.clear();
        }
        for (const auto& drawitem : draws)
        {
            drawer.draw(drawitem);
        }

        drawer.flushFrame();
    }
    return 0;
}

