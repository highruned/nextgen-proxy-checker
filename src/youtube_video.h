#ifndef PROXOS_YOUTUBE_VIDEO
#define PROXOS_YOUTUBE_VIDEO

#include "common.h"
#include "proxy_checker.h"

bool YOUTUBE_DEBUG_1 = 1;

namespace youtube
{
    class video
    {
        private: void connect(proxos::proxy proxy = 0)
        {
            auto self = *this;

            std::string host;
            uint32_t port;

            if(proxy != 0)
            {
                host = proxy->host;
                port = proxy->port;

                switch(proxy->type)
                {
                    case proxos::proxy::types::transparent:
                    case proxos::proxy::types::distorting:
                    case proxos::proxy::types::anonymous:
                        self->client->proxy = "http"; break;

                    case proxos::proxy::types::socks4: self->client->proxy = "socks4"; break;
                    case proxos::proxy::types::socks5: self->client->proxy = "socks5"; break;
                    case proxos::proxy::types::socks4n5: self->client->proxy = "socks4"; break;
                }
            }
            else
            {
                host = "youtube.com";
                port = 80;
            }

            self->client.connect("youtube.com", 80, nextgen::network::ipv4_address(host, port), [=]()
            {
                if(YOUTUBE_DEBUG_1)
                    std::cout << "[proxos:youtube] Connected." << std::endl;

            });
        }

        public: void view(nextgen::string video_id, size_t max, proxos::proxy proxy = 0, proxos::agent agent = 0) const
        {
            auto self = *this;

            std::string user_agent;

            if(agent != 0)
            {
                user_agent = agent->title;
            }
            else
            {
                user_agent = "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";
            }

            if(YOUTUBE_DEBUG_1)
                std::cout << "user agent: " << user_agent << std::endl;

            std::string host;
            uint32_t port;

            if(proxy != 0)
            {
                host = proxy->host;
                port = proxy->port;

                switch(proxy->type)
                {
                    case proxos::proxy::types::transparent:
                    case proxos::proxy::types::distorting:
                    case proxos::proxy::types::anonymous:
                        self->client->proxy = "http"; break;

                    case proxos::proxy::types::socks4: self->client->proxy = "socks4"; break;
                    case proxos::proxy::types::socks5: self->client->proxy = "socks5"; break;
                    case proxos::proxy::types::socks4n5: self->client->proxy = "socks4"; break;
                }
            }
            else
            {
                host = "youtube.com";
                port = 80;
            }

            self->client.connect("youtube.com", 80, nextgen::network::ipv4_address(host, port), [=]()
            {
                if(YOUTUBE_DEBUG_1)
                    std::cout << "[proxos:youtube] Connected." << std::endl;

                nextgen::network::http_message m1;

                m1->method = "GET";
                m1->url = "http://www.youtube.com/watch?v=" + video_id;
                m1->header_list["Host"] = "www.youtube.com";
                m1->header_list["User-Agent"] = user_agent;
                m1->header_list["Keep-Alive"] = "300";
                m1->header_list["Connection"] = "keep-alive";

                self->client.send_and_receive(m1, [=](nextgen::network::http_message r1)
                {
                    if(YOUTUBE_DEBUG_1)
                        std::cout << "[proxos:youtube] Received video page response." << std::endl;

                    if(YOUTUBE_DEBUG_1)
                        std::cout << "c_length " << r1->content.length() << std::endl;

                    if(r1->status_code != 200)
                    {
                        if(YOUTUBE_DEBUG_1)
                            std::cout << "[proxos:youtube] Error receiving video page. " << r1->status_code << std::endl;

                        return;
                    }

                    nextgen::network::http_message m2;

                    m2->method = "GET";
                    m2->url = "http://www.youtube.com/get_video_info?video_id=" + video_id;
                    m2->header_list["Host"] = "www.youtube.com";
                    m2->header_list["User-Agent"] = user_agent;
                    m2->header_list["Keep-Alive"] = "300";
                    m2->header_list["Connection"] = "keep-alive";
                    m2->header_list["Cookie"] = r1->header_list["set-cookie"];

                    if(YOUTUBE_DEBUG_1)
                        std::cout << "COOKIES: " << r1->header_list["set-cookie"] << std::endl;

                    auto video_info_request = [=](nextgen::network::http_message r2)
                    {
                        if(YOUTUBE_DEBUG_1)
                            std::cout << "[proxos:youtube] Received video info response." << std::endl;

                        if(YOUTUBE_DEBUG_1)
                            std::cout << r2->content << std::endl;

                        if(r2->status_code != 200)
                        {
                            if(YOUTUBE_DEBUG_1)
                                std::cout << "[proxos:youtube] Error receiving video info. " << r2->status_code << std::endl;

                            return;
                        }

                        nextgen::string token = nextgen::regex_single_match("&token=(.+?)&thumbnail_url", r2->content);

                        nextgen::network::http_message m3;

                        m3->method = "GET";
                        m3->url = "http://www.youtube.com/get_video?video_id=" + video_id + "&t=" + token + "&el=detailpage&ps=&fmt=5&noflv=1";
                        m3->header_list["Host"] = "www.youtube.com";
                        m3->header_list["User-Agent"] = user_agent;
                        m3->header_list["Keep-Alive"] = "300";
                        m3->header_list["Connection"] = "keep-alive";
                        m3->header_list["Cookie"] = r1->header_list["set-cookie"];

                        self->view_count = 0;
                        self->view_max = max;

                        if(r1->header_list.find("proxy-connection") != r1->header_list.end()
                        && r1->header_list["proxy-connection"] == "close")
                        // reconnect if proxy closes the connection after one HTTP request
                        {
                            self->client.reconnect([=]()
                            {
                                self.receive_download(m3);
                            });

                            return;
                        }

                        self.receive_download(m3);
                    };

                    std::cout << "pccon: " << r1->header_list["proxy-connection"] << std::endl;

                    if(r1->header_list.find("proxy-connection") != r1->header_list.end()
                    && r1->header_list["proxy-connection"] == "close")
                    // reconnect if proxy closes the connection after one HTTP request
                    {
                        self->client.reconnect([=]()
                        {
                            self->client.send_and_receive(m2, video_info_request);
                        });

                        return;
                    }

                    // wait to avoid spam filter
                    nextgen::timeout(self->network_service, [=]()
                    {
                        self->client.send_and_receive(m2, video_info_request);
                    }, nextgen::random(1000, 3000));
                });
            });
        }

