#pragma once

#include <algorithm>
#include <cmath>
#include <list>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

template<typename K, typename V, bool is_const = false>
class TableIterator;
template<typename K, typename V>
class TableNode;
template<typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K> >
class Table;

template<class K, class V>
class TableNode
{
public:
    using value_type = std::pair<K, V>;

private:
    value_type value_;
    bool engaged = false;

public:
    TableNode() = default;

    TableNode &operator =(const TableNode &) requires std::copy_constructible<V> = default;

    TableNode &operator =(const TableNode &) requires (!std::copy_constructible<V>) = delete;

    explicit TableNode(value_type &&v) : value_(std::move(v)), engaged(true) {}

    [[nodiscard]] bool empty() const noexcept { return !engaged; }

    value_type &value() noexcept { return value_; }
    const value_type &value() const noexcept { return value_; }
};

template<typename K, typename V, bool is_const>
class TableIterator
{
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<const K, V>;
    using difference_type = ptrdiff_t;
    using pointer = std::conditional_t<is_const, const value_type, value_type> *;
    using reference = std::conditional_t<is_const, const value_type, value_type> &;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::forward_iterator_tag;

private:
    using Bucket = std::list<value_type>;
    using bucket_iterator = std::conditional_t<is_const, typename Bucket::const_iterator, typename Bucket::iterator>;
    using bucket_container = std::conditional_t<is_const, const std::vector<Bucket>, std::vector<Bucket> >;
    bucket_container *buckets;
    size_t bucket_index;
    bucket_iterator it;

    template<typename, typename, typename, typename>
    friend class Table;

    friend TableIterator<K, V, !is_const>;

    void advance_to_valid()
    {
        while (bucket_index < buckets->size() && (*buckets)[bucket_index].empty())
            ++bucket_index;
        if (bucket_index < buckets->size())
            it = (*buckets)[bucket_index].begin();
    }

public:
    TableIterator() noexcept : buckets(nullptr), bucket_index(0), it() {};

    TableIterator(std::vector<Bucket> *buckets_ptr, const size_t index,
                           bucket_iterator it_) noexcept : buckets(buckets_ptr), bucket_index(index), it(it_)
    {
        if (buckets && bucket_index < buckets->size() && it == (*buckets)[bucket_index].end())
        {
            advance_to_valid();
        }
    }

    TableIterator(const std::vector<Bucket> *buckets_ptr, const size_t index,
                           bucket_iterator it_) noexcept : buckets(const_cast<std::vector<Bucket> *>(buckets_ptr)),
                                                           bucket_index(index), it(it_)
    {
        if (buckets && bucket_index < buckets->size() && it == (*buckets)[bucket_index].end())
        {
            advance_to_valid();
        }
    }

    template<bool B = is_const> requires B
    explicit TableIterator(const TableIterator<K, V, false> &other) noexcept : buckets(other.buckets),
                                                                               bucket_index(other.bucket_index),
                                                                               it(other.it) {}

    reference operator *() const { return *it; }
    pointer operator ->() const { return &(*it); }

    TableIterator &operator ++()
    {
        if (!buckets)
            return *this;
        ++it;
        if (it == (*buckets)[bucket_index].end())
        {
            ++bucket_index;
            advance_to_valid();
        }
        if (bucket_index == buckets->size())
        {
            buckets = nullptr;
            bucket_index = 0;
            it = bucket_iterator();
        }
        return *this;
    }

    TableIterator operator ++(int)
    {
        TableIterator tmp(buckets, bucket_index, it);
        ++(*this);
        return tmp;
    }

    bool operator ==(const TableIterator &other) const
    {
        if (!buckets && !other.buckets)
            return true;
        if (!buckets || !other.buckets)
            return false;
        if (buckets == other.buckets)
        {
            return bucket_index == other.bucket_index && (bucket_index == buckets->size() || it == other.it);
        }
        return false;
    }

    bool operator !=(const TableIterator &other) const
    {
        return !(*this == other);
    }

    TableIterator(const TableIterator &) = default;

    TableIterator &operator=(const TableIterator &) = default;

    TableIterator(TableIterator &&) noexcept = default;

    TableIterator &operator=(TableIterator &&) noexcept = default;
};

