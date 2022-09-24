#pragma once

#include <stdexcept>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

namespace Engine
{
	class IBaseInterface
	{
	protected:
		IBaseInterface& operator=(const IBaseInterface&) = delete;
		virtual ~IBaseInterface() = default;
	};
}
