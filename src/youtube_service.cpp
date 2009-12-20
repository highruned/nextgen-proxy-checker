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

    nextgen::string video_id = "FF3ciqD9iks";
    nextgen::string user_agent = "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";

    //proxos::proxy proxy("159.213.87.22", 80);

    youtube::video v1(self->network_service);

    std::string query("SELECT proxy_host, proxy_port FROM proxies WHERE type_id = 4 AND state_id != 6 ORDER BY proxy_rating DESC, proxy_hits DESC, proxy_latency ASC"); //ORDER BY proxy_rating DESC

    std::cout << query << std::endl;

    auto list = *self->proxy_database.get_row_list(query);

    std::for_each(list.begin(), list.end(), [=](nextgen::database::row& row)
    {
        std::cout << (*row)["proxy_host"] << " " << (*row)["proxy_port"] << std::endl;

        proxos::proxy proxy((*row)["proxy_host"], to_int((*row)["proxy_port"]));

        v1.view(video_id, user_agent, 250, proxy);
    });

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
