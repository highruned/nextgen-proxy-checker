#ifndef NEXTGEN_COMMON
#define NEXTGEN_COMMON

#include <cstddef>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <csignal>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include <set>
#include <list>
#include <queue>
#include <algorithm>         // copy, min.
#include <cassert>
#include <deque>
#include <cstdlib>
#include <cstring>

//#include <time.h>
//#include <math.h>
#include <errno.h>
//#include <signal.h>
//#include <typeinfo.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <boost/iostreams/stream.hpp>
#include <boost/config.hpp>  // BOOST_NO_STDC_NAMESPACE.
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/detail/ios.hpp>  // failure.
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/ref.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/random.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_map.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_archive_exception.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/date_time/gregorian/parsers.hpp>

#include "content_gzip.h"
#include "mysql/mysql.h"


#define NEXTGEN_PLATFORM_DOS 0
#define NEXTGEN_PLATFORM_WINDOWS 1
#define NEXTGEN_PLATFORM_OS2 2
#define NEXTGEN_PLATFORM_APPLE 3
#define NEXTGEN_PLATFORM_INTEL 4
#define NEXTGEN_PLATFORM_UNIX 5

#if defined(__MSDOS__) || defined(MSDOS)
    #define NEXTGEN_PLATFORM NEXTGEN_PLATFORM_DOS
#elif defined(_WIN64)
    #define NEXTGEN_PLATFORM NEXTGEN_PLATFORM_WINDOWS
#elif defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__) || defined(WIN32)
    #define NEXTGEN_PLATFORM NEXTGEN_PLATFORM_WINDOWS
#elif defined(OS_2) || defined(__OS2__) || defined(OS2)
    #define NEXTGEN_PLATFORM NEXTGEN_PLATFORM_OS2
#elif defined(__APPLE_CC__)
    #define NEXTGEN_PLATFORM NEXTGEN_PLATFORM_APPLE
#elif defined(__INTEL_COMPILER)
    #define NEXTGEN_PLATFORM NEXTGEN_PLATFORM_INTEL
#else
    #define NEXTGEN_PLATFORM NEXTGEN_PLATFORM_UNIX
#endif

#if NEXTGEN_PLATFORM == NEXTGEN_PLATFORM_WINDOWS
    #define _WIN32_WINNT 0x0900
    #include <iphlpapi.h>
    #include <Psapi.h>
    #include <stdio.h>
    #include <Wincrypt.h>
#endif


#define NEXTGEN_ATTACH_SHARED_VARIABLES(this_type_arg, data_type_arg, ...) \
    public: typedef data_type_arg ng_data_type; \
    public: typedef this_type_arg ng_this_type; \
    protected: boost::shared_ptr<data_type_arg> ng_data; \
    public: this_type_arg(this_type_arg& t) : ng_data(t.ng_data) { } \
    public: this_type_arg(this_type_arg const& t) : ng_data(t.ng_data) { } \
    public: this_type_arg(nextgen::null_t& t) { __VA_ARGS__ } \
    public: template<typename ...ng_argument_types> this_type_arg(ng_argument_types&& ...argument_list) : ng_data(new data_type_arg(argument_list...)) { __VA_ARGS__ } \
    public: template<typename ng_argument_type> this_type_arg(ng_argument_type&& t, typename boost::enable_if<boost::is_base_of<this_type_arg, ng_argument_type>>::type* dummy = 0) : ng_data(t.ng_data) { } \
    public: bool operator==(this_type_arg const& t) const { return &(*this->ng_data) == &(*t.ng_data); } \
    public: bool operator==(int t) const { if(t == 0) return this->ng_data == 0; else return 0; } \
    public: bool operator!=(this_type_arg const& t) const { return !this->operator==(t); } \
    public: bool operator!=(int t) const { return !this->operator==(t); } \
    public: void operator=(int t) { if(t == 0) this->ng_data.reset(); } \
    public: boost::shared_ptr<data_type_arg> const& operator->() const { if(this->ng_data == 0) std::cout << "undefined ng_data in " << typeid(*this).name() << std::endl; return this->ng_data; }

