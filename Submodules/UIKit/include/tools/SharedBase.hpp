//
//  SharedBase.hpp
//  NXKit
//
//  Created by Даниил Виноградов on 08.10.2022.
//

#pragma once

#include <memory>

namespace NXKit {

class enable_shared_from_this_pointer_holder {
public:
    mutable std::shared_ptr<void> __strong_this_ctor_;
};

template <class Base>
class enable_shared_from_this: public enable_shared_from_this_pointer_holder {
public:
//    mutable std::shared_ptr<Base> __strong_this_ctor_;
    mutable std::weak_ptr<Base> __weak_this_;

    enable_shared_from_this() {
        __strong_this_ctor_ = std::shared_ptr<Base>(static_cast<Base*>(this));
        __weak_this_ = std::static_pointer_cast<Base>(__strong_this_ctor_);
    }


protected:
    std::shared_ptr<Base> shared_from_this() {
        return std::shared_ptr<Base>(__weak_this_);
    }

    std::weak_ptr<Base> weak_from_this() {
        return __weak_this_;
    }

    template <class Derived>
    std::shared_ptr<Derived> shared_from_base() {
        return std::dynamic_pointer_cast<Derived>(this->shared_from_this());
    }

    template <class Derived>
    std::weak_ptr<Derived> weak_from_base() {
        if (!this->weak_from_this().expired()) {
            return std::dynamic_pointer_cast<Derived>(this->weak_from_this().lock());
        }
        return std::weak_ptr<Derived>();
    }
};

template<class _Tp, class ..._Args>
typename std::enable_if<std::is_base_of<enable_shared_from_this_pointer_holder, _Tp>::value, std::shared_ptr<_Tp>>::type new_shared(_Args&& ...__args) {
    std::allocator<_Tp> alloc;
    using traits_t = std::allocator_traits<decltype(alloc)>;
    auto obj = alloc.allocate(1);
    traits_t::construct(alloc, obj, std::forward<_Args>(__args)...);

    auto ptr = std::static_pointer_cast<_Tp>(obj->__strong_this_ctor_);
    obj->__strong_this_ctor_ = nullptr;
    return ptr;
}

template<class _Tp, class ..._Args>
typename std::enable_if<!std::is_base_of<enable_shared_from_this_pointer_holder, _Tp>::value, std::shared_ptr<_Tp>>::type new_shared(_Args&& ...__args) {
    auto res = std::make_shared<_Tp>(std::forward<_Args>(__args)...);
    return res;
}

}