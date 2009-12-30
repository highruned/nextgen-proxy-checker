#ifndef PROXOS_PROXY_CHECKER
#define PROXOS_PROXY_CHECKER

#include "nextgen/common.h"
#include "nextgen/network.h"
#include "nextgen/database.h"
#include "nextgen/social.h"

bool PROXOS_DEBUG_1 = 1;

namespace proxos
{


	class proxy
	{
	    public: typedef proxy this_type;
        public: typedef std::string host_type;
        public: typedef uint32_t port_type;
        public: typedef uint32_t id_type;
        public: typedef uint32_t type_type;
        public: typedef float latency_type;
        public: typedef nextgen::timer timer_type;

        public: struct types
        {
            static const uint32_t none = 0;
            static const uint32_t transparent = 1;
            static const uint32_t distorting = 2;
            static const uint32_t anonymous = 3;
            static const uint32_t elite = 4;
            static const uint32_t socks4 = 5;
            static const uint32_t socks5 = 6;
            static const uint32_t socks4n5 = 7;
        };

        public: struct states
        {
            static const uint32_t none = 0;
            static const uint32_t can_only_send = 1;
            static const uint32_t cannot_send_back = 2;
            static const uint32_t bad_return_headers = 3;
            static const uint32_t bad_return_data = 4;
            static const uint32_t cannot_send = 5;
            static const uint32_t cannot_connect = 6;
            static const uint32_t codeen = 7;
            static const uint32_t perfect = 8;
            static const uint32_t banned = 9;
            static const uint32_t invalid = 10;
            //user_agent_via, cache_control, cache_info, connection_close, connection_keep_alive,
        };

		public: host_type const& get_host() const
		{
		    auto self = *this;

			return self->host;
		}

		public: void set_host(host_type const& host) const
		{
		    auto self = *this;

			self->host = host;
		}

		public: port_type get_port() const
		{
		    auto self = *this;

			return self->port;
		}

		public: void set_port(port_type port) const
		{
		    auto self = *this;

			self->port = port;
		}

		public: id_type get_id() const
		{
		    auto self = *this;

			return self->id;
		}

		public: void set_id(id_type id) const
		{
		    auto self = *this;

			self->id = id;
		}


		public: latency_type get_latency() const
		{
		    auto self = *this;

			return self->latency;
		}

		public: void set_latency(latency_type latency) const
		{
		    auto self = *this;

			self->latency = latency;
		}

		public: timer_type get_timer() const
		{
		    auto self = *this;

			return self->timer;
		}

		public: void set_timer(timer_type timer) const
		{
		    auto self = *this;

			self->timer = timer;
		}

		public: uint32_t get_state() const
		{
		    auto self = *this;

		    return self->state;
		}

		public: void set_state(uint32_t state) const
		{
		    auto self = *this;

            self->state = state;
		}


		public: void from_row(nextgen::database::row& row) const
		{
		    auto self = *this;

			self->host = (*row)["proxy_host"];
			self->port = to_int((*row)["proxy_port"]);
			self->id = to_int((*row)["proxy_id"]);
		}

        private: struct variables
        {
            variables(nextgen::database::row& row)
            {
                variables((*row)["proxy_host"], to_int((*row)["proxy_port"]), to_int((*row)["proxy_id"]));
            }

            variables(host_type const& host = "undefined.com", port_type port = 0, id_type id = 0, type_type type = 0, latency_type latency = 0) : rating(0), host(host), port(port), id(id), type(0), latency(0.0), state(0), check_delay(6 * 60 * 60)
            {

            }

            ~variables()
            {

            }

            int32_t rating;
            host_type host;
            port_type port;
            id_type id;
            uint32_t type;
            latency_type latency;
            timer_type timer;
            uint32_t state;
            uint32_t check_delay;
        };

        NEXTGEN_SHARED_DATA(proxy, variables);
	};

}

namespace proxos
{
    class proxy_checker
    {
        class job_type
        {
            public: typedef nextgen::network::service network_service_type;
            public: typedef nextgen::network::http_client client_type;
            public: typedef std::function<void()> callback_type;
            public: typedef proxy proxy_type;

