#include <vector>
#include <functional>   // std::hash
#include <cmath>        // std::ceil
#include <stdexcept>    // std::out_of_range

using std::vector;

template<typename K, typename V>
struct Entry {
    K key;
    V value;
};

template<typename K, typename V>
class HashMap {
    private:
        std::vector<std::vector<Entry<K,V>*>> buckets;
        size_t bucket_count;    // Number of buckets
        size_t entry_count;     // Number of entries
        float max_load_factor = 0.75f;

    public:
        // Default Constructor
        /**
         * @brief Constructs a HashMap with a default capacity of 32 and max load factor of 0.75.
         */
        HashMap() :
            bucket_count(32),
            buckets(32),
            entry_count(0) {}

        // Constructor
        /**
         * @brief Constructs a HashMap with a specified capacity and default max load factor of 0.75.
         * @param cap The initial number of buckets (capacity). Must be > 0.
         * @throws std::invalid_argument if cap is 0.
         */
        HashMap(size_t cap) : 
            bucket_count(cap), 
            buckets(cap), 
            entry_count(0) {

            if (bucket_count == 0) 
                throw std::invalid_argument("HashMap: capacity must be > 0");
        }

        /**
         * @brief Constructs a HashMap with specified capacity and max load factor.
         * @param cap The initial number of buckets (capacity). Must be > 0.
         * @param mlf The maximum load factor. Must be positive.
         * @throws std::invalid_argument if cap is 0 or mlf is not positive.
         */
        HashMap(size_t cap, float mlf) : 
            bucket_count(cap), 
            buckets(cap), 
            entry_count(0), 
            max_load_factor(mlf) {

            if (bucket_count == 0) 
                throw std::invalid_argument("HashMap: capacity must be > 0");
            if (max_load_factor <= 0.0f) 
                throw std::invalid_argument("HashMap: max_load_factor must be positive");
        }

        // Destructor
        /**
         * @brief Destroys the HashMap and frees all dynamically allocated Entry objects.
         */
        ~HashMap() {
            for (auto& b : buckets) {
                for (Entry<K,V>* e : b) {
                    delete(e);
                }
                b.clear();
            }
        }

        // Copy Constructor
        /**
         * @brief Constructs a deep copy of the given HashMap.
         * @param other The HashMap to copy from.
         */
        HashMap(const HashMap& other) : 
            bucket_count(other.bucket_count), 
            buckets(other.bucket_count), 
            entry_count(other.entry_count) {

            for (size_t i = 0; i < other.bucket_count; i++) {
                for (Entry<K,V>* e : other.buckets[i]) {
                    buckets[i].push_back(new Entry<K,V>{e->key, e->value});
                }
            }
        }

        // Move Constructor
        /**
         * @brief Constructs a HashMap by moving the resources from the given HashMap.
         * @param other The HashMap to move resources from. Left in a valid, empty state.
         */
        HashMap(HashMap&& other) noexcept :
            bucket_count(other.bucket_count),
            buckets(std::move(other.buckets)),
            entry_count(other.entry_count),
            max_load_factor(other.max_load_factor) {

                other.bucket_count = 0;
                other.entry_count = 0;
            }

        // Copy Assignment
        /**
         * @brief Assigns the contents of another HashMap to this one via deep copy.
         * @param other The HashMap to copy from.
         * @return A reference to the assigned HashMap.
         */
        HashMap& operator=(const HashMap& other) {
            if (this == &other) return *this;
            // Clear current
            this->clear();
            // Copy data
            bucket_count = other.bucket_count;
            entry_count = other.entry_count;
            buckets.clear();
            buckets.resize(bucket_count);
            // Insert data
            for (size_t i = 0; i < other.bucket_count; i++) {
                for (Entry<K,V>* e : other.buckets[i]) {
                    buckets[i].push_back(new Entry<K,V>{e->key, e->value});
                }
            }
            return *this;
        }

        // Move Assignmet
        /**
         * @brief Assigns the contents of another HashMap to this one via resource move.
         * @param other The HashMap to move resources from. Left in a valid, empty state.
         * @return A reference to the assigned HashMap.
         */
        HashMap& operator=(HashMap&& other) noexcept {
            if (this == &other) return *this;
            // Clear current
            this->clear();
            // Steal resources
            bucket_count = other.bucket_count;
            buckets = std::move(other.buckets);
            entry_count = other.entry_count;
            max_load_factor = other.max_load_factor;
            // Leave other empty
            other.bucket_count = 0;
            other.entry_count = 0;
            return *this;
        }

