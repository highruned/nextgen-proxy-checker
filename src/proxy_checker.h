#ifndef PROXOS_PROXY_CHECKER
#define PROXOS_PROXY_CHECKER

#include "nextgen/common.h"
#include "nextgen/network.h"
#include "nextgen/database.h"
#include "nextgen/social.h"

bool PROXOS_DEBUG_1 = 1;

namespace proxos
{
    class proxy_checker
    {
        class job_type
        {
            public: typedef nextgen::network::service network_service_type;
            public: typedef nextgen::network::http_client client_type;
            public: typedef std::function<void()> callback_type;
            public: typedef nextgen::network::http_proxy proxy_type;

            private: struct variables
            {
                variables(proxy_type proxy, network_service_type network_service, callback_type callback = 0) : complete(false), client(network_service), callback(callback)
                {
                    this->client->proxy = proxy;

                    if(callback == 0)
                        callback = [] { std::cout << "No callback." << std::endl; };
                }

                bool complete;
                client_type client;
                callback_type callback;
            };

            NEXTGEN_ATTACH_SHARED_VARIABLES(job_type, variables);
        };

        public: typedef nextgen::network::http_proxy proxy_type;
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
                return id == job->client->proxy->id;
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

            self->judge_server.accept(
            [=](client_type client)
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

                        if((i = self.find_job(nextgen::to_int(r1->header_list["pid"]))) != self->job_list.end())
                        // check if job wasnt removed because proxy server killed the connection before sending out packets
                        {
                            auto proxy = (*i)->client->proxy;

                            if(r1->raw_header_list.find("127.0.0.1") != std::string::npos
                            || r1->raw_header_list.find("174.1.157.98") != std::string::npos)
                                proxy->type = proxy_type::types::transparent;
                            else if(r1->header_list.find("Via") != r1->header_list.end()
                            || r1->header_list.find("x-forwarded-for") != r1->header_list.end()
                            || r1->header_list.find("forwarded") != r1->header_list.end()
                            || r1->header_list.find("client-ip") != r1->header_list.end()
                            || r1->header_list.find("x-cache") != r1->header_list.end())
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
                                std::cout << "[proxos:proxy_server] Server sending response to " << proxy->host << ":" << proxy->port << std::endl;

                            client.send(r2,
                            [=]
                            {
                                if(PROXOS_DEBUG_1)
                                    std::cout << "[proxos:proxy_server] Server response successful to " << proxy->host << proxy->port << std::endl;

                                // set proxy property to successful send
                                proxy->state = proxy_type::states::can_only_send;

                                client.disconnect();
                            },
                            [=]
                            {
                                if(PROXOS_DEBUG_1)
                                    std::cout << "[proxos:proxy_server] Server response failure to " << proxy->host << proxy->port << std::endl;

                                proxy->state = proxy_type::states::cannot_send_back;
                            });
                        }
                        // special case for initial confirmation we're hooked up correctly
                        else if(nextgen::to_int(r1->header_list["pid"]) == 0)
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
                            [=]
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
                [=]
                {
                    std::cout << "[proxos:proxy_server] Failed to receive data from HTTP client." << std::endl;
                });
            },
            [=]
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
            client->proxy = nextgen::network::http_proxy(self->host, self->port);