#define NEXTGEN_ATTACH_SHARED_BASE(this_type_arg, base_type_arg, ...) \
    public: typedef this_type_arg ng_this_type; \
    public: this_type_arg(this_type_arg& t) : base_type_arg(*((base_type_arg*)(&t))) { } \
    public: this_type_arg(this_type_arg const& t) : base_type_arg(*((base_type_arg*)(&t))) { } \
    public: this_type_arg(nextgen::null_t& t) : base_type_arg() { } \
    public: template<typename ...ng_argument_types> this_type_arg(ng_argument_types&& ...argument_list) : base_type_arg(argument_list...) { __VA_ARGS__ } \
    public: template<typename ng_argument_type> this_type_arg(ng_argument_type&& t, typename boost::enable_if<boost::is_base_of<this_type_arg, ng_argument_type>>::type* dummy = 0) : base_type_arg(*((base_type_arg*)(&t))) { }

int readHex(const char* s)
{
int i;
std::istringstream (s) >> std::hex >> i;
return i;
}

bool NEXTGEN_DEBUG_1 = 1;
bool NEXTGEN_DEBUG_2 = 1;
bool NEXTGEN_DEBUG_3 = 0;
bool NEXTGEN_DEBUG_4 = 1;
bool NEXTGEN_DEBUG_5 = 1;


std::string url_encode(std::string const& str)
{
    std::string str2;

	for(size_t i = 0, l = str.size(); i < l; ++i)
	{
		switch(str[i])
		{
            case '%': str2 += "%25"; break;
            case ' ': str2 += "%20"; break;
            case '^': str2 += "%5E"; break;
            case '&': str2 += "%26"; break;
            case '`': str2 += "%60"; break;
            case '{': str2 += "%7B"; break;
            case '}': str2 += "%7D"; break;
            case '|': str2 += "%7C"; break;
            case ']': str2 += "%5D"; break;
            case '[': str2 += "%5B"; break;
            case '"': str2 += "%22"; break;
            case '<': str2 += "%3C"; break;
            case '>': str2 += "%3E"; break;
            case '\\': str2 += "%5C"; break;
            case '#': str2 += "%23"; break;
            case '?': str2 += "%3F"; break;
            case '/': str2 += "%2F"; break;
            case ':': str2 += "%3A"; break;
            case '@': str2 += "%40"; break;
            case '=': str2 += "%3D"; break;
            default: str2 += str[i]; break;
        }
	}

	return str2;
}