        // === Helper Functions ===

        /**
         * @brief Computes the bucket index for a given key.
         * @param key The key to hash.
         * @return The index of the bucket (0 to bucket_count - 1).
         */
        size_t hash_func(const K& key) const {
            return std::hash<K>{}(key) % bucket_count;
        }

        /**
         * @brief Finds a mutable Entry pointer associated with the given key.
         * @param key The key to look for.
         * @return A pointer to the Entry if found, otherwise nullptr.
         */
        Entry<K,V>* find_entry(const K& key) {
            size_t index = hash_func(key);
            for (Entry<K,V>* e : buckets[index]) {
                if (e->key == key) return e;
            }
            return nullptr;
        }

        /**
         * @brief Finds a constant Entry pointer associated with the given key.
         * @param key The key to look for.
         * @return A constant pointer to the Entry if found, otherwise nullptr.
         */
        const Entry<K,V>* find_entry(const K& key) const {
            size_t index = hash_func(key);
            for (Entry<K,V>* e : buckets[index]) {
                if (e->key == key) return e;
            }
            return nullptr;
        }

        class Const_Iterator; // Forward declaration

        class Iterator {
            public:
                HashMap* map;
                size_t bucket_idx;
                size_t entry_idx;
                
                Iterator(HashMap* m, size_t b, size_t e) : map(m), bucket_idx(b), entry_idx(e) {}
                Iterator() : map(nullptr), bucket_idx(0), entry_idx(0) {}

                // Dereference
                Entry<K,V>& operator*() const {
                    if (!map || bucket_idx >= map->bucket_count || entry_idx >= map->buckets[bucket_idx].size())
                        throw std::out_of_range("Iterator::operator*: invalid iterator");
                    return *(map->buckets[bucket_idx][entry_idx]);
                }

                // Move Forward
                Iterator& operator++() {
                    if (!map || bucket_idx >= map->bucket_count) {
                        return *this;
                    }
                    // Move to next entry
                    entry_idx++;
                    // Still in the same bucket
                    if (bucket_idx < map->bucket_count && entry_idx < map->buckets[bucket_idx].size()) {
                        return *this;
                    }
                    // Otherwise move to next bucket
                    bucket_idx++;
                    while (bucket_idx < map->bucket_count && map->buckets[bucket_idx].empty()) {
                        bucket_idx++;   // Skip empty buckets
                    }
                    // Reset entry index
                    entry_idx = 0;
                    return *this;
                }

                bool operator==(const Iterator& other) const {
                    return map == other.map &&
                            bucket_idx == other.bucket_idx &&
                            entry_idx == other.entry_idx;
                }

                bool operator==(const Const_Iterator& other) const {
                    return map == other.map &&
                            bucket_idx == other.bucket_idx &&
                            entry_idx == other.entry_idx;
                }

                bool operator!=(const Iterator& other) const {
                    return !(*this == other);
                }

                bool operator!=(const Const_Iterator& other) const {
                    return !(*this == other);
                }
        };

        class Const_Iterator {
            public:
                const HashMap* map;
                size_t bucket_idx;
                size_t entry_idx;
                
                Const_Iterator(const HashMap* m, size_t b, size_t e) : map(m), bucket_idx(b), entry_idx(e) {}

                Const_Iterator() : map(nullptr), bucket_idx(0), entry_idx(0) {}

                // Dereference
                const Entry<K,V>& operator*() const {
                    if (!map || bucket_idx >= map->bucket_count || entry_idx >= map->buckets[bucket_idx].size())
                        throw std::out_of_range("Const_Iterator::operator*: invalid iterator");
                    return *(map->buckets[bucket_idx][entry_idx]);
                }

