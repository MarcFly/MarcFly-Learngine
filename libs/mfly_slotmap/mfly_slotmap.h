#ifndef MFLY_SLOTMAP
#define MFLY_SLOTMAP

#include <vector>
#include <mutex> // Should only lock when resizing indices
#include <shared_mutex>
#include <optional> // Check if things are there or not
// Include spans?

// Make the key templated too?
// Would solve some ambiguity issues

namespace mfly {
    // TODO: structure that groups in constant groups (paged vector?)

    // pagesize for number of items
    template<typename T, size_t pagesize>
    struct paged_vec {
        private:
        size_t capacity; // in number of pages
        size_t size;
        T** data;
        public:

        void reserve(size_t num_items) {
            size_t num_pages = (num_items / pagesize) + 1;
            if(num_pages <= capacity) return;
            T** new_alloc = new T*[num_pages];
            for(int i = 0; i < capacity; ++i)
                new_alloc[i] == data[i];
            for(int i = capacity; i < num_pages; ++i)
                new_alloc[i] = new T[pagesize*sizeof(T)];

            T** temp = data;
            data = new_alloc;
            delete[] temp;
        }

        inline size_t calcpage(const size_t& pos) {return pos%pagesize;}

        T& push(T& data) {
            if(size == capacity*pagesize) reserve(capacity+1); // reserve one page if needed
            size_t page = size%pagesize;
            data[page][size - page*pagesize] = data;
            ++size;
        }

        T& operator[](const size_t pos) {return data[calcpage(pos)][pos - calcpage(pos)*pagesize];}


    };
    
    struct sm_key {
        uint32_t own_index = UINT32_MAX;
        uint32_t data_index = UINT32_MAX;
        uint32_t version = 0;
        uint32_t tags = UINT32_MAX;
    };

    template<class T, size_t pagesize>
    class slotmap {
        private:
        std::shared_mutex index_mtx;
        paged_vec<sm_key,pagesize> indices; // We do not care, after certain time, indices will be filled with holes :D
        
        std::shared_mutex data_mtx;
        paged_vec<T,pagesize> _data;
        paged_vec<uint32_t, pagesize> _data_to_index;

        public:
        //typedef T* iterator;
        //typedef T value_type;

        slotmap<T, pagesize>(uint32_t minsize = 4096, uint32_t pagesize = 4096) {
            indices.reserve(minsize);
            _data.reserve(minsize);
            _data_to_index.reserve(minsize);
        };

        ~slotmap<T>() {clear();};

        // Basic _data structure operations:
        slotmap<T>(const slotmap<T>& copy) {
            indices.resize(copy.indices.size());
            memcpy(indices.data(), copy.indices.data(), copy.indices.size() * sizeof(sm_key));

            _data.resize(copy._data.size());
            memcpy(_data.data(), copy._data.data(), copy._data.size()*sizeof(T));

            _data_to_index.resize(copy._data_to_index.size());
            memcpy(_data_to_index.data(), copy._data_to_index.data(), copy._data_to_index.size()*sizeof(uint32_t));
        }

        slotmap<T>(slotmap<T>&& move) {
            std::move(move.indices.begin(), move.indices.end(), indices);
            std::move(move._data.begin(), move._data.end(), _data);
            std::move(move._data_to_index.begin(), move._data_to_index.end(), _data_to_index);
        }

        sm_key push(T _data_in, uint32_t tags = 0) {
            sm_key ret{
                .own_index = (uint32_t)indices.size(),
                .data_index = (uint32_t)_data.size(),
                .version = 0,
                .tags = tags
            };

            _data.push_back(_data_in);
            indices.push_back(ret);
            _data_to_index.push_back(ret.own_index);
            return ret;
        }  
        sm_key push_back(T _data_in, uint32_t tags = 0) {return push(_data_in, tags);}

        T& insert(sm_key& k, T _data_in) {
            if(!has_key(k)) k = push(_data_in, k.tags);
            else (*this)[k] = _data_in;
            return (*this)[k];
        }

        inline bool has_key(sm_key key) { 
            return int64_t(key.own_index) < int64_t(indices.size()); 
        }

        bool erase (sm_key key) {
            if(!has_key(key)) return false;

            indices[key.own_index].version = UINT32_MAX;

            {
                std::shared_lock<std::shared_mutex> s_lock(data_mtx);
                uint32_t _data_last = _data.size()-1;
                _data[key.data_index] = _data[_data_last];
                _data_to_index[key.data_index] = key.own_index;
            }
            
            std::unique_lock<std::shared_mutex> lock(data_mtx);
            _data.pop_back();
            _data_to_index.pop_back();

            return true;
        }

        
        std::optional<T> pop(sm_key key) {
            std::optional<T> ret;
            
            if(key.own_index > indices.size()-1) return ret;

            indices[key.own_index].version = UINT32_MAX;
            ret = _data[key.data_index];

            // check _data for reads
            uint32_t _data_last = _data.size()-1;
            _data[key.data_index] = _data[_data_last];
            _data_to_index[key.data_index] = key.own_index;

            // Lock _data for write
            _data.pop_back();
            _data_to_index.pop_back();

            return ret;
        }

        T& operator[](sm_key& key) {
            sm_key& curr = indices[key.own_index];
            key.version = ++curr.version; // assume we are chaning _data if asked for it

            return _data[curr.data_index];            
        }

        const T& operator[](sm_key& key) const {
            const sm_key& curr = indices[key.own_index];
            key.version = curr.version;
            return _data[curr.data_index];
        }

        T& operator[](const uint64_t index) {
            ++indices[_data_to_index[index]].version;
            return _data[index];
        }

        sm_key GetKeyAtIDX(const uint64_t index) {
            return indices[_data_to_index[index]];
        }

        //std::vector<T>::iterator begin() { return _data.begin();}
        //std::vector<T>::iterator end() { return _data.end();}
        
        T* back() {return _data[data.size()-1];}
        
        void from_handles(std::vector<sm_key>& keys, std::vector<T>& out) {
            out.clear();
            for(sm_key& k : keys)
                out.push_back(_data[k.data_index]);
        }

        T* data() {return _data.data();}

        uint64_t size() { return _data.size(); }
        
        void clear() {
            indices.clear();
            _data.clear();
            _data_to_index.clear();
        }
    };


    typedef std::unique_lock<std::shared_mutex> ulock;
    typedef std::shared_lock<std::shared_mutex> shrlock;

    // Scoped usage of data
    template<typename T>
    struct handle_user {
        handle_user(T& _ref, const shrlock& _slotmap_readlock, const ulock& _data_writelock) :
            data(_ref), slotmap_readlock(_slotmap_readlock), data_writelock(_data_writelock) {}
        T& data;
        const shrlock slotmap_readlock;
        const ulock data_writelock;
    };

    // Handle should know about a slotmap
    template<typename T>
    struct sm_handle {
        std::shared_ptr<sm_key> key;
        T* data; // How to know when it is invalidated?
        std::shared_mutex use; // Copy of the mutex from the slotmap
        std::
        typedef std::pair<std::shared_lock<std::shared_mutex>, T&> handle_user;
        handle_user Use() { // Lock the vector from resizing, when out of scope, data is not longer safe to change
            ++key.version;
            return handle_user(std::shared_lock<std::shared_mutex>(use), *data);
        }
    };

    void example() {
        std::shared_mutex mtx;
        std::shared_lock<std::shared_mutex> lock(mtx);
        
    }

};

#endif