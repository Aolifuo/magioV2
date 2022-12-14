#pragma once

#include <cstring>
#include <type_traits>

namespace magio {

template<typename...Ts>
struct TypeList;

namespace util {

template<typename>
struct MemFnTraits;

template<typename T, typename U>
struct MemFnTraits<U (T::*)> {
    using FunctionType = U;
    using ClassType = T;
};

template<size_t, typename...>
struct AtImpl;

template<typename Target, typename...Rest>
struct AtImpl<0, Target, Rest...> {
    using type = Target;
};

template<size_t N>
struct AtImpl<N> {
    using type = void;
};

template<size_t N, typename First, typename...Rest>
struct AtImpl<N, First, Rest...>: AtImpl<N - 1, Rest...> {};

template<typename, typename>
struct EqImpl: std::false_type {};

template<typename ...Left, typename ...Right>
struct EqImpl<TypeList<Left...>, TypeList<Right...>>
    : std::conjunction<std::is_same<Left, Right>...> {};

template<typename>
struct IsFunctionWithoutModifier: std::false_type {};

template<typename Ret, typename...Args>
struct IsFunctionWithoutModifier<Ret(Args...)>: std::true_type {};

template<typename>
struct IsFunctionPointer: std::false_type {};

template<typename F>
requires std::is_function_v<F>
struct IsFunctionPointer<F *>: std::true_type {};

}

template<typename...Ts>
struct TypeList {
    template<size_t N>
    using At = typename util::AtImpl<N, Ts...>::type;
    template<typename ...Others>
    constexpr static bool Eq = util::EqImpl<TypeList<Ts...>, TypeList<Others...>>::value;
    constexpr static size_t Length = sizeof...(Ts);
};

// no volatile & &&
// support noexcept

template<typename Functor>
struct FunctorTraits
    : FunctorTraits<decltype(&Functor::operator())> {};

template<typename Ret, typename...Args>
struct FunctorTraits<Ret(Args...)> {
    using ReturnType = Ret;
    using Arguments = TypeList<Args...>;
    using FunctionType = Ret(Args...);
};

template<typename Ret, typename...Args>
struct FunctorTraits<Ret(Args...) noexcept>
    : FunctorTraits<Ret(Args...)> {};
    
template<typename Ret, typename...Args>
struct FunctorTraits<Ret (*)(Args...)>
    : FunctorTraits<Ret(Args...)> {};

template<typename Ret, typename...Args>
struct FunctorTraits<Ret (*)(Args...) noexcept>
    : FunctorTraits<Ret(Args...)> {};

template<typename Ret, typename Class, typename...Args>
struct FunctorTraits<Ret (Class::*)(Args...)>
    : FunctorTraits<Ret(Args...)> {};

template<typename Ret, typename Class, typename...Args>
struct FunctorTraits<Ret (Class::*)(Args...) noexcept>
    : FunctorTraits<Ret(Args...)> {};

template<typename Ret, typename Class, typename...Args>
struct FunctorTraits<Ret (Class::*)(Args...) const>
    : FunctorTraits<Ret(Args...)> {};

template<typename Ret, typename Class, typename...Args>
struct FunctorTraits<Ret (Class::*)(Args...) const noexcept>
    : FunctorTraits<Ret(Args...)> {};

template<typename Fn>
concept IsFunctor = requires(Fn fn) {
    typename FunctorTraits<Fn>::FunctionType;
};

template<typename Fn, typename Ret, typename...Args>
concept CheckFunction = 
    std::is_invocable_v<Fn, Args...> &&
    std::is_invocable_r_v<Ret, Fn, Args...>;

template<typename T>
requires std::is_member_function_pointer_v<T>
class MemFnBinder {
public:
    using ClassType = typename util::MemFnTraits<T>::ClassType;
    using FunctionType = typename util::MemFnTraits<T>::FunctionType;

    MemFnBinder(T mem_fn_ptr, ClassType* obj_ptr)
        : mfp_(mem_fn_ptr), obj_(obj_ptr) {}
    
    template<typename ...Args>
    requires std::is_invocable_v<T, ClassType*, Args...>
    auto operator()(Args&& ...args) {
        return (obj_->*mfp_)(std::forward<Args>(args)...);
    }

    operator bool() {
        return mfp_ != nullptr;
    }
private:
    T mfp_ = nullptr;
    ClassType* obj_ = nullptr;
};

}