#define FD_SETSIZE 32768

#include "proxy_checker.h"

class application : public nextgen::singleton<application>
{
    public: void run(int, char**);

    public: void refill()
    {
auto self = *this;

    std::string q1("SELECT * FROM agents WHERE agent_type_id = 1");

    auto agent_list = *self->proxy_database.get_row_list(q1);

    static uint32_t count = 0;
        std::string q2("SELECT proxy_host, proxy_port, type_id FROM proxies WHERE state_id != 6 LIMIT " + nextgen::to_string(count) + ",1000");//state_id = 8 ORDER BY proxy_rating DESC, proxy_hits DESC, proxy_latency ASC LIMIT 1,1");
    count += 1000;
        auto proxy_list = *self->proxy_database.get_row_list(q2);

        typedef nextgen::network::http_proxy proxy_type;



        std::for_each(proxy_list.begin(), proxy_list.end(), [=](nextgen::database::row& row)
        {
            auto r1 = *row;

            auto r2 = *agent_list[nextgen::random(0, (int)agent_list.size()-1)];

            nextgen::network::http_agent a1(r2["agent_title"]);

            nextgen::network::http_client c1(self->network_service);

            c1->proxy->host = r1["proxy_host"];
            c1->proxy->port = nextgen::to_int(r1["proxy_port"]);
            c1->proxy->type = nextgen::to_int(r1["type_id"]);

            c1.connect("polls.polldaddy.com", 80, [=]()
            {
                std::cout << "[proxos:youtube] Connected." << std::endl;

                nextgen::network::http_message m1;

                m1->method = "GET";
                m1->url = "http://static.polldaddy.com/p/2450401.js?45980173";
                m1->header_list["Host"] = "polls.polldaddy.com";
                m1->header_list["User-Agent"] = a1->title;
                m1->header_list["Keep-Alive"] = "300";
                m1->header_list["Connection"] = "keep-alive";
                m1->header_list["Referer"] = "http://bubblyphotography.weebly.com/new-years-baby-contest.html";
                m1->header_list["Cookie"] = "PD_poll_2450401=true";

                c1.send_and_receive(m1, [=](nextgen::network::http_message r1)
                {
                    std::cout << "[proxos:youtube] Received video page response." << std::endl;

                    if(r1->status_code != 200)
                    {
                        std::cout << "[proxos:youtube] Error receiving video page. " << r1->status_code << std::endl;

                        c1.disconnect();

                        return;
                    }

                    m1->url = "http://polls.polldaddy.com/vote-js.php?s=108&b=1&p=2450401&a=11851288,&o=&l=0&sl=1&pr=1&pt=0&va=0&cookie=1&rdm=" + nextgen::to_string(nextgen::random(0, 9)) + "&url=http%3A//bubblyphotography.weebly.com/new-years-baby-contest.html&w=2450401";

                    if(r1->header_list["proxy-connection"] == "close"
                    || r1->header_list["connection"] == "close")
                    // reconnect if proxy closes the connection after one HTTP request
                    {
                        c1.reconnect([=]()
                        {

                    c1.send_and_receive(m1, [=](nextgen::network::http_message r1)
                    {
                        std::cout << "[proxos:youtube] Received video page response." << std::endl;


    std::cout << r1->content << std::endl;
                        if(r1->status_code != 200)
                        {
                            std::cout << "[proxos:youtube] Error receiving video page. " << r1->status_code << std::endl;

                            c1.disconnect();

                            return;
                        }

                        m1->url = "http://stats.polldaddy.com/p.gif?host=bubblyphotography.weebly.com&rand=0." + nextgen::to_string(nextgen::random(11111111111111, 99999999999999)) + "&p=2450401&v=1&ref=";
                        m1->header_list["Referer"] = "http://bubblyphotography.weebly.com/new-years-baby-contest.html";

                        if(r1->header_list["proxy-connection"] == "close"
                        || r1->header_list["connection"] == "close")
                        // reconnect if proxy closes the connection after one HTTP request
                        {
                            c1.reconnect([=]()
                            {
                                c1.send_and_receive(m1, [=](nextgen::network::http_message r1)
                                {
                                    std::cout << "[proxos:youtube] Received video page response 2." << std::endl;

                                    if(r1->status_code != 200)
                                    {
                                        std::cout << "[proxos:youtube] Error receiving video page. " << r1->status_code << std::endl;

                                        c1.disconnect();

                                        return;
                                    }

                                    c1.disconnect();
                                });
                            });

                            return;
                        }

                        c1.send_and_receive(m1, [=](nextgen::network::http_message r1)
                        {
                            std::cout << "[proxos:youtube] Received video page response 2." << std::endl;

                            if(r1->status_code != 200)
                            {
                                std::cout << "[proxos:youtube] Error receiving video page. " << r1->status_code << std::endl;

                                c1.disconnect();

                                return;
                            }

                            c1.disconnect();
                        });


                    });
                        });

                        return;
                    }

                    c1.send_and_receive(m1, [=](nextgen::network::http_message r1)
                    {
                        std::cout << "[proxos:youtube] Received video page response." << std::endl;


    std::cout << r1->content << std::endl;

                        if(r1->status_code != 200)
                        {
                            std::cout << "[proxos:youtube] Error receiving video page. " << r1->status_code << std::endl;

                            c1.disconnect();

                            return;
                        }

                        m1->url = "http://stats.polldaddy.com/p.gif?host=bubblyphotography.weebly.com&rand=0." + nextgen::to_string(nextgen::random(11111111111111, 99999999999999)) + "&p=2450401&v=1&ref=";
                        m1->header_list["Referer"] = "http://bubblyphotography.weebly.com/new-years-baby-contest.html";

                        if(r1->header_list["proxy-connection"] == "close"
                        || r1->header_list["connection"] == "close")
                        // reconnect if proxy closes the connection after one HTTP request
                        {
                            c1.reconnect([=]()
                            {
                                c1.send_and_receive(m1, [=](nextgen::network::http_message r1)
                                {
                                    std::cout << "[proxos:youtube] Received video page response 2." << std::endl;

                                    if(r1->status_code != 200)
                                    {
                                        std::cout << "[proxos:youtube] Error receiving video page. " << r1->status_code << std::endl;

                                        c1.disconnect();

                                        return;
                                    }

                                    c1.disconnect();
                                });
                            });

                            return;
                        }

                        c1.send_and_receive(m1, [=](nextgen::network::http_message r1)
                        {
                            std::cout << "[proxos:youtube] Received video page response 2." << std::endl;

                            if(r1->status_code != 200)
                            {
                                std::cout << "[proxos:youtube] Error receiving video page. " << r1->status_code << std::endl;

                                c1.disconnect();

                                return;
                            }

                            c1.disconnect();
                        });


                    });

                });
            });
        });
    }

