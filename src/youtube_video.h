#ifndef PROXOS_YOUTUBE_VIDEO
#define PROXOS_YOUTUBE_VIDEO

#include "common.h"
#include "proxy_checker.h"

namespace youtube
{
    class video
    {
        public: void view(nextgen::string video_id, nextgen::string user_agent, size_t max, proxos::proxy proxy = 0) const
        {
            auto self = *this;

            nextgen::network::http_client client(self->network_service);

            std::string host;
            uint32_t port;

            if(proxy != 0)
            {
                host = proxy->host;
                port = proxy->port;
            }
            else
            {
                host = "youtube.com";
                port = 80;
            }

            client.connect(host, port, [=]()
            {
                std::cout << "[proxos:youtube] Connected." << std::endl;

                nextgen::network::http_message m1;

                m1->method = "GET";
                m1->url = "http://www.youtube.com/watch?v=" + video_id;
                m1->header_list["Host"] = "www.youtube.com";
                m1->header_list["User-Agent"] = user_agent;
                m1->header_list["Keep-Alive"] = "300";
                m1->header_list["Connection"] = "keep-alive";

                client.send_and_receive(m1, [=](nextgen::network::http_message r1)
                {
                    std::cout << "[proxos:youtube] Received video page response." << std::endl;

                    std::cout << "c_length " << r1->content.length() << std::endl;

                    if(r1->status_code != 200)
                        std::cout << "[proxos:youtube] Error receiving video page. " << r1->status_code << std::endl;

                    nextgen::network::http_message m2;

                    m2->method = "GET";
                    m2->url = "http://www.youtube.com/get_video_info?video_id=" + video_id;
                    m2->header_list["Host"] = "www.youtube.com";
                    m2->header_list["User-Agent"] = user_agent;
                    m2->header_list["Keep-Alive"] = "300";
                    m2->header_list["Connection"] = "keep-alive";
                    m2->header_list["Cookie"] = r1->header_list["Set-Cookie"];

                    client.send_and_receive(m2, [=](nextgen::network::http_message r2)
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
                        m3->url = "http://www.youtube.com/get_video?video_id=" + video_id + "&t=" + token + "&el=detailpage&ps=&fmt=5&noflv=1";
                        m3->header_list["Host"] = "www.youtube.com";
                        m3->header_list["User-Agent"] = user_agent;
                        m3->header_list["Keep-Alive"] = "300";
                        m3->header_list["Connection"] = "keep-alive";
                        m3->header_list["Cookie"] = r1->header_list["Set-Cookie"];

                        self->view_count = 0;
                        self->view_max = max;

                        self.receive_download(client, m3);
                    });
                });
            });
        }

        public: void receive_download(nextgen::network::http_client client, nextgen::network::http_message m1) const
        {
            auto self = *this;

            if(self->view_count < self->view_max)
                client.send_and_receive(m1, [=](nextgen::network::http_message r1)
                {
                    if(r1->status_code != 204)
                        std::cout << "[proxos:youtube] Error receiving video download. " << r1->status_code << std::endl;
                    else
                         std::cout << "[proxos:youtube] Received video download response." << std::endl;

                    ++self->view_count;

                    self.receive_download(client, m1);

                });
        }

        private: struct variables
        {
            variables(nextgen::network::service network_service) : network_service(network_service)
            {

            }

            ~variables()
            {

            }

            nextgen::network::service network_service;
            size_t view_count;
            size_t view_max;
        };

        NEXTGEN_SHARED_DATA(video, variables);
    };
}

#endif
