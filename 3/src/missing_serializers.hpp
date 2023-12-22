#pragma once

#ifndef MISSING_SERIALIZERS_H
#define MISSING_SERIALIZERS_H

#include <atomic>
#include <SFML/System.hpp>
#include <boost/serialization/access.hpp>

namespace boost {
    namespace serialization {
        template <class Archive, typename T>
        void serialize(Archive &ar, sf::Vector2<T> &a, const unsigned int version) {
            ar &a.x;
            ar &a.y;
        }

    }  // namespace serialization
}  // namespace boost

namespace boost {
    namespace serialization {
        template <class Archive, typename T>
        void save(Archive &ar, const std::atomic<T> &obj, const unsigned int version) {
            ar &obj.load();
        }

        template <class Archive, typename T>
        void load(Archive &ar, std::atomic<T> &obj, const unsigned int version) {
            T value;
            ar &value;
            obj.store(value);
        }

        template <class Archive, typename T>
        void serialize(Archive &ar, std::atomic<T> &t, const unsigned int file_version) {
            split_free(ar, t, file_version);
        }
    }  // namespace serialization
}  // namespace boost

namespace boost {
    namespace serialization {
        template <class Archive>
        void save(Archive &ar, const sf::Time &obj, const unsigned int version) {
            ar &obj.asMicroseconds();
        }

        template <class Archive>
        void load(Archive &ar, sf::Time &obj, const unsigned int version) {
            sf::Int64 value;
            ar &value;
            obj = sf::microseconds(value);
        }

        template <class Archive>
        void serialize(Archive &ar, sf::Time &t, const unsigned int file_version) {
            split_free(ar, t, file_version);
        }
    }  // namespace serialization
}  // namespace boost

#endif  // MISSING_SERIALIZERS_H