void find_and_replace(std::string& source, std::string const& find, std::string const& replace)
{
	for(std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
	{
		source.replace(i, find.length(), replace);

		i += replace.length() - find.length() + 1;
	}
}

template<typename element_type>
std::string to_string(element_type element)
{
	return boost::lexical_cast<std::string>(element);
}

template<typename element_type>
int to_int(element_type element)
{
	return boost::lexical_cast<int32_t>(element);
}

template<>
int to_int(std::string element)
{
    boost::regex_error paren(boost::regex_constants::error_paren);

    try
    {
        boost::match_results<std::string::const_iterator> what;
        boost::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;

        std::string::const_iterator start = element.begin();
        std::string::const_iterator end = element.end();

        if(boost::regex_search(start, end, what, boost::regex("([\\-0-9]+)"), flags))
        {
            if(NEXTGEN_DEBUG_3)
                std::cout << "converting to int: " << what[1] << std::endl;

            return boost::lexical_cast<int>(what[1]);
        }
        else
        {
            std::cout << "Couldn't convert to int" << std::endl;
        }
    }
    catch(boost::regex_error const& e)
    {
        std::cout << "regex error: " << (e.code() == paren.code() ? "unbalanced parentheses" : "?") << std::endl;
    }

    return 0;
}

std::string reverse_string(std::string const& s)
{
	std::string temp(s);
	std::reverse(temp.begin(), temp.end());

	return temp;
}

namespace nextgen
{
    typedef boost::uint8_t byte; //typedef unsigned char byte;

    std::string const null_str = "null_t";
    int const null_num = 0;

    class null_t
    {
        public: template<typename element_type> operator element_type() const
        {
            return 0;
        }

        public: template<typename element_type> bool operator==(element_type& e) const
        {
            return e == 0;
        }
    };

    template<>
    null_t::operator std::string() const
    {
        return null_str;
    }

    template<>
    bool null_t::operator ==(std::string& s) const
    {
        return s == null_str;
    }

    null_t null;

    void getline(std::string& source, std::string& destination)
    {
        size_t pos = source.find("\r\n");

        if(pos != std::string::npos)
        {
            std::cout << "getline" << pos << std::endl;

            destination = source.substr(0, pos);

            boost::erase_head(source, pos + 2);
        }
    }

    std::string to_hex(std::string const& str)
    {
        std::string output;

        char ch[3];

        for(size_t i = 0, l = str.size(); i < l; ++i)
        {
            sprintf(ch, "\\x%02x", (byte)str[i]);

            output += ch;
        }

        return output;
    }

    #if NEXTGEN_PLATFORM == NEXTGEN_PLATFORM_WINDOWS
        typedef HANDLE handle;
        typedef HWND window_handle;
        typedef HMODULE module_handle;
    	typedef PVOID void_pointer;
    	typedef STARTUPINFOA startup_information;
    	typedef PROCESS_INFORMATION pProcess_information;
    #else
        typedef int handle;
        typedef int window_handle;
        typedef int module_handle;
        typedef void* void_pointer;
    	typedef int startup_information;
    	typedef int process_information;
    #endif

    void sleep(float s)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds((uint32_t)(s * 1000)));
    }

    class byte_array
    {
        public: void reverse_bytes(byte* input, byte* output, size_t length)
        {
            for(size_t i = 0; i < length; ++i)
                output[i] = input[length - i - 1];
        }

        public: template<typename element_type> void read(element_type&& output, size_t length = 0)
        {
            auto self = *this;

            std::istream data_stream(&self->data);

            length = length > 0 ? length : sizeof(element_type);

            memset((byte*)&output, 0, length);

            byte* input = (byte*)&output;

            if(length > self.length())
                length = self.length();

            if((self->little_endian && !self.is_little_endian())
            || (!self->little_endian && self.is_little_endian()))
                for(size_t i = length; i > 0; --i)
                    data_stream >> input[i-1];
            else
                for(size_t i = 0; i < length; ++i)
                    data_stream >> input[i];
        }

        public: template<typename element_type> void write(element_type&);
        public: template<typename element_type> void write(element_type&&, size_t);

        public: template<typename element_type> byte_array operator>>(element_type&& output) const
        {
            auto self = *this;

            self.read(output);

            return *this;
        }

        public: template<typename element_type> byte_array operator<<(element_type&& input) const
        {
            auto self = *this;

            self.write(input);

            return *this;
        }

        public: size_t available()
        {
            auto self = *this;

            return self->data.size();
        }

        public: size_t size()
        {
            auto self = *this;

            return self->data.size();
        }

        public: size_t length()
        {
            auto self = *this;

            return self->data.size();
        }

        public: bool is_little_endian()
        {
            int i = 1;

            char *p = (char*)&i;

            if (p[0] == 1)
                return true;
            else
                return false;
        }

        public: std::string read_all()
        {
            auto self = *this;

            std::istream data_stream(&self->data);

            std::string all("");

            return std::string((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());
        }

        public: void write_all(std::string const& all)
        {
            auto self = *this;

            std::ostream data_stream(&self->data);

            for(size_t i = 0, l = all.length(); i < l; ++i)
                data_stream << all[i];
        }

        public: std::string to_string() const
        {
            auto self = *this;

            std::string output;

            char ch[3];

            std::iostream data_stream(&self->data);

            std::vector<byte> all;

            while(self->data.sgetc() != EOF)
            {
                all.push_back((byte)self->data.sbumpc());
            }

            for(size_t i = 0, l = all.size(); i < l; ++i)
            {
                sprintf(ch, "\\x%02x", (byte)all[i]);

                output += ch;

                self->data.sputc((byte)all[i]);
            }

            return output;
        }

        public: asio::streambuf& get_buffer() const
        {
            auto self = *this;

            return self->data;
        }

        private: struct variables
        {
            variables() : little_endian(false)
            {

            }

            variables(byte_array& ba, size_t length) : little_endian(false)
            {
                // todo(daemn) fix this
                std::ostream ostream(&this->data);

                ostream << &ba->data;
            }

            bool little_endian;
            asio::streambuf data;
        };

        NEXTGEN_ATTACH_SHARED_VARIABLES(byte_array, variables,
        {
            auto self = *this;

            self->little_endian = self.is_little_endian();
        });
    };

    template<typename element_type>
    inline void byte_array::write(element_type&& input, size_t length)
    {
        auto self = *this;

        std::ostream data_stream(&self->data);

        length = length > 0 ? length : sizeof(element_type);

        byte* output = (byte*)&input;

        if((self->little_endian && !self.is_little_endian())
        || (!self->little_endian && self.is_little_endian()))
            for(size_t i = length; i > 0; --i)
                data_stream << output[i-1];
        else
            for(size_t i = 0; i < length; ++i)
                data_stream << output[i];
    }


    template<typename element_type>
    inline void byte_array::write(element_type& input)
    {
        auto self = *this;

        self.write(input, 0);
    }


    template<>
    inline void byte_array::write(std::string& input)
    {
        auto self = *this;

        std::ostream data_stream(&self->data);

        size_t length = input.length();

        for(size_t i = 0; i < length; ++i)
            data_stream << input[i];
    }

    template<>
    inline void byte_array::write(byte_array& input)
    {
        auto self = *this;

        std::ostream ostream(&self->data);

        ostream << &input->data;
    }

    template<typename element_type>
    class singleton
    {
        public: explicit singleton();

        public: virtual ~singleton();

        public: static element_type& instance();
        public: static element_type* pointer_instance();

        private: static element_type* element;
    };

    template<typename element_type>
    element_type* singleton<element_type>::element = 0;

    template<typename element_type>
    inline singleton<element_type>::singleton()
    {
        //Assert(!instance);
    }

    template<typename element_type>
    inline singleton<element_type>::~singleton()
    {
        //Assert(instance);

        element = 0;
    }

    template<typename element_type>
    inline element_type& singleton<element_type>::instance()
    {
        return *pointer_instance();
    }

    template<typename element_type>
    inline element_type* singleton<element_type>::pointer_instance()
    {
        if(null == element)
            element = new element_type();

        //Assert(element);

        return element;
    }

	template<typename callback_type>
	class event
	{
	    public: typedef std::list<callback_type> callback_list_type;

		public: template<typename ...element_type_list> void call(element_type_list&& ...element_list)
		{
			for(typename callback_list_type::const_iterator i = this->list.begin(), l = this->list.end(); i != l; ++i)
			{
			    if(NEXTGEN_DEBUG_4)
                    std::cout << "CALLING CALLBACK" << std::endl;

				(*i)(element_list...);
			}
		}

		public: void add(callback_type&& t)
		{
			this->list.push_back(t);
		}

		public: void remove(callback_type& t)
		{
			this->list.remove(t);
		}

		public: bool operator!()
		{
			if(this->list.size() == 0)
				return true;
			else
				return false;
		}

		public: operator bool()
		{
			if(this->list.size() == 0)
				return true;
			else
				return false;
		}

		public: template<typename ...element_list_type> event<callback_type>& operator()(element_list_type&& ...element_list)
		{
			this->call(element_list...);

			return *this;
		}

		public: event<callback_type>& operator+(callback_type& t)
		{
			this->add(t);

			return *this;
		}

		public: event<callback_type>& operator-(callback_type& t)
		{
			this->remove(t);

			return *this;
		}

		public: void operator+=(callback_type&& t)
		{
			this->add(t);
		}

		public: template<typename element_type> void operator+=(element_type&& a)
		{
			this->add(callback_type(a));
		}

		public: void operator-=(callback_type& t)
		{
			this->remove(t);
		}

		public: template<typename element_type> void operator-=(element_type&& a)
		{
			this->remove(callback_type(a));
		}

		private: callback_list_type list;
	};

    namespace detail
    {
        template<typename element_type>
        class random
        {
            public: random(element_type a1) : gen((boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()) - boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1))).ticks()), dst(0, a1), rand(gen, dst) {}
            public: random(element_type a1, element_type a2) : gen((boost::posix_time::ptime(boost::posix_time::microsec_clock::local_time()) - boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1))).ticks()), dst(a1, a2), rand(gen, dst) {}

            public: element_type get() { return this->rand(); }

            public: std::ptrdiff_t operator()(std::ptrdiff_t arg)
            {
                return static_cast<std::ptrdiff_t>(this->rand());
            }

            private: boost::mt19937 gen;
            private: boost::uniform_int<element_type> dst;
            private: boost::variate_generator<boost::mt19937, boost::uniform_int<element_type>> rand;
        };
    }

    template<typename element_type>
    element_type random(element_type a1, element_type a2)
    {
        return detail::random<element_type>(a1, a2).get();
    }

    #if PLATFORM == PLATFORM_UNIX
        #include <sys/time.h>
        #include <stdio.h>
        #include <unistd.h>
    #endif

    class timer
    {
        public: void start() const
        {
            auto self = *this;

            #if NEXTGEN_PLATFORM == NEXTGEN_PLATFORM_WINDOWS
                QueryPerformanceCounter(&self->begin);
            #elif NEXTGEN_PLATFORM == NEXTGEN_PLATFORM_UNIX
                gettimeofday(&self->begin, NULL);
            #endif
        }

        public: float stop() const
        {
            auto self = *this;

            #if NEXTGEN_PLATFORM == NEXTGEN_PLATFORM_WINDOWS
                LARGE_INTEGER end;
                QueryPerformanceCounter(&end);

                return set_precision(float(end.QuadPart - self->begin.QuadPart) / self->freq.QuadPart, 2);
            #elif NEXTGEN_PLATFORM == NEXTGEN_PLATFORM_UNIX
                timeval end;

                gettimeofday(&end, NULL);

                long seconds  = end.tv_sec  - self->begin.tv_sec;
                long useconds = end.tv_usec - self->begin.tv_usec;

                return float((((seconds) * 1000 + useconds/1000.0) + 0.5) / 1000);
            #endif
        }

        private: struct variables
        {
            variables()
            {
                #if NEXTGEN_PLATFORM == NEXTGEN_PLATFORM_WINDOWS
                    QueryPerformanceFrequency(&this->freq);

                    QueryPerformanceCounter(&this->begin);
                #elif NEXTGEN_PLATFORM == NEXTGEN_PLATFORM_UNIX
                    gettimeofday(&this->begin, NULL);
                #endif
            }

            ~variables()
            {

            }

            #if NEXTGEN_PLATFORM == NEXTGEN_PLATFORM_WINDOWS
                LARGE_INTEGER freq, begin;
            #elif NEXTGEN_PLATFORM == NEXTGEN_PLATFORM_UNIX
                timeval begin;
            #endif
        };

        NEXTGEN_ATTACH_SHARED_VARIABLES(timer, variables,
        {
            this->start();
        });
    };

    std::string regex_single_match(std::string const& pattern, std::string const& subject)
    {
        boost::regex_error paren(boost::regex_constants::error_paren);

        try
        {
            boost::match_results<std::string::const_iterator> what;
            boost::regex_constants::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;

            if(boost::regex_search((std::string::const_iterator)subject.begin(), (std::string::const_iterator)subject.end(), what, boost::regex(pattern), flags))
                return what[1];
        }
        catch(boost::regex_error const& e)
        {
            std::cout << "regex error: " << (e.code() == paren.code() ? "unbalanced parentheses" : "?") << std::endl;
        }

        return null_str;
    }

    void exit(std::string const& message)
    {
        std::cout << message << std::endl;

        exit(0);
    }
}

#endif