        public: void receive_download(nextgen::network::http_message m1) const
        {
            auto self = *this;

            if(YOUTUBE_DEBUG_1)
                std::cout << "[youtube] Receiving download" << std::endl;

            if(self->view_count < self->view_max)
            {
                // wait to avoid spam filter
                nextgen::timeout(self->network_service, [=]()
                {
                    self->client.send_and_receive(m1, [=](nextgen::network::http_message r1)
                    {
                        if(r1->status_code != 204)
                        {
                            if(YOUTUBE_DEBUG_1)
                                std::cout << "[proxos:youtube] Error receiving video download. " << r1->status_code << std::endl;

                            return;
                        }

                        if(YOUTUBE_DEBUG_1)
                            std::cout << "[proxos:youtube] Received video download response." << std::endl;

                        ++self->view_count;

                        if(YOUTUBE_DEBUG_1)
                            std::cout << "[proxos::youtube] VIEWED " << self->view_count << " TIMES" << std::endl;

                        if(r1->header_list.find("proxy-connection") != r1->header_list.end()
                        && r1->header_list["proxy-connection"] == "close")
                        // reconnect if proxy closes the connection after one HTTP request
                        {
                            self->client.reconnect([=]()
                            {
                                self.receive_download(m1);
                            });

                            return;
                        }

                        self.receive_download(m1);
                    });
                }, nextgen::random(1000, 3000));
            }
        }

        private: struct variables
        {
            variables(nextgen::network::service network_service) : network_service(network_service), client(network_service)
            {

            }

            ~variables()
            {

            }

            nextgen::network::service network_service;
            nextgen::network::http_client client;
            size_t view_count;
            size_t view_max;
        };

        NEXTGEN_SHARED_DATA(video, variables);
    };
}

#endif
