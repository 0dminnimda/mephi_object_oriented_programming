#pragma once

#ifndef DEEPCOPY_H
#define DEEPCOPY_H

#include <memory>

template <typename T>
std::shared_ptr<T> deepcopy_shared(const T &self) {
    std::shared_ptr<T> value = std::make_shared<T>(self);
    self.deepcopy_to(*value);
    return value;
}

template <typename OutT, typename InT>
std::shared_ptr<OutT> deepcopy_shared(const InT &self) {
    return static_pointer_cast<OutT>(deepcopy_shared<InT>(self));
}

template <typename T>
T deepcopy(const T &self) {
    T value(self);
    self.deepcopy_to(value);
    return value;
}

#define DeepCopy(T) void deepcopy_to(T &other) const
#define DeepCopyCls(T) void T::deepcopy_to(T &other) const

#endif  // DEEPCOPY_H
