// InputFactory.cpp
// KlayGE 输入引擎抽象工厂 实现文件
// Ver 3.1.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.1.0
// 初次建立 (2005.10.29)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <base/Input.h>
#include <base/InputFactory.h>


namespace RenderWorker
{
	InputFactory::InputFactory() noexcept = default;
	InputFactory::~InputFactory() noexcept = default;

	InputEngine& InputFactory::InputEngineInstance()
	{
		if (!ie_)
		{
			ie_ = this->DoMakeInputEngine();
		}

		return *ie_;
	}

	void InputFactory::Suspend()
	{
		if (ie_)
		{
			ie_->Suspend();
		}
		this->DoSuspend();
	}

	void InputFactory::Resume()
	{
		this->DoResume();
		if (ie_)
		{
			ie_->Resume();
		}
	}
}