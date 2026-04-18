
#include <common/CustomizedStreamBuf.h>
#include <common/Log.h>
#include <common/util.h>
#include <cstdarg>
#include <cstdio>
#include <iostream>

#ifdef ZENGINE_PLATFORM_ANDROID
    #include <android/log.h>
    #include <cstring>
#else
    #include <fstream>
#endif


namespace
{
using namespace CommonWorker;

#ifdef ZENGINE_PLATFORM_ANDROID
	class AndroidLogStreamCallback
	{
		ZENGINE_NONCOPYABLE(AndroidLogStreamCallback);

	public:
		explicit AndroidLogStreamCallback(int prio)
			: prio_(prio)
		{
		}
		AndroidLogStreamCallback(AndroidLogStreamCallback&& rhs)
			: prio_(rhs.prio_)
		{
		}

		std::streambuf::int_type operator()(void const * buff, std::streamsize count)
		{
			auto tmp = MakeUniquePtr<char[]>(count + 1);
			std::memcpy(tmp.get(), buff, count);
			tmp[count] = 0;
			__android_log_write(prio_, "KlayGE", tmp.get());
			return static_cast<std::streambuf::int_type>(count);
		}

	private:
		int prio_;
	};

	template <int PRIO>
	std::ostream& AndroidLog()
	{
		static CallbackOutputStreamBuf<AndroidLogStreamCallback> log_stream_buff((AndroidLogStreamCallback(PRIO)));
		static std::ostream log_stream(&log_stream_buff);
		return log_stream;
	}
#else
	class MultiOStreamsCallback
	{
		ZENGINE_NONCOPYABLE(MultiOStreamsCallback);

	public:
		explicit MultiOStreamsCallback(std::span<std::ostream*> oss)
			: oss_(oss)
		{
		}
		MultiOStreamsCallback(MultiOStreamsCallback&& rhs) noexcept
			: oss_(std::move(rhs.oss_))
		{
		}

		std::streambuf::int_type operator()(void const * buff, std::streamsize count)
		{
			for (auto& os : oss_)
			{
				os->write(static_cast<char const *>(buff), count);
			}
			return static_cast<std::streambuf::int_type>(count);
		}

	private:
		std::span<std::ostream*> oss_;
	};

	std::ostream& Log()
	{
#ifdef ZENGINE_DEBUG
		static std::ofstream log_file("KlayGE.log");
#endif

		static std::ostream* oss[] =
		{
#ifdef ZENGINE_DEBUG
			&log_file,
#endif
			&std::clog
		};
		static CallbackOutputStreamBuf<MultiOStreamsCallback> log_stream_buff((MultiOStreamsCallback(oss)));
		static std::ostream log_stream(&log_stream_buff);
		return log_stream;
	}
#endif

#ifndef ZENGINE_DEBUG
	class EmptyOStreamsCallback
	{
		ZENGINE_NONCOPYABLE(EmptyOStreamsCallback);

	public:
		EmptyOStreamsCallback()
		{
		}
		EmptyOStreamsCallback([[maybe_unused]] EmptyOStreamsCallback&& rhs) noexcept
		{
		}

		std::streambuf::int_type operator()([[maybe_unused]] void const * buff, std::streamsize count)
		{
			return static_cast<std::streambuf::int_type>(count);
		}
	};

	std::ostream& EmptyLog()
	{
		static CallbackOutputStreamBuf<EmptyOStreamsCallback> empty_stream_buff((EmptyOStreamsCallback()));
		static std::ostream empty_stream(&empty_stream_buff);
		return empty_stream;
	}
#endif
}

namespace CommonWorker
{
	std::ostream& LogDebug()
	{
#ifdef ZENGINE_DEBUG
#ifdef ZENGINE_PLATFORM_ANDROID
		return AndroidLog<ANDROID_LOG_DEBUG>();
#else
		return Log() << "(DEBUG) KlayGE: ";
#endif
#else
		return EmptyLog();
#endif
	}

	std::ostream& LogInfo()
	{
#ifdef ZENGINE_PLATFORM_ANDROID
		return AndroidLog<ANDROID_LOG_INFO>();
#else
		return Log() << "(INFO) KlayGE: ";
#endif
	}

	std::ostream& LogWarn()
	{
#ifdef ZENGINE_PLATFORM_ANDROID
		return AndroidLog<ANDROID_LOG_WARN>();
#else
		return Log() << "(WARN) KlayGE: ";
#endif
	}

	std::ostream& LogError()
	{
#ifdef ZENGINE_PLATFORM_ANDROID
		return AndroidLog<ANDROID_LOG_ERROR>();
#else
		return Log() << "(ERROR) KlayGE: ";
#endif
    }
}