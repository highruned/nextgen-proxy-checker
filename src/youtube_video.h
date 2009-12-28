#ifndef PROXOS_YOUTUBE_VIDEO
#define PROXOS_YOUTUBE_VIDEO

#include "nextgen/common.h"
#include "nextgen/network.h"
#include "nextgen/database.h"
#include "nextgen/social.h"

#include "proxy_checker.h"

bool YOUTUBE_DEBUG_1 = 1;

namespace youtube
{
    class video
    {
        private: struct variables
        {
            variables()
            {

            }

            ~variables()
            {

            }

            std::string id;
        };

        NEXTGEN_SHARED_DATA(video, variables);
    };


    class client
    {
        public: void view_video(video v, size_t view_max) const
        {
            auto self = *this;

            size_t view_count = 0;

            if(self->agent == 0)
                self->agent = proxos::agent("Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5");

            if(YOUTUBE_DEBUG_1)
                std::cout << "user agent: " << self->agent->title << std::endl;

            std::string host;
            uint32_t port;

            if(self->proxy != 0)
            {
                host = self->proxy->host;
                port = self->proxy->port;

                switch(self->proxy->type)
                {
                    case proxos::proxy::types::transparent: self->client->proxy = "http"; break;
                    case proxos::proxy::types::distorting: self->client->proxy = "http"; break;
                    case proxos::proxy::types::anonymous: self->client->proxy = "http"; break;
                    case proxos::proxy::types::elite: self->client->proxy = "http"; break;
                    case proxos::proxy::types::socks4: self->client->proxy = "socks4"; break;
                    case proxos::proxy::types::socks5: self->client->proxy = "socks5"; break;
                    case proxos::proxy::types::socks4n5: self->client->proxy = "socks4"; break;
                    default: std::cout << "WTF" << std::endl;
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
                m1->url = "http://www.youtube.com/watch?v=" + v->id;
                m1->header_list["Host"] = "www.youtube.com";
                m1->header_list["User-Agent"] = self->agent->title;
                m1->header_list["Keep-Alive"] = "300";
                m1->header_list["Connection"] = "keep-alive";

                self->client.send_and_receive(m1, [=](nextgen::network::http_message r1)
                {
                    if(YOUTUBE_DEBUG_1)
                        std::cout << "[proxos:youtube] Received video page response." << std::endl;

                    if(YOUTUBE_DEBUG_1)
                        std::cout << "c_length " << r1->content.length() << std::endl;

                        std::cout << "c_length " << r1->content << std::endl;

                    if(r1->status_code != 200
                    || r1->header_list["set-cookie"].find("youtube.com") == std::string::npos)
                    {
                        if(YOUTUBE_DEBUG_1)
                            std::cout << "[proxos:youtube] Error receiving video page. " << r1->status_code << std::endl;

                        return;
                    }

                    nextgen::string token = nextgen::regex_single_match("\"t\"\\: \"(.+?)\"\\,", r1->content);

                    if(token == "null")
                    {
                        std::cout << "[youtube] error: null token" << std::endl;

                        return;
                    }

                    nextgen::network::http_message m3;

                    m3->method = "GET";
                    m3->url = "http://www.youtube.com/get_video?video_id=" + v->id + "&t=" + token + "&el=detailpage&ps=";//&noflv=1";
                    m3->header_list["Host"] = "www.youtube.com";
                    m3->header_list["User-Agent"] = self->agent->title;
                    m3->header_list["Keep-Alive"] = "300";
                    m3->header_list["Connection"] = "keep-alive";
                    m3->header_list["Cookie"] = r1->header_list["set-cookie"];

                    if(YOUTUBE_DEBUG_1)
                        std::cout << "[youtube] Receiving download" << std::endl;

                    if(r1->header_list["proxy-connection"] == "close"
                    || r1->header_list["connection"] == "close")
                    // reconnect if proxy closes the connection after one HTTP request
                    {
                        self->client.reconnect([=]()
                        {
                            self.video_download_detail(m3, view_count, view_max);
                        });

                        return;
                    }

                    self.video_download_detail(m3, view_count, view_max);
                });
            });
        }

        private: void video_download_detail(nextgen::network::http_message m3, size_t view_count, size_t view_max) const
        {
            auto self = *this;

            if(view_count < view_max)
            {
                nextgen::timeout(self->network_service, [=]()
                {
                    self->client.send_and_receive(m3, [=](nextgen::network::http_message r3)
                    {
                        if((r3->status_code != 204 && r3->status_code != 303)
                        || to_int(r3->header_list["content-length"]) != 0)
                        {
                            if(YOUTUBE_DEBUG_1)
                                std::cout << "[proxos:youtube] Error receiving video download. " << r3->status_code << std::endl;

                            return;
                        }


                        if(YOUTUBE_DEBUG_1)
                            std::cout << "[proxos::youtube] VIEWED " << view_count+1 << " TIMES" << std::endl;

                        if(r3->header_list["proxy-connection"] == "close"
                        || r3->header_list["connection"] == "close")
                        // reconnect if proxy closes the connection after one HTTP request
                        {
                            self->client.reconnect([=]()
                            {
                                self.video_download_detail(m3, view_count+1, view_max);
                            });

                            return;
                        }

                        self.video_download_detail(m3, view_count+1, view_max);
                    });
                }, nextgen::random(1000, 3000));
            }
        }