                // Move Forward
                Const_Iterator& operator++() {
                    if (!map || bucket_idx >= map->bucket_count) {
                        return *this;
                    }
                    // Move to next entry
                    entry_idx++;
                    // Still in the same bucket
                    if (bucket_idx < map->bucket_count && entry_idx < map->buckets[bucket_idx].size()) {
                        return *this;
                    }
                    // Otherwise move to next bucket
                    bucket_idx++;
                    while (bucket_idx < map->bucket_count && map->buckets[bucket_idx].empty()) {
                        bucket_idx++;   // Skip empty buckets
                    }
                    // Reset entry index
                    entry_idx = 0;
                    return *this;
                }

                bool operator==(const Const_Iterator& other) const {
                    return map == other.map &&
                            bucket_idx == other.bucket_idx &&
                            entry_idx == other.entry_idx;
                }

                bool operator==(const Iterator& other) const {
                    return map == other.map &&
                            bucket_idx == other.bucket_idx &&
                            entry_idx == other.entry_idx;
                }

                bool operator!=(const Const_Iterator& other) const {
                    return !(*this == other);
                }

                bool operator!=(const Iterator& other) const {
                    return !(*this == other);
                }
        };

        // === Iterators ===

        /**
         * @brief Returns a mutable iterator pointing to the first element in the map.
         * @return Iterator to the first element, or end() if the map is empty.
         */
        Iterator begin() noexcept {
            for (size_t i = 0; i < bucket_count; i++) {
                // First non-empty bucket
                if (!buckets[i].empty()) {
                    return Iterator(this, i, 0);
                }
            }
            // No entries
            return end();
        }

        /**
         * @brief Returns a constant iterator pointing to the first element in the map.
         * @return Const_Iterator to the first element, or end() if the map is empty.
         */
        Const_Iterator begin() const noexcept{
            for (size_t i = 0; i < bucket_count; i++) {
                // First non-empty bucket
                if (!buckets[i].empty()) {
                    return Const_Iterator(this, i, 0);
                }
            }
            // No entries
            return end();
        }

        /**
         * @brief Returns a mutable iterator pointing past the last element in the map.
         * @return Iterator pointing to the theoretical element after the last.
         */
        Iterator end() noexcept{
            return Iterator(this, bucket_count, 0);
        }

        /**
         * @brief Returns a constant iterator pointing past the last element in the map.
         * @return Const_Iterator pointing to the theoretical element after the last.
         */
        Const_Iterator end() const noexcept {
            return Const_Iterator(this, bucket_count, 0);
        }

        // === Capacity ===

        /**
         * @brief Checks if the container is empty (contains no elements).
         * @return true if the container is empty, false otherwise.
         */
        bool empty() const noexcept {
            return entry_count == 0;
        }

        /**
         * @brief Returns the number of elements in the container.
         * @return The number of key-value pairs (entry_count).
         */
        size_t size() const noexcept {
            return entry_count;
        }

        /**
         * @brief Returns the theoretical maximum number of elements the container can hold.
         * @return The maximum size.
         */
        size_t max_size() const noexcept {
            return bucket_count * std::vector<Entry<K,V>*>().max_size();
        }

        // === Modifiers ===

        /**
         * @brief Removes all elements from the container, leaving it empty.
         */
        void clear() noexcept {
            for (auto& b : buckets) {
                for (Entry<K,V>* e : b) {
                    delete e;   // Free memory
                }
                b.clear();      // Clear bucket
            }
            // Reset entry_count
            entry_count = 0;
        }

        /**
         * @brief Inserts or updates an element with a specified key and value.
         * @param key The key to insert or update.
         * @param value The value associated with the key.
         */
        void insert(const K& key, const V& value) {
            Entry<K,V>* e = find_entry(key);
            if (e) {
                e->value = value;
                return;
            }
            // Not found, create new
            size_t index = hash_func(key);
            e = new Entry<K,V>{key, value};
            buckets[index].push_back(e);
            entry_count++;
            // Rehash if needed
            if (load_factor() > max_load_factor) {
                rehash(bucket_count * 2);
            }
        }