            private: struct variables
            {
                variables(proxy_type proxy, network_service_type network_service, callback_type callback = 0) : complete(false), proxy(proxy), client(network_service), callback(callback)
                {
                    if(callback == 0)
                        callback = []() { std::cout << "No callback." << std::endl; };
                }

                ~variables()
                {

                }

                bool complete;
                proxy_type proxy;
                client_type client;
                callback_type callback;
            };

            NEXTGEN_SHARED_DATA(job_type, variables);
        };

        public: typedef proxos::proxy proxy_type;
        public: typedef std::list<job_type> job_list_type;
        public: typedef nextgen::network::service network_service_type;
        public: typedef uint32_t client_max_type;
        public: typedef nextgen::timer timer_type;
        public: typedef nextgen::network::http_message message_type;
        public: typedef nextgen::network::http_client client_type;
        public: typedef nextgen::network::http_server server_type;
        public: typedef nextgen::database::link database_type;
        public: typedef std::function<void(size_t)> refill_event_type;

        public: void check_proxy(proxy_type proxy, std::function<void()> callback = 0) const
        {
            auto self = *this;

            if(self.find_job(proxy->id) == self->job_list.end())
            // add the proxy to the list if it doesnt already exist
            {
                auto v = job_type(proxy, self->network_service, callback);

                self->job_list.push_back(v);
            }
        }

        public: job_list_type::iterator find_job(uint32_t id) const
        {
            auto self = *this;

            return std::find_if(self->job_list.begin(), self->job_list.end(), [=](job_type& job) -> bool
            {
                return id == job->proxy.get_id();
            });
        }

        public: void remove_job(uint32_t id) const
        {
            auto self = *this;

            job_list_type::iterator i;

            if((i = self.find_job(id)) != self->job_list.end())
            {
                self->job_list.erase(i);
            }
        }

        public: void initialize()
        {
            auto self = *this;

            self->server.accept([=](client_type client)
            {
                if(PROXOS_DEBUG_1)
                    std::cout << "[proxos:proxy_server] Proxy server (port 8080) accepted HTTP client." << std::endl;

                client.receive(
                [=](message_type r1)
                {
                    if(PROXOS_DEBUG_1)
                        std::cout << "[proxos:proxy_server] Received data from HTTP client." << std::endl;
                    //  can use cookies, can

                    // proxy can send headers
                    if(r1->header_list.find("pid") != r1->header_list.end())
                    {
                        job_list_type::iterator i;

                        if((i = self.find_job(to_int(r1->header_list["pid"]))) != self->job_list.end())
                        // check if job wasnt removed because proxy server killed the connection before sending out packets
                        {
                            auto proxy = (*i)->proxy;

                            if(r1->raw_header_list.find("127.0.0.1") != std::string::npos
                            || r1->raw_header_list.find("174.1.157.98") != std::string::npos)
                                proxy->type = proxy_type::types::transparent;
                            else if(r1->header_list.find("Via") != r1->header_list.end()
                            || r1->header_list.find("x-forwarded-for") != r1->header_list.end()
                            || r1->header_list.find("forwarded") != r1->header_list.end()
                            || r1->header_list.find("client-ip") != r1->header_list.end())
                                proxy->type = proxy_type::types::anonymous;
                            else
                                proxy->type = proxy_type::types::elite;

                            message_type r2;

                            r2->version = "1.x";
                            r2->status_code = 200;
                            r2->header_list["Content-Type"] = "text/html";
                            //r2->header_list["Proxy-Connection"] = "close";
                            r2->header_list["Connection"] = "close";
                            r2->header_list["Set-Cookie"] = "my_cookie";
                            //r2->header_list["PID"] = r1->header_list["PID"];
                            r2->content = "my_data";

                            if(PROXOS_DEBUG_1)
                                std::cout << "[proxos:proxy_server] Server sending response to " << proxy.get_host() << ":" << proxy.get_port() << std::endl;

                            client.send(r2,
                            [=]()
                            {
                                if(PROXOS_DEBUG_1)
                                    std::cout << "[proxos:proxy_server] Server response successful to " << proxy.get_host() << proxy.get_port() << std::endl;

                                // set proxy property to successful send
                                proxy.set_state(proxy_type::states::can_only_send);

                                client.disconnect();
                            },
                            [=]()
                            {
                                if(PROXOS_DEBUG_1)
                                    std::cout << "[proxos:proxy_server] Server response failure to " << proxy.get_host() << proxy.get_port() << std::endl;

                                proxy.set_state(proxy_type::states::cannot_send_back);
                            });
                        }
                        // special case for initial confirmation we're hooked up correctly
                        else if(to_int(r1->header_list["pid"]) == 0)
                        {
                            if(PROXOS_DEBUG_1)
                                std::cout << "[proxos:proxy_server] got confirmation" << std::endl;

                            message_type r2;

                            r2->version = "1.x";
                            r2->status_code = 200;
                            r2->header_list["Content-Type"] = "text/html";
                            r2->header_list["Connection"] = "close";
                            r2->header_list["Set-Cookie"] = "my_cookie";
                            r2->content = "my_data";

                            client.send(r2,
                            [=]()
                            {
                                client.disconnect();
                            });
                        }
                        else
                        {
                            if(PROXOS_DEBUG_1)
                                std::cout << "[proxos:proxy_server] GAY PROXY" << std::endl;

                            client.disconnect();
                        }
                    }
                    else
                    // this request didn't come from checker, or has been modified
                    {
                        client.disconnect();
                    }
                },
                [=]()
                {
                    std::cout << "[proxos:proxy_server] Failed to receive data from HTTP client." << std::endl;
                });
            },
            [=]()
            {
                std::cout << "[proxos:proxy_server] Failed to accept HTTP client." << std::endl;
            });

            self.test_connection();
        }

