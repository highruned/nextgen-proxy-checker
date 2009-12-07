#ifndef PROXOS_PROXY_CHECKER
#define PROXOS_PROXY_CHECKER

#include "common.h"


namespace proxos
{
	class proxy
	{
	    public: typedef proxy this_type;
        public: typedef std::string host_type;
        public: typedef unsigned int port_type;
        public: typedef unsigned int id_type;
        public: typedef std::string type_type;
        public: typedef float latency_type;
        public: typedef nextgen::timer timer_type;

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

		public: type_type const& get_type() const
		{
		    auto self = *this;

			return self->type;
		}

		public: void set_type(type_type const& type) const
		{
		    auto self = *this;

			self->type = type;
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

		public: timer_type& get_timer() const
		{
		    auto self = *this;

			return self->timer;
		}

		public: void set_timer(timer_type& timer) const
		{
		    auto self = *this;

			self->timer = timer;
		}

		public: void from_row(nextgen::database::row row) const
		{
		    auto self = *this;

			self->host = (*row)["proxy_host"];
			self->port = to_int((*row)["proxy_port"]);
			self->id = to_int((*row)["proxy_id"]);
		}

        private: struct variables
        {
            variables()
            {
                variables("undefined.com", 0);
            }

            variables(host_type const& host, port_type port) : host(host), port(port), id(0), type(""), latency(0.0)
            {

            }

            ~variables()
            {

            }

            host_type host;
            port_type port;
            id_type id;
            type_type type;
            latency_type latency;
            timer_type timer;
        };

        NEXTGEN_SHARED_DATA(proxy, variables);
	};


    class proxy_checker
    {
        public: typedef std::list<nextgen::network::http_client> client_list_type;
        public: typedef nextgen::hash_map<nextgen::uint32_t, proxos::proxy> proxy_list_type;
        public: typedef nextgen::database::row_list proxy_row_list_type;
        public: typedef nextgen::network::service network_service_type;
        public: typedef nextgen::uint32_t proxy_row_start_type;
        public: typedef nextgen::uint32_t proxy_row_end_type;
        public: typedef proxy_row_list_type::value_type::iterator proxy_row_iter_type;
        public: typedef nextgen::uint32_t client_max_type;
        public: typedef nextgen::timer timer_type;
        public: typedef proxos::proxy proxy_type;

        public: client_list_type& get_client_list() const
        {
            auto self = *this;

            return self->client_list;
        }

        public: proxy_list_type& get_proxy_list() const
        {
            auto self = *this;

            return self->proxy_list;
        }

        public: void initialize()
        {
            auto self = *this;

            std::string query("SELECT proxy_host, proxy_port, proxy_id FROM proxies WHERE proxy_last_checked < (NOW() - proxy_check_delay) ORDER BY proxy_id LIMIT " + to_string(self->proxy_row_start) + ", " + to_string(self->proxy_row_end)); //ORDER BY proxy_rating DESC

            std::cout << query << std::endl;

            self->proxy_row_list = self->database_link.get_row_list(query); //proxy_rating DESC, proxy_hits DESC, proxy_latency ASC,
            self->proxy_row_iter = self->proxy_row_list->begin();

            nextgen::network::create_server<nextgen::network::http_client>(self->network_service, self->port,
            [=](nextgen::network::http_client client)
            {
                std::cout << "[proxos:proxy_server] Proxy server (port 8080) accepted HTTP client." << std::endl;

                client.receive(
                [=](nextgen::network::http_message r1)
                {
                    std::cout << "[proxos:proxy_server] Received data from HTTP client." << std::endl;


                    //  can use cookies, can

                    // proxy can send headers
                    if(r1->header_list.find("PID") != r1->header_list.end())
                    {
                        auto proxy = self->proxy_list[to_int(r1->header_list["PID"])];

                        std::string type;

                        if(r1->raw_header_list.find("127.0.0.1") != std::string::npos)
                            type = "transparent";
                        else if(r1->header_list.find("Via") != r1->header_list.end()
                            || r1->header_list.find("X-Forwarded-For") != r1->header_list.end()
                            || r1->header_list.find("Forwarded") != r1->header_list.end()
                            || r1->header_list.find("Client-ip") != r1->header_list.end())
                            type = "anonymous";
                        else
                            type = "elite";

                        proxy.set_type(type);

                        nextgen::network::http_message r2;

                        //r2->raw_header_list = "Content-Type: text/html\r\nProxy-Connection:close\r\nConnection: close\r\nServer: Proxos\r\nPID: " + response->header_list["PID"] + "\r\n";
                        r2->version = "1.1";
                        r2->status_code = 200;
                        r2->status_description = "OK";
                        r2->header_list["Content-Type"] = "text/html";
                        r2->header_list["Proxy-Connection"] = "close";
                        r2->header_list["Connection"] = "close";
                        r2->header_list["Server"] = "proxos";
                        r2->header_list["PID"] = r1->header_list["PID"];
                        r2->content = "proxos";

                        if(DEBUG_MESSAGES2)
                            std::cout << "[proxos:proxy_server] Server sending response to " << proxy.get_host() << ":" << proxy.get_port() << std::endl;

                        client.send(r2,
                        [=]()
                        {
                            if(DEBUG_MESSAGES2)
                                std::cout << "[proxos:proxy_server] Server response successful to " << proxy.get_host() << proxy.get_port() << std::endl;

                            // set proxy property to successful send


                            client.disconnect();
                        },
                        [=]()
                        {
                            if(DEBUG_MESSAGES2)
                                std::cout << "[proxos:proxy_server] Server response failure to " << proxy.get_host() << proxy.get_port() << std::endl;

                            proxy.set_type("broken");
                        });
                    }

                },
                []()
                {
                    std::cout << "[proxos:proxy_server] Failed to receive data from HTTP client." << std::endl;

                });
            });
        }

