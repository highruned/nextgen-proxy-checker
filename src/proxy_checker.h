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

		public: std::string const& get_check_delay() const
		{
		    auto self = *this;

		    return self->check_delay;
		}

		public: void set_check_delay(std::string const& check_delay) const
		{
		    auto self = *this;

            self->check_delay = check_delay;
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
            variables(nextgen::database::row row)
            {
                variables((*row)["proxy_host"], to_int((*row)["proxy_port"]), to_int((*row)["proxy_id"]));

                //this->last_checked.from_string((*row)["proxy_last_checked"]);
                //this->check_delay.from_string((*row)["proxy_check_delay"]);

            }

            variables(host_type const& host = "undefined.com", port_type port = 0, id_type id = 0, type_type const& type = "", latency_type latency = 0) : rating(0), host(host), port(port), id(id), type(""), latency(0.0), state(0), check_delay("0000-00-00 10:00:00")
            {

            }

            ~variables()
            {

            }

            int32_t rating;
            host_type host;
            port_type port;
            id_type id;
            type_type type;
            latency_type latency;
            timer_type timer;
            uint32_t state;
            std::string check_delay;
            //nextgen::timestamp last_checked;
            //nextgen::timestamp check_delay; // todo(daemn) rename to interval?
        };

        NEXTGEN_SHARED_DATA(proxy, variables);
	};

}


namespace nextgen
{
    namespace network
    {


    }

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

