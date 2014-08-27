/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/

#ifndef _REDEMPTION_CORE_RDP_CACHES_BMPCACHE_HPP_
#define _REDEMPTION_CORE_RDP_CACHES_BMPCACHE_HPP_

#include <map>
#include <set>

#include "bitmap.hpp"
#include "RDP/PersistentKeyListPDU.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "fileutils.hpp"
#include "unique_ptr.hpp"

#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>

enum {
      BITMAP_FOUND_IN_CACHE
    , BITMAP_ADDED_TO_CACHE
};

class BmpCache {

public:
    static const uint8_t  MAXIMUM_NUMBER_OF_CACHES        = 5;

private:
    static const uint16_t MAXIMUM_NUMBER_OF_CACHE_ENTRIES = 8192;

    static const uint8_t IN_WAIT_LIST = 0x80;


    class bitmap_free_list_t {
        struct operator_delete {
            void operator()(void * data) {
                ::operator delete(data);
            }
        };

        unique_ptr<Bitmap, operator_delete> data;
        unique_ptr<Bitmap*[]> first;
        Bitmap* * last;
        size_t sz;

        bitmap_free_list_t(bitmap_free_list_t const &);
        bitmap_free_list_t& operator=(bitmap_free_list_t const &);

    public:
        bitmap_free_list_t(size_t sz)
        : data(static_cast<Bitmap*>(::operator new(sizeof(Bitmap) * (sz + 1))))
        , first(new Bitmap*[sz + 1])
        , last(this->first.get())
        , sz(sz + 1)
        {
            this->initialize_free_list();
        }

        ~bitmap_free_list_t() {
            this->destruct_initialized_bitmap();
        }

        void clear() {
            this->destruct_initialized_bitmap();
            this->last = this->first.get();
            this->initialize_free_list();
        }

        const Bitmap * pop(uint8_t bpp, const Bitmap & bmp) {
            assert(this->last != this->first.get());
            Bitmap * ret = *--this->last;
            return new (ret) Bitmap(bpp, bmp);
        }

        void push(const Bitmap * bmp) {
            Bitmap * p = this->data.get() + (bmp - this->data.get());
            p->~Bitmap();
            *this->last = p;
            ++this->last;
        }

    private:
        void initialize_free_list() {
            Bitmap * p = this->data.get();
            Bitmap * pe = p + this->sz;
            for (; p != pe; ++p, ++this->last) {
                *this->last = p;
            }
        }

        void destruct_initialized_bitmap() {
            std::sort(this->first.get(), this->last);

            Bitmap * pdata = this->data.get();
            Bitmap * pdata_last = pdata + this->sz;
            for (Bitmap* * p = this->first.get(); pdata != pdata_last; ++pdata) {
                if (*p != pdata) {
                    pdata->~Bitmap();
                }
                else if (++p == this->last) {
                    while (++pdata != pdata_last) {
                        pdata->~Bitmap();
                    }
                    break;
                }
            }
        }
    };

    struct cache_element
    {
        const Bitmap * bmp;
        int_fast32_t stamp;
        union {
            uint8_t  sig_8[8];
            uint32_t sig_32[2];
        } sig;
        uint8_t sha1[20];

        void reset() {
            this->bmp = 0;
            this->stamp = 0;
        }

        operator bool () const {
            return this->bmp;
        }
    };

    class storage_value_set {
        size_t elem_size;
        unique_ptr<uint8_t[]> data;
        unique_ptr<void*[]> free_list;
        void* * free_list_cur;

        storage_value_set(storage_value_set const &);
        storage_value_set& operator=(storage_value_set const &);

    public:
        storage_value_set()
        : elem_size(0)
        {}

        template<class T>
        void update() {
            this->elem_size = std::max(this->elem_size, sizeof(T));
        }

