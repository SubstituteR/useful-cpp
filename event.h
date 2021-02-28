#pragma once
#include <functional>

template<typename ...T>
class event
{
	std::vector <std::function<bool(T...)>> callbacks;
public:
	auto operator+=(std::function<bool(T...)> callback) -> event<T...>&
	{
		callbacks.emplace_back(callback);
		return *this;
	}
	auto reset() -> void
    {
	    callbacks.clear();
    }
	auto execute(T... data) -> void
	{
		for (auto& callback : callbacks)
			if (callback(data...)) /* true = handled... */
				break;
	}
};
