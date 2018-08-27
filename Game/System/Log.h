#ifndef STX_LOG_H
#define STX_LOG_H
#pragma once

#include "../Core/entity/Entity.h"
#include "../Core/Context.h"
#include "../Container/var_type.h"
#include "../Platform/system_table.h"

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <locale>

enum LogLevel
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

enum PrinterType
{
    PRINTER_NOTE,
    PRINTER_WINDOWS,
    PRINTER_CONSULE,
    PRINTER_FILE,
};

// �궨��
// ��������Ԥ����Ŀ��Ժܷ������ӻ���ɾ����־���,���Ч��
#define  LOGER_USING(loger) extern LogManager g_Loger;
#define  LOGER_CLOG(loger, ll, fmt, ...) \
do \
{ \
    if ((ll) <= (g_Loger).GetLogLevel()) \
    { \
        (g_Loger).log((ll), fmt, __VA_ARGS__);\
    } \
} while (0)
//c++ style
#define LOGER_ERROR(msg) \
    (Log::Instance()) << LogLevel::LOG_ERROR << ##msg## << (g_Loger)
#define LOGER_WARN(msg)\
    (Log::Instance()) << LogLevel::LOG_WARING << ##msg## << (g_Loger)
#define LOGER_INFO(msg)\
    (Log::Instance()) << LogLevel::LOG_INFO << ##msg## << (g_Loger)
#define LOGER_DEBUG(msg)\
    (Log::Instance()) << LogLevel::LOG_DEBUG << ##msg## << (g_Loger)
#define LOGER_TRACE(msg)\
    (Log::Instance()) << LogLevel::LOG_TRACE << ##msg## << (g_Loger)
//c style
#define CLOGER_ERROR(fmt, ...) \
    LOGER_CLOG((g_Loger),  LogLevel::LOG_ERROR, (fmt), __VA_ARGS__)
#define CLOGER_WARN(fmt, ...)\
    LOGER_CLOG((g_Loger), LogLevel::LOG_WARING, (fmt), __VA_ARGS__)
#define CLOGER_INFO(fmt, ...) \
    LOGER_CLOG((g_Loger), LogLevel::LOG_INFO, (fmt), __VA_ARGS__)
#define CLOGER_DEBUG(fmt, ...) \
    LOGER_CLOG((g_Loger), LogLevel::LOG_DEBUG, (fmt), __VA_ARGS__)
#define CLOGER_TRACE(fmt, ...) \
    LOGER_CLOG((g_Loger), LogLevel::LOG_TRACE, (fmt), __VA_ARGS__)

//

#if defined(_UNICODE) || defined(UNICODE) 
    typedef wchar_t tchar;
    typedef std::wstring tstring;
    typedef std::wostringstream tostringstream;
    typedef std::wostream tostream;
    typedef std::wofstream tofstream;
    #define tcout std::wcout   
    #define  LOG_TIME_FORMAT L"%Y-%m-%d %H:%M:%S"
    #define T(sz) L##sz
#else
    typedef char tchar;//�ַ�
    typedef std::string tstring;//�ַ���
    typedef std::ostringstream tostringstream;//�ַ���
    typedef std::ostream tostream;//�ļ�������
    typedef std::ofstream tofstream;//�ļ��������
    #define tcout std::cout
    #define  LOG_TIME_FORMAT "%Y-%m-%d %H:%M:%S"
    #define T(sz) sz
#endif//

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
    virtual int print(const tstring& msg) = 0;
    void SetType(int nType) {m_type = nType; }
    int GetType() { return m_type; }
protected:
	zint8_t m_type;
};
typedef std::vector<Printer*> print_vec_t;

//���Դ������
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
    virtual int print(const tstring& msg);
};

//ƽ̨���
class ConsolePrinter :public Printer
{
public:
    ConsolePrinter(tostream& os);
    virtual bool open() 
    { 
        SetType(PRINTER_CONSULE);
        return true; 
    }

    virtual void close() {}
    virtual bool auto_release() { return true; }
    virtual int print(const tstring& msg);
private:
    tostream& m_os;
};

//�ļ����
class FIlePrinter :public Printer
{
public:
    FIlePrinter(const tchar* filename, uint64_t maxsize, bool mulfile);
    ~FIlePrinter();
    virtual bool open();
    virtual void close();
    virtual bool auto_release(){ return true; }
    virtual int print(const tstring& msg);

private:
    bool m_mulfiles;
    std::locale m_locate;
    tstring m_filename;
    tofstream m_filecontent;
    uint32_t m_fileindex;
    uint64_t m_cursize;
    uint64_t m_maxsize;
};

class Log :public IEntityEx
{
public:
	STX_ENTITY(Log, IEntityEx);

	explicit Log(Context* pContext);

    virtual ~Log();

	static Log* Instance();

	virtual bool OnInit() override;

	virtual bool OnShut() override;

	virtual void Update() override;

    //�Ƿ��
    bool open(bool mt = true, LogLevel nlevel = LOG_TRACE);
    bool close();
    bool Isopen() { return m_isopen; }

    //��־����
    LogLevel GetLogLevel() { return m_log_type; }
    LogLevel SetLogLevel(LogLevel nType);

    // ��������
    int AddPrint(Printer *app);

    // ����3�����͵����
    int AddFileprint(const char* szfilename, uint64_t len = 5 * 1024 * 1024, bool ismulfile = true);
    int AddConsoleprint(tostream &os = tcout);
    int AddDebugprint();

    // ɾ�������
    bool RemovePrint(int index);

    // c style ���ø�ʽ,�� sprintf һ�� log(ll_info, _T("ip address:%s, port:%d\n"), ipAddress, port);
    int log(LogLevel nlevel, char* sformat, ...);

    //C++
    tostringstream& operator<<(const LogLevel ll);
    friend Log& operator<<(tostream &os, Log& loger);
	void write(const LogLevel II, const char* szMsg);
private:
    void unlock();

    void lock();

    //
    int do_log(LogLevel nlevel, const tstring strbuffer);

    //
    int flush();
private:
	static std::unique_ptr<Log> m_InstanceLog;
    LogLevel m_log_type;
    LogLevel m_tmp;
    tostringstream m_buff;
    print_vec_t m_print_vec;
    bool m_isthread; // multiple thread mode
    bool m_isopen; // is open
    pthread_t m_locker;
};
#endif//STX_LOG_H