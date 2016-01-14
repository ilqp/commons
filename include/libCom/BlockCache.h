//
// Created by steffen on 10.09.15.
//

#ifndef LIBCOM_BLOCKCACHE_H
#define LIBCOM_BLOCKCACHE_H

#include <unordered_map>
#include <cinttypes>

template<class T>
class BlockInfo {

public:
    /**
     * Creates a BlockInfo object with address set to 0.
     */
    BlockInfo() : address(0) { };

    /**
     * Creates a BlockInfo object with specified address.
     * @param address Address
     */
    BlockInfo(const T address) : address(address) { }

    /**
     * Address
     */
    T address;

    /**
     * Generation
     */
    uint16_t generation = 1;
};

template<class K, class V>
class BlockCache {

public:
    /**
     * Creates a BlockCache with fixed capacity.
     * The capacity CANNOT change!
     * @param capacity Cache's fixed capacity (default: BlockCache::DEFAULT_CAPACITY)
     */
    BlockCache(const uint32_t capacity = DEFAULT_CAPACITY) : mCapacity(capacity) { }

    /**
     * Returns V associated with that K
     * @param id K
     * @return V of this K or object V with default constructor if not found
     */
    const V read(const K id) const {
        return mMap.find(id) != mMap.end() ? mMap.find(id)->second.address : V();
    }

    /**
     * Checks if a K exists in cache
     * @param id K
     * @return True if K exists, false if not
     */
    bool contains(const K id) const {
        return mMap.find(id) != mMap.end();
    }

    /**
     * Returns an item's current generation
     * @param id K
     * @return Generation or 0 if no such block is present
     */
    uint16_t generation(const K id) const {
        return mMap.find(id) != mMap.end() ? mMap.find(id)->second.generation : 0;
    }

    /**
     * Writes a K with its V into the cache
     * @param id K
     * @param address V
     * @return True if there was enough (no block removed), false if not
     */
    bool write(const K id, const V address) {
        // element is present
        auto e = mMap.find(id);
        if (e != mMap.end()) {
            // increase generation
            BlockInfo<V> &block = e->second;
            // find all elements with same generation
            auto range = mGenMap.equal_range(block.generation);
            // sadly we cannot use "for (auto &el: range)" here..
            for (typename std::unordered_multimap<uint16_t , K>::iterator it = range.first; it != range.second; ++it)
                if ((*it).second == id) {       // if this is our block, remove it from the generation map...
                    mGenMap.erase(it);
                    break;
                }

            // if this is a least block
            if (block.generation == mLeast && mGenMap.find(mLeast) == mGenMap.end())       // look if there aren't any other blocks with same generation
                mLeast++;                   // if so, adjust lowest generation

            block.generation++;
            mGenMap.emplace(block.generation, id);      // ... to insert it again in an increased generation
        } else {        // element is not present
            bool retValue = true;
            // check if element insertion would exceed map size. If so, remove one block
            if (mMap.size() >= mCapacity) {
                auto leastElem = mGenMap.find(mLeast);      // get one least used block id (with lowest generation). There is always one, so leastElem cannot be it::end
                mMap.erase(leastElem->second);     // remove it
                mGenMap.erase(leastElem);           // and remove it form generation map
                retValue = false;               // we had to remove one block, so return false
            }
            mMap[id] = BlockInfo<V>(address);          // insert new block
            mGenMap.emplace(1, id);         // and place the block in generation map
            mLeast = 1;             // just inserted a generation 1 element

            return retValue;
        }
        return true;
    }

    /**
     * Returns the current cache size
     */
    inline uint32_t size() const {
        return static_cast<uint32_t>(mMap.size());
    }

    /**
     * Returns the cache capacity
     */
    inline uint32_t capacity() const {
        return mCapacity;
    }

    /**
     * Clears the Cache
     */
    void clear() {
        mMap.clear();
        mGenMap.clear();
        mLeast = 0;
    }

    /**
     * Default maximum number of elements within the cache
     */
    const static uint32_t DEFAULT_CAPACITY = 4096;

private:
    /**
     * Maps: K -> BlockInfo
     */
    std::unordered_map<K, BlockInfo<V>> mMap;
    /**
     * Maps: Generation -> K
     */
    std::unordered_multimap<uint16_t, K> mGenMap;

    /**
     * The lowest generation
     */
    uint16_t mLeast = 0;

    /**
     * This cache's capacity
     */
    const uint32_t mCapacity;
};

#endif //LIBCOM_BLOCKCACHE_H
