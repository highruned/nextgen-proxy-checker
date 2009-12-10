#define FD_SETSIZE 65535//4096

#include "common.h"
#include "proxy_checker.h"

namespace proxos
{
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
        };

        NEXTGEN_SHARED_DATA(application, variables);
	};
}

void proxos::application::run()
{
    auto self = *this;

    nextgen::database::link proxy_database;

    proxy_database.connect("localhost", "root", "swoosh", "proxies");

    proxos::proxy_checker proxy_checker("www.proxyprobe.com", 8080, self->network_service, proxy_database);

    proxy_checker->refill_event += [=](size_t amount)
    {
        std::cout << "Loading " << amount << " proxy..." << std::endl;

        static uint32_t start = 0;

        std::string query("SELECT proxy_host, proxy_port, proxy_id FROM proxies WHERE proxy_last_checked < (NOW() - proxy_check_delay) ORDER BY proxy_id LIMIT " + to_string(start) + ", " + to_string(amount)); //ORDER BY proxy_rating DESC

        std::cout << query << std::endl;

        auto list = *proxy_database.get_row_list(query);

        //auto proxy2 = proxy; // bugfix(daemn) gah

        std::for_each(list.begin(), list.end(), [=](nextgen::database::row& row)
        {
            proxos::proxy proxy((*row)["proxy_host"], to_int((*row)["proxy_port"]), to_int((*row)["proxy_id"]));

            proxy_checker.check_proxy(proxy, [=]()
            {
                if(proxy.get_type() == "codeen")
                    proxy.set_check_delay("0000-00-07 00:00:00");
                else if(proxy.get_type() == "broken")
                    proxy.set_check_delay("0000-00-01 00:00:00");

                // todo(daemn) check mysql table check table proxies.proxies; for status OK
                {
                    std::string query = "UPDATE proxies SET proxy_type = \"" + proxy.get_type() + "\", proxy_latency = 0, proxy_rating = proxy_rating - 1, proxy_last_checked = NOW(), proxy_check_delay = \"" + proxy.get_check_delay() + "\" WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";

                    std::cout << query << " after " << to_string(proxy.get_latency()) << " seconds. " << std::endl;

                    std::cout << "state: " << proxy.get_state() << std::endl;

                    proxy_database.query(query);
                }
            });
        });

        start += amount;

        if(list.size() < amount)
        // we've hit the end of the proxy list, loop back around and check for changes
        {
            start = 0;
            amount = list.size();
        }

        std::cout << "Loaded " << amount << " proxies." << std::endl;
        //proxy_checker.add_list(); //proxy_rating DESC, proxy_hits DESC, proxy_latency ASC,


    };

    nextgen::timer timer;

    while(true)
    {
        try
        {

            if(timer.stop() > 1)
            {
                timer.start();

                std::cout << "[proxos:application:run] Updating services..." << std::endl;
                std::cout << "C" << proxy_checker->job_list.size() << std::endl;
                std::cout << "D" << proxy_checker->server->client_list.size() << std::endl;
                //std::cout << "e" << proxy_checker->client_count << std::endl;
            }

            proxy_checker.update();

            self->network_service.update();

            boost::this_thread::sleep(boost::posix_time::milliseconds(10));
        }
        catch(boost::exception& e)
        {
            std::cout << "[proxos:application:run] " << "Unexpected exception caught in " << BOOST_CURRENT_FUNCTION << std::endl << boost::current_exception_diagnostic_information();
        }
    }
}

int main()
{
    proxos::application::instance().run();
}
