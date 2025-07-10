#include <common/Log.h>
#include <common/util.h>
#include <common/instance.h>
#include <cstdarg>
#include <format>
#include <utility>
#include <chrono>

#ifdef ZENGINE_PLATFORM_WINDOWS
    #include <windows.h>
#endif

namespace CommonWorker
{

#define BUFFDATA_SIZE 512 //临时缓存大小
#define TIME_BUF_SZIE 128
const char* szLevelName[]
{
    ("NONE:"),
    ("FATAL:"),
    ("ERROR:"),
    ("WARNING:"),
    ("INFO:"),
    ("DEBUG:"),
    ("TRACE:"),
    ("ALL:"),
};

//vs ide 输出
int WindowsDebugPrinter::print(const std::string& msg)
{
 #ifdef ZENGINE_PLATFORM_WINDOWS
     OutputDebugStringA(msg.c_str());
#endif
    //std::cout << msg << std::endl;
    return 0;
}

//控制台打印
ConsolePrinter::ConsolePrinter(std::ostream& os)
    :m_os(os)
{
}

int ConsolePrinter::print(const std::string& msg)
{
    m_os << msg << std::flush;
    return 0;
}

//文件打印
FIlePrinter::FIlePrinter(const char* filename, uint64_t maxsize, bool mulfile)
    :m_mulfiles(mulfile), m_locate(""), m_filename(filename)
    , m_fileindex(0), m_cursize(0), m_maxsize(maxsize)
{
}

FIlePrinter::~FIlePrinter()
{
    close();
}

bool FIlePrinter::open()
{
    // wofstream 输出时,接受wchar_t字符串, 输出时根据环境把宽字符串转化后再写文件. 
    // 如果没有设置, 那么是C现场, 无法输出中文.
    // 所以必须 imbue 当前系统的环境.
    SetType(PRINTER_FILE);
    try 
    {
        m_filecontent.open(m_filename.c_str(), std::ios::app || std::ios::out);
        if (m_filecontent.is_open())
        {
            m_filecontent.imbue(m_locate);
            m_filecontent.seekp(0, std::ios::end);
            m_cursize = static_cast<uint64_t>(m_filecontent.tellp());
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Failed to open file '" << m_filename << "'. Exception: " << e.what() << std::endl;
    }

    return true;
}

void FIlePrinter::close()
{
    if (m_filecontent.is_open())
    {
        m_filecontent.close();
    }
}

int FIlePrinter::print(const std::string& msg)
{
    if (m_filecontent.is_open())
    {
        if (m_mulfiles && m_cursize > m_maxsize)
        {
            m_filecontent.close();
			COMMON_ASSERT(m_filecontent.is_open());
            //create file print msg
            auto now = std::chrono::system_clock::now();
            auto local_time = std::chrono::zoned_time(std::chrono::current_zone(), now);
            std::string new_file = std::format(
                "{0:%Y%m%d_%H%M%S}_{1}", 
                local_time, 
                ++m_fileindex);
            //path
            std::string strNewfile(m_filename);
            std::string::size_type dot = m_filename.find_last_of(".");
            if (std::string::npos == dot)
            {
                strNewfile += m_filename;
            }
            else
            {
                strNewfile.insert(dot, strNewfile);
            }
            m_filename.swap(strNewfile);
            //open new file
            m_filecontent.open(m_filename.c_str(), std::ios::app || std::ios::out);
            if (m_filecontent.is_open())
            {
                m_filecontent.imbue(m_locate);
                m_cursize = 0;
            }
            else
				COMMON_ASSERT(1);
        }

        if (m_filecontent.is_open())
        {
            m_filecontent << msg << std::flush;
            m_cursize = static_cast<uint64_t>(m_filecontent.tellp());
        }
    }

    return 0;
}



/*Log管理*****************************************************************/
void Log::unlock()
{
    mtx_.unlock();
}

void Log::lock()
{
    mtx_.lock();
}

Log::Log()
	:m_log_type(LogLevel::LOG_NONE), m_isthread(false), m_isopen(false)
{
    auto now = std::chrono::system_clock::now();
    auto local_time = std::chrono::zoned_time(std::chrono::current_zone(), now);
    std::string tmString = std::format("{:%Y-%m-%d-%H:%M:%S}", local_time);
    std::string strFileName = tmString;
    strFileName += "_0_log.log";

    AddFileprint(strFileName.c_str(), 5 * 1024 * 1024, true);
    AddConsoleprint();
}

Log::~Log()
{
    close();
}

Log* Log::Instance()
{
    return Singleton<Log>::Instance();
}


bool Log::open(bool mt /*= true*/, LogLevel nlevel /*= LOG_TRACE*/)
{
    if (Isopen()) 
        return false;
    m_isthread = mt;
    m_log_type = nlevel;
    m_isopen = true;
    return true;
}

bool Log::close()
{
    if (!Isopen())
        return false;

    
    lock();
    for (auto it: m_print_vec)
    {
        if ((*it).auto_release())
        {
            (*it).close();
        }
    }
    m_print_vec.clear();
    unlock();
    m_isthread = false;
    m_isopen = false;
    return true;
}

LogLevel Log::SetLogLevel(LogLevel nType)
{
    LogLevel tmp = m_log_type;
    std::mutex mtx;
    lock();
    m_log_type = nType;
    unlock();
    return tmp;
}

int Log::AddPrint(const PrintPtr& app)
{
    lock();
    m_print_vec.push_back(app);
    unlock();

    return static_cast<int>(m_print_vec.size());
}

int Log::AddFileprint(const char* szfilename, uint64_t len /*= 5 * 1024 * 1024*/, bool ismulfile /*= true*/)
{
    auto printer = MakeSharedPtr<FIlePrinter>(szfilename, len, ismulfile);
    if (!printer->open())
    {
        return 0;
    }
    else
    {
        return AddPrint(printer);
    }

    return 0;
}

int Log::AddConsoleprint(std::ostream &os /*= tcout*/)
{
    auto printer = MakeSharedPtr<ConsolePrinter>(os);
    if (!printer->open())
    {
        return 0;
    } 
    else
    {
        return AddPrint(printer);
    }

    return 0;
}

int Log::AddDebugprint()
{
    auto printer =  MakeSharedPtr<WindowsDebugPrinter>();
    if (!printer->open())
    {
        return 0;
    }
    else
    {
        return AddPrint(printer);
    }

    return 0;
}

bool Log::RemovePrint(int index)
{
    bool res = false;
    lock();
    print_vec_t::iterator it = m_print_vec.begin();
    for (std::size_t i = 0; it != m_print_vec.end(); ++it, ++i)
    {
        if (index == i)
        {
            if ((*it)->auto_release())
            {
                (*it)->close();
            }
            m_print_vec.erase(it);
            res = true;
            break;
        }
    }
    unlock();
    return res;
}

int Log::do_log(LogLevel nlevel, const std::string& strbuffer)
{
    // get time string
    auto now = std::chrono::system_clock::now();
    auto local_time = std::chrono::zoned_time(std::chrono::current_zone(), now);
    std::string tmString = std::format("{:%Y-%m-%d-%H:%M:%S}", local_time);

    std::ostringstream o;
    o << tmString << ("-")  << szLevelName[static_cast<int>(nlevel)] << (":") << strbuffer << std::endl;
    for (auto it : m_print_vec)
    {
        (*it).print(o.str());
    }
    return 0;
}

int Log::flush()
{
    int res = 0;
    res = do_log(m_tmp, m_buff.str().c_str());
    m_buff.str("");
    return res;
}

std::ostringstream& Log::operator<<(const LogLevel ll)
{
    lock();
    m_tmp = ll;
    return m_buff;
}

void Log::write(const LogLevel II, const char* szMsg)
{
	lock();
	m_tmp = II;
	m_buff << szMsg;
}

Log& operator<<(std::ostream& os, Log& loger)
{
    loger.flush();
    loger.unlock();
    return loger;
}
}