#define FD_SETSIZE 8192//4096

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
            variables()// : world(network_service, content_service)
            {

            }

            ~variables()
            {

            }

            network_service_type network_service;
            //nextgen::content::service content_service;
        };

        NEXTGEN_SHARED_DATA(application, variables);
	};
}

void proxos::application::run()
{
    auto self = *this;

    nextgen::string video_id = "vipprK5CnBo";
    nextgen::string user_agent = "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";

    youtube::video v1(self->network_service);
    //v1.view(video_id, user_agent, 200);

    nextgen::database::link proxy_database;

    proxy_database.connect("localhost", "root", "swoosh", "proxies");

    proxos::proxy_checker proxy_checker("www.proxyprobe.com", 8080, self->network_service, proxy_database);

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
