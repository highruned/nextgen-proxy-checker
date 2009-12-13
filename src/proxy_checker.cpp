#define FD_SETSIZE 65535//4096

#include "common.h"
#include "proxy_checker.h"

class address
{
    union v4
    {
        int value;

        struct
        {
            uint8_t first;
            uint8_t second;
            uint8_t third;
            uint8_t forth;
        };
    };

    public: v4 value;
    public: bool valid;

    public: bool is_valid()
    {
        return this->valid;
    }

    public: void from_string(std::string const& s)
    {
        boost::regex_error paren(boost::regex_constants::error_paren);

        try
        {
            boost::match_results<std::string::const_iterator> what;
            boost::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;

            std::string::const_iterator start = s.begin();
            std::string::const_iterator end = s.end();

            if(boost::regex_search(start, end, what, boost::regex("^([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)$"), flags))
            {
                auto first = boost::lexical_cast<uint32_t>(what[1]);
                auto second = boost::lexical_cast<uint32_t>(what[2]);
                auto third = boost::lexical_cast<uint32_t>(what[3]);
                auto forth = boost::lexical_cast<uint32_t>(what[4]);

                if(!(first >= 0 && first <= 255
                && second >= 0 && second <= 255
                && third >= 0 && third <= 255
                && forth >= 0 && forth <= 255))
                {
                    this->valid = false;

                    return;
                }

                this->value.first = boost::numeric_cast<uint8_t>(first);
                this->value.second = boost::numeric_cast<uint8_t>(second);
                this->value.third = boost::numeric_cast<uint8_t>(third);
                this->value.forth = boost::numeric_cast<uint8_t>(forth);
            }
            else
            {
                this->valid = false;

                return;
            }
        }
        catch(boost::regex_error const& e)
        {
            std::cout << "regex error: " << (e.code() == paren.code() ? "unbalanced parentheses" : "?") << std::endl;
        }
    }

    public: address(uint32_t address) : valid(true)
    {
        this->value.value = address;
    }

    public: address(std::string const& address) : valid(true)
    {
        this->from_string(address);
    }
};

class address_range
{
    public: address lower;
    public: address upper;

    public: bool is_within_range(address& a)
    {
        if(a.value.first <= this->upper.value.first && a.value.first >= this->lower.value.first
        && a.value.second <= this->upper.value.second && a.value.second >= this->lower.value.second
        && a.value.third <= this->upper.value.third && a.value.third >= this->lower.value.third
        && a.value.forth <= this->upper.value.forth && a.value.forth >= this->lower.value.forth)
            return true;

        return false;
    }

    public: address_range(address&& lower, address&& upper) : lower(lower), upper(upper)
    {

    }

    public: address_range(std::string const& lower, std::string const& upper) : lower(lower), upper(upper)
    {

    }
};

class application : public nextgen::singleton<application>
{
    public: typedef nextgen::network::service network_service_type;

    public: void initialize()
    {
        auto self = *this;

    }

    public: void check_proxy(proxos::proxy proxy, std::function<void()> callback = 0) const
    {
        auto self = *this;

        std::cout << proxy->host << ":" << proxy->port << std::endl;

        address a(proxy->host);

        if(!a.is_valid())
        {
            std::cout << "INVALID PROXY ####################################" << std::endl;

            proxy->check_delay = 365 * 24 * 60 * 60;

            std::string query = "UPDATE proxies SET proxy_type = \"invalid\", proxy_latency = 0, proxy_last_checked = NOW(), proxy_check_delay = " + to_string(proxy->check_delay) + " WHERE proxy_id = " + to_string(proxy->id) + " LIMIT 1";

            self->proxy_database.query(query);

            if(callback != 0)
                callback();

            return;
        }

        if(self.proxy_is_banned(a))
        // dont check proxy for another month
        {
            std::cout << "BANNED PROXY ####################################" << std::endl;

            proxy->check_delay = 365 * 24 * 60 * 60;

            std::string query = "UPDATE proxies SET proxy_type = \"banned\", proxy_latency = 0, proxy_last_checked = NOW(), proxy_check_delay = " + to_string(proxy->check_delay) + " WHERE proxy_id = " + to_string(proxy->id) + " LIMIT 1";

            self->proxy_database.query(query);

            if(callback != 0)
                callback();

            return;
        }

        self->proxy_checker.check_proxy(proxy, [=]()
        {
            if(proxy->type == "dead")
            {
                proxy->check_delay = 1 * 24 * 60 * 60;
                proxy->rating -= 1;
            }
            else if(proxy->type == "broken")
            {
                proxy->check_delay = 1 * 24 * 60 * 60;
                proxy->rating -= 1;
            }
            else if(proxy->type == "transparent")
            {
                proxy->check_delay = 6 * 60 * 60;
                proxy->rating += 1;
            }
            else if(proxy->type == "anonymous")
            {
                proxy->check_delay = 6 * 60 * 60;
                proxy->rating += 1;
            }
            else if(proxy->type == "elite")
            {
                proxy->check_delay = 6 * 60 * 60;
                proxy->rating += 1;
            }
               else if(proxy->type == "socks4")
            {
                proxy->check_delay = 6 * 60 * 60;
                proxy->rating += 1;
            }
            else if(proxy->type == "socks5")
            {
                proxy->check_delay = 6 * 60 * 60;
                proxy->rating += 1;
            }
            else if(proxy->type == "codeen")
            {
                proxy->check_delay = 7 * 24 * 60 * 60;
            }
            // todo(daemn) check mysql table check table proxies.proxies; for status OK
            {
                std::string query = "UPDATE proxies SET proxy_type = \"" + proxy->type + "\", proxy_latency = " + to_string(proxy->latency) + ", proxy_state = " + to_string(proxy->state) + ", proxy_rating = " + to_string(proxy->rating) + ", proxy_last_checked = " + to_string(time(0)) + ", proxy_check_delay = " + to_string(proxy->check_delay) + " WHERE proxy_id = " + to_string(proxy.get_id()) + " LIMIT 1";

                std::cout << query << " after " << to_string(proxy->latency) << " seconds. " << std::endl;

                std::cout << "state: " << proxy->state << std::endl;

                self->proxy_database.query(query);
            }

            if(callback != 0)
                callback();
        });
    }

