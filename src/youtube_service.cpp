#define FD_SETSIZE 65535//4096

#include "youtube_video.h"

class application : public nextgen::singleton<application>
{
    public: typedef nextgen::network::service network_service_type;

    public: void initialize()
    {
        auto self = *this;
    }

    public: void run(int argc, char* argv[]);

    private: struct variables
    {
        variables()
        {

        }

        ~variables()
        {

        }

        network_service_type network_service;
        nextgen::database::link proxy_database;
        nextgen::network::server<nextgen::network::smtp_client> mail_server;
    };

    NEXTGEN_SHARED_DATA(application, variables);
};

void application::run(int argc, char* argv[])
{
    auto self = *this;

    self->proxy_database.connect("localhost", "root", "swoosh", "proxies");

    if(argc > 1)
    {
        std::string command = argv[1];

        if(command == "view_video")
        {
            youtube::video video;
            video->id = argv[2];//"46r3LHabaFU";//pCAwU3-9n18";//xdEy5oAh3dk";

            std::string q1("SELECT * FROM agents WHERE agent_type_id = 1");

            if(YOUTUBE_DEBUG_1)
                std::cout << q1 << std::endl;

            auto agent_list = *self->proxy_database.get_row_list(q1);

            {//proxy_id = 1049
                std::string query("SELECT proxy_host, proxy_port, type_id FROM proxies WHERE state_id = 8 AND proxy_port != 3124 AND proxy_port != 3127");//state_id = 8 ORDER BY proxy_rating DESC, proxy_hits DESC, proxy_latency ASC LIMIT 1,1");

                if(YOUTUBE_DEBUG_1)
                    std::cout << query << std::endl;

                auto proxy_list = *self->proxy_database.get_row_list(query);

                std::for_each(proxy_list.begin(), proxy_list.end(), [=](nextgen::database::row& row)
                {
                    if(YOUTUBE_DEBUG_1)
                        std::cout << (*row)["proxy_host"] << " " << (*row)["proxy_port"] << std::endl;

                    proxos::proxy proxy((*row)["proxy_host"], to_int((*row)["proxy_port"]));
                    proxy->type = to_int((*row)["type_id"]);

                    auto row2 = agent_list[nextgen::random(0, (int)agent_list.size()-1)];

                    proxos::agent agent((*row2)["agent_title"]);

                    youtube::client c1(self->network_service);
                    c1->proxy = proxy;
                    c1->agent = agent;

                    c1.view_video(video, 250);
                });
            }
        }
        else if(command == "create_account")
        {
            youtube::client c1(self->network_service);

            c1.create_account("abcd", "dcba", "abdk@adalda.com");
        }
    }

    nextgen::timer timer;

    while(true)
    {
        if(timer.stop() > 1)
        {
            //std::cout << "[application:run] Updating services..." << std::endl;

            timer.start();
        }

        self->network_service.update();

        nextgen::sleep(0.01);
    }
}

int main(int argc, char* argv[])
{
    application::instance().run(argc, argv);
}