        void reserve(size_t sz) {
            assert(!this->data.get());
            this->data.reset(new uint8_t[sz * this->elem_size]);
            this->free_list.reset(new void*[sz]);
            this->free_list_cur = this->free_list.get();

            uint8_t * p = this->data.get();
            uint8_t * pe = p + sz * this->elem_size;
            for (; p != pe; p += this->elem_size, ++this->free_list_cur) {
                *this->free_list_cur = p;
            }
        }

        void * pop() {
            assert(this->free_list_cur != this->free_list.get());
            return *--this->free_list_cur;
        }

        void push(void * p) {
            *this->free_list_cur = p;
            ++this->free_list_cur;
        }
    };

    template<class T>
    class aligned_set_allocator
    : public std::allocator<T>
    {
    public:
        storage_value_set & storage;

        typedef typename std::allocator<T>::pointer pointer;
        typedef typename std::allocator<T>::size_type size_type;

        template<class U>
        struct rebind {
            typedef aligned_set_allocator<U> other;
        };

        aligned_set_allocator(storage_value_set & storage)
        : storage(storage)
        {
            this->storage.template update<T>();
        }

        aligned_set_allocator(aligned_set_allocator const & other)
        : std::allocator<T>(other)
        , storage(other.storage)
        {}

        template<class U>
        aligned_set_allocator(aligned_set_allocator<U> const & other)
        : std::allocator<T>(other)
        , storage(other.storage)
        {
            this->storage.template update<T>();
        }

        T * allocate(size_type /*n*/, std::allocator<void>::const_pointer /*hint*/ = 0)
        {
            return static_cast<T*>(this->storage.pop());
        }

        void deallocate(pointer p, size_type /*n*/)
        {
            this->storage.push(p);
        }
    };

    struct value_set
    {
        cache_element const & elem;

        value_set(cache_element const & elem)
        : elem(elem)
        {}

        bool operator<(value_set const & other) const {
            if (this->elem.bmp->cx < other.elem.bmp->cx) {
                return true;
            }
            else if (this->elem.bmp->cx > other.elem.bmp->cx) {
                return false;
            }

            if (this->elem.bmp->cy < other.elem.bmp->cy) {
                return true;
            }
            else if (this->elem.bmp->cy > other.elem.bmp->cy) {
                return false;
            }

            typedef std::pair<const uint8_t *, const uint8_t *> iterator_pair;
            const uint8_t * e = this->elem.sha1 + sizeof(this->elem.sha1);
            iterator_pair p = std::mismatch(this->elem.sha1 + 0, e, other.elem.sha1 + 0);
            return p.first == e ? false : *p.first < *p.second;
        }
    };

    class cache_range {
        cache_element * first;
        cache_element * last;

        typedef aligned_set_allocator<value_set> set_allocator;
        typedef std::less<value_set> set_compare;
        typedef std::set<value_set, set_compare, set_allocator> set_type;

        set_type sorted_elements;

    public:
        cache_range(cache_element * first, size_t sz, storage_value_set & storage)
        : first(first)
        , last(first + sz)
        , sorted_elements(set_compare(), storage)
        {}

        cache_element & operator[](size_t i) const {
            return this->first[i];
        }

        size_t size() const {
            return this->last - this->first;
        }

        void clear() {
            this->sorted_elements.clear();
            for (cache_element * p = this->first; p != this->last; ++p) {
                p->reset();
            }
        }

        static const uint32_t invalid_cache_index = 0xFFFFFFFF;

        inline uint16_t get_index(int_fast16_t cidx_default, int_fast16_t entries_max) const {
            int_fast32_t oldstamp = this->first->stamp;
            for (int_fast16_t cidx = 1; cidx < entries_max; ++cidx) {
                if (this->first[cidx].stamp < oldstamp) {
                    cidx_default = cidx;
                    oldstamp     = this->first[cidx].stamp;
                }
            }
            return cidx_default;
        }

        uint32_t get_cache_index(const cache_element & e) const {
            set_type::const_iterator it = this->sorted_elements.find(e);
            if (it == this->sorted_elements.end()) {
                return invalid_cache_index;
            }

            return &it->elem - this->first;
        }