        private: void test_connection() const
        {
            auto self = *this;

            if(PROXOS_DEBUG_1)
                std::cout << "[proxos:proxy_checker] Testing connection." << std::endl;

            nextgen::network::http_client client(self->network_service);

            client.connect(self->host, self->port, nextgen::network::ipv4_address(self->host, self->port),
            [=]()
            {
                if(PROXOS_DEBUG_1)
                    std::cout << "[proxos:proxy_checker] Connected to self." << std::endl;

                message_type r1;

                r1->method = "GET";
                r1->url = "http://" + self->host + ":" + to_string(self->port) + "/" + to_string(0);

                r1->version = "1.1";

                r1->header_list["Host"] = self->host + ":" + to_string(self->port);
                r1->header_list["User-Agent"] = "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.1) Gecko/20090624 Firefox/3.5 (.NET CLR 3.5.30729)"; //"Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";//
                r1->header_list["PID"] = to_string(0);
                r1->header_list["Keep-Alive"] = "300";
                r1->header_list["Connection"] = "keep-alive";

                client.send_and_receive(r1,
                [=](message_type r2)
                {
                    // can receive data
                    if(r2->content.find("my_data") != std::string::npos)
                    {
                        self->enabled = true;
                    }
                    else
                    {
                        self->enabled = false;
                    }
                },
                [=]()
                {
                    self->enabled = false;
                });
            },
            [=]()
            {
                self->enabled = false;
            });
        }

