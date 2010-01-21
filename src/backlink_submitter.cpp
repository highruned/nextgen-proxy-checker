#include "common.h"

class application : public nextgen::singleton<application>
{
    public: void initialize()
    {
        auto self = *this;

    }

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

            std::cout << "site: " << argv[2] << std::endl;

            auto backlinks = self->content_service.get_asset<nextgen::content::file_asset>("backlinks.txt");

            std::cout << "Data: " << backlinks->data << std::endl;

            boost::regex_error paren(boost::regex_constants::error_paren);

            try
            {
                boost::match_results<std::string::const_iterator> what;
                boost::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;

                std::string::const_iterator start = backlinks->data.begin();
                std::string::const_iterator end = backlinks->data.end();

                while(boost::regex_search(start, end, what, boost::regex("(.+?)\n"), flags))
                {
                    nextgen::network::http_client client(self->network_service);

                    auto backlink = static_cast<std::string>(what[1]);

                    boost::trim(backlink);

                    nextgen::string host = nextgen::regex_single_match("http\\:\\/\\/(.+?)\\/", backlink);
                    nextgen::string path = nextgen::regex_single_match("http\\:\\/\\/.+\\/(.+?)$", backlink);

                    find_and_replace(path, "[URL]", url_encode(site));

                    std::cout << "path: " << path << std::endl;
                    std::cout << "host: " << host << std::endl;
                    std::cout << "backlink: " << backlink << std::endl;
//if(0) {
                    client.connect(host, 80, nextgen::network::ipv4_address(host, 80),
                    [=]
                    {
                        std::cout << "connected" << std::endl;

                        nextgen::network::http_message request;

                        request->method = "GET";
                        request->url = "/" + path;
                        request->header_list["Host"] = host;
                        request->header_list["User-Agent"] = "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";
                        request->header_list["Keep-Alive"] = "300";
                        request->header_list["Connection"] = "keep-alive";

                        client.send_and_receive(request,
                        [=](nextgen::network::http_message response)
                        {
                            std::cout << "submitted" << std::endl;

                            client.disconnect();
                        });
                    });// }

                    nextgen::sleep(0.01);

                    // update search position:
                    start = what[0].second;

                    // update flags:
                    flags |= boost::match_prev_avail;
                    flags |= boost::match_not_bob;
                }
            }
            catch(boost::regex_error const& e)
            {
                std::cout << "regex error: " << (e.code() == paren.code() ? "unbalanced parentheses" : "?") << std::endl;
            }
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

