#include "main.h"
/*
namespace nextgen
{
    class asset
    {

    };
}

namespace nextgen
{
    namespace content
    {
        class tile_asset
        {
            private: struct variables
            {
                variables()
                {

                }

                ~variables()
                {

                }

                uint32_t id;
                string data;
            };

            NEXTGEN_SHARED_DATA(tile_asset, variables);
        };

        class service
        {
            public: template<typename element_type> element_type get_asset(string const& name)
            {
                auto self = *this;

                if(self->asset_list.size() > 0)
                {
                    if(auto i = self->asset_list.find(name) != self->asset_list.end())
                    {
                        return self->asset_list[name];
                    }
                }

                std::ifstream f;
                f.open(name, std::ios::in | std::ios::binary);

                if(f.is_open())
                {
                    // get length of file:
                    f.seekg(0, std::ios::end);
                    size_t length = f.tellg();
                    f.seekg(0, std::ios::beg);

                    element_type e;

                    // read data as a block:
                    char data[length];
                    f.read(data, length);

                    f.close();

                    e->data = string(data);

                    return e;
                }
                else
                {
                    throw "File not open.";
                }


            }

            private: struct variables
            {
                variables()
                {

                }

                ~variables()
                {

                }

                hash_map<string, tile_asset> asset_list;
            };

            NEXTGEN_SHARED_DATA(service, variables);
        };

    }
}
*/
/*
namespace nextgen
{
    namespace engine
    {
            class application_object
            {
                public: typedef uint32_t id_type;
                public: typedef string name_type;
                public: typedef int vector_type;
                public: typedef math::vector<vector_type> position_type;
                public: typedef math::vector<vector_type> rotation_type;

                private: struct variables
                {
                    variables() : id(0), name("Object")
                    {

                    }

                    ~variables()
                    {

                    }

                    id_type id;
                    name_type name;
                    position_type position;
                    rotation_type rotation;
                    content::tile_asset model;
                };

                NEXTGEN_SHARED_DATA(application_object, variables);
            };

            class application_npc
            {
                public: typedef uint32_t id_type;
                public: typedef string name_type;
                public: typedef int vector_type;
                public: typedef math::vector<vector_type> position_type;
                public: typedef math::vector<vector_type> rotation_type;

                private: struct variables
                {
                    variables() : id(0), name("Undefined")
                    {

                    }

                    ~variables()
                    {

                    }

                    id_type id;
                    name_type name;
                    position_type position;
                    rotation_type rotation;
                };

                NEXTGEN_SHARED_DATA(application_npc, variables);
            };

            class application_monster
            {

            };

            class application_player
            {
                public: typedef uint32_t id_type;
                public: typedef string name_type;
                public: typedef int vector_type;
                public: typedef math::vector<vector_type> position_type;
                public: typedef math::vector<vector_type> rotation_type;
                public: typedef network::ngp_client client_type;

                private: struct variables
                {
                    variables(client_type client) : id(0), name("Undefined"), client(client)
                    {

                    }

                    ~variables()
                    {

                    }

                    id_type id;
                    name_type name;
                    position_type position;
                    rotation_type rotation;
                    client_type client;
                };

                NEXTGEN_SHARED_DATA(application_player, variables);
            };


        namespace realm_message
        {
            const uint32_t keep_alive = 1;
            const uint32_t user_login = 2;
            const uint32_t create_world = 3;
            const uint32_t update_world = 4;
            const uint32_t remove_world = 5;
            const uint32_t create_object = 6;
            const uint32_t update_object = 7;
            const uint32_t remove_object = 8;
            const uint32_t create_player = 9;
            const uint32_t update_player = 10;
            const uint32_t remove_player = 11;
            const uint32_t chat_message = 12;
            const uint32_t load_asset = 13;

            const uint32_t update_player_position = 41;
            const uint32_t update_player_rotation = 42;

        }

        struct application_message
        {
            static const uint32_t keep_alive;
        };

        struct chat_message
        {

        };



        class application_world
        {
            public: typedef network::service network_service_type;
            public: typedef array<application_player> player_list_type;
            public: typedef math::vector<int> vector_type;
            public: typedef vector_type position_type;
            public: typedef vector_type rotation_type;

            public: void initialize()
            {
                auto self = *this;

                network::create_server<network::ngp_client>(self->network_service, 6110,
                [=](network::ngp_client client)
                {
                    std::cout << "[nextgen:engine:application_world] Server (port 6110) accepted NGP client." << std::endl;

                    client.receive(
                    [=](network::ngp_message request)
                    {
                        std::cout << "[nextgen:engine:application_world] Received message from NGP client." << std::endl;


                    },
                    [=]()
                    {
                        std::cout << "Removing player #" << std::endl;

                    });
                });
            }

            private: struct variables
            {
                variables(network_service_type network_service, content::service content_service) : network_service(network_service), content_service(content_service)
                {

                }

                ~variables()
                {

                }

                network_service_type network_service;
                content::service content_service;
            };

            NEXTGEN_SHARED_DATA(application_world, variables,
            {
                this->initialize();
            });
        };

    }
}


*/
namespace nextgen
{
    string regex_single_match(string const& pattern, string const& subject)
    {

        boost::regex_error paren(boost::regex_constants::error_paren);

        try
        {
            boost::match_results<std::string::const_iterator> what;
            boost::regex_constants::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;

            if(boost::regex_search((std::string::const_iterator)subject.begin(), (std::string::const_iterator)subject.end(), what, boost::regex(pattern), flags))
                return what[1];
        }
        catch(boost::regex_error const& e)
        {
            std::cout << "regex error: " << (e.code() == paren.code() ? "unbalanced parentheses" : "?") << std::endl;
        }

        return "Null";
    }
}


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

    nextgen::string video_id = "-48L4FAOcQg";
    nextgen::string user_agent = "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";

    nextgen::network::http_client youtube(self->network_service);

    youtube.connect("youtube.com", 80, [=]()
    {
        std::cout << "[proxos:youtube] Connected." << std::endl;

        nextgen::network::http_message m1;

        m1->method = "GET";
        m1->url = "http://www.youtube.com/watch?v=" + video_id;
        m1->header_list["Host"] = "www.youtube.com";
        m1->header_list["User-Agent"] = user_agent;
        m1->header_list["Keep-Alive"] = "300";
        m1->header_list["Connection"] = "keep-alive";

        youtube.send_and_receive(m1, [=](nextgen::network::http_message r1)
        {
            std::cout << "[proxos:youtube] Received video page response." << std::endl;

            std::cout << "c_length " << r1->content.length() << std::endl;

            if(r1->status_code != 200)
                std::cout << "[proxos:youtube] Error receiving video page. " << r1->status_code << std::endl;

            nextgen::network::http_message m2;

            m2->method = "GET";
            m2->url = "/get_video_info?video_id=" + video_id;
            m2->header_list["Host"] = "www.youtube.com";
            m2->header_list["User-Agent"] = user_agent;
            m2->header_list["Keep-Alive"] = "300";
            m2->header_list["Connection"] = "keep-alive";
            m2->header_list["Cookie"] = r1->header_list["Set-Cookie"];

            youtube.send_and_receive(m2, [=](nextgen::network::http_message r2)
            {
                std::cout << "[proxos:youtube] Received video info response." << std::endl;

                std::cout << r2->content << std::endl;

                if(r2->status_code != 200)
                    std::cout << "[proxos:youtube] Error receiving video info. " << r2->status_code << std::endl;

                nextgen::string token = nextgen::regex_single_match("&token=(.+?)&thumbnail_url", r2->content);



                // parse token
                //nextgen::string token = regex(r2->content, "&token=(.+?)&thumbnail_url");

                nextgen::network::http_message m3;

                m3->method = "GET";
                m3->url = "/get_video?video_id=" + video_id + "&t=" + token + "&el=detailpage&ps=&fmt=5&noflv=1";
                m3->header_list["Host"] = "www.youtube.com";
                m3->header_list["User-Agent"] = user_agent;
                m3->header_list["Keep-Alive"] = "300";
                m3->header_list["Connection"] = "keep-alive";
                m3->header_list["Cookie"] = r1->header_list["Set-Cookie"];

                youtube.send_and_receive(m3, [=](nextgen::network::http_message r3)
                {
                    if(r3->status_code != 204)
                        std::cout << "[proxos:youtube] Error receiving video download. " << r3->status_code << std::endl;
                    else
                         std::cout << "[proxos:youtube] Received video download response." << std::endl;



                });
            });
        });
    });