        private: void clean()
        {
            auto self = *this;

            if(self->timer.stop() > 5.0f)
            {
                std::cout << "[proxos:proxy_checker] Cleaning out expired clients.";

                for(client_list_type::iterator i = self->client_list.begin(), l = self->client_list.end(); i != l;)
                {
                    if((*i)->counter.stop() > 240.0f)
                    {
                        client_list_type::iterator j = i; ++j;

                        std::cout << ".";

                        (*i)->transport_layer_.cancel(); //self->socket_list.erase(i);

                        ++i; //i = j;
                    }
                    else
                        ++i;
                }

                std::cout << std::endl;

                self->timer.start();
            }
        }

        public: void remove_client(nextgen::network::http_client client) const
        {
            auto self = *this;

            client_list_type::iterator location = std::find(self->client_list.begin(), self->client_list.end(), client);

            if(location != self->client_list.end())
                self->client_list.erase(location);
        }

        public: void update()
        {
            typedef nextgen::network::http_message message_type;
            typedef nextgen::network::http_client client_type;

            auto self = *this;

            static bool initialized = false;

            if(!initialized)
            {


                initialized = true;
            }

            self.clean();

            //std::cout << "row size: " << self->rows.size() << std::endl;
            //if(get_process_total_connections(get_process_id()) < self->max_sockets)
            if(self->client_list.size() < self->client_max)
            {
                //std::cout << "<ProxyChecker> There are " << self->socket_list.size() << " concurrent sockets." << std::endl;

                if(self->proxy_row_list->size() == 0 || self->proxy_row_iter == self->proxy_row_list->end() - 1)
                {
                    self->proxy_row_start += self->proxy_row_end;

                    //social_satan::nge::Instance().Database.Connect("Proxies");

                    std::string query("SELECT proxy_host, proxy_port, proxy_id FROM proxies WHERE proxy_last_checked < (NOW() - proxy_check_delay) ORDER BY proxy_id LIMIT " + to_string(self->proxy_row_start) + ", " + to_string(self->proxy_row_end));

                    std::cout << query << std::endl;

                    self->proxy_row_list = self->database_link.get_row_list(query); //proxy_rating DESC, proxy_hits DESC, proxy_latency ASC,

                    if(self->proxy_row_list->size() == 0)
                    {
                        std::cout << "[proxos:proxy_checker] No more proxies to check." << std::endl;

                        self->proxy_row_start = 0;

                        return;
                    }
                    else
                    {
                        std::cout << "[proxos:proxy_checker] Checking " << self->proxy_row_list->size() << " proxies. " << std::endl;
                    }

                    self->proxy_row_iter = self->proxy_row_list->begin();
                }
                else
                {
                    if(self->proxy_row_list->size() == 0)
                    {
                        std::cout << "[proxos:proxy_checker] No more proxies." << std::endl;

                        self->proxy_row_start = 0;

                        return;
                    }

                    ++self->proxy_row_iter;
                }

                proxy_type proxy;

                proxy.from_row((*self->proxy_row_iter));

                self->proxy_list[proxy.get_id()] = proxy;

                client_type client(self->network_service);

                self->client_list.push_back(client);

                if(DEBUG_MESSAGES)
                    std::cout << "[proxos:proxy_client] Attempting to connect to " << proxy.get_host() << ":" << proxy.get_port() << " (" << proxy.get_id() << ")" << std::endl;

                client.connect(proxy.get_host(), proxy.get_port(),
                [=]()
                {
                    if(DEBUG_MESSAGES2)
                        std::cout << "[proxos:proxy_client] Connected to proxy " << proxy.get_host() + ":" + to_string(proxy.get_port()) << "." << std::endl;

                    message_type r1;

                    r1->method = "GET";
                    r1->url = "http://" + self->host + ":" + to_string(self->port) + "/" + to_string(proxy.get_id());

                    r1->version = "1.1";

                    r1->header_list["Host"] = self->host;
                    r1->header_list["User-Agent"] = "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.1) Gecko/20090624 Firefox/3.5 (.NET CLR 3.5.30729)";
                    r1->header_list["PID"] = to_string(proxy.get_id());
                    r1->header_list["Proxy-Connection"] = "keep-alive";
                    r1->header_list["Connection"] = "keep-alive";
                    //r1->header_list["Connection"] = "close";

                    //r1->raw_header_list = "Host: " + my_ip + "\r\n"
    //"User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.1) Gecko/20090624 Firefox/3.5 (.NET CLR 3.5.30729)" "\r\n"
   // "Accept-Language: en-us,en;q=0.5" "\r\n"
   // "Accept-Encoding: gzip,deflate" "\r\n"
   // "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7" "\r\n"
   // "PID: " + to_string(proxy.get_id()) + "\r\n"
    //"If-Modified-Since: Fri, 14 Oct 2005 16:29:56 GMT" "\r\n"
    //"If-None-Match: 5e02c3-3f8-5e2e4900" "\r\n"
   // "Cache-Control: max-age=0" "\r\n"
    //"Proxy-Connection: keep-alive" "\r\n"
    //"Connection: close" "\r\n";

                    proxy.get_timer().start();

                    client.send_and_receive(r1,
                    [=](message_type r2)
                    {
                        if(DEBUG_MESSAGES2)
                            std::cout << "[proxos:proxy_client] Client send/receive successful." << std::endl;

                        std::cout << r2->raw_header_list << std::endl;
                        std::cout << r2->content << std::endl;

                        //switch(r2->status_code)
                        //{
                           // case 200:
                            //case 302:
                           //case 303:
                           // {
                                // proxy can receive data
                                if(r2->content.find("proxos") != std::string::npos)
                                {
                                    // proxy can receive headers
                                    if(r2->header_list.find("Server") != r2->header_list.end()
                                        && r2->header_list["Server"] == "proxos"
                                        && r2->header_list.find("PID") != r2->header_list.end())
                                    {
                                        //social_satan::nge::Instance().Database.Connect("Proxies");

                                        proxy.set_latency(proxy.get_timer().stop());

                                        std::string query = "UPDATE proxies SET proxy_type = \"" + proxy.get_type() + "\", proxy_browser_capable = 1, proxy_rating = proxy_rating + 1, proxy_latency = " + to_string(proxy.get_latency()) + ", proxy_last_checked = NOW() WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";

                                        std::cout << query << " after " << to_string(proxy.get_timer().stop()) << " seconds. " << std::endl;

                                        self->database_link.query(query);

                                        if(DEBUG_MESSAGES2)
                                            std::cout << "good proxy" << std::endl;
                                    }
                                    // proxy cannot receive headers
                                    else
                                    {
                                        //social_satan::nge::Instance().Database.Connect("Proxies");

                                        proxy.set_latency(proxy.get_timer().stop());

                                        proxy.set_type("broken");

                                        std::string query = "UPDATE proxies SET proxy_type = \"" + proxy.get_type() + "\", proxy_browser_capable = 0, proxy_rating = proxy_rating + 1, proxy_latency = " + to_string(proxy.get_latency()) + ", proxy_last_checked = NOW() WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";

                                        std::cout << query << " after " << to_string(proxy.get_timer().stop()) << " seconds. " << std::endl;

                                        self->database_link.query(query);

                                        if(DEBUG_MESSAGES2)
                                            std::cout << "goodish proxy - doesnt forward headers correctly - correct_headers = false" << std::endl;
                                    }
                                }
                                // proxy cannot receive data
                                else
                                {
                                    if(proxy.get_type() == "anonymous" ||
                                        proxy.get_type() == "transparent" ||
                                        proxy.get_type() == "socks4" ||
                                        proxy.get_type() == "socks5")
                                    {
                                        if(DEBUG_MESSAGES2)
                                            std::cout << "bad proxy via returning data" << std::endl;
                                    }
                                    else
                                    {
                                        if(DEBUG_MESSAGES2)
                                            std::cout << "bad proxy" << std::endl;
                                    }

                                    std::string query;

                                    if(r2->content.find("CoDeeN") != std::string::npos)
                                    {
                                        proxy.set_type("codeen");

                                        // check codeen proxies once a week
                                        query = "UPDATE proxies SET proxy_type = \"" + proxy.get_type() + "\", proxy_latency = 0, proxy_rating = proxy_rating - 1, proxy_last_checked = NOW(), proxy_check_delay = \"0000-00-07 00:00:00\" WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";
                                    }
                                    else
                                    {
                                        proxy.set_type("broken");

                                        // check broken proxies once a day
                                        query = "UPDATE proxies SET proxy_type = \"" + proxy.get_type() + "\", proxy_latency = 0, proxy_rating = proxy_rating - 1, proxy_last_checked = NOW(), proxy_check_delay = \"0000-00-01 00:00:00\" WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";
                                    }

                                    std::cout << query << " after " << to_string(proxy.get_timer().stop()) << " seconds. " << std::endl;

                                    self->database_link.query(query);
                                }

                               // break;
                           // }
                           // default:
                           // {
                           //     if(DEBUG_MESSAGES2)
                           //         std::cout << "[proxos:proxy_client] bad proxy via sending headers" << std::endl;
                           // }
                        //}


                        client.disconnect();

                        self.remove_client(client);
                    },
                    [=]()
                    {
                        if(DEBUG_MESSAGES2)
                            std::cout << "[proxos:proxy_client] Client send/receive failure." << std::endl;

                        proxy.set_type("dead");

                        nextgen::string query = "UPDATE proxies SET proxy_type = \"" + proxy.get_type() + "\", proxy_latency = 0, proxy_rating = proxy_rating - 1, proxy_last_checked = NOW() WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";

                        std::cout << query << " after " << to_string(proxy.get_timer().stop()) << " seconds. " << std::endl;

                        self->database_link.query(query);

                        self.remove_client(client);
                    });
                },
                [=]()
                {
                    proxy.set_type("dead");

                    nextgen::string query = "UPDATE proxies SET proxy_type = \"" + proxy.get_type() + "\", proxy_latency = 0, proxy_rating = proxy_rating - 1, proxy_last_checked = NOW() WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";

                    //if(DEBUG_MESSAGES)
                        std::cout << query << " after " << to_string(proxy.get_timer().stop()) << " seconds. " << std::endl;

                    self->database_link.query(query);

                    self.remove_client(client);
                });
            }
        }

        private: struct variables
        {
            variables(std::string const& host, uint32_t port, network_service_type network_service, nextgen::database::link database_link) : host(host), port(port), network_service(network_service), database_link(database_link), proxy_row_start(0), proxy_row_end(1000), client_max(1000)
            {

            }

            ~variables()
            {

            }

            std::string host;
            uint32_t port;
            proxy_row_start_type proxy_row_start;
            proxy_row_end_type proxy_row_end;
            proxy_row_list_type proxy_row_list;
            proxy_row_iter_type proxy_row_iter;
            proxy_list_type proxy_list;
            client_list_type client_list;
            client_max_type client_max;
            timer_type timer;
            network_service_type network_service;
            nextgen::database::link database_link;

        };

        NEXTGEN_SHARED_DATA(proxy_checker, variables,
        {
            this->initialize();
        });
    };
}

#endif
