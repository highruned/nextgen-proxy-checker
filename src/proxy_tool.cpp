#include "common.h"

class application : public nextgen::singleton<application>
{
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

        nextgen::content::service content_service;
        nextgen::database::link proxy_database;
    };

    NEXTGEN_SHARED_DATA(application, variables);
};

void application::run()
{
    auto self = *this;

    self->proxy_database.connect("localhost", "root", "swoosh", "proxies");

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

            std::string query = "INSERT IGNORE INTO proxies SET proxy_host = \"" + host + "\", proxy_port = " + port + "";

            //std::cout << query << std::endl;

            boost::this_thread::sleep(boost::posix_time::milliseconds(10));

            self->proxy_database.query(query);

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

int main()
{
    application::instance().run();
}