template<class K, class V, class Hash, class KeyEqual>
class Table
{
public:
    using key_type = K;
    using value_type = std::pair<const K, V>;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = TableIterator<K, V, false>;
    using const_iterator = TableIterator<K, V, true>;
    using difference_type = ptrdiff_t;
    using size_type = size_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using node_type = TableNode<K, V>;
    using allocator_type = std::allocator<value_type>;

private:
    using Bucket = std::list<value_type>;
    Hash hasher_;
    std::vector<std::list<value_type> > buckets;
    size_type table_size;
    KeyEqual key_eq_;
    float max_lf;

public:
    using local_iterator = Bucket::iterator;
    using const_local_iterator = Bucket::const_iterator;

    explicit Table(const size_t initial = 101, Hash hf = std::hash<K>(),
                   KeyEqual eq = std::equal_to<K>()) : hasher_(hf), buckets(initial), table_size(0), key_eq_(eq),
                                                       max_lf(1.0) {}

    Table(const Table &) requires std::copy_constructible<V> = default;

    Table(const Table &) requires (!std::copy_constructible<V>) = delete;

    Table(Table &&other) noexcept : hasher_(std::move(other.hasher_)), buckets(std::move(other.buckets)),
                                    table_size(other.table_size), key_eq_(std::move(other.key_eq_)),
                                    max_lf(other.max_lf)
    {
        other.table_size = 0;
    }

    template<std::input_iterator Iter> requires std::convertible_to<std::iter_value_t<Iter>, value_type>
    Table(Iter from, Iter to, const size_t initial = 101, Hash hf = std::hash<K>(),
          KeyEqual eq = std::equal_to<K>()) : hasher_(hf), buckets(initial), table_size(0), key_eq_(eq), max_lf(1.0)
    {
        for (auto it = from; it != to; ++it)
        {
            insert(*it);
        }
    }

    Table(std::initializer_list<value_type> il, const size_t initial = 101, Hash hf = std::hash<K>(),
          KeyEqual eq = std::equal_to<K>()) : Table(il.begin(), il.end(), initial, hf, eq) {}

    Table &operator =(const Table &other) requires (std::copy_constructible<V>)
    {
        if (this != &other)
        {
            clear();
            hasher_ = other.hasher_;
            max_lf = other.max_lf;
            key_eq_ = other.key_eq_;
            for (const auto &[k, v]: other)
                insert({k, v});
        }
        return *this;
    }

    Table &operator =(const Table &other) requires (!std::copy_constructible<V>) = delete;

    Table &operator =(Table &&other) noexcept
    {
        if (this != &other)
        {
            table_size = other.table_size;
            max_lf = other.max_lf;
            key_eq_ = other.key_eq_;
            std::swap(hasher_, other.hasher_);
            std::swap(buckets, other.buckets);
        }
        return *this;
    }

    Table &operator =(std::initializer_list<value_type> il)
    {
        clear();
        for (auto &el: il)
        {
            insert(el);
        }
        return *this;
    }

    template<class... Args>
    std::pair<iterator, bool> emplace(Args &&... args)
    {
        value_type tmp(std::forward<Args>(args)...);
        const K &k = tmp.first;
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
            if (key_eq_(it->first, k))
                return {iterator(&buckets, idx, it), false};
        bucket.emplace_back(std::move(tmp));
        auto it = std::prev(bucket.end());
        ++table_size;
        return {iterator(&buckets, idx, it), true};
    }

    std::pair<iterator, bool> insert(const value_type &value)
    {
        const K &k = value.first;
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
            if (key_eq_(it->first, k))
                return {iterator(&buckets, idx, it), false};
        bucket.push_back(value);
        ++table_size;
        auto it = std::prev(bucket.end());
        return {iterator(&buckets, idx, it), true};
    }

    std::pair<iterator, bool> insert(value_type &&value)
    {
        const K &k = value.first;
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
            if (key_eq_(it->first, k))
                return {iterator(&buckets, idx, it), false};
        bucket.push_back(std::move(value));
        ++table_size;
        auto it = std::prev(bucket.end());
        return {iterator(&buckets, idx, it), true};
    }

    template<class P> requires std::convertible_to<P, value_type>
    std::pair<iterator, bool> insert(P &&value)
    {
        value_type tmp(std::forward<P>(value));
        return insert(std::move(tmp));
    }

    void insert(const_iterator from, const_iterator to)
    {
        for (auto it = from; it != to; ++it)
            insert(*it);
    }

    template<std::ranges::input_range R> requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
    void insert_range(R &&r)
    {
        for (auto &&elem: r)
            insert(std::forward<decltype (elem)>(elem));
    }

    void insert(std::initializer_list<value_type> il)
    {
        for (auto &&elem : il)
            insert(elem);
    }

