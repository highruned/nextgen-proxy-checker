#define FD_SETSIZE 65535//4096

#include "youtube_video.h"

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
        nextgen::database::link proxy_database;
    };

    NEXTGEN_SHARED_DATA(application, variables);
};

void application::run()
{
    auto self = *this;

    self->proxy_database.connect("localhost", "root", "swoosh", "proxies");

    if(argc > 1)
    {
        std::string command = argv[1];

        if(command == "view_video")
        {
            youtube::video video;
            video->id = argv[2];//"46r3LHabaFU";//pCAwU3-9n18";//xdEy5oAh3dk";

            std::string q1("SELECT * FROM agents WHERE agent_type_id = 1");

            if(YOUTUBE_DEBUG_1)
                std::cout << q1 << std::endl;

            auto agent_list = *self->proxy_database.get_row_list(q1);

            {//proxy_id = 1049
                std::string query("SELECT proxy_host, proxy_port, type_id FROM proxies WHERE state_id = 8 AND proxy_port != 3124 AND proxy_port != 3127");//state_id = 8 ORDER BY proxy_rating DESC, proxy_hits DESC, proxy_latency ASC LIMIT 1,1");

                if(YOUTUBE_DEBUG_1)
                    std::cout << query << std::endl;

                auto proxy_list = *self->proxy_database.get_row_list(query);

                std::for_each(proxy_list.begin(), proxy_list.end(), [=](nextgen::database::row& row)
                {
                    if(YOUTUBE_DEBUG_1)
                        std::cout << (*row)["proxy_host"] << " " << (*row)["proxy_port"] << std::endl;

                    proxos::proxy proxy((*row)["proxy_host"], to_int((*row)["proxy_port"]));
                    proxy->type = to_int((*row)["type_id"]);

                    auto row2 = agent_list[nextgen::random(0, (int)agent_list.size()-1)];

                    proxos::agent agent((*row2)["agent_title"]);

                    youtube::client c1(self->network_service);
                    c1->proxy = proxy;
                    c1->agent = agent;

                    c1.view_video(video, 250);
                });
            }
        }
        else if(command == "create_account")
        {
            nextgen::network::http_message m1;

            m1->method = "GET";
            m1->url = "https://www.google.com/accounts/NewAccount";
            m1->header_list["Host"] = "www.google.com";
            m1->header_list["User-Agent"] = "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.5) Gecko/20091109 Ubuntu/9.10 (karmic) Firefox/3.5.5";
            m1->header_list["Keep-Alive"] = "300";
            m1->header_list["Connection"] = "keep-alive";

            youtube::client c1(self->network_service);

            c1.connect("youtube.com", 80, nextgen::network::ipv4_address("youtube.com", 80),
            {
                c1.send_and_receive(m1,
                [=](nextgen::network::http_message r1)
                {
                    if(r1->status_code != 200)
                    {
                        std::cout << "failed to receive signup page" << r1->status_code << std::endl;

                        return;
                    }

                    std::string ctoken = nextgen::regex_single_match("accounts/Captcha\\?ctoken=(.+?)\"", r1->content);
                    std::string dsh = nextgen::regex_single_match("id=\"dsh\".{1,15}value=\"(.+?)\"", r1->content);
                    std::string newaccounttoken = nextgen::regex_single_match"id=\"newaccounttoken\".{1,15}value=\"(.+?)\"", r1->content);
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

                    std::cout << m2->content << std::endl;

                    m1->url = "https://www.google.com/accounts/CreateAccount";
                    m1->header_list["Referer"] = m1->url;

                    client.send_and_receive(m2,
                    [=](nextgen::network::http_message r2)
                    {

                    },
                    [=]
                    {

                    });
                });
            });
        }
    }

    nextgen::timer timer;

    while(true)
    {
        if(timer.stop() > 1)
        {
            //std::cout << "[application:run] Updating services..." << std::endl;

            timer.start();
        }

        self->network_service.update();

        nextgen::sleep(0.01);
    }
}

int main()
{
    application::instance().run();
}