/*
    nextgen::network::create_server<nextgen::network::http_client>(self->network_service, 80,
    [=](nextgen::network::http_client client)
    {
        std::cout << "[proxos:application:run:server] Server (port 80) accepted HTTP client." << std::endl;

        client.receive(
        [=](nextgen::network::http_message request)
        {
            std::cout << "[proxos:application:run:server] Received data from HTTP client." << std::endl;

            std::cout << request->raw_header_list << std::endl;
            std::cout << request->content << std::endl;

            nextgen::network::http_message response;

            response->version = "1.1";
            response->status_code = 200;
            response->header_list["Connection"] = "close";
            response->content = "hi";

            client.send(response);
            client.disconnect();

        },
        []()
        {

        });
    });

    nextgen::network::create_server<nextgen::network::xml_client>(self->network_service, 843,
    [=](nextgen::network::xml_client client)
    {
        std::cout << "[proxos:application:run:server] Server (port 843) accepted XML client." << std::endl;

        client.receive(
        [=](nextgen::network::xml_message request)
        {
            std::cout << "[proxos:application:run:server] Received data from XML client (length: " << request->data.length() << ")" << std::endl;

            std::cout << request->data << std::endl;

            if(request->data.find("<policy-file-request/>") != nextgen::string::npos)
            {
                nextgen::network::xml_message response;

                response->data = "<?xml version=\"1.0\"?><cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"6110-6112\" /></cross-domain-policy>\0";

                client.send(response);
            }

            client.disconnect();
        },
        []()
        {

        });
    });*/

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
            }

            self->network_service.update();

            boost::this_thread::sleep(boost::posix_time::milliseconds(5));
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