        private: void connect(job_type job) const
        {
            auto self = *this;

            auto client = job->client;
            auto proxy = job->proxy;
            auto callback = job->callback;

            if(PROXOS_DEBUG_1)
                std::cout << "[proxos:proxy_client] Attempting to connect to " << proxy->host << ":" << proxy->port << " (" << proxy->id << ")" << std::endl;

std::cout << "proxy_type2: " << proxy->type << std::endl;

            switch(proxy->type)
            {
                case proxy_type::types::transparent:
                case proxy_type::types::distorting:
                case proxy_type::types::anonymous:
                    client->proxy = "http"; break;

                case proxy_type::types::socks4: client->proxy = "socks4"; break;
                case proxy_type::types::socks5: client->proxy = "socks5"; break;
                case proxy_type::types::socks4n5: client->proxy = "socks4"; break;
            }

            client.connect(self->host, self->port, nextgen::network::ipv4_address(proxy->host, proxy->port),
            [=]()
            {
                if(PROXOS_DEBUG_1)
                    std::cout << "[proxos:proxy_client] Connected to proxy " << proxy->host + ":" + to_string(proxy->port) << "." << std::endl;

                message_type r1;

                r1->method = "GET";
                r1->url = "http://" + self->host + ":" + to_string(self->port) + "/" + to_string(proxy->id);

                r1->version = "1.1";

                r1->header_list["Host"] = self->host + ":" + to_string(self->port);
                r1->header_list["User-Agent"] = "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.1) Gecko/20090624 Firefox/3.5 (.NET CLR 3.5.30729)"; //"Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";//
                r1->header_list["PID"] = to_string(proxy->id);
                r1->header_list["Keep-Alive"] = "300";
                r1->header_list["Connection"] = "keep-alive";

                proxy.get_timer().start();

                client.send_and_receive(r1,
                [=](message_type r2)
                {
                    if(PROXOS_DEBUG_1)
                    {
                        std::cout << "[proxos:proxy_client] Client send/receive successful." << std::endl;

                        std::cout << r2->raw_header_list << std::endl;
                        std::cout << r2->content << std::endl;
                    }

                    proxy.set_latency(proxy.get_timer().stop());

                    if(r2->content.find("CoDeeN") != std::string::npos
                    || r2->content.find("PlanetLab") != std::string::npos)
                    {
                        proxy.set_state(proxy_type::states::codeen);
                    }
                    // proxy can receive data
                    else if(r2->content.find("my_data") != std::string::npos)
                    {
                        // proxy can receive headers
                        if(r2->header_list.find("set-cookie") != r2->header_list.end()
                        && r2->header_list["set-cookie"].find("my_cookie") != std::string::npos)
                        {
                            proxy.set_state(proxy_type::states::perfect);

                            if(PROXOS_DEBUG_1)
                                std::cout << "good proxy" << std::endl;
                        }
                        // proxy cannot receive headers
                        else
                        {
                            proxy.set_state(proxy_type::states::bad_return_headers);

                            if(PROXOS_DEBUG_1)
                                std::cout << "goodish proxy - doesnt forward headers correctly - correct_headers = false" << std::endl;
                        }

                        if(client->proxy == "socks4")
                        {
                            if(proxy->type != proxy_type::types::socks4)
                            // we don't want to check if it's also a socks5 proxy if we already know it's a socks4 proxy
                            {
                                proxy->type = proxy_type::types::socks4;

                                // we want to know if this can also be used for socks5

                                // wait 5 seconds to try and avoid spam filter
                                /*
                                nextgen::timeout(self->network_service, [=]()
                                {
                                    client->proxy = "socks4n5";

                                    self.connect(job);
                                }, 5000);

                                return;*/
                            }
                        }
                        else if(client->proxy == "socks5")
                            proxy->type = proxy_type::types::socks5;
                        else if(client->proxy == "socks4n5")
                            proxy->type = proxy_type::types::socks4n5;
                    }
                    // proxy cannot receive data
                    else
                    {
                        proxy.set_state(proxy_type::states::bad_return_data);
                    }

                    client.disconnect();

                    if(self->active_clients >= 0) --self->active_clients;

                    self.remove_job(proxy.get_id());

                    if(callback != 0)
                        callback();
                },
                [=]()
                {
                    if(PROXOS_DEBUG_1)
                        std::cout << "[proxos:proxy_client] Client send/receive failure." << std::endl;

                    if(client->proxy == "http")
                    {
                        if(job->proxy->type == proxy_type::types::transparent
                        || job->proxy->type == proxy_type::types::distorting
                        || job->proxy->type == proxy_type::types::anonymous
                        || job->proxy->type == proxy_type::types::elite)
                        // we know this could only be a http proxy
                        {
                            if(self->active_clients >= 0) --self->active_clients;

                            self.remove_job(proxy.get_id());

                            if(callback != 0)
                                callback();
                        }
                        // we know this isn't an http proxy, so check the next type
                        {
                            // wait 5 seconds to try and avoid spam filter
                            nextgen::timeout(self->network_service, [=]()
                            {
                                client->proxy = "socks4";

                                self.connect(job);
                            }, 5000);
                        }
                    }
                    else if(client->proxy == "socks4")
                    {
                        if(job->proxy->type == proxy::types::socks4)
                        // we know this could only be a socks4 proxy
                        {
                            if(self->active_clients >= 0) --self->active_clients;

                            self.remove_job(proxy.get_id());

                            if(callback != 0)
                                callback();
                        }
                        else
                        // we know this isn't an socks4 proxy, so check the next type
                        {
                            // wait 5 seconds to try and avoid spam filter

                            nextgen::timeout(self->network_service, [=]()
                            {
                                client->proxy = "socks5";

                                self.connect(job);
                            }, 5000);
                        }
                    }
                    else if(client->proxy == "socks5")
                    {
                        if(self->active_clients >= 0) --self->active_clients;

                        self.remove_job(proxy.get_id());

                        if(callback != 0)
                            callback();
                    }
                    else if(client->proxy == "socks4n5")
                    {
                        if(self->active_clients >= 0) --self->active_clients;

                        self.remove_job(proxy.get_id());

                        if(callback != 0)
                            callback();
                    }
                    else
                    {
                        proxy.set_state(proxy_type::states::cannot_send);

                        if(self->active_clients >= 0) --self->active_clients;

                        self.remove_job(proxy.get_id());

                        if(callback != 0)
                            callback();
                    }
                });
            },
            [=]()
            {
                proxy.set_state(proxy_type::states::cannot_connect);

                self.remove_job(proxy.get_id());

                if(self->active_clients >= 0) --self->active_clients;

                if(callback != 0)
                    callback();
            });
        }

