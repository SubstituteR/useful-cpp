#pragma once
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <Windows.h>
#include <type_traits>
struct VA_ARGS {};
struct NO_VA {};
template<typename RT, typename VA, typename ...A>
class hookableFunction
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

	auto operator&() -> void**  // NOLINT(google-runtime-operator)
	{
        return &address_ptr;
    }

    template<class = typename std::enable_if<std::is_same<VA, NO_VA>::value>::type>
    RT operator()(A... args) {
        if (!immutable_ptr) throw "Attempted to call null pointer.";
        return static_cast<RT(*)(A ...)>(immutable_ptr)(args...);
        /* This calls the original address at all times. IF you hook the func, this will include your hook. */
    }

    template<typename ...B, class = typename std::enable_if<std::is_same<VA, VA_ARGS>::value>::type>
    RT operator()(A... args, B... va) {
        if (!immutable_ptr) throw std::exception("Attempted to call null pointer.");
        return static_cast<RT(*)(A ..., B ...)>(immutable_ptr)(args..., va...);
        /* This calls the original address at all times. IF you hook the func, this will include your hook. */
    }

    template<class = typename std::enable_if<std::is_same<VA, NO_VA>::value>::type>
    RT original(A... args)
    {
        if (!address_ptr) throw std::exception("Attempted to call null pointer.");
        return static_cast<RT(*)(A ...)>(address_ptr)(args...);
        /* This calls the updated pointer IF you hook the func. This is what you want to call from your hook. */
    }

    template<typename ...B, class = typename std::enable_if<std::is_same<VA, VA_ARGS>::value>::type>
    RT original(A... args, B... va)
    {
        if (!address_ptr) throw "Attempted to call null pointer.";
        return static_cast<RT(*)(A ..., B ...)>(address_ptr)(args..., va...);
        /* This calls the updated pointer IF you hook the func. This is what you want to call from your hook. */
    }
};

template<typename T>
class processVariable
{
    void* immutable_ptr;
public:
    processVariable(void* address)
    {
        this->immutable_ptr = address;
    }
    processVariable(const LPCTSTR module, const int offset) : processVariable(reinterpret_cast<void*>(reinterpret_cast<int>(GetModuleHandle(module)) + offset)) {} /* module + offset */
    processVariable() : processVariable(nullptr) {} /* No Arg Constructor */

    auto operator*() -> T
    {
        if (!immutable_ptr)
            throw std::exception("Attempting to read null pointer.");
        return *static_cast<T*>(immutable_ptr);
    }

    auto operator&() -> T*  // NOLINT(google-runtime-operator)
    {
        return static_cast<T*>(immutable_ptr);
    }

    auto operator()() -> T
    {
        return static_cast<T>(immutable_ptr);
    }
};
#ifndef __clang_analyzer__
#define HOOKABLE(name, rt, ...) \
using name = hookableFunction<rt, NO_VA, __VA_ARGS__>;
#define HOOKABLE_VA(name, rt, ...) \
using name = hookableFunction<rt, VA_ARGS, __VA_ARGS__>;
#endif

template<typename ReturnType, typename ...Args>
constexpr auto hookable() -> hookableFunction<ReturnType, NO_VA, Args...>
{
    return hookableFunction <ReturnType, NO_VA, Args...>{};
}
template<typename ReturnType, typename ...Args>
constexpr auto hookable_va() -> hookableFunction<ReturnType, VA_ARGS, Args...>
{
    return hookableFunction <ReturnType, NO_VA, Args...>{};
}
