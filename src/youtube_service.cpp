#define FD_SETSIZE 65535//4096

#include "youtube_video.h"

class application : public nextgen::singleton<application>
{
    public: typedef nextgen::network::service network_service_type;

    public: void initialize()
    {
        auto self = *this;
    }

    public: void run(int argc, char* argv[]);

    private: struct variables
    {
        variables()
        {

        }

        ~variables()
        {

        }

        network_service_type network_service;
        nextgen::database::link main_database;
        nextgen::database::link proxy_database;
        nextgen::network::server<nextgen::network::smtp_client> mail_server;
    };

    NEXTGEN_SHARED_DATA(application, variables);
};

namespace pgen
{
	class country
	{
		public: std::string code;
		public: uint32_t id;
	};

	class gender
	{
		public: std::string code;
		public: uint32_t id;
	};

	class name
	{
	    public: typedef name this_type;

        private: struct variables
        {
            variables()
            {

            }

            ~variables()
            {

            }

            std::string first;
            std::string middle;
            std::string last;
        };

        NEXTGEN_SHARED_DATA(name, variables);
	};

	class person
	{
	    public: typedef person this_type;

        private: struct variables
        {
            variables() : id(0)
            {

            }

            ~variables()
            {

            }

            name name;
            openll::datetime birthday;
            std::string postal_code;
            country country;
            gender gender;
            uint32_t id;
        };

        NEXTGEN_SHARED_DATA(person, variables);
	};

	class account
	{
	    public: typedef account this_type;

        private: struct variables
        {
            variables() : id(0)
            {

            }

            ~variables()
            {

            }

            std::string username;
            std::string password;
            uint32_t type;
            person person;
            std::string email;
        };

        NEXTGEN_SHARED_DATA(account, variables);
	};
}

void application::run(int argc, char* argv[])
{
    auto self = *this;

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

                boost::regex_error paren(boost::regex_constants::error_paren);

                try
                {
                    boost::match_results<std::string::const_iterator> what;
                    boost::regex_constants::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;// | boost::regex_constants::extended | boost::regex_constants::mod_s;//boost::regex_constants::match_max;// | boost::regex_constants::match_stop;

                    boost::regex re1("To: (.+?)\r\n");

                    if(boost::regex_search((std::string::const_iterator)r1->content.begin(), (std::string::const_iterator)r1->content.end(), what, re1, flags))
                    {
                        std::string email = what[1];

                        pgen::account account(self->accountRequests[email]);

                        if(!account.get())
                        {
                            std::cout << "<Mail Server> Ignoring old/unassigned email." << std::endl;

                            return;
                        }

                        if(account->type == pgen::account::type_type::google)
                        {
                            boost::regex re2("accounts/VE\\?c\\=(.+)\\&hl\\=en");

                            if(boost::regex_search((std::string::const_iterator)message->content.begin(), (std::string::const_iterator)message->content.end(), what, re2, flags))
                            {
                                std::cout << "found: " << what[1] << std::endl;

                                nextgen::network::http_client c2(self->network_service);

                                nextgen::network::http_message m2;

                                m2->url = "https://www.google.com/accounts/VE?c=" + what[1] + "&hl=en";

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
                            }
                            else
                                std::cout << "not found";
                        }
                    }
                    else
                        std::cout << "not found";

                }
                catch(const boost::regex_error& e)
                {
                    std::cout << "regex error: " << (e.code() == paren.code() ? "unbalanced parentheses" : "?") << std::endl;
                }
            });
        });



    });

    self->main_database.connect("localhost", "root", "swoosh", "main");
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
        }
        else if(command == "create_account")
        {
            youtube::client c1(self->network_service);

            c1.create_account("abcd", "dcba", "abdk@adalda.com");
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

int main(int argc, char* argv[])
{
    application::instance().run(argc, argv);
}