        void remove(cache_element const & e) {
            this->sorted_elements.erase(e);
        }

        void add(cache_element const & e) {
            this->sorted_elements.insert(e);
        }

    private:
        cache_range(cache_range const &);
        cache_range& operator=(cache_range const &);
    };

public:
    struct CacheOption {
        uint16_t entries;
        uint16_t size;
        bool is_persistent;

        CacheOption(uint16_t entries = 0, uint16_t size = 0, bool is_persistent = false)
        : entries(entries)
        , size(size)
        , is_persistent(is_persistent)
        {}
    };

private:
    class Cache : public cache_range {
        uint16_t size_;
        bool is_persistent_;

    public:
        Cache(cache_element * pdata, const CacheOption & opt, storage_value_set & storage)
        : cache_range(pdata, opt.entries, storage)
        , size_(opt.size)
        , is_persistent_(opt.is_persistent)
        {}

        cache_range & range() {
            return static_cast<cache_range&>(*this);
        }

        const cache_range & range() const {
            return static_cast<const cache_range&>(*this);
        }

        uint16_t entries() const {
            return this->range().size();
        }

        uint16_t size() const {
            return this->size_;
        }

        bool persistent() const {
            return this->is_persistent_;
        }
    };

    struct Caches {
        Cache * caches[MAXIMUM_NUMBER_OF_CACHES + 1 /* wait_list */];

        Caches(Cache & c0, Cache & c1, Cache & c2, Cache & c3, Cache & c4, Cache & wait_list)
        {
            this->caches[0] = &c0;
            this->caches[1] = &c1;
            this->caches[2] = &c2;
            this->caches[3] = &c3;
            this->caches[4] = &c4;
            this->caches[5] = &wait_list;
        }

        Cache & operator[](size_t i) const {
            return *this->caches[i];
        }
    };


public:
    const enum Owner {
          Front
        , Mod_rdp
        , Recorder
    } owner;

private:
    const uint8_t bpp;

    const uint8_t number_of_cache;
    const bool    use_waiting_list;

    const unique_ptr<cache_element[]> elements;
    storage_value_set storage;

    Cache cache0;
    Cache cache1;
    Cache cache2;
    Cache cache3;
    Cache cache4;
    Cache cache_wait_list;

    Caches caches;

    bitmap_free_list_t bitmap_free_list;

