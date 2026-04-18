#ifndef ZENGINE_LOG_H
#define ZENGINE_LOG_H
#pragma once

#include <iostream>

namespace CommonWorker
{
	std::ostream& LogDebug();
	std::ostream& LogInfo();
	std::ostream& LogWarn();
	std::ostream& LogError();
}
#endif// ZENGINE_LOG_H
