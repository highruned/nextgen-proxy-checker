#define FD_SETSIZE 65535

#include "youtube_video.h"

class application : public nextgen::singleton<application>
{
    NEXTGEN_SHARED_CLASS(application, NEXTGEN_SHARED_CLASS_VARS(
    {
        nextgen::network::service network_service;
        nextgen::database::link main_database;
        nextgen::database::link proxy_database;
        nextgen::network::server<nextgen::network::smtp_client> mail_server;
    }));

    public: void run(int, char**);
};


void application::run(int argc, char* argv[])
{
    auto self = *this;

    self->main_database.connect("localhost", "root", "swoosh", "main");
    self->proxy_database.connect("localhost", "root", "swoosh", "proxies");

    self->mail_server.accept(
    [=](nextgen::network::smtp_client c1)
    {
        std::cout << "<OnMailServerAddClient>" << std::endl;

        std::cout << "S: " << "220 localhost\r\n" << std::endl;

        c1.send_helo(
        [=]
        {
            c1.receive(
            [=](nextgen::network::smtp_message r1)
            {
                std::cout << "mail data: " << r1->content << std::endl;

                std::string email = nextgen::regex_single_match("To: (.+?)\r\n", r1->content);

                if(email == "null")
                {
                    std::cout << "[youtube] error: null email" << std::endl;

                    return;
                }

               // if(self->email_handler_list.find(email) != self->email_handler_list.end())
                 //   self->email_handler_list[email](r1->content);

            });
        });
    });

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

            std::string query("SELECT proxy_host, proxy_port, type_id FROM proxies WHERE state_id = 8 AND proxy_port != 3124 AND proxy_port != 3127 LIMIT 1");//state_id = 8 ORDER BY proxy_rating DESC, proxy_hits DESC, proxy_latency ASC LIMIT 1,1");

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
        else if(command == "create_account")
        {
            youtube::client c1(self->network_service);

            youtube::account a1;
            a1->username = "abcd";
            a1->password = "dcba";
            a1->email = nextgen::social::email(self->mail_server);
            a1->email->user = "abcd";
            a1->email->host = "blah.com";

            c1.create_account(a1,
            [=]()
            {
                std::string q1("SELECT * FROM accounts WHERE accounts.person_id = " + to_string(a1->person->id) + " LIMIT 1");

                std::cout << "Executing SQL: " << q1 << std::endl;

                nextgen::database::row_list row_list = self->main_database.get_row_list(q1);

                if(row_list->size() == 0)
                {
                    std::string q2("INSERT INTO accounts SET account_type_id = " + to_string(a1->type) + ", account_username = \"" + a1->username + "\", account_email = \"" + a1->email + "\", account_password = \"" + a1->password + "\", person_id = " + to_string(a1->person->id));

                    std::cout << "Executing SQL: " << q2 << std::endl;

                    self->main_database.query(q2);

                    ///
                }
            });
        }
    }

    nextgen::timer timer;

    while(true)
    {
        self->network_service.update();

        nextgen::sleep(0.01);
    }
}

int main(int argc, char* argv[])
{
    application::instance().run(argc, argv);
}
