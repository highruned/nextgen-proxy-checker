#define FD_SETSIZE 65535//4096

#include "youtube_video.h"

class application : public nextgen::singleton<application>
{
    public: typedef nextgen::network::service network_service_type;

    public: void initialize()
    {
        auto self = *this;
    }

    public: void run();

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
    };

    NEXTGEN_SHARED_DATA(application, variables);
};

void application::run()
{
    auto self = *this;

    self->proxy_database.connect("localhost", "root", "swoosh", "proxies");

    nextgen::string video_id = "BlhlpNwUvCw";

    youtube::video v1(self->network_service);

    std::string q1("SELECT * FROM agents");

    if(YOUTUBE_DEBUG_1)
        std::cout << q1 << std::endl;

    auto agent_list = *self->proxy_database.get_row_list(q1);

    {
        std::string query("SELECT proxy_host, proxy_port, type_id FROM proxies WHERE proxy_id = 181294");//state_id = 8 ORDER BY proxy_rating DESC, proxy_hits DESC, proxy_latency ASC LIMIT 1,1");

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

            v1.view(video_id, 250, proxy, agent);
        });
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

int main()
{
    application::instance().run();
}
