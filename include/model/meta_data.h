////
//// Created by qing on 25-3-6.
////
//
#ifndef HTTP_SERVER_META_DATA_H
#define HTTP_SERVER_META_DATA_H
#include <string>
#include <functional>
#include <typeindex>
#include <any>
template<typename Model>
struct FieldMeta {
    FieldMeta() : typeIndex(std::type_index(typeid(Model))) {}
    FieldMeta(std::type_index typeIndex,
              std::function<std::any(const Model &)> getter,
              std::function<void(Model &, std::any)> setter
    );

    std::type_index typeIndex;
    std::function<std::any(const Model &)> getter;
    std::function<void(Model &, std::any)> setter;
};

template<typename Model>
FieldMeta<Model>::FieldMeta(std::type_index index, std::function<std::any(const Model &)> getter,
                            std::function<void(Model &, std::any)> setter):
        typeIndex(index)
        ,getter(getter),setter(setter) {}

#endif //HTTP_SERVER_META_DATA_H
