//
//  SharedBase.hpp
//  NXKit2
//
//  Created by Даниил Виноградов on 08.10.2022.
//

#pragma once

#include <memory>

template <class Base>
class enable_shared_from_base //: public std::enable_shared_from_this<Base>
{
public:
    mutable std::shared_ptr<Base> __strong_this_ctor_;
    mutable std::weak_ptr<Base> __weak_this_;

protected:
    std::shared_ptr<Base> shared_from_ctor() {
        if (!__strong_this_ctor_) {
            __strong_this_ctor_ = std::shared_ptr(this);
        }
        return __strong_this_ctor_;
    }

    std::shared_ptr<Base> shared_from_this() {
        return std::shared_ptr<Base>(__weak_this_);
    }

    std::weak_ptr<Base> weak_from_this() {
        return __weak_this_;
    }

    template <class Derived>
    std::shared_ptr<Derived> shared_from_base() {
        return std::static_pointer_cast<Derived>(this->shared_from_this());
    }
};

template<class _Tp, class ..._Args, class = std::_EnableIf<!std::is_array<_Tp>::value> >
std::shared_ptr<_Tp> make_shared(_Args&& ...__args) {
    std::allocator<_Tp> alloc;
    using traits_t = std::allocator_traits<decltype(alloc)>;
    auto obj = alloc.allocate(1);
    traits_t::construct(alloc, obj, std::forward<_Args>(__args)...);

    if (obj->__strong_this_ctor_) {
        auto copy = obj->__strong_this_ctor_;
        obj->__strong_this_ctor_ = nullptr;
        obj->__weak_this_ = copy;
        return std::static_pointer_cast<_Tp>(copy);
    }

    auto ptr = std::shared_ptr<_Tp>(obj);
    obj->__weak_this_ = ptr;
    return ptr;
}