    public: bool proxy_is_banned(address& a) const
    {
        auto self = *this;

        for(auto i = self->banlist.begin(), l = self->banlist.end(); i != l; ++i)
        {
            if((*i).is_within_range(a))
                return true;
        }

        return false;
    }

    public: void run(int argc, char* argv[]);

    private: struct variables
    {
        variables() : proxy_checker("www.proxyprobe.com", 8080, network_service)
        {

        }

        ~variables()
        {

        }

        network_service_type network_service;
        nextgen::database::link proxy_database;
        proxos::proxy_checker proxy_checker;
        std::vector<address_range> banlist;
    };

    NEXTGEN_SHARED_DATA(application, variables);
};


void application::run(int argc, char* argv[])
{
    auto self = *this;

    self->proxy_database.connect("localhost", "root", "swoosh", "proxies");

    std::string query("SELECT * FROM banlist"); //ORDER BY proxy_rating DESC

    std::cout << query << std::endl;

    auto list = *self->proxy_database.get_row_list(query);

    // fill the banlist
    std::for_each(list.begin(), list.end(), [=](nextgen::database::row& row)
    {
        auto host = (*row)["ban_host"];
        auto description = (*row)["ban_description"];

        std::vector<std::string> ip;

        boost::split(ip, host, boost::is_any_of("-"));

        address_range s(ip[0], ip[1]);

        self->banlist.push_back(s);
    });

    auto refill = [=](size_t amount)
    {
        std::cout << "Attempting to load " << amount << " proxies..." << std::endl;

        static uint32_t start = 0;

        std::string query("SELECT proxy_host, proxy_port, proxy_id, proxy_rating FROM proxies WHERE proxy_last_checked < (" + to_string(time(0)) + " - proxy_check_delay) ORDER BY proxy_id LIMIT " + to_string(start) + ", " + to_string(amount)); //ORDER BY proxy_rating DESC

        std::cout << query << std::endl;

        auto list = *self->proxy_database.get_row_list(query);

        std::for_each(list.begin(), list.end(), [=](nextgen::database::row& row)
        {
            std::cout << (*row)["proxy_host"] << " " << (*row)["proxy_port"] << " " << (*row)["proxy_rating"] << std::endl;
            proxos::proxy proxy((*row)["proxy_host"], to_int((*row)["proxy_port"]), to_int((*row)["proxy_id"]));
            proxy->rating = to_int((*row)["proxy_rating"]);

            // check the proxy against banlist
            self.check_proxy(proxy);
        });

        start += amount;

        if(list.size() < amount)
        // we've hit the end of the proxy list, loop back around and check for changes
        {
            start = 0;
            amount = list.size();
        }

        std::cout << "Loaded " << amount << " proxies." << std::endl;
    };

    //self->proxy_checker->refill_event += refill;

/*
unsigned long ulAddr = inet_addr("24.128.21.23");

                                    nextgen::byte_array r1;

                                    r1 << (nextgen::byte)4;
                                    r1 << (nextgen::byte)1;
                                    //r1 << (byte)0;
                                    //r1 << (byte)50;

                                    //r1 << (unsigned char)(((unsigned short)self->transport_layer_->socket_.remote_endpoint().port() >> 8) & 0xff);
                                    //r1 << (unsigned char)((unsigned short)self->transport_layer_->socket_.remote_endpoint().port() & 0xff);
                                    r1 << htons(21505);
                                    r1 << ulAddr;//self->transport_layer_->socket_.remote_endpoint().address().to_v4().to_bytes();//04 01 00 50 ' .. sip .. ' 6e 6d 61 70 00
                                    //r1 << 1852662128;
                                    //r1 << "blank";
                                    r1 << (nextgen::byte)0;

                                    std::cout << r1.to_string() << std::endl;

                                    std::cout << r1.to_string() << std::endl;

                                    exit(0);*/

    if(argc > 1)
    {
        proxos::proxy p1(argv[1], to_int(argv[2]));//("71.56.194.245", 8226);//
        //("24.128.21.23", 21505); //


    // bad ("195.70.235.198", 10000); //("193.178.200.71", 1080);//
    //("85.90.84.132", 1080);//("24.128.21.23", 21505);//("24.128.21.23", 21505);("85.90.84.132", 1080);
        self.check_proxy(p1);
    }

    nextgen::timer timer;

    while(true)
    {
        if(timer.stop() > 1)
        {
            std::cout << "[proxos:application:run] Updating services..." << std::endl;
            std::cout << "C" << self->proxy_checker->job_list.size() << std::endl;
            std::cout << "D" << self->proxy_checker->server->client_list.size() << std::endl;
            //std::cout << "e" << proxy_checker->client_count << std::endl;

            timer.start();
        }

        self->proxy_checker.update();

        self->network_service.update();

        nextgen::usleep(10);
    }
}

int main(int argc, char* argv[])
{
    application::instance().run(argc, argv);
}
