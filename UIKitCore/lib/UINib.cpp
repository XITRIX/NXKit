//
//  UINib.cpp
//  SDLTest
//
//  Created by Даниил Виноградов on 14.03.2023.
//

#include <UINib.h>
#include <tools/Tools.hpp>
#include <utility>
#include <UIView.h>
#include <tinyxml2.h>

namespace NXKit {

std::unordered_map<std::string, XIBViewCreator> UINib::xibViewsRegister;

void UINib::registerXIB(const std::string& name, XIBViewCreator creator) {
    xibViewsRegister[name] = std::move(creator);
}

std::shared_ptr<UINib> UINib::fromPath(const std::string& path) {
    auto data = NXData::fromPath(path);
    if (data == nullptr) return nullptr;
    return fromData(data);
}

std::shared_ptr<UINib> UINib::fromRes(std::string path) {
    auto data = NXData::fromRes(path);
    if (data == nullptr) return nullptr;
    return fromData(data);
}

std::shared_ptr<UINib> UINib::fromString(std::string rawData) {
    auto res = new_shared<UINib>();
    res->_rawXml = std::move(rawData);
    return res;
}

std::shared_ptr<UINib> UINib::fromData(const std::shared_ptr<NXData>& data) {
    auto rawData = std::string((char*)data->data(), data->count());
    return fromString(rawData);
}

std::shared_ptr<UIView> UINib::instantiate(std::map<std::string, std::shared_ptr<UIView>>* idStorage) {
    // Load XML
    std::shared_ptr<tinyxml2::XMLDocument> document = new_shared<tinyxml2::XMLDocument>();
    tinyxml2::XMLError error        = document->Parse(_rawXml.c_str());

    if (error != tinyxml2::XMLError::XML_SUCCESS)
        fatal("Invalid XML when inflating XIB: error " + std::to_string(error));

    tinyxml2::XMLElement* element = document->RootElement();

    if (!element)
        fatal("Invalid XML: no element found");

    return UIView::instantiateFromXib(element, idStorage);
}

bool UINib::isEqual(const std::shared_ptr<UINib>& other) const {
    if (!other) return false;
    return _rawXml == other->_rawXml;
}

}
