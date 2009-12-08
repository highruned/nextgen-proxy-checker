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

    {
        std::string query("SELECT proxy_host, proxy_port, proxy_id FROM proxies WHERE proxy_last_checked < (NOW() - proxy_check_delay) ORDER BY proxy_id LIMIT " + to_string(self->proxy_row_start) + ", " + to_string(self->proxy_row_end)); //ORDER BY proxy_rating DESC

        std::cout << query << std::endl;

        auto list = proxy_database.get_row_list(query);

        std::for_each(list.begin(), list.end(), [=](nextgen::database::row row)
        {
            proxy_checker.add_proxy(proxos::proxy(row));
        });

        //proxy_checker.add_list(); //proxy_rating DESC, proxy_hits DESC, proxy_latency ASC,
    }

    nextgen::timer timer;
    timer.start();

    while(true)
    {
        try
        {
            if(timer.stop() > 1)
            {
                timer.start();

                std::cout << "[proxos:application:run] Updating services..." << std::endl;
                std::cout << proxy_checker.get_client_list().size() << std::endl;
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