    iterator insert(node_type &&nh)
    {
        if (nh.empty())
            return end();
        const K &k = nh.value().first;
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
            if (key_eq_(it->first, k))
                return iterator(&buckets, idx, it);
        bucket.push_front(std::move(nh.value()));
        auto it = bucket.begin();
        ++table_size;
        return iterator(&buckets, idx, it);
    }

    iterator insert(const_iterator, node_type &&nh)
    {
        return insert(nh);
    }

    node_type extract(const K &k)
    {
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
            if (key_eq_(it->first, k))
            {
                node_type nh(std::move(*it));
                bucket.erase(it);
                --table_size;
                return nh;
            }
        return node_type();
    }

    node_type extract(iterator q)
    {
        if (q.bucket_index >= buckets.size())
            return node_type{};
        auto &bucket = buckets[q.bucket_index];
        auto it = q.it;
        if (it == bucket.end())
            return node_type{};
        node_type nh(std::move(*it));
        bucket.erase(it);
        --table_size;
        return nh;
    }

    void merge(Table &other)
    {
        if (this == &other)
            return;
        for (size_t i = 0; i < other.buckets.size(); ++i)
        {
            auto &src_bucket = other.buckets[i];
            auto it = src_bucket.begin();
            while (it != src_bucket.end())
            {
                if (const K &k = it->first; find(k) != end())
                {
                    ++it;
                    continue;
                }
                auto nh = other.extract(iterator(&other.buckets, i, it++));
                insert(std::move(nh));
            }
        }
    }

    size_t erase(const K &k)
    {
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
        {
            if (key_eq_(k, it->first))
            {
                bucket.erase(it);
                --table_size;
                return 1;
            }
        }
        return 0;
    }

    iterator erase(iterator it)
    {
        if (it.buckets == nullptr || it.bucket_index >= buckets.size())
            return end();
        size_t idx = it.bucket_index;
        auto &bucket = buckets[idx];
        auto next_it = bucket.erase(it.it);
        --table_size;
        if (next_it != bucket.end())
            return iterator(&buckets, idx, next_it);
        for (size_t i = idx + 1; i < buckets.size(); ++i)
            if (!buckets[i].empty())
                return iterator(&buckets, i, buckets[i].begin());
        return end();
    }

    iterator erase(const_iterator it)
    {
        if (it.buckets == nullptr || it.bucket_index >= buckets.size())
            return end();
        size_t idx = it.bucket_index;
        auto &bucket = buckets[idx];
        auto next_it = bucket.erase(it.it);
        --table_size;
        if (next_it != bucket.end())
            return iterator(&buckets, it.bucket_index, next_it);
        for (size_t i = idx + 1; i < buckets.size(); ++i)
            if (!buckets[i].empty())
                return iterator(&buckets, i, buckets[i].begin());
        return end();
    }

    iterator erase(const_iterator from, const_iterator to)
    {
        const_iterator it = from;
        iterator res = end();
        while (it != to)
        {
            res = erase(it);
            it = const_iterator(res);
        }
        return res;
    }

    iterator find(const K &k)
    {
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
        {
            if (key_eq_(k, it->first))
                return iterator(&buckets, idx, it);
        }
        return end();
    }

    const_iterator find(const K &k) const
    {
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
        {
            if (key_eq_(k, it->first))
                return const_iterator(&buckets, idx, it);
        }
        return end();
    }

    bool contains(const K &k) const
    {
        return find(k) != end();
    }

    [[nodiscard]] size_t bucket_count() const
    {
        return buckets.size();
    }

    [[nodiscard]] size_t max_bucket_count() const
    {
        return buckets.max_size();
    }

    [[nodiscard]] size_t bucket(const K &k) const
    {
        return hasher_(k) % buckets.size();
    }

    [[nodiscard]] size_t bucket_size(const size_t n) const
    {
        if (n >= buckets.size())
            return 0;
        return buckets[n].size();
    }

    V &operator [](const K &k)
    {
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
        {
            if (key_eq_(it->first, k))
                return it->second;
        }
        bucket.emplace_back(k, V{});
        ++table_size;
        return bucket.back().second;
    }

    V &operator [](K &&k)
    {
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
        {
            if (key_eq_(it->first, k))
                return it->second;
        }
        bucket.emplace_back(std::move(k), V{});
        ++table_size;
        return bucket.back().second;
    }

    V &at(const K &k)
    {
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
            if (key_eq_(it->first, k))
                return it->second;
        throw std::out_of_range("No such value");
    }

