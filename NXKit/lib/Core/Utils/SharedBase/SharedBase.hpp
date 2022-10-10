//
//  SharedBase.hpp
//  NXKit2
//
//  Created by Даниил Виноградов on 08.10.2022.
//

#pragma once

#include <memory>

namespace NXKit {

template <class Base>
class enable_shared_from_base //: public std::enable_shared_from_this<Base>
{
public:
    mutable std::shared_ptr<Base> __strong_this_ctor_;
    mutable std::weak_ptr<Base> __weak_this_;

    enable_shared_from_base() {
        __strong_this_ctor_ = std::shared_ptr<Base>(static_cast<Base*>(this));
        __weak_this_ = __strong_this_ctor_;
    }
//    virtual ~enable_shared_from_base() {}

//protected:
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

template<class _Tp, class ..._Args, class = std::_EnableIf<std::is_polymorphic<_Tp>::value>, class = std::_EnableIf<!std::is_array<_Tp>::value> >
std::shared_ptr<_Tp> make_shared(_Args&& ...__args) {
    std::allocator<_Tp> alloc;
    using traits_t = std::allocator_traits<decltype(alloc)>;
    auto obj = alloc.allocate(1);
    traits_t::construct(alloc, obj, std::forward<_Args>(__args)...);

//    auto esfb = dynamic_cast<enable_shared_from_base<_Tp>*>(obj);
//    if (esfb) {
    auto ptr = std::static_pointer_cast<_Tp>(obj->__strong_this_ctor_);
    obj->__strong_this_ctor_ = nullptr;
    return ptr;
//    }
//
//    return std::shared_ptr<_Tp>(obj);
}

}
