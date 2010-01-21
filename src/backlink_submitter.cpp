#include "nextgen/common.h"
#include "nextgen/network.h"
#include "nextgen/content.h"
#include "nextgen/database.h"

class application : public nextgen::singleton<application>
{
    public: void run(int, char**);

    private: struct variables
    {
        variables()
        {

        }

        nextgen::network::service network_service;
        nextgen::content::service content_service;
        nextgen::database::link proxy_database;
    };

    NEXTGEN_ATTACH_SHARED_VARIABLES(application, variables);
};

void application::run(int argc, char* argv[])
{
    auto self = *this;

    if(argc > 1)
    {
        std::string command = argv[1];

        if(command == "submit_site")
        {
            std::string site = argv[2];

            auto backlinks_txt = self->content_service.get_asset<nextgen::content::file_asset>("backlinks.txt");

            auto backlinks = nextgen::preg_match_all("(.+?)\n", backlinks_txt->data);

            std::for_each(backlinks.begin(), backlinks.end(), [=](std::string& backlink)
            {
                nextgen::network::http_client client(self->network_service);

                boost::trim(backlink);

                std::string host = nextgen::preg_match("http\\:\\/\\/(.+?)\\/", backlink);
                std::string path = nextgen::preg_match("http\\:\\/\\/.+\\/(.+?)$", backlink);

                nextgen::find_and_replace(path, "[URL]", nextgen::url_encode(site));

                std::cout << "path: " << path << std::endl;
                std::cout << "host: " << host << std::endl;
                std::cout << "backlink: " << backlink << std::endl;

                client.connect(host, 80,
                [=]
                {
                    nextgen::network::http_message m1;

                    m1->method = "GET";
                    m1->url = "/" + path;
                    m1->header_list["Host"] = host;
                    m1->header_list["User-Agent"] = "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";
                    m1->header_list["Keep-Alive"] = "300";
                    m1->header_list["Connection"] = "keep-alive";

                    client.send_and_receive(m1,
                    [=](nextgen::network::http_message r1)
                    {
                        std::cout << "Site submitted." << std::endl;

                        client.disconnect();
                    });
                });

                nextgen::sleep(0.01);
            });
        }
    }

    nextgen::timer timer;

    while(true)
    {
        if(timer.stop() > 1)
        {
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