          uint32_t stamp;
    const uint32_t verbose;

public:
    BmpCache(Owner owner,
             const uint8_t bpp,
             uint8_t number_of_cache,
             bool use_waiting_list,
             CacheOption c0 = CacheOption(),
             CacheOption c1 = CacheOption(),
             CacheOption c2 = CacheOption(),
             CacheOption c3 = CacheOption(),
             CacheOption c4 = CacheOption(),
             uint32_t verbose = 0)
    : owner(owner)
    , bpp(bpp)
    , number_of_cache(number_of_cache)
    , use_waiting_list(use_waiting_list)
    , elements(new cache_element[c0.entries + c1.entries + c2.entries + c3.entries + c4.entries + MAXIMUM_NUMBER_OF_CACHE_ENTRIES])
    , cache0(this->elements.get(), c0, this->storage)
    , cache1(this->elements.get() + c0.entries, c1, this->storage)
    , cache2(this->elements.get() + c0.entries + c1.entries, c2, this->storage)
    , cache3(this->elements.get() + c0.entries + c1.entries + c2.entries, c3, this->storage)
    , cache4(this->elements.get() + c0.entries + c1.entries + c2.entries + c3.entries, c4, this->storage)
    , cache_wait_list(this->elements.get() + c0.entries + c1.entries + c2.entries + c3.entries + c4.entries, CacheOption(MAXIMUM_NUMBER_OF_CACHE_ENTRIES), this->storage)
    , caches(this->cache0, this->cache1, this->cache2, this->cache3, this->cache4, this->cache_wait_list)
    , bitmap_free_list(c0.entries + c1.entries + c2.entries + c3.entries + c4.entries + MAXIMUM_NUMBER_OF_CACHE_ENTRIES)
    , stamp(0)
    , verbose(verbose)
    {
        this->storage.reserve(c0.entries + c1.entries + c2.entries + c3.entries + c4.entries + MAXIMUM_NUMBER_OF_CACHE_ENTRIES);

        if (this->verbose) {
            LOG( LOG_INFO
                , "BmpCache: %s bpp=%u number_of_cache=%u use_waiting_list=%s "
                    "cache_0(%u, %u, %s) cache_1(%u, %u, %s) cache_2(%u, %u, %s) "
                    "cache_3(%u, %u, %s) cache_4(%u, %u, %s)"
                , ((this->owner == Front) ? "Front" : ((this->owner == Mod_rdp) ? "Mod_rdp" : "Recorder"))
                , this->bpp, this->number_of_cache, (this->use_waiting_list ? "yes" : "no")
                , this->cache0.entries(), this->cache0.size(), (cache0.persistent() ? "yes" : "no")
                , this->cache1.entries(), this->cache1.size(), (cache1.persistent() ? "yes" : "no")
                , this->cache2.entries(), this->cache2.size(), (cache2.persistent() ? "yes" : "no")
                , this->cache3.entries(), this->cache3.size(), (cache3.persistent() ? "yes" : "no")
                , this->cache4.entries(), this->cache4.size(), (cache4.persistent() ? "yes" : "no")
                );
        }

        if (this->number_of_cache > MAXIMUM_NUMBER_OF_CACHES) {
            LOG( LOG_ERR, "BmpCache: %s number_of_cache(%u) > %u"
                , ((this->owner == Front) ? "Front" : ((this->owner == Mod_rdp) ? "Mod_rdp" : "Recorder"))
                , this->number_of_cache
                , MAXIMUM_NUMBER_OF_CACHES);
            throw Error(ERR_RDP_PROTOCOL);
        }
    }

    ~BmpCache() {
        if (this->verbose) {
            this->log();
        }
    }

    void reset() {
        if (this->verbose) {
            this->log();
        }
        this->stamp = 0;
        this->cache0.range().clear();
        this->cache1.range().clear();
        this->cache2.range().clear();
        this->cache3.range().clear();
        this->cache4.range().clear();
        this->cache_wait_list.range().clear();
        this->bitmap_free_list.clear();
    }

    void put(uint8_t id, uint16_t idx, const Bitmap * const bmp, uint32_t key1, uint32_t key2) {
        REDASSERT((id & IN_WAIT_LIST) == 0);
        if (idx == RDPBmpCache::BITMAPCACHE_WAITING_LIST_INDEX) {
            // Last bitmap cache entry is used by waiting list.
            //LOG(LOG_INFO, "BmpCache: Put bitmap to waiting list.");
            idx = MAXIMUM_NUMBER_OF_CACHE_ENTRIES - 1;
        }

        cache_range & r = this->caches[id].range();
        cache_element & e = r[idx];
        if (e) {
            r.remove(e);
            this->bitmap_free_list.push(e.bmp); //TODO only if free_list allocated
        }
        e.bmp = this->bitmap_free_list.pop(bmp->original_bpp, *bmp);
        e.stamp = ++this->stamp;

        e.bmp->compute_sha1(e.sha1);
        if (this->caches[id].persistent()) {
            REDASSERT(key1 && key2);
            e.sig.sig_32[0] = key1;
            e.sig.sig_32[1] = key2;
        }
        REDASSERT(this->caches[id].persistent() || (!key1 && !key2));
        r.add(e);
    }

    void restamp(uint8_t id, uint16_t idx) {
        REDASSERT((id & IN_WAIT_LIST) == 0);
        this->caches[id].range()[idx].stamp = ++this->stamp;
    }

