#include "../System/Log.h"
#include "../Container/momery.h"
#include <cstdarg>
#include <tchar.h>

#include <boost/assert.hpp>
#define BUFFDATA_SIZE 512 //临时缓存大小
#define TIME_BUF_SZIE 128
const tchar* szLevelName[]
{
    T("NONE:"),
    T("FATAL:"),
    T("ERROR:"),
    T("WARNING:"),
    T("INFO:"),
    T("DEBUG:"),
    T("TRACE:"),
    T("ALL:"),
};

//vs ide 输出
int WindowsDebugPrinter::print(const tstring& msg)
{
    OutputDebugString(msg.c_str());
    return 0;
}

//控制台打印
ConsolePrinter::ConsolePrinter(tostream& os)
    :m_os(os)
{
}

int ConsolePrinter::print(const tstring& msg)
{
    m_os << msg << std::flush;
    return 0;
}

//文件打印
FIlePrinter::FIlePrinter(const tchar* filename, uint64_t maxsize, bool mulfile)
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
    m_filecontent.open(m_filename.c_str(), std::ios::app || std::ios::out);
    if (m_filecontent.is_open())
    {
        m_filecontent.imbue(m_locate);
        m_filecontent.seekp(0, std::ios::end);
        m_cursize = static_cast<uint64_t>(m_filecontent.tellp());
    }
    else
        return false;

    return true;
}

void FIlePrinter::close()
{
    if (m_filecontent.is_open())
    {
        m_filecontent.close();
    }
}

int FIlePrinter::print(const tstring& msg)
{
    if (m_filecontent.is_open())
    {
        if (m_mulfiles && m_cursize > m_maxsize)
        {
            m_filecontent.close();
			BOOST_ASSERT(m_filecontent.is_open());
            //create file print msg
            time_t t;
            time(&t);
            struct tm *tt = localtime(&t);
            tchar new_file[129] = {};
            _stprintf(new_file, _T("%04d%02d%02d_%02d%02d%02d_%d")
                , tt->tm_year + 1900, tt->tm_mon + 1, tt->tm_mday
                , tt->tm_hour, tt->tm_min, tt->tm_sec, ++m_fileindex);
            //path
            tstring strNewfile(m_filename);
            tstring::size_type dot = m_filename.find_last_of(_T("."));
            if (tstring::npos == dot)
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
				BOOST_ASSERT(1);
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
std::unique_ptr<Log> Log::m_InstanceLog = nullptr;
void Log::unlock()
{
    if (m_isthread)
        Port_ThreadUnLock(&m_locker);
}

void Log::lock()
{
    if (m_isthread)
        Port_ThreadLock(&m_locker);
}

Log::Log(Context* pContext)
	:IEntityEx(pContext), m_log_type(LOG_NONE), m_isthread(false), m_isopen(false)
{
	m_InstanceLog = std::unique_ptr<Log>(this);
}

Log::~Log()
{
    close();
}

Log* Log::Instance()
{
	if (nullptr != m_InstanceLog)
	{
		return m_InstanceLog.get();
	}

	return nullptr;
}

bool Log::OnInit()
{
	return true;
}

bool Log::OnShut()
{
	return true;
}

void Log::Update()
{

}

bool Log::open(bool mt /*= true*/, LogLevel nlevel /*= LOG_TRACE*/)
{
    if (Isopen()) 
        return false;
    m_isthread = mt;
    m_log_type = nlevel;

    if (m_isthread)
    {
        Port_CreateThread(&m_locker);
    }
    m_isopen = true;
    return true;
}

bool Log::close()
{
    if (!Isopen())
        return false;

    lock();
    for (auto it: m_print_vec)
       if ((*it).auto_release())
        {
            (*it).close();
            delete it;
        }
    m_print_vec.clear();
    unlock();

    if (m_isthread)
        Port_DestroyTread(&m_locker);
    m_isthread = false;
    m_isopen = false;
    return true;
}

LogLevel Log::SetLogLevel(LogLevel nType)
{
    LogLevel tmp = m_log_type;
    lock();
    m_log_type = nType;
    unlock();
    return tmp;
}

int Log::AddPrint(Printer *app)
{
    lock();
    m_print_vec.push_back(app);
    unlock();

    return static_cast<int>(m_print_vec.size());
}

int Log::AddFileprint(const char* szfilename, uint64_t len /*= 5 * 1024 * 1024*/, bool ismulfile /*= true*/)
{
    Printer *printer = NEW FIlePrinter(szfilename, len, ismulfile);
    if (!printer->open())
    {
        delete printer;
        return 0;
    }
    else
    {
        return AddPrint(printer);
    }

    return 0;
}

int Log::AddConsoleprint(tostream &os /*= tcout*/)
{
    Printer *printer = NEW ConsolePrinter(os);
    if (!printer->open())
    {
        delete printer;
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
    Printer *printer = NEW WindowsDebugPrinter();
    if (!printer->open())
    {
        delete printer;
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
    for (; it != m_print_vec.end(); ++it)
    {
        if (index == reinterpret_cast<int>(*it))
        {
            if ((*it)->auto_release())
            {
                (*it)->close();
                delete *it;
            }
            m_print_vec.erase(it);
            res = true;
            break;
        }
    }
    unlock();
    return res;
}

int Log::do_log(LogLevel nlevel, const tstring strbuffer)
{
    // get time string
    tchar tmString[TIME_BUF_SZIE + 1] = { 0 };
    __time64_t t;
    _time64(&t);
    _tcsftime(tmString, TIME_BUF_SZIE, LOG_TIME_FORMAT, _localtime64(&t));

    tostringstream o;
    o << tmString << T("-")  << szLevelName[nlevel] << T(":") << strbuffer;
    for (auto it : m_print_vec)
    {
        switch (nlevel)
        {
        case LOG_NONE:
            return 0;
        case LOG_INFO:
            if (PRINTER_CONSULE == (*it).GetType())
            {
                (*it).print(o.str());
            }
        case LOG_DEBUG:
            if (PRINTER_WINDOWS== (*it).GetType())
            {
                (*it).print(o.str());
            }
            break;
        default:
            (*it).print(o.str());
            break;
        }
    }
    return 0;
}

int Log::flush()
{
    int res = 0;
    if (m_tmp <= m_log_type)
        res = do_log(m_tmp, m_buff.str().c_str());
    m_tmp = m_log_type;
    m_buff.str(T(""));
    return res;
}

int Log::log(LogLevel nlevel, char* sformat, ...)
{
    va_list args;

    va_start(args, sformat);
    int size_1 = _vsctprintf(sformat, args);
    tchar *szBuffer = NEW tchar[size_1 + 1];
    if (nullptr == szBuffer)
        return 0;
    int res = _vstprintf(szBuffer, sformat, args);
    va_end(args);
    szBuffer[res] = 0;

    lock();
    do_log(nlevel, szBuffer);
    unlock();

    delete[] szBuffer;
    return 0;
}

tostringstream& Log::operator<<(const LogLevel ll)
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

Log& operator<<(tostream& os, Log& loger)
{
    loger.flush();
    loger.unlock();
    return loger;
}