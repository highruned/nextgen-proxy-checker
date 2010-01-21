#define FD_SETSIZE 32768

#include "youtube_video.h"

class application : public nextgen::singleton<application>
{
    struct variables
    {
        variables() : mail_server(network_service, 25)
        {

        }

        nextgen::network::service network_service;
        nextgen::database::link main_database;
        nextgen::database::link proxy_database;
        nextgen::network::smtp_server mail_server;
        nextgen::social::service social_service;
    };

    NEXTGEN_ATTACH_SHARED_VARIABLES(application, variables);

    public: void run(int, char**);
};

void application::run(int argc, char* argv[])
{
    auto self = *this;

    self->main_database.connect("localhost", "user", "boom7441", "main");
    self->proxy_database.connect("localhost", "user", "boom7441", "proxies");

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

                if(email == nextgen::null_str)
                {
                    std::cout << "[youtube] error: null email" << std::endl;

                    return;
                }

                if(self->mail_server->handler_list[email])
                    self->mail_server->handler_list[email](r1->content);
            });
        });
    });

    self->social_service->person_list_empty_event += [=]()
    {
        std::string query("SELECT * FROM people WHERE people.person_id NOT IN (SELECT accounts.person_id FROM accounts)");

        if(YOUTUBE_DEBUG_1)
            std::cout << query << std::endl;

        auto person_list = *self->main_database.get_row_list(query);

        std::for_each(person_list.begin(), person_list.end(), [=](nextgen::database::row& row)
        {
            auto r1 = *row;

            nextgen::social::person person;

            person->id = nextgen::to_int(r1["person_id"]);
            person->country->id = nextgen::to_int(r1["country_id"]);
            person->gender->id = nextgen::to_int(r1["gender_id"]);
            person->postal_code = r1["person_postal_code"];
            person->birthday = boost::gregorian::from_simple_string(r1["person_birthday"].substr(0, 10));

            {
                auto r2 = *self->main_database.get_row("SELECT name_title, name_id FROM names WHERE name_id = " + r1["name_id_first"] + " LIMIT 1");

                person->name->first = r2["name_title"];
            }

            {
                auto r2 = *self->main_database.get_row("SELECT name_title, name_id FROM names WHERE name_id = " + r1["name_id_last"] + " LIMIT 1");

                person->name->last = r2["name_title"];
            }

            {
                auto r2 = *self->main_database.get_row("SELECT country_code FROM countries WHERE country_id = " + nextgen::to_string(person->country->id) + " LIMIT 1");

                person->country->code = r2["country_code"];
            }

            {
                auto r2 = *self->main_database.get_row("SELECT gender_code FROM genders WHERE gender_id = " + nextgen::to_string(person->gender->id) + " LIMIT 1");

                person->gender->code = r2["gender_code"];
            }

            self->social_service.add_person(person);
        });
    };

    self->social_service->person_list_empty_event();

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

            std::string q2("SELECT proxy_host, proxy_port, type_id FROM proxies WHERE state_id = 8 AND proxy_port != 3124 AND proxy_port != 3127 LIMIT 50");//state_id = 8 ORDER BY proxy_rating DESC, proxy_hits DESC, proxy_latency ASC LIMIT 1,1");

            if(YOUTUBE_DEBUG_1)
                std::cout << q2 << std::endl;

            auto proxy_list = *self->proxy_database.get_row_list(q2);

            std::for_each(proxy_list.begin(), proxy_list.end(), [=](nextgen::database::row& row)
            {
                auto r1 = *row;

                if(YOUTUBE_DEBUG_1)
                    std::cout << r1["proxy_host"] << " " << r1["proxy_port"] << std::endl;

                nextgen::network::http_proxy proxy;
                proxy->host = r1["proxy_host"];
                proxy->port = nextgen::to_int(r1["proxy_port"]);
                proxy->type = nextgen::to_int(r1["type_id"]);

                auto r2 = *agent_list[nextgen::random(0, (int)agent_list.size()-1)];

                nextgen::network::http_agent agent(r2["agent_title"]);

                youtube::client c1(self->network_service);
                c1->client->proxy = proxy;
                c1->agent = agent;

                c1.view_video(video, 3);
            });
        }
        else if(command == "create_account")
        {
            youtube::client c1(self->network_service);

            youtube::account a1;

            a1->person = self->social_service.get_random_person();
            a1->user = nextgen::social::user();

            auto id = nextgen::random(100, 10000);

            switch(nextgen::random(1, 3))
            {
                case 1: a1->user->username = a1->person->name->first + nextgen::to_string(id); break;
                case 2: a1->user->username = a1->person->name->last + nextgen::to_string(id); break;
                case 3: a1->user->username = a1->person->name->first + nextgen::to_string(id) + "x"; break;
                //case 4: a1->user->username = a1->person->name->first + "_" + to_string(id) + "x"; break;
                //case 5: a1->user->username = a1->person->name->first + "_" + to_string(id); break;
                // nicknames
            }

            a1->user->password = nextgen::reverse_string(a1->user->username);
            a1->user->email = nextgen::social::email(self->mail_server);
            a1->user->email->user = a1->user->username;
            a1->user->email->host = "69labs.com";//social.0-x.pl";

            std::string q1("SELECT * FROM accounts WHERE accounts.person_id = " + nextgen::to_string(a1->person->id) + " LIMIT 1");

            std::cout << "Executing SQL: " << q1 << std::endl;

            nextgen::database::row_list row_list = self->main_database.get_row_list(q1);

            std::cout << a1->user->email.to_string() << std::endl;

            if(row_list->size() == 0)
            {
                c1.create_account(a1,
                [=]
                {
                    std::string q2("INSERT INTO accounts SET account_type_id = " + nextgen::to_string(a1->type) + ", account_username = \"" + a1->user->username + "\", account_email = \"" + a1->user->email.to_string() + "\", account_password = \"" + a1->user->password + "\", person_id = " + nextgen::to_string(a1->person->id));

                    std::cout << "Executing SQL: " << q2 << std::endl;

                    self->main_database.query(q2);
                });
            }
        }
    }

    nextgen::timer timer;

    while(true)
    {
        self->network_service.update();
        self->social_service.update();

        nextgen::sleep(0.05);
    }
}

int main(int argc, char* argv[])
{
    application::instance().run(argc, argv);
}