            client.connect(self->host, self->port,
            [=]
            {
                if(PROXOS_DEBUG_1)
                    std::cout << "[proxos:proxy_checker] Connected to self." << std::endl;

                message_type r1;

                r1->method = "GET";
                r1->url = "http://" + self->host + ":" + nextgen::to_string(self->port) + "/" + nextgen::to_string(0);

                r1->version = "1.1";

                r1->header_list["Host"] = self->host + ":" + nextgen::to_string(self->port);
                r1->header_list["User-Agent"] = "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.1) Gecko/20090624 Firefox/3.5 (.NET CLR 3.5.30729)"; //"Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";//
                r1->header_list["PID"] = nextgen::to_string(0);
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
                [=]
                {
                    self->enabled = false;
                });
            },
            [=]
            {
                self->enabled = false;
            });
        }

        private: void connect(job_type job) const
        {
            auto self = *this;

            auto client = job->client;
            auto proxy = job->client->proxy;
            auto callback = job->callback;

            if(PROXOS_DEBUG_1)
                std::cout << "[proxos:proxy_client] Attempting to connect to " << proxy->host << ":" << proxy->port << " (" << proxy->id << ")" << std::endl;

            client.connect(self->host, self->port,
            [=]
            {
                if(PROXOS_DEBUG_1)
                    std::cout << "[proxos:proxy_client] Connected to proxy " << proxy->host + ":" + nextgen::to_string(proxy->port) << "." << std::endl;

                message_type r1;

                r1->method = "GET";
                r1->url = "http://" + self->host + ":" + nextgen::to_string(self->port) + "/" + nextgen::to_string(proxy->id);

                r1->version = "1.1";

                r1->header_list["Host"] = self->host + ":" + nextgen::to_string(self->port);
                r1->header_list["User-Agent"] = "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.1) Gecko/20090624 Firefox/3.5 (.NET CLR 3.5.30729)"; //"Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";//
                r1->header_list["PID"] = nextgen::to_string(proxy->id);
                r1->header_list["Keep-Alive"] = "300";
                r1->header_list["Connection"] = "keep-alive";

                proxy->timer.start();

                client.send_and_receive(r1,
                [=](message_type r2)
                {
                    if(PROXOS_DEBUG_1)
                    {
                        std::cout << "[proxos:proxy_client] Client send/receive successful." << std::endl;

                        std::cout << r2->raw_header_list << std::endl;
                        std::cout << r2->content << std::endl;
                    }

                    proxy->latency = proxy->timer.stop();

                    if(r2->content.find("CoDeeN") != std::string::npos
                    || r2->content.find("PlanetLab") != std::string::npos)
                    {
                        proxy->state = proxy_type::states::codeen;
                    }
                    // proxy can receive data
                    else if(r2->content.find("my_data") != std::string::npos)
                    {
                        // proxy can receive headers
                        if(r2->header_list["set-cookie"].find("my_cookie") != std::string::npos)
                        {
                            proxy->state = proxy_type::states::perfect;

                            if(PROXOS_DEBUG_1)
                                std::cout << "good proxy" << std::endl;
                        }
                        // proxy cannot receive headers
                        else
                        {
                            proxy->state = proxy_type::states::bad_return_headers;

                            if(PROXOS_DEBUG_1)
                                std::cout << "goodish proxy - doesnt forward headers correctly - correct_headers = false" << std::endl;
                        }

                        if(proxy->type == proxy_type::types::socks4)
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
                    }
                    // proxy cannot receive data
                    else
                    {
                        proxy->state = proxy_type::states::bad_return_data;
                    }

                    client.disconnect();

                    self.remove_job(proxy->id);

                    if(callback != 0)
                        callback();
                },
                [=]
                {
                    if(PROXOS_DEBUG_1)
                        std::cout << "[proxos:proxy_client] Client send/receive failure." << std::endl;

                    if(proxy->type == proxy_type::types::transparent
                    || proxy->type == proxy_type::types::distorting
                    || proxy->type == proxy_type::types::anonymous
                    || proxy->type == proxy_type::types::elite)
                    // we know this could only be a http proxy
                    {
                        self.remove_job(proxy->id);

                        if(callback != 0)
                            callback();
                    }
                    /*
                    // we know this isn't an http proxy, so check the next type
                    {
                        // wait 5 seconds to try and avoid spam filter
                        nextgen::timeout(self->network_service, [=]()
                        {
                            client->proxy = "socks4";

                            self.connect(job);
                        }, 5000);
                    }*/
                    else if(proxy->type == proxy_type::types::socks4)
                    // we know this could only be a socks4 proxy
                    {
                        self.remove_job(proxy->id);

                        if(callback != 0)
                            callback();
                    }
                    /*
                    else
                    // we know this isn't an socks4 proxy, so check the next type
                    {
                        // wait 5 seconds to try and avoid spam filter

                        nextgen::timeout(self->network_service, [=]()
                        {
                            proxy = "socks5";

                            self.connect(job);
                        }, 5000);
                    }*/
                    else if(proxy->type == proxy_type::types::socks5)
                    {
                        self.remove_job(proxy->id);

                        if(callback != 0)
                            callback();
                    }
                    else if(proxy->type == proxy_type::types::socks4n5)
                    {
                        self.remove_job(proxy->id);

                        if(callback != 0)
                            callback();
                    }
                    else
                    {
                        proxy->state = proxy_type::states::cannot_send;

                        self.remove_job(proxy->id);

                        if(callback != 0)
                            callback();
                    }
                });
            },
            [=]
            {
                proxy->state = proxy_type::states::cannot_connect;

                self.remove_job(proxy->id);

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
                    self->judge_server.clean();

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

                    std::cout << "maybe refilling2: " << self->job_list.size() << " / " << self->client_max << std::endl;

                    // notify listeners we could use more jobs
                    if(self->job_list.size() < self->client_max)
                    {
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
            variables(std::string const& host, uint32_t port, network_service_type network_service) : enabled(false), host(host), port(port), network_service(network_service), judge_server(network_service, port), client_max(2000)
            {

            }

            bool enabled;
            std::string host;
            uint32_t port;
            job_list_type job_list;
            network_service_type network_service;
            server_type judge_server;
            size_t client_max;
            timer_type timer;
            nextgen::event<refill_event_type> refill_event;
        };

        NEXTGEN_ATTACH_SHARED_VARIABLES(proxy_checker, variables,
        {
            this->initialize();
        });
    };
}

#endif