    const Bitmap * get(uint8_t id, uint16_t idx) {
        if (id & IN_WAIT_LIST) {
            REDASSERT(this->owner != Mod_rdp);
            return this->caches[MAXIMUM_NUMBER_OF_CACHES].range()[idx].bmp;
        }
        if (idx == RDPBmpCache::BITMAPCACHE_WAITING_LIST_INDEX) {
            REDASSERT(this->owner != Front);
            // Last bitmap cache entry is used by waiting list.
            //LOG(LOG_INFO, "BmpCache: Get bitmap from waiting list.");
            idx = MAXIMUM_NUMBER_OF_CACHE_ENTRIES - 1;
        }
        return this->caches[id].range()[idx].bmp;
    }

    unsigned get_stamp(uint8_t id, uint16_t idx) {
        REDASSERT((id & IN_WAIT_LIST) == 0);
        return this->caches[id].range()[idx].stamp;
    }

    bool is_cache_persistent(uint8_t id) {
        if (id < MAXIMUM_NUMBER_OF_CACHES) {
            return this->caches[id].persistent();
        }
        if (id == MAXIMUM_NUMBER_OF_CACHES) {
            return true;
        }

        LOG( LOG_ERR, "BmpCache: %s index_of_cache(%u) > %u"
            , ((this->owner == Front) ? "Front" : ((this->owner == Mod_rdp) ? "Mod_rdp" : "Recorder"))
            , id, MAXIMUM_NUMBER_OF_CACHES);
        throw Error(ERR_RDP_PROTOCOL);
        return false;
    }

    uint16_t get_cache_usage(uint8_t cache_id) const {
        REDASSERT((cache_id & IN_WAIT_LIST) == 0);
        uint16_t cache_entries = 0;
        unsigned cache_index = 0;
        const cache_range & r = this->caches[cache_id].range();
        const unsigned last_index = this->caches[cache_id].persistent();
        for (; cache_index < last_index; ++cache_index) {
            if (r[cache_index]) {
                ++cache_entries;
            }
        }
        return cache_entries;
    }

    void log() const {
        LOG( LOG_INFO
            , "BmpCache: %s (0=>%u, %u%s) (1=>%u, %u%s) (2=>%u, %u%s) (3=>%u, %u%s) (4=>%u, %u%s)"
            , ((this->owner == Front) ? "Front" : ((this->owner == Mod_rdp) ? "Mod_rdp" : "Recorder"))
            , get_cache_usage(0), this->cache0.entries(), (this->cache0.persistent() ? ", persistent" : "")
            , get_cache_usage(1), this->cache1.entries(), (this->cache1.persistent() ? ", persistent" : "")
            , get_cache_usage(2), this->cache2.entries(), (this->cache2.persistent() ? ", persistent" : "")
            , get_cache_usage(3), this->cache3.entries(), (this->cache3.persistent() ? ", persistent" : "")
            , get_cache_usage(4), this->cache4.entries(), (this->cache4.persistent() ? ", persistent" : ""));
    }

private:
    struct Deleter {
        const Bitmap & r;
        bitmap_free_list_t & bmp_free_list;

        Deleter(const Bitmap & bmp, bitmap_free_list_t & bitmap_free_list)
        : r(bmp)
        , bmp_free_list(bitmap_free_list)
        {}