        public: void create_account(std::string const& username, std::string const& password, pgen::email email) const
        {
            auto self = *this;

            pgen::account account;
            account->type = pgen::account::types::youtube;

            self->client.connect("google.com", 80, nextgen::network::ipv4_address("google.com", 80),
            [=]
            {
                nextgen::network::http_message m1;

                m1->method = "GET";
                m1->url = "https://www.google.com/accounts/NewAccount";
                m1->header_list["Host"] = "www.google.com";
                m1->header_list["User-Agent"] = "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";
                m1->header_list["Keep-Alive"] = "300";
                m1->header_list["Connection"] = "keep-alive";

                self->client.send_and_receive(m1,
                [=](nextgen::network::http_message r1)
                {
                    if(r1->status_code != 200)
                    {
                        std::cout << "failed to receive signup page" << r1->status_code << std::endl;

                        return;
                    }

                    std::cout << "new_account response: " << r1->content << std::endl;

                    std::string ctoken = nextgen::regex_single_match("accounts/Captcha\\?ctoken=(.+?)\"", r1->content);
                    std::string dsh = nextgen::regex_single_match("id=\"dsh\".{1,15}value=\"(.+?)\"", r1->content);
                    std::string newaccounttoken = nextgen::regex_single_match("id=\"newaccounttoken\".{1,15}value=\"(.+?)\"", r1->content);
                    std::string newaccounturl = nextgen::regex_single_match("id=\"newaccounturl\".{1,15}value=\"(.+?)\"", r1->content);
                    std::string newaccounttoken_audio = nextgen::regex_single_match("id=\"newaccounttoken_audio\".{1,15}value=\"(.+?)\"", r1->content);
                    std::string newaccounturl_audio = nextgen::regex_single_match("id=\"newaccounturl_audio\".{1,15}value=\"(.+?)\"", r1->content);

                    std::cout << "https://www.google.com/accounts/Captcha?ctoken=" + ctoken << std::endl;

                    std::string newaccountcaptcha;

                    std::cin >> newaccountcaptcha;

                    m1->content = "dsh=" + url_encode(dsh)
                    + "&ktl=&ktf=&Email=" + url_encode(email)
                    + "&Passwd=" + url_encode(password)
                    + "&PasswdAgain=" + url_encode(password)
                    + "&rmShown=1&smhck=1&nshk=1&loc=CA&newaccounttoken=" + url_encode(newaccounttoken)
                    + "&newaccounturl=" + url_encode(newaccounturl)
                    + "&newaccounttoken_audio=" + url_encode(newaccounttoken_audio)
                    + "&newaccounturl_audio=" + url_encode(newaccounturl_audio)
                    + "&newaccountcaptcha=" + url_encode(newaccountcaptcha)
                    + "&privacy_policy_url=http%3A%2F%2Fwww.google.com%2Fintl%2Fen%2Fprivacy.html&requested_tos_location=CA&requested_tos_language=en&served_tos_location=CA&served_tos_language=en&submitbutton=I+accept.+Create+my+account.";

                    std::cout << m1->content << std::endl;

                    m1->url = "https://www.google.com/accounts/CreateAccount";
                    m1->header_list["Referer"] = m1->url;

                    self->client.send(m1,
                    [=]
                    {
                        email.receive(
                        [=](std::string content)
                        {
                            std::string c = nextgen::regex_single_match("accounts/VE\\?c\\=(.+?)\\&hl\\=en", content);

                            if(c == "null")
                            {
                                std::cout << "[youtube] error: null c" << std::endl;

                                return;
                            }

                            nextgen::network::http_client c2(self->network_service);

                            nextgen::network::http_message m2;

                            m2->method = "GET";
                            m2->url = "https://www.google.com/accounts/VE?c=" + c + "&hl=en";
                            m2->header_list["Host"] = "www.google.com";
                            m2->header_list["User-Agent"] = "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";
                            m2->header_list["Keep-Alive"] = "300";
                            m2->header_list["Connection"] = "keep-alive";

                            c2.send_and_receive(m2,
                            [=](nextgen::network::http_message r2)
                            {
                                std::cout << r2->content << std::endl;

                                std::string q1("SELECT * FROM accounts WHERE accounts.person_id = " + to_string(account->person->id) + " LIMIT 1");

                                std::cout << "Executing SQL: " << q1 << std::endl;

                                nextgen::database::row_list row_list = self->main_database.get_row_list(q1);

                                if(rows->size() == 0)
                                {
                                    std::string q2("INSERT INTO accounts SET account_type_id = " + to_string(account->type) + ", account_username = \"" + account->username + "\", account_email = \"" + account->email + "\", account_password = \"" + account->password + "\", person_id = " + to_string(account->person->id));

                                    std::cout << "Executing SQL: " << q2 << std::endl;

                                    self->main_database.query(q2);

                                    ///
                                }
                            });
                        });

                        self->client.receive(
                        [=](nextgen::network::http_message r2)
                        {
                            std::cout << "create_account response: " << r2->content << std::endl;

                            self->client.disconnect();
                        });
                    });
                });
            });
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

            proxos::proxy proxy;
            proxos::agent agent;
        };

        NEXTGEN_SHARED_DATA(client, variables);
    };
}

#endif