    private: struct variables
    {
        variables()
        {

        }

        nextgen::network::service network_service;
        nextgen::database::link proxy_database;
        std::vector<nextgen::network::address_range> banlist;
    };

    NEXTGEN_ATTACH_SHARED_VARIABLES(application, variables);
};

void application::run(int argc, char* argv[])
{
    auto self = *this;

    self->proxy_database.connect("localhost", "root", "swoosh", "proxies");



self.refill();


//http://polls.polldaddy.com/vote-js.php?s=108&b=1&p=2450401&a=11851288,&o=&l=0&sl=1&pr=1&pt=0&va=0&cookie=1&rdm=2&url=http%3A//bubblyphotography.weebly.com/new-years-baby-contest.html%23pd_a_2450401&w=2450401
//http://polls.polldaddy.com/vote-js.php?s=108&b=1&p=2450401&a=11851288,&o=&l=0&sl=1&pr=1&pt=0&va=0&cookie=1&rdm=0&url=http%3A//bubblyphotography.weebly.com/new-years-baby-contest.html%23pd_a_2450401&w=2450401


    nextgen::timer timer;
    nextgen::timer timer2;

    while(true)
    {
        if(timer.stop() > 10)
        {

            std::cout << "[proxos:application:run] Updating services..." << std::endl;

            std::cout << "descriptors: " << nextgen::get_process_total_descriptors(nextgen::get_current_process_id()) << std::endl;

            std::cout << "object registry: " << std::endl;

            for(nextgen::object_registry_type::iterator i = nextgen::object_registry.begin(), l = nextgen::object_registry.end(); i != l; ++i)
            {
                std::cout << (*i).first << ": " << (*i).second << std::endl;
            }

            timer.start();
        }

        if(timer2.stop() > 300)
        {
            self.refill();


            timer2.start();
        }

        self->network_service.update();

        nextgen::sleep(0.01);
    }
}

int main(int argc, char* argv[])
{
    application::instance().run(argc, argv);
}