        /**
         * @brief Removes the element pointed to by the iterator.
         * @param pos Iterator pointing to the element to be erased.
         * @return An iterator pointing to the element immediately following the erased one, or end().
         */
        Iterator erase(Iterator pos) {
            // Validate iterator
            if (pos.map != this) return end();
            if (pos.bucket_idx >= bucket_count) return end();
            auto& b = buckets[pos.bucket_idx];
            size_t i = pos.entry_idx;
            if (i >= b.size()) return end();
            // O(1) delete
            delete b[i];    // Free memory
            if (i + 1 != b.size()) b[i] = b.back();
            b.pop_back();   // Remove duplicate
            entry_count--;
            // Return next entry in bucket
            if (i < b.size()) return Iterator(this, pos.bucket_idx, i);
            // Otherwise find next non-empty bucket
            size_t nb_idx = pos.bucket_idx + 1;
            while (nb_idx < bucket_count && buckets[nb_idx].empty()) nb_idx++;
            if (nb_idx < bucket_count) return Iterator(this, nb_idx, 0);
            // No more entries
            return end();
        }

        /**
         * @brief Removes the element with the specified key.
         * @param key The key of the element to be erased.
         * @return 1 if an element was erased, 0 otherwise.
         */
        size_t erase(const K& key) {
            size_t index = hash_func(key);
            auto& b = buckets[index];
            for (size_t i = 0; i < b.size(); i++) {
                if (b[i]->key == key) {
                    // O(1) delete
                    delete b[i];    // Free memory
                    if (i + 1 != b.size()) b[i] = b.back();
                    b.pop_back();   // Remove duplicate
                    entry_count--;
                    return 1;
                }
            }
            // Not found
            return 0;
        }

        // === Lookup ===

        /**
         * @brief Accesses the value associated with the given key (mutable).
         * @param key The key to look for.
         * @return A mutable reference to the value.
         * @throws std::out_of_range if the key is not found.
         */
        V& at(const K& key) {
            Entry<K,V>* e = find_entry(key);
            if (!e) throw std::out_of_range("HashMap::at: key not found");
            return e->value;
        }

        /**
         * @brief Accesses the value associated with the given key (constant).
         * @param key The key to look for.
         * @return A constant reference to the value.
         * @throws std::out_of_range if the key is not found.
         */
        const V& at(const K& key) const {
            const Entry<K,V>* e = find_entry(key);
            if (!e) throw std::out_of_range("HashMap::at: key not found");
            return e->value;
        }

        /**
         * @brief Accesses or inserts a value associated with the given key.
         * @param key The key to look up.
         * @return A mutable reference to the value. If the key is new, a default-constructed value is inserted.
         */
        V& operator[](const K& key) {
            Entry<K,V>* e = find_entry(key);
            if (e) return e->value;
            // Not found, create new
            size_t index = hash_func(key);
            e = new Entry<K,V>{key, V{}};
            buckets[index].push_back(e);
            entry_count++;
            // Rehash if needed
            if (load_factor() > max_load_factor) {
                rehash(bucket_count * 2);
            }
            return e->value;
        }

        /**
         * @brief Returns the number of elements with the given key (always 1 or 0).
         * @param key The key to look for.
         * @return 1 if the key exists, 0 otherwise.
         */
        size_t count(const K& key) const {
            size_t index = hash_func(key);
            const Entry<K,V>* e = find_entry(key);
            return e ? 1 : 0;
        }

        /**
         * @brief Finds the first element with the given key (mutable).
         * @param key The key to look for.
         * @return An Iterator to the element if found, or end() if not found.
         */
        Iterator find(const K& key) {
            size_t index = hash_func(key);
            auto& b = buckets[index];
            for (size_t i = 0; i < b.size(); i++) {
                if (b[i]->key == key) {
                    return Iterator(this, index, i);
                }
            }
            // Not found
            return end();
        }

        /**
         * @brief Finds the first element with the given key (constant).
         * @param key The key to look for.
         * @return A Const_Iterator to the element if found, or end() if not found.
         */
        Const_Iterator find(const K& key) const {
            size_t index = hash_func(key);
            auto& bucket = buckets[index];
            for (size_t i = 0; i < bucket.size(); i++) {
                if (bucket[i]->key == key) {
                    return Const_Iterator(this, index, i);
                }
            }
            // Not found
            return end();
        }

        /**
         * @brief Checks if the container contains an element with the given key.
         * @param key The key to check for.
         * @return true if the key exists, false otherwise.
         */
        bool contains(const K& key) const {
            const Entry<K,V>* e = find_entry(key);
            return e ? true : false;
        }

        // === Bucket Interface

