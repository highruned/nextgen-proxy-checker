#include "nextgen/common.h"
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

        nextgen::content::service content_service;
        nextgen::database::link proxy_database;
    };

    NEXTGEN_ATTACH_SHARED_VARIABLES(application, variables);
};

void application::run(int argc, char* argv[])
{
    auto self = *this;

    self->proxy_database.connect("localhost", "user", "boom7441", "proxies");

    if(argc > 1)
    {
        std::string command = argv[1];

        if(command == "add_proxies")
        {
            auto proxies = self->content_service.get_asset<nextgen::content::file_asset>("proxies.txt");

            boost::regex_error paren(boost::regex_constants::error_paren);

            try
            {
                boost::match_results<std::string::const_iterator> what;
                boost::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;

                std::string::const_iterator start = proxies->data.begin();
                std::string::const_iterator end = proxies->data.end();

                while(boost::regex_search(start, end, what, boost::regex("([1-2]\?[0-9]\{1,3\}\.[1-2]\?[0-9]\{1,3\}\.[1-2]\?[0-9]\{1,3\}\.[1-2]\?[0-9]\{1,3\})[:\ ]([1-9]\?[0-9]\{1,5\})"), flags))
                {
                    auto host = static_cast<std::string>(what[1]);
                    auto port = static_cast<std::string>(what[2]);

                    boost::trim(host);
                    boost::trim(port);

                    std::string q1 = "INSERT IGNORE INTO proxies SET proxy_host = \"" + host + "\", proxy_port = " + port + "";

                    self->proxy_database.query(q1);

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
        else if(command == "add_agents")
        {
            auto agents_txt = self->content_service.get_asset<nextgen::content::file_asset>("agents.txt");

            auto agents = nextgen::preg_match_all("(.+?)\r\n", agents_txt->data);

            std::for_each(agents.begin(), agents.end(), [=](std::string& agent)
            {
                boost::trim(agent);

                std::string q1 = "INSERT IGNORE INTO agents SET agent_title = \"" + agent + "\"";

                nextgen::sleep(0.01);

                self->proxy_database.query(q1);
            });
        }
    }
}

int main(int argc, char* argv[])
{
    application::instance().run(argc, argv);
}