    const V &at(const K &k) const
    {
        size_t idx = hasher_(k) % buckets.size();
        auto &bucket = buckets[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it)
            if (key_eq_(it->first, k))
                return it->second;
        throw std::out_of_range("No such value");
    }

    void clear()
    {
        for (auto &bucket: buckets)
            bucket.clear();
        table_size = 0;
    }

    allocator_type get_allocator() const noexcept
    {
        return std::allocator<value_type>{};
    }

    void rehash(size_type n)
    {
        if (n == 0)
            n = 1;
        const size_type required_buckets = std::max<size_type>(
            1, std::max(n, static_cast<size_type>(std::ceil(static_cast<double>(table_size) / max_lf))));

        if (required_buckets <= buckets.size())
            return;
        std::vector<Bucket> new_buckets(required_buckets);
        for (auto &bucket: buckets)
            for (auto &elem: bucket)
            {
                size_t new_idx = hasher_(elem.first) % required_buckets;
                new_buckets[new_idx].push_back(std::move(elem));
            }
        buckets = std::move(new_buckets);
    }

    void reserve(const size_type n)
    {
        if (n == 0)
        {
            rehash(0);
            return;
        }
        const auto required_buckets = static_cast<size_t>(ceil(static_cast<double>(n) / max_lf));
        rehash(required_buckets);
    }

    size_type count(const K &k) const
    {
        return contains(k) ? 1 : 0;
    }

    std::pair<iterator, iterator> equal_range(const K &k)
    {
        auto it = find(k);
        if (it != end())
            return {it, std::next(it)};
        return {end(), end()};
    }

    std::pair<const_iterator, const_iterator> equal_range(const K &k) const
    {
        auto it = find(k);
        if (it != end())
            return {it, std::next(it)};
        return {end(), end()};
    }

    iterator begin() noexcept
    {
        for (size_t i = 0; i < buckets.size(); ++i)
        {
            if (!buckets[i].empty())
                return iterator(&buckets, i, buckets[i].begin());
        }
        return end();
    }

    iterator end() noexcept
    {
        return iterator{};
    }

    const_iterator begin() const noexcept
    {
        for (size_t i = 0; i < buckets.size(); ++i)
            if (!buckets[i].empty())
                return const_iterator(&buckets, i, buckets[i].begin());
        return end();
    }

    const_iterator end() const noexcept
    {
        return const_iterator{};
    }

    const_iterator cbegin() const noexcept
    {
        for (size_t i = 0; i < buckets.size(); ++i)
            if (!buckets[i].empty())
                return const_iterator(&buckets, i, buckets[i].begin());
        return cend();
    }

    const_iterator cend() const noexcept
    {
        return const_iterator{};
    }

    local_iterator begin(size_type idx)
    {
        return buckets[idx].begin();
    }

    local_iterator end(size_type idx)
    {
        return buckets[idx].end();
    }

    const_local_iterator begin(size_type idx) const
    {
        return buckets[idx].begin();
    }

    const_local_iterator end(size_type idx) const
    {
        return buckets[idx].end();
    }

    const_local_iterator cbegin(size_type idx) const
    {
        return buckets[idx].begin();
    }

    const_local_iterator cend(size_type idx) const
    {
        return buckets[idx].end();
    }

    bool operator ==(const Table &other) const noexcept
    {
        if (size() != other.size())
            return false;
        for (const auto &[k, v]: *this)
        {
            auto it = other.find(k);
            if (it == other.end() || it->second != v)
                return false;
        }
        return true;
    }

    bool operator !=(const Table &other) const noexcept
    {
        return !(*this == other);
    }

    void swap(Table &other) noexcept
    {
        std::swap(hasher_, other.hasher_);
        std::swap(buckets, other.buckets);
        std::swap(table_size, other.table_size);
        std::swap(key_eq_, other.key_eq_);
        std::swap(max_lf, other.max_lf);
    }

    [[nodiscard]] size_type size() const noexcept { return table_size; }

    [[nodiscard]] size_type max_size() const noexcept
    {
        return buckets.max_size();
    }

    [[nodiscard]] float load_factor() const noexcept
    {
        return bucket_count() ? static_cast<float>(size()) / bucket_count() : 0.0f;
    }

    [[nodiscard]] float max_load_factor() const noexcept { return max_lf; }

    void max_load_factor(const float z) noexcept
    {
        if (z > 0)
            max_lf = z;
    }

    [[nodiscard]] bool empty() const noexcept { return table_size == 0; }

    hasher hash_function() const noexcept { return hasher_; }

    key_equal key_eq() const noexcept { return key_eq_; }

    ~Table() noexcept = default;
};
