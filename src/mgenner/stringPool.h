//
// Created by admin on 2022/7/8.
//

#ifndef MIDILIB_STRINGPOOL_H
#define MIDILIB_STRINGPOOL_H
#include <string.h>
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
namespace mgnr {
struct stringPool {
    struct stringNode {
        int count;
        std::string value;
        stringPool* parent_pool = nullptr;
    };

    struct stringPtr {
        stringNode* parent_node = nullptr;

        inline stringPtr() {
            parent_node = nullptr;
        }

        inline stringPtr(stringPool* parent_pool, const std::string& value) {
            if (value.empty()) {
                parent_node = nullptr;
                return;
            }
            //查找
            auto it = parent_pool->indexer.find(value);
            if (it == parent_pool->indexer.end()) {
                //创建
                std::unique_ptr<stringNode> p(new stringNode);
                p->count = 1;
                p->parent_pool = parent_pool;
                p->value = value;
                this->parent_node = p.get();
                parent_pool->indexer[value] = std::move(p);
            } else {
                //计数
                ++it->second->count;
                this->parent_node = it->second.get();
            }
        }

        inline stringPtr(const stringPtr& c) {
            parent_node = c.parent_node;
            if (parent_node) {
                ++parent_node->count;
            }
        }

        inline const stringPtr& operator=(const stringPtr& c) {
            drop();
            parent_node = c.parent_node;
            if (parent_node) {
                ++parent_node->count;
            }
            return *this;
        }

        const std::string& value() const;

        inline bool operator<(const stringPtr& c) const {
            return this->value() < c.value();
        }

        inline bool operator>(const stringPtr& c) const {
            return this->value() > c.value();
        }

        inline bool operator==(const stringPtr& c) const {
            return this->value() == c.value();
        }

        inline bool operator!=(const stringPtr& c) const {
            return this->value() != c.value();
        }

        inline bool operator==(const std::string& c) const {
            return this->value() == c;
        }

        inline bool operator!=(const std::string& c) const {
            return this->value() != c;
        }

        inline bool empty() const {
            return this->value().empty();
        }

        inline const char& at(int i) const {
            return this->value().at(i);
        }

        inline const char& operator[](int i) const {
            return this->value().at(i);
        }

        inline const char* c_str() const {
            return this->value().c_str();
        }

        inline size_t size() const {
            return this->value().size();
        }

        inline void clear() {
            drop();
        }

        inline void drop() {
            if (parent_node) {
                auto pp = parent_node->parent_pool;
                --parent_node->count;
                if (parent_node->count <= 0) {
                    std::string val(value());
                    //::__android_log_print(ANDROID_LOG_INFO,
                    //                      "stringPool",
                    //                      "release '%s':%d",
                    //                      val.c_str(),parent_node->count);
                    pp->indexer.erase(val);
                }
                parent_node = nullptr;
            }
        }

        inline ~stringPtr() {
            drop();
        }
    };

    std::unordered_map<std::string, std::unique_ptr<stringNode> > indexer;

    inline stringPtr create(const std::string& value) {
        return stringPtr(this, value);
    }

    inline void getStrings(std::vector<stringNode*>& res, bool sort = true) {
        res.clear();
        for (auto& it : indexer) {
            res.push_back(it.second.get());
        }
        if (sort) {
            std::sort(res.begin(), res.end(),
                      [](const stringNode* x, const stringNode* y) {
                          return x->count > y->count;
                      });
        }
    }
};
}  // namespace mgnr
#endif  //MIDILIB_STRINGPOOL_H