                proxy_type proxy;
                client_type client;
                callback_type callback;
                bool complete;
            };

            NEXTGEN_SHARED_DATA(job_type, variables);
        };

        public: typedef proxos::proxy proxy_type;
        public: typedef std::list<job_type> job_list_type;
        public: typedef nextgen::network::service network_service_type;
        public: typedef nextgen::uint32_t client_max_type;
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
            {
                //std::pair<proxy_type::id_type, job_type>(proxy.get_id(), job_type(proxy, self->network_service, callback));
            // add the proxy to the list if it doesnt already exist
            auto v = job_type(proxy, self->network_service, callback);
                self->job_list.push_back(v);
std::cout << self->job_list.size() << std::endl;
                //if(self->job_list.size() == 1)
               //     self->job_position = self->job_list.begin();
               //else if(t)
                //    self->job_position = --self->job_list.end();
                //self.get_proxy(proxy.get_id());

            }
        }

        public: job_list_type::iterator find_job(nextgen::uint32_t id) const
        {
            auto self = *this;

            return std::find_if(self->job_list.begin(), self->job_list.end(), [=](job_type& job) -> bool
            {
                return (job->proxy.get_id() == id);
            });


        }

        public: void remove_job(nextgen::uint32_t id) const
        {
            auto self = *this;

            job_list_type::iterator i;

            if((i = self.find_job(id)) != self->job_list.end())
            {
                self->job_list.erase(i);
std::cout << "REMOVEDDDDDDDDDDDDD" << std::endl;
            }
        }

        public: void initialize()
        {
            auto self = *this;

            self->server.accept([=](client_type client)
            {
                std::cout << "[proxos:proxy_server] Proxy server (port 8080) accepted HTTP client." << std::endl;

                client.receive(
                [=](message_type r1)
                {
                    std::cout << "[proxos:proxy_server] Received data from HTTP client." << std::endl;
                    //  can use cookies, can

                    // proxy can send headers
                    if(r1->header_list.find("PID") != r1->header_list.end())
                    {
                        job_list_type::iterator i;

                        if((i = self.find_job(to_int(r1->header_list["PID"]))) != self->job_list.end())
                        // check if job wasnt removed because proxy server killed the connection before sending out packets
                        {
                            auto proxy = (*i)->proxy;

                            if(r1->raw_header_list.find("127.0.0.1") != std::string::npos
                            || r1->raw_header_list.find("174.1.157.98") != std::string::npos)
                                proxy.set_type("transparent");
                            else if(r1->header_list.find("Via") != r1->header_list.end()
                            || r1->header_list.find("X-Forwarded-For") != r1->header_list.end()
                            || r1->header_list.find("Forwarded") != r1->header_list.end()
                            || r1->header_list.find("Client-ip") != r1->header_list.end())
                                proxy.set_type("anonymous");
                            else
                                proxy.set_type("elite");

                            message_type r2;

                            r2->version = "1.1";
                            r2->status_code = 200;
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

                                proxy.set_state(proxy_type::states::can_only_send);


                                client.disconnect();
                            },
                            [=]()
                            {
                                if(DEBUG_MESSAGES2)
                                    std::cout << "[proxos:proxy_server] Server response failure to " << proxy.get_host() << proxy.get_port() << std::endl;

                                proxy.set_state(proxy_type::states::cannot_send_back);

                                //proxy.set_type("broken");
                            });
                        }
                        else
                        {
                                if(DEBUG_MESSAGES2)
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
        }



        public: void update()
        {

            auto self = *this;

            if(self->timer.stop() >= 5)
            // try to clean out old server clients every 5 seconds
            {
                self->server.clean();

                self->timer.start();
            }



            //std::cout << "row size: " << self->rows.size() << std::endl;
            //if(get_process_total_connections(get_process_id()) < self->max_sockets)

            if(self->active_clients < self->client_max)
            {
                size_t before = self->job_list.size();

                size_t amount = self->client_max - self->job_list.size();

                if(amount > 0)
                    self->refill_event(amount);

                size_t after = self->job_list.size();

                size_t total = after - before;
//std::cout << "total: " << total << std::endl;
                size_t i = 0;
//std::cout << "active: " << self->active_clients << std::endl;
//std::cout << "size: " << self->job_list.size() << std::endl;
               // if(self->job_list.size() == 1)
                //    self->job_position = self->job_list.begin();
            }

std::for_each(self->job_list.begin(), self->job_list.end(), [=](job_type& job)
{
    if(!job->complete)
    {
        job->complete = true;


                   //auto j = self->job_position;

                //std::cout << "<ProxyChecker> There are " << self->socket_list.size() << " concurrent sockets." << std::endl;
/*
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
                else if(self->proxy_row_list->size() == 0)
                {
                    std::cout << "[proxos:proxy_checker] No more proxies." << std::endl;

                    self->proxy_row_start = 0;

                    return;
                }
                else
                {
                    ++self->proxy_row_iter;
                }
*/
                //client_type(self->network_service);

                //self->client_list.push_back(client);

                    ++self->active_clients;

                    auto client = job->client;
                    auto proxy = job->proxy;
                    auto callback = job->callback;

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
                        //r1->header_list["Connection"] = "keep-alive";
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

                        //self->pending_job_list.push_back(job);

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
                               proxy.set_latency(proxy.get_timer().stop());

                                    // proxy can receive data
                                    if(r2->content.find("proxos") != std::string::npos)
                                    {
                                        // proxy can receive headers
                                        if(r2->header_list.find("Server") != r2->header_list.end()
                                            && r2->header_list["Server"] == "proxos"
                                            && r2->header_list.find("PID") != r2->header_list.end())
                                        {
                                            //social_satan::nge::Instance().Database.Connect("Proxies");

                                            proxy.set_state(proxy_type::states::perfect);

                                           // std::string query = "UPDATE proxies SET proxy_type = \"" + proxy.get_type() + "\", proxy_browser_capable = 1, proxy_rating = proxy_rating + 1, proxy_latency = " + to_string(proxy.get_latency()) + ", proxy_last_checked = NOW() WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";

                                            //std::cout << query << " after " << to_string(proxy.get_timer().stop()) << " seconds. " << std::endl;

                                            //self->database_link.query(query);

                                            if(DEBUG_MESSAGES2)
                                                std::cout << "good proxy" << std::endl;
                                        }
                                        // proxy cannot receive headers
                                        else
                                        {
                                            //social_satan::nge::Instance().Database.Connect("Proxies");

                                            proxy.set_state(proxy_type::states::bad_return_headers);

                                            //std::string query = "UPDATE proxies SET proxy_type = \"" + proxy.get_type() + "\", proxy_browser_capable = 0, proxy_rating = proxy_rating + 1, proxy_latency = " + to_string(proxy.get_latency()) + ", proxy_last_checked = NOW() WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";

                                            //std::cout << query << " after " << to_string(proxy.get_timer().stop()) << " seconds. " << std::endl;

                                            //self->database_link.query(query);

                                            if(DEBUG_MESSAGES2)
                                                std::cout << "goodish proxy - doesnt forward headers correctly - correct_headers = false" << std::endl;
                                        }
                                    }
                                    // proxy cannot receive data
                                    else
                                    {
                                        //std::string query;

                                        if(r2->content.find("CoDeeN") != std::string::npos)
                                        {
                                            //proxy.set_type("codeen");

                                            proxy.set_state(proxy_type::states::codeen);
                                            //proxy.set_check_delay("0000-00-07 00:00:00");

                                            // check codeen proxies once a week
                                            //query = "UPDATE proxies SET proxy_type = \"" + proxy.get_type() + "\", proxy_latency = 0, proxy_rating = proxy_rating - 1, proxy_last_checked = NOW(), proxy_check_delay = \"0000-00-07 00:00:00\" WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";
                                        }
                                        else
                                        {
                                            proxy.set_type("broken");

                                            proxy.set_state(proxy_type::states::bad_return_data);
                                            //proxy.set_check_delay("0000-00-01 00:00:00");

                                            // check broken proxies once a day
                                            //query = "UPDATE proxies SET proxy_type = \"" + proxy.get_type() + "\", proxy_latency = 0, proxy_rating = proxy_rating - 1, proxy_last_checked = NOW(), proxy_check_delay = \"0000-00-01 00:00:00\" WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";
                                        }

                                        //std::cout << query << " after " << to_string(proxy.get_timer().stop()) << " seconds. " << std::endl;

                                        //self->database_link.query(query);


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
std::cout << "REM1" << std::endl;

--self->active_clients;
                            self.remove_job(proxy.get_id());

                            if(callback != 0)
                                callback();
                        },
                        [=]()
                        {
                            if(DEBUG_MESSAGES2)
                                std::cout << "[proxos:proxy_client] Client send/receive failure." << std::endl;

                            proxy.set_type("dead");

                            proxy.set_state(proxy_type::states::cannot_send);
std::cout << "REM2" << std::endl;
--self->active_clients;
                            self.remove_job(proxy.get_id());

                            if(callback != 0)
                                callback();
                        });
                    },
                    [=]()
                    {
                            proxy.set_type("dead");

                            proxy.set_state(proxy_type::states::cannot_connect);
std::cout << "REM3" << std::endl;

                            self.remove_job(proxy.get_id());
--self->active_clients;
                            if(callback != 0)
                                callback();
                    });

                    //self->job_position = ++self->job_position;
    }
});
                    //self.remove_job(self->job_position);


        }

        private: struct variables
        {
            variables(std::string const& host, uint32_t port, network_service_type network_service) : active_clients(0), host(host), port(port), network_service(network_service), server(network_service, port), client_max(850)
            {

            }

            ~variables()
            {

            }

            std::string host;
            uint32_t port;
            //proxy_row_start_type proxy_row_start;
            //proxy_row_end_type proxy_row_end;
           // proxy_row_list_type proxy_row_list;
           // proxy_row_iter_type proxy_row_iter;
           // proxy_list_type proxy_list;
            job_list_type job_list;
            network_service_type network_service;
            server_type server;
            size_t client_max;
            timer_type timer;
            size_t active_clients;
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
