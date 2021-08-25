#pragma once
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <Windows.h>
#include <type_traits>
#include <concepts>
#include <functional>
#include "./hack/module.h"
#include <any>

struct VA_ARGS {};
struct NO_VA {};

template<typename T>
concept is_va = std::is_same_v<VA_ARGS, T>;

class IHookable {};


template<typename RT, typename VA> requires std::is_function_v<RT>
class hookableFunction : public IHookable {}; //Empty default template

template<typename RT, typename VA, typename ...A>
class hookableFunction<RT(A...), VA>
{
    void* immutable_ptr = 0;
    void* address_ptr = 0;

    static inline auto from_virtual(void* object, int index) -> void*
    {
        return (*static_cast<void***>(object))[index];
    }

    static inline auto from_module(LPCTSTR module, int offset) -> void*
    {
        return reinterpret_cast<void*>(reinterpret_cast<int>(GetModuleHandle(module)) + offset);
    }

public:
	explicit hookableFunction(void* address)
    {
        address_ptr = address;
        immutable_ptr = address;
    }
    
    hookableFunction(LPCTSTR module, int offset) : hookableFunction(from_module(module, offset)) {} /* module + offset */
    
    hookableFunction(void* object, int index) : hookableFunction(from_virtual(object, index)) {} /* object + from virtual function index */

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

    auto reset(void* address)
    {
        address_ptr = address;
        immutable_ptr = address;
    }
    auto reset() -> void { reset(nullptr); }
    auto reset(LPCTSTR module, int offset) { reset(from_module(module, offset)); }
    auto reset(void* object, int index) { reset(from_virtual(object, index)); }

    /// <summary>
    /// Check if function we pointed too has been modified (e.g. Hooked.)
    /// </summary>
    /// <returns>True if the function has been modifed.</returns>
    auto dirty() -> bool { return immutable_ptr != address_ptr; }

    /// <summary>
    /// This calls the original address. This means it will include your hook if you have set one.
    /// </summary>
    /// <param name="...args">The args to pass into the function.</param>
    /// <returns>The type specified in the hook.</returns>
    RT operator()(A... args) requires !is_va<VA> {
        if (!immutable_ptr) throw "Attempted to call null pointer.";
        return static_cast<RT(*)(A ...)>(immutable_ptr)(args...);
    }

    /// <summary>
    /// This calls the original address. This means it will include your hook if you have set one.
    /// </summary>
    /// <param name="...args">The args to pass into the function.</param>
    /// <param name="...va">The VA_ARGS to pass into the function.</param>
    /// <returns>The type specified in the hook.</returns>
    template<typename ...B> requires is_va<VA>
    RT operator()(A... args, B... va) {
        if (!immutable_ptr) throw std::exception("Attempted to call null pointer.");
        return static_cast<RT(*)(A ..., B ...)>(immutable_ptr)(args..., va...);
    }

    /// <summary>
    /// This calls the original function by address. This is what you want to call from your hook.
    /// </summary>
    /// <param name="...args">The args to pass into the function.</param>
    /// <returns>The type specified in the hook.</returns>
    RT original(A... args) requires !is_va<VA>
    {
        if (!address_ptr) throw std::exception("Attempted to call null pointer.");
        return static_cast<RT(*)(A ...)>(address_ptr)(args...);
    }

    /// <summary>
    /// This calls the original function by address. This is what you want to call from your hook.
    /// </summary>
    /// <param name="...args">The args to pass into the function.</param>
    /// <param name="...va">The VA_ARGS to pass into the function.</param>
    /// <returns>The type specified in the hook.</returns>
    template<typename ...B> requires is_va<VA>
    RT original(A... args, B... va)
    {
        if (!address_ptr) throw "Attempted to call null pointer.";
        return static_cast<RT(*)(A ..., B ...)>(address_ptr)(args..., va...);
    }
};



template<typename T>
using hookable  = hookableFunction<T, NO_VA>;

template<typename T>
using hookable_va = hookableFunction <T, VA_ARGS>;
