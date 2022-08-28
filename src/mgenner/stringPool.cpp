//
// Created by admin on 2022/7/8.
//
#include "stringPool.h"
namespace mgnr {
    const std::string emptyString="";
    const std::string &stringPool::stringPtr::value() const {
        if(parent_node) {
            return parent_node->value;
        }else{
            return emptyString;
        }
    }
}

