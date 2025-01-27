//
//  UINib.hpp
//  SDLTest
//
//  Created by Даниил Виноградов on 14.03.2023.
//

#pragma once

#include <NXData.h>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <unordered_map>

namespace NXKit {

#define REGISTER_NIB(n) UINib::registerXIB(#n, n::init);

class UIView;
typedef std::function<std::shared_ptr<UIView>()> XIBViewCreator;

class UINib {
public:
    static std::shared_ptr<UINib> fromPath(const std::string& path);
    static std::shared_ptr<UINib> fromString(std::string rawData);
    static std::shared_ptr<UINib> fromData(const std::shared_ptr<NXData>& data);

    static void registerXIB(const std::string& name, XIBViewCreator creator);

    std::shared_ptr<UIView> instantiate(std::map<std::string, std::shared_ptr<UIView>>* idStorage = nullptr);

    template<class T>
    std::shared_ptr<T> instantiate(std::map<std::string, std::shared_ptr<UIView>>* idStorage = nullptr) {
        return std::static_pointer_cast<T>(instantiate(idStorage));
    }

    [[nodiscard]] bool isEqual(const std::shared_ptr<UINib>& other) const;
private:
    std::string _rawXml;

    static std::unordered_map<std::string, XIBViewCreator> xibViewsRegister;

    friend class UIView;
};

}