        public: void update()
        {
            auto self = *this;

            if(self->enabled)
            {
                if(self->timer.stop() >= 10)
                // run every 10 seconds
                {
                    // try to clean out old server clients
                    self->server.clean();


                    if(NEXTGEN_DEBUG_4)
                        std::cout << "[proxy_checker] Cleaning out expired jobs.";

                    std::remove_if(self->job_list.begin(), self->job_list.end(), [=](job_type& job) -> bool
                    {
                        if(job->client.is_alive())
                        {
                            return false;
                        }
                        else
                        {
                            if(NEXTGEN_DEBUG_4)
                                std::cout << ".";

                            job->client.disconnect();

                            return true;
                        }
                    });

                    std::cout << "maybe refilling: " << self->active_clients << " / " << self->client_max << std::endl;

                    std::cout << "maybe refilling2: " << self->job_list.size() << " / " << self->client_max << std::endl;

                    // notify listeners we could use more jobs
                    if(self->job_list.size() < self->client_max)
                    {
                        size_t before = self->job_list.size();

                        size_t amount = self->client_max - self->job_list.size();

                        if(amount > 0)
                            self->refill_event(amount);
                    }

                    self->timer.start();
                }

                std::for_each(self->job_list.begin(), self->job_list.end(), [=](job_type& job)
                {
                    if(!job->complete)
                    {
                        job->complete = true;

                        ++self->active_clients;

                        if(job->proxy->type == proxy::types::socks4
                        || job->proxy->type == proxy::types::socks4n5)
                            job->client->proxy = "socks4";
                        else if(job->proxy->type == proxy::types::socks5)
                            job->client->proxy = "socks5";
                        else
                            job->client->proxy = "http";

                        self.connect(job);
                    }
                });
            }
            else
            {
                //std::cout << "[proxy_checker] disabled" << std::endl;
            }
        }

        private: struct variables
        {
            variables(std::string const& host, uint32_t port, network_service_type network_service) : enabled(false), active_clients(0), host(host), port(port), network_service(network_service), server(network_service, port), client_max(850)
            {

            }

            ~variables()
            {

            }

            bool enabled;
            size_t active_clients;
            std::string host;
            uint32_t port;
            job_list_type job_list;
            network_service_type network_service;
            server_type server;
            size_t client_max;
            timer_type timer;
            job_list_type::iterator job_position;
            nextgen::event<refill_event_type> refill_event;
        };

        NEXTGEN_SHARED_DATA(proxy_checker, variables,
        {
            this->initialize();
        });
    };
}

#endif
