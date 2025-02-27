#pragma once

#include <map>
#include <utility>
#include <optional>
#include <tuple>

template <typename I, typename V>
class range_map {
public:
    std::map<I, std::pair<I, V>> map;

    void add(const std::pair<I, I>& range, const V& value) {
        if (range.second == 0) {
            return;
        }

        add_impl(normalize_range(range), value);
    }

    void remove(const std::pair<I, I>& range) {
        if (range.second == 0) {
            return;
        }

        remove_impl(normalize_range(range));
    }

    std::optional<V> get(const I& index) const {
        auto iterator = map.upper_bound(index);

        if (iterator != map.cbegin()) {
            const auto& [begin, pair] = *--iterator;
            const auto& [length, value] = pair;
            const auto end = begin + length;

            if (begin <= index && index < end) {
                return value;
            }
        }

        return std::nullopt;
    }

private:
    void add_impl(const std::pair<I, I>& range, const V& value) {
        const auto& [begin, length] = range;

        remove_impl(range);

        // TODO: Check success
        auto [iterator, success] = map.insert_or_assign(begin, std::make_pair(length, value));

        if (iterator != map.cbegin()) {
            iterator = try_merge(std::prev(iterator), iterator);
        }

        if (std::next(iterator) != map.cend()) {
            try_merge(iterator, std::next(iterator));
        }
    }

    void remove_impl(const std::pair<I, I>& range) {
        const auto& [begin, length] = range;
        const auto end = begin + length;

        auto current = map.upper_bound(begin);

        if (current != map.cbegin()) {
            --current;
        }

        const auto last = map.lower_bound(end);

        while (current != last) {
            const auto next = std::next(current);
            const auto& [current_begin, current_pair] = *current;
            const auto& [current_length, current_value] = current_pair;
            const auto current_end = current_begin + current_length;

            if (current_end > begin) {
                if (current_end > end) {
                    map[end] = std::make_pair(current_end - end, current_value);
                }

                if (current_begin >= begin) {
                    map.erase(current);
                } else {
                    map[current_begin].first = begin - current_begin;
                }
            }

            current = next;
        }
    }

    auto normalize_range(const auto& range) const {
        const auto& [begin, length] = range;
        return length < 0 ? std::make_pair(begin + length, -length) : std::make_pair(begin, length);
    }

    auto try_merge(const auto& left, const auto& right) {
        const auto& [left_begin, left_pair] = *left;
        const auto& [left_length, left_value] = left_pair;
        const auto left_end = left_begin + left_length;

        const auto& [right_begin, right_pair] = *right;
        const auto& [right_length, right_value] = right_pair;
        const auto right_end = right_begin + right_length;

        if (left_end != right_begin || left_value != right_value) {
            return right;
        }

        map[left_begin].first = right_end - left_begin;
        map.erase(right);

        return left;
    }
};