        void operator()(const Bitmap * bmp) const
        {
            if (bmp != &this->r) {
                this->bmp_free_list.push(bmp);
            }
        }
    };

public:
    TODO("palette to use for conversion when we are in 8 bits mode should be passed from memblt.cache_id, not stored in bitmap")
    uint32_t cache_bitmap(const Bitmap & oldbmp) {
        REDASSERT(this->owner != Mod_rdp);
        // Generating source code for unit test.
        //if (this->verbose & 8192) {
        //    if (this->finding_counter == 500) {
        //        BOOM;
        //    }
        //    LOG(LOG_INFO, "uint8_t palette_data_%d[] = {", this->finding_counter);
        //    hexdump_d((const char *)(void *)oldbmp.original_palette, sizeof(oldbmp.original_palette));
        //    LOG(LOG_INFO, "};", this->finding_counter);
        //    LOG(LOG_INFO, "uint8_t bitmap_data_%d[] = {", this->finding_counter);
        //    hexdump_d((const char *)(void *)oldbmp.data_bitmap.get(), oldbmp.bmp_size);
        //    LOG(LOG_INFO, "};", this->finding_counter);
        //    LOG(LOG_INFO, "memcpy(palette, palette_data_%d, sizeof(palette));", this->finding_counter);
        //    LOG(LOG_INFO, "init_palette332(palette);", this->finding_counter);
        //    LOG(LOG_INFO,
        //        "Bitmap * bmp_%d = new Bitmap(%u, %u, &palette, %u, %u, bitmap_data_%d, %u, false);",
        //        this->finding_counter, this->bpp, oldbmp.original_bpp, oldbmp.cx, oldbmp.cy,
        //        this->finding_counter, oldbmp.bmp_size);
        //}

        unique_ptr<const Bitmap, Deleter> bmp(
            (this->bpp == oldbmp.original_bpp
            || !((this->owner == Recorder) || (oldbmp.original_bpp > this->bpp)))
            ? &oldbmp
            : this->bitmap_free_list.pop(this->bpp, oldbmp)
            , Deleter(oldbmp, this->bitmap_free_list)
        );

        uint8_t        id_real  = 0;

        for (const uint32_t bmp_size = bmp->bmp_size; id_real < MAXIMUM_NUMBER_OF_CACHES; ++id_real) {
            if (this->caches[id_real].entries() && (bmp_size <= this->caches[id_real].size())) {
                break;
            }
        }

        uint16_t oldest_cidx = 0;

        uint16_t entries    = 0;
        bool     persistent = false;

        if (id_real == MAXIMUM_NUMBER_OF_CACHES) {
            LOG( LOG_ERR
                , "BmpCache: %s bitmap size(%u) too big: cache_0=%u cache_1=%u cache_2=%u cache_3=%u cache_4=%u"
                , ((this->owner == Front) ? "Front" : ((this->owner == Mod_rdp) ? "Mod_rdp" : "Recorder"))
                , bmp->bmp_size
                , (this->cache0.entries() ? this->cache0.size() : 0)
                , (this->cache1.entries() ? this->cache1.size() : 1)
                , (this->cache2.entries() ? this->cache2.size() : 2)
                , (this->cache3.entries() ? this->cache3.size() : 3)
                , (this->cache4.entries() ? this->cache4.size() : 4)
                );
            REDASSERT(0);
            throw Error(ERR_BITMAP_CACHE_TOO_BIG);
        }
        entries    = this->caches[id_real].entries();
        persistent = this->caches[id_real].persistent();
        if (this->use_waiting_list) {
            // Last bitmap cache entry is used by waiting list.
            entries--;
        }

        cache_element e_compare;
        e_compare.bmp = bmp.get();
        bmp->compute_sha1(e_compare.sha1);

        uint8_t   id     = id_real;
        cache_range & cache = this->caches[id].range();

        uint32_t cache_index_32 = cache.get_cache_index(e_compare);
        if (cache_index_32 == cache_range::invalid_cache_index) {
            oldest_cidx = cache.get_index(oldest_cidx, entries);
        }
        else {
            if (this->verbose & 512) {
                if (persistent) {
                    LOG( LOG_INFO
                        , "BmpCache: %s use bitmap %02X%02X%02X%02X%02X%02X%02X%02X stored in persistent disk bitmap cache"
                        , ((this->owner == Front) ? "Front" : ((this->owner == Mod_rdp) ? "Mod_rdp" : "Recorder"))
                        , e_compare.sha1[0], e_compare.sha1[1], e_compare.sha1[2], e_compare.sha1[3]
                        , e_compare.sha1[4], e_compare.sha1[5], e_compare.sha1[6], e_compare.sha1[7]);
                }
            }
            cache[cache_index_32].stamp = ++this->stamp;
            // Generating source code for unit test.
            //if (this->verbose & 8192) {
            //    LOG(LOG_INFO, "cache_id    = %u;", id);
            //    LOG(LOG_INFO, "cache_index = %u;", cache_index_32);
            //    LOG(LOG_INFO,
            //        "BOOST_CHECK_EQUAL(((BITMAP_FOUND_IN_CACHE << 24) | (cache_id << 16) | cache_index), "
            //            "bmp_cache.cache_bitmap(*bmp_%d));",
            //        this->finding_counter - 1);
            //    LOG(LOG_INFO, "delete bmp_%d;", this->finding_counter - 1);
            //    LOG(LOG_INFO, "");
            //}
            return (BITMAP_FOUND_IN_CACHE << 24) | (id << 16) | cache_index_32;
        }

        if (persistent && this->use_waiting_list) {
            // The bitmap cache is persistent.

            cache_range & cache_max = this->caches[MAXIMUM_NUMBER_OF_CACHES].range();

            cache_index_32 = cache_max.get_cache_index(e_compare);
            if (cache_index_32 == cache_range::invalid_cache_index) {
                oldest_cidx = cache_max.get_index(oldest_cidx, MAXIMUM_NUMBER_OF_CACHE_ENTRIES);
                id_real     =  MAXIMUM_NUMBER_OF_CACHES;
                id          |= IN_WAIT_LIST;

                if (this->verbose & 512) {
                    LOG( LOG_INFO, "BmpCache: %s Put bitmap %02X%02X%02X%02X%02X%02X%02X%02X into wait list."
                        , e_compare.sha1[0], e_compare.sha1[1], e_compare.sha1[2], e_compare.sha1[3]
                        , e_compare.sha1[4], e_compare.sha1[5], e_compare.sha1[6], e_compare.sha1[7]);
                }
            }
            else {
                cache_max.remove(e_compare);
                cache_max[cache_index_32].reset();

                if (this->verbose & 512) {
                    LOG( LOG_INFO
                        , "BmpCache: %s Put bitmap %02X%02X%02X%02X%02X%02X%02X%02X into persistent cache, cache_index=%u"
                        , e_compare.sha1[0], e_compare.sha1[1], e_compare.sha1[2], e_compare.sha1[3]
                        , e_compare.sha1[4], e_compare.sha1[5], e_compare.sha1[6], e_compare.sha1[7], oldest_cidx);
                }
            }
        }

        // find oldest stamp (or 0) and replace bitmap
        cache_element & e = this->caches[id_real].range()[oldest_cidx];
        if (e) {
            this->caches[id_real].remove(e);
            this->bitmap_free_list.push(e.bmp);
        }
        if (id_real == id) {
            ::memcpy(e.sig.sig_8, e_compare.sha1, sizeof(e.sig.sig_8));
        }
        ::memcpy(e.sha1, e_compare.sha1, 20);
        e.bmp = (&bmp.get_deleter().r == &oldbmp)
            ? this->bitmap_free_list.pop(oldbmp.original_bpp, oldbmp)
            : bmp.release();
        e.stamp = ++this->stamp;
        this->caches[id_real].add(e);
        // Generating source code for unit test.
        //if (this->verbose & 8192) {
        //    LOG(LOG_INFO, "cache_id    = %u;", id);
        //    LOG(LOG_INFO, "cache_index = %u;", oldest_cidx);
        //    LOG(LOG_INFO,
        //        "BOOST_CHECK_EQUAL(((BITMAP_ADDED_TO_CACHE << 24) | (cache_id << 16) | cache_index), "
        //            "bmp_cache.cache_bitmap(*bmp_%d));",
        //        this->finding_counter - 1);
        //    LOG(LOG_INFO, "delete bmp_%d;", this->finding_counter - 1);
        //    LOG(LOG_INFO, "");
        //}

        return (BITMAP_ADDED_TO_CACHE << 24) | (id << 16) | oldest_cidx;
    }
};

#endif
