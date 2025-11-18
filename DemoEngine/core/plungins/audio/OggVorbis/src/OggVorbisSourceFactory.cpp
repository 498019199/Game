// OggVorbisSourceFactory.cpp
// KlayGE Ogg vorbis audio data source implement file
// Ver 3.11.0
// Copyright(C) Minmin Gong, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// First release (2010.8.22)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////
#include <common/Util.h>
#include "OggVorbisSource.h"

namespace RenderWorker
{
	class OggVorbisAudioDataSourceFactory final : public AudioDataSourceFactory
	{
	public:
		std::wstring const & Name() const override
		{
			static std::wstring const name(L"OggVorbis Audio Data Source Factory");
			return name;
		}

	private:
		AudioDataSourcePtr MakeAudioDataSource() override
		{
			return CommonWorker::MakeSharedPtr<OggVorbisSource>();
		}

		virtual void DoSuspend() override
		{
			// TODO
		}
		virtual void DoResume() override
		{
			// TODO
		}
	};
}

extern "C"
{
	ZENGINE_CORE_API void MakeAudioDataSourceFactory(std::unique_ptr<RenderWorker::AudioDataSourceFactory>& ptr)
	{
		ptr = CommonWorker::MakeUniquePtr<RenderWorker::OggVorbisAudioDataSourceFactory>();
	}
}
