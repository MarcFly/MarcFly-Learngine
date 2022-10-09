#ifndef MFLY_SLOTMAP
#define MFLY_SLOTMAP

#include <vector>
#include <mutex> // Should only lock when resizing indices
#include <shared_mutex>
#include <optional> // Check if things are there or not
// Include spans?

namespace mfly {
    // TODO: structure that groups in constant groups (paged vector?)

    struct sm_key {
        const uint32_t own_index;
        uint32_t data_index;
        uint32_t version = 0;
        const uint32_t tags;
    };

    template<class T>
    class slotmap {
        private:
        std::shared_mutex index_mtx;
        std::vector<sm_key> indices; // We do not care, after certain time, indices will be filled with holes :D
        
        std::shared_mutex data_mtx;
        std::vector<T> data;
        std::vector<uint32_t> data_to_index;

        public:
        slotmap<T>(uint32_t minsize = 4096, uint32_t pagesize = 4096) {
            indices.reserve(minsize);
            data.reserve(minsize);
            data_to_index.reserve(minsize);
        };

        ~slotmap<T>() {clear();};

        // Basic data structure operations:
        slotmap<T>(const slotmap<T>& copy) {
            indices.resize(copy.indices.size());
            memcpy(indices.data(), copy.indices.data(), copy.indices.size() * sizeof(sm_key));

            data.resize(copy.data.size());
            memcpy(data.data(), copy.data.data(), copy.data.size()*sizeof(T));

            data_to_index(copy.data_to_index.size());
            memcpy(data_to_index.data(), copy.data_to_index.data(), copy.data_to_index.size()*sizeof(uint32_t));
        }

        slotmap<T>(slotmap<T>&& move) {
            std::move(move.indices.begin(), move.indices.end(), indices);
            std::move(move.data.begin(), move.data.end(), data);
            std::move(move.data_to_index.begin(), move.data_to_index.end(), data_to_index);
        }

        sm_key push(T data_in, uint32_t tags = 0) {
            sm_key ret{
                .own_index = indices.size(),
                .data_index = data.size(),
                .version = 0,
                .tags = tags
            };

            data.push_back(data_in);
            indices.push_back(ret);
            data_to_index.push_back(ret.own_index);
            return ret;
        }  

        bool has_key(sm_key key) { return key.own_index > indices.size()-1; }

        bool erase (sm_key key) {
            if(key.own_index > indices.size()-1) return false;

            indices[key.own_index].version = UINT32_MAX;

            {
                std::shared_lock<std::shared_mutex> s_lock(data_mtx);
                uint32_t data_last = data.size()-1;
                data[key.data_index] = data[data_last];
                data_to_index[key.data_index] = key.own_index;
            }
            
            std::unique_lock<std::shared_mutex> lock(data_mtx);
            data.pop_back();
            data_to_index.pop_back();

            return true;
        }

        
        std::optional<T> pop(sm_key key) {
            std::optional<T> ret;
            
            if(key.own_index > indices.size()-1) return ret;

            indices[key.own_index].version = UINT32_MAX;
            ret = data[key.data_index];

            // check data for reads
            uint32_t data_last = data.size()-1;
            data[key.data_index] = data[data_last];
            data_to_index[key.data_index] = key.own_index;

            // Lock data for write
            data.pop_back();
            data_to_index.pop_back();

            return ret;
        }

        T& operator[](sm_key& key) {
            sm_key& curr = indices[key.own_index];
            key.version = ++curr.version; // assume we are chaning data if asked for it

            return data[curr.data_index];            
        }

        const T& operator[](sm_key& key) const {
            const sm_key& curr = indices[key.own_index];
            key.version = curr.version;
            return data[curr.data_index];
        }

        uint64_t size() { return data.size(); }
        
        void clear() {
            indices.clear();
            data.clear();
            data_to_index.clear();
        }
    };

};

#endif