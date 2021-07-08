#pragma once
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <Windows.h>
#include <type_traits>
#include <concepts>


struct VA_ARGS {};
struct NO_VA {};

template<typename T>
concept is_va = std::is_same<VA_ARGS, T>::value;

class IHookable {};


template<typename RT, typename VA, typename ...A>
class hookableFunction : public IHookable
{
	void* immutable_ptr;
	void* address_ptr;
public:
	explicit hookableFunction(void* address)
    {
        address_ptr = address;
        immutable_ptr = address;
    }
    hookableFunction(LPCTSTR module, int offset) : hookableFunction(reinterpret_cast<LPVOID>(reinterpret_cast<int>(GetModuleHandle(module)) + offset)) {} /* module + offset */
    hookableFunction() : hookableFunction(nullptr) {} /* No Arg Constructor */

	[[nodiscard]] auto immutable() const -> void*
    {
        return immutable_ptr; /* Read-Only copy of the original pointer value */
    }

	[[nodiscard]] auto address() const -> void*
    {
        return address_ptr; /* Read-Only copy of the address */
    }

    [[nodiscard]] auto get() -> void**
    {
        return &address_ptr;
    }

    RT operator()(A... args) requires !is_va<VA> {
        if (!immutable_ptr) throw "Attempted to call null pointer.";
        return static_cast<RT(*)(A ...)>(immutable_ptr)(args...);
        /* This calls the original address at all times. IF you hook the func, this will include your hook. */
    }

    template<typename ...B> requires is_va<VA>
    RT operator()(A... args, B... va) {
        if (!immutable_ptr) throw std::exception("Attempted to call null pointer.");
        return static_cast<RT(*)(A ..., B ...)>(immutable_ptr)(args..., va...);
        /* This calls the original address at all times. IF you hook the func, this will include your hook. */
    }

    template<class = void> requires !is_va<VA>
    RT original(A... args)
    {
        if (!address_ptr) throw std::exception("Attempted to call null pointer.");
        return static_cast<RT(*)(A ...)>(address_ptr)(args...);
        /* This calls the updated pointer IF you hook the func. This is what you want to call from your hook. */
    }

    template<typename ...B> requires is_va<VA>
    RT original(A... args, B... va)
    {
        if (!address_ptr) throw "Attempted to call null pointer.";
        return static_cast<RT(*)(A ..., B ...)>(address_ptr)(args..., va...);
        /* This calls the updated pointer IF you hook the func. This is what you want to call from your hook. */
    }
};
