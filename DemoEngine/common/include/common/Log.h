#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <locale>
#include <mutex>   

namespace CommonWorker
{
enum class LogLevel:uint8_t
{
    LOG_NONE,
    LOG_FATAL,
    LOG_ERROR,
    LOG_WARING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_TRACE,
    LOG_ALL,
};

enum PrinterType:uint8_t
{
    PRINTER_NOTE,
    PRINTER_WINDOWS,
    PRINTER_CONSULE,
    PRINTER_FILE,
};

class Printer
{
public:
    Printer()
        :m_type(PRINTER_NOTE)
    {}
    ~Printer(){}
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool auto_release() = 0;
    virtual int print(const std::string& msg) = 0;
    void SetType(int nType) {m_type = static_cast<uint8_t>(nType); }
    int GetType() { return m_type; }
protected:
	uint8_t m_type;
};
using PrintPtr = std::shared_ptr<Printer>;
using print_vec_t = std::vector<PrintPtr>;


//调试窗口输出
class WindowsDebugPrinter :public Printer
{
public:
    virtual bool open() 
    { 
        SetType(PRINTER_WINDOWS);
        return true; 
    }

    virtual void close() {}
    virtual bool auto_release() { return true; }
    virtual int print(const std::string& msg);
};

//平台输出
class ConsolePrinter :public Printer
{
public:
    ConsolePrinter(std::ostream& os);
    virtual bool open() 
    { 
        SetType(PRINTER_CONSULE);
        return true; 
    }

    virtual void close() {}
    virtual bool auto_release() { return true; }
    virtual int print(const std::string& msg);
private:
    std::ostream& m_os;
};

//文件输出
class FIlePrinter :public Printer
{
public:
    FIlePrinter(const char* filename, uint64_t maxsize, bool mulfile);
    ~FIlePrinter();
    virtual bool open();
    virtual void close();
    virtual bool auto_release(){ return true; }
    virtual int print(const std::string& msg);

private:
    bool m_mulfiles;
    std::locale m_locate;
    std::string m_filename;
    std::ofstream m_filecontent;
    uint32_t m_fileindex;
    uint64_t m_cursize;
    uint64_t m_maxsize;
};

class Log
{
public:
	explicit Log();

    virtual ~Log();

	static Log* Instance();

    //是否打开
    bool open(bool mt = true, LogLevel nlevel = LogLevel::LOG_TRACE);
    bool close();
    bool Isopen() { return m_isopen; }

    //日志级别
    LogLevel GetLogLevel() { return m_log_type; } 
    LogLevel SetLogLevel(LogLevel nType);

    // 添加输出器
    int AddPrint(const PrintPtr& app);

    // 增加3种类型的输出
    int AddFileprint(const char* szfilename, uint64_t len = 5 * 1024 * 1024, bool ismulfile = true);
    int AddConsoleprint(std::ostream &os = std::cout);
    int AddDebugprint();

    // 删除输出器
    bool RemovePrint(int index);

    //C++
    std::ostringstream& operator<<(const LogLevel ll);
    friend Log& operator<<(std::ostream &os, Log& loger);
	void write(const LogLevel II, const char* szMsg);
private:
    void unlock();

    void lock();

    //
    int do_log(LogLevel nlevel, const std::string& strbuffer);

    //
    int flush();
private:
    LogLevel m_log_type;
    LogLevel m_tmp;
    std::ostringstream m_buff;
    print_vec_t m_print_vec;
    bool m_isthread; // multiple thread mode
    bool m_isopen; // is open
    std::mutex mtx_;
};
}


// 宏定义
// 依靠下列预定义的可以很方便的添加或者删除日志输出,提高效率
#define LOGER_ERROR() std::cout
    //(*RenderWorker::Log::Instance()) << RenderWorker::LogLevel::LOG_ERROR  
#define LOGER_WARN() std::cout
    //(*RenderWorker::Log::Instance()) << RenderWorker::LogLevel::LOG_WARING 
#define LOGER_INFO() std::cout
    //(*RenderWorker::Log::Instance()) << RenderWorker::LogLevel::LOG_INFO 
#define LOGER_DEBUG() std::cout
    //(*RenderWorker::Log::Instance()) << RenderWorker::LogLevel::LOG_DEBUG 
#define LOGER_TRACE() std::cout
    //(*RenderWorker::Log::Instance()) << RenderWorker::LogLevel::LOG_TRACE 