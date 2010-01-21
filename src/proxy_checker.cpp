#define FD_SETSIZE 32768

#include "proxy_checker.h"

class application : public nextgen::singleton<application>
{
    typedef nextgen::network::http_proxy proxy_type;

    public: void check_proxy(proxy_type proxy, std::function<void()> callback = 0) const
    {
        auto self = *this;

        nextgen::network::address a(proxy->host);

        if(!a.is_valid())
        {
            std::cout << "INVALID PROXY ####################################" << std::endl;

            proxy->check_delay = 365 * 24 * 60 * 60;

            std::string query = "UPDATE proxies SET state_id = " + nextgen::to_string(proxy_type::states::invalid) + ", proxy_latency = 0, proxy_last_checked = NOW(), proxy_check_delay = " + nextgen::to_string(proxy->check_delay) + " WHERE proxy_id = " + nextgen::to_string(proxy->id) + " LIMIT 1";

            self->proxy_database.query(query);

            if(callback != 0)
                callback();

            return;
        }

        if(self.proxy_is_banned(a))
        // dont check proxy for another month
        {
            std::cout << "BANNED PROXY ####################################" << std::endl;

            proxy->check_delay = 365 * 24 * 60 * 60;

            std::string query = "UPDATE proxies SET state_id = " + nextgen::to_string(proxy_type::states::banned) + ", proxy_latency = 0, proxy_last_checked = NOW(), proxy_check_delay = " + nextgen::to_string(proxy->check_delay) + " WHERE proxy_id = " + nextgen::to_string(proxy->id) + " LIMIT 1";

            self->proxy_database.query(query);

            if(callback != 0)
                callback();

            return;
        }

        self->proxy_checker.check_proxy(proxy, [=]
        {
            if(proxy->type == proxy_type::types::none)
            {
                proxy->check_delay = 1 * 24 * 60 * 60;
                proxy->rating -= 1;
            }
            else if(proxy->type == proxy_type::types::transparent)
            {
                proxy->check_delay = 6 * 60 * 60;
                proxy->rating += 1;
            }
            else if(proxy->type == proxy_type::types::anonymous)
            {
                proxy->check_delay = 6 * 60 * 60;
                proxy->rating += 1;
            }
            else if(proxy->type == proxy_type::types::elite)
            {
                proxy->check_delay = 6 * 60 * 60;
                proxy->rating += 1;
            }
               else if(proxy->type == proxy_type::types::socks4)
            {
                proxy->check_delay = 6 * 60 * 60;
                proxy->rating += 1;
            }
            else if(proxy->type == proxy_type::types::socks5)
            {
                proxy->check_delay = 6 * 60 * 60;
                proxy->rating += 1;
            }

            if(proxy->state == proxy_type::states::codeen)
            {
                proxy->check_delay = 7 * 24 * 60 * 60;
            }

            // todo(daemn) check mysql table check table proxies.proxies; for status OK
            {
                std::string query = "UPDATE proxies SET type_id = " + nextgen::to_string(proxy->type)
                + ", proxy_latency = " + nextgen::to_string(proxy->latency)
                + ", state_id = " + nextgen::to_string(proxy->state)
                + ", proxy_rating = " + nextgen::to_string(proxy->rating)
                + ", proxy_last_checked = " + nextgen::to_string(time(0))
                + ", proxy_check_delay = " + nextgen::to_string(proxy->check_delay)
                + " WHERE proxy_id = " + nextgen::to_string(proxy->id) + " LIMIT 1";

                std::cout << query << " after " << nextgen::to_string(proxy->latency) << " seconds. " << std::endl;

                std::cout << "state: " << proxy->state << std::endl;

                self->proxy_database.query(query);
            }

            if(callback != 0)
                callback();
        });
    }

    public: bool proxy_is_banned(nextgen::network::address a) const
    {
        auto self = *this;

        for(auto i = self->banlist.begin(), l = self->banlist.end(); i != l; ++i)
        {
            if((*i).is_within_range(a))
                return true;
        }

        return false;
    }

    public: void run(int, char**);

    private: struct variables
    {
        variables() : proxy_checker("www.proxyprobe.com", 8080, network_service)
        {

        }

        nextgen::network::service network_service;
        nextgen::database::link proxy_database;
        proxos::proxy_checker proxy_checker;
        std::vector<nextgen::network::address_range> banlist;
    };

    NEXTGEN_ATTACH_SHARED_VARIABLES(application, variables);
};


void application::run(int argc, char* argv[])
{
    auto self = *this;

    self->proxy_database.connect("174.1.157.98", "root", "swoosh", "proxies");

    std::string query("SELECT * FROM banlist"); //ORDER BY proxy_rating DESC

    std::cout << query << std::endl;

    auto list = *self->proxy_database.get_row_list(query);

    // fill the banlist
    std::for_each(list.begin(), list.end(), [=](nextgen::database::row& row)
    {
        auto host = (*row)["ban_host"];
        auto description = (*row)["ban_description"];

        std::vector<std::string> ip;

        boost::split(ip, host, boost::is_any_of("-"));

        nextgen::network::address_range s(ip[0], ip[1]);

        self->banlist.push_back(s);
    });

    auto refill = [=](size_t amount)
    {
        std::cout << "Attempting to load " << amount << " proxies..." << std::endl;

        static uint32_t start = 0;

        std::string query("SELECT * "
        "FROM proxies "
        "WHERE state_id != " + nextgen::to_string(proxy_type::states::codeen) + " "
        "AND state_id != " + nextgen::to_string(proxy_type::states::banned) + " "
        "AND state_id != " + nextgen::to_string(proxy_type::states::invalid) + " "
        "AND proxy_last_checked < " + nextgen::to_string(time(0)) + " "
        "AND proxy_last_checked < (" + nextgen::to_string(time(0)) + " - proxy_check_delay) "
        "ORDER BY proxy_rating DESC "
        "LIMIT " + nextgen::to_string(start) + ", " + nextgen::to_string(amount));

        std::cout << query << std::endl;

        auto list = *self->proxy_database.get_row_list(query);

        std::for_each(list.begin(), list.end(), [=](nextgen::database::row& row)
        {
            auto r1 = *row;

            proxy_type proxy;
            proxy->host = r1["proxy_host"];
            proxy->port = nextgen::to_int(r1["proxy_port"]);
            proxy->id = nextgen::to_int(r1["proxy_id"]);
            proxy->rating = nextgen::to_int(r1["proxy_rating"]);
            proxy->state = nextgen::to_int(r1["state_id"]);
            proxy->type = nextgen::to_int(r1["type_id"]);

            // check the proxy against banlist
            self.check_proxy(proxy);
        });

        start += amount;

        if(list.size() < amount)
        // we've hit the end of the proxy list, loop back around and check for changes
        {
            start = 0;
            amount = list.size();
        }

        std::cout << "Loaded " << amount << " proxies." << std::endl;
    };

    if(argc > 1)
    {
        std::string command = argv[1];

        if(command == "check_proxy")
        {
            proxy_type proxy;
            proxy->host = argv[2];
            proxy->port = nextgen::to_int(argv[3]);

            self.check_proxy(proxy);
        }
        else if(command == "check_proxy_id")
        {
            std::string query("SELECT proxy_host, state_id, type_id, proxy_port, proxy_id, proxy_rating "
            "FROM proxies "
            "WHERE proxy_id = " + std::string(argv[2]));

            std::cout << query << std::endl;

            auto r1 = *self->proxy_database.get_row(query);

            proxy_type proxy;
            proxy->host = r1["proxy_host"];
            proxy->port = nextgen::to_int(r1["proxy_port"]);
            proxy->rating = nextgen::to_int(r1["proxy_rating"]);
            proxy->state = nextgen::to_int(r1["state_id"]);
            proxy->type = nextgen::to_int(r1["type_id"]);

            self.check_proxy(proxy);
        }
    }
    else
    {
        self->proxy_checker->refill_event += refill;
    }

    nextgen::timer timer;

    while(true)
    {
        if(timer.stop() > 10)
        {
            std::cout << "[proxos:application:run] Updating services..." << std::endl;
            std::cout << "C" << self->proxy_checker->job_list.size() << std::endl;
            std::cout << "D" << self->proxy_checker->judge_server->client_list.size() << std::endl;
            std::cout << "descriptors: " << nextgen::get_process_total_descriptors(nextgen::get_current_process_id()) << std::endl;

            for(nextgen::object_registry_type::iterator i = nextgen::object_registry.begin(), l = nextgen::object_registry.end(); i != l; ++i)
            {
                std::cout << (*i).first << ": " << (*i).second << std::endl;
            }

            timer.start();
        }

        self->proxy_checker.update();
        self->network_service.update();

        nextgen::sleep(0.05);
    }
}

int main(int argc, char* argv[])
{
    application::instance().run(argc, argv);
}