        /**
         * @brief Returns a mutable iterator to the first element in a specific bucket.
         * @param n The bucket index.
         * @return Iterator to the first element in bucket n, or end() if the bucket is empty.
         * @throws std::out_of_range if n is out of range.
         */
        Iterator begin(size_t n) {
            if (n >= bucket_count) 
                throw std::out_of_range("HashMap::begin(bucket): bucket index out of range");
            if (buckets[n].empty()) return end();
            return Iterator(this, n, 0);
        }

        /**
         * @brief Returns a constant iterator to the first element in a specific bucket.
         * @param n The bucket index.
         * @return Const_Iterator to the first element in bucket n, or end() if the bucket is empty.
         * @throws std::out_of_range if n is out of range.
         */
        Const_Iterator begin(size_t n) const {
            if (n >= bucket_count) 
                throw std::out_of_range("HashMap::begin(bucket): bucket index out of range");
            if (buckets[n].empty()) return end();
            return Const_Iterator(this, n, 0);
        }

        /**
         * @brief Returns a mutable iterator pointing past the last element in a specific bucket.
         * @param n The bucket index.
         * @return Iterator to the end of bucket n.
         * @throws std::out_of_range if n is out of range.
         */
        Iterator end(size_t n) {
            if (n >= bucket_count) 
                throw std::out_of_range("HashMap::end(bucket): bucket index out of range");
            return Iterator(this, n, buckets[n].size());
        }

        /**
         * @brief Returns a constant iterator pointing past the last element in a specific bucket.
         * @param n The bucket index.
         * @return Const_Iterator to the end of bucket n.
         * @throws std::out_of_range if n is out of range.
         */
        Const_Iterator end(size_t n) const {
            if (n >= bucket_count) 
                throw std::out_of_range("HashMap::end(bucket): bucket index out of range");
            return Const_Iterator(this, n, buckets[n].size());
        }

        /**
         * @brief Returns the total number of buckets (capacity).
         * @return The current bucket count.
         */
        size_t get_bucket_count() const {
            return bucket_count;
        }

        /**
         * @brief Returns the number of elements currently in a specific bucket.
         * @param n The bucket index.
         * @return The size of the bucket.
         * @throws std::out_of_range if n is out of range.
         */
        size_t bucket_size(size_t n) const {
            if (n >= bucket_count) 
                throw std::out_of_range("HashMap::bucket_size: bucket index out of range");
            return buckets[n].size();
        }

        /**
         * @brief Returns the bucket index where the element with the given key is or would be stored.
         * @param key The key to check.
         * @return The bucket index.
         */
        size_t bucket(const K& key) const {
            return hash_func(key);
        }

        // === Hash Policy ===

        /**
         * @brief Returns the current load factor (average number of elements per bucket).
         * @return The load factor (entry_count / bucket_count).
         */
        float load_factor() const {
            return static_cast<double>(entry_count) / bucket_count;
        }

        /**
         * @brief Returns the maximum allowed load factor.
         * @return The max load factor.
         */
        float get_max_load_factor() const {
            return max_load_factor;
        }

        /**
         * @brief Sets the maximum allowed load factor and potentially triggers a rehash.
         * @param ml The new maximum load factor.
         */
        void set_max_load_factor(float ml) {
            max_load_factor = ml;
            // Rehash if needed
            if (load_factor() > max_load_factor) {
                rehash(bucket_count * 2);
            }
        }

        /**
         * @brief Rehashes the container into a new set of buckets.
         * @param count The suggested minimum new bucket count. Will be adjusted based on load factor.
         */
        void rehash(size_t count) {
            size_t new_bucket_count = count;
            size_t min_required = static_cast<size_t>(
                std::ceil(static_cast<double>(entry_count) / max_load_factor)
            );
            if (new_bucket_count < min_required) new_bucket_count = min_required;
            if (new_bucket_count == bucket_count) return;   // Large enough
            // Resize hashmap
            auto old_buckets = std::move(buckets);
            buckets.clear();
            buckets.resize(new_bucket_count);
            bucket_count = new_bucket_count;
            // Move pointers into new buckets
            for (auto& b : old_buckets) {
                for (auto* e : b) {
                    size_t index = hash_func(e->key);
                    buckets[index].push_back(e);
                }
            }
        }
};