// The MIT License (MIT)
// Copyright (c) 2014 Mohammad Dashti
// (mohammad.dashti [at] epfl.ch - mdashti [at] GMail)
#define DEFAULT_CHUNK_SIZE 1024
#define DEFAULT_FORCE_CLEAR true
template<typename T>
class Pool
{
private:
    typedef union __El
    {
        __El() {};
        T obj;
        union __El *next;
        ~__El() {};
    } El;
    El *free_;
    El *data_;
    size_t size_;
    bool forceClear;
    void add_chunk()   // precondition: no available elements
        {
            size_ = size_ << 1;
            El *chunk = new El[size_ + 1];
            for (size_t i = 0; i < size_ - 1; ++i)
                chunk[i].next = &chunk[i + 1];
            chunk[size_ - 1].next = nullptr;
            chunk[size_].next = data_;
            data_ = chunk;
            free_ = chunk;
        }
public:
    Pool(size_t chunk_size = DEFAULT_CHUNK_SIZE) : data_(nullptr), size_(chunk_size >> 1), forceClear(false)
        {
            add_chunk();
        }
    ~Pool()
        {
            size_t sz = size_;
            while (data_ != nullptr)
            {
                El *el = data_[sz].next;
                delete[] data_;
                data_ = el;
                sz = sz >> 1;
            }
        }
    inline T *add()
        {
            if (!free_)
            {
                forceClear = true;
                add_chunk();
            }
            El *el = free_;
            free_ = free_->next;
            return &(el->obj);
        }
    inline void del(T *obj)
        {
            ((El *)obj)->next = free_;
            free_ = (El *)obj;
        }
    void clear(bool force = DEFAULT_FORCE_CLEAR)
        {
            if (force || forceClear)
            {
                El *prevChunk = nullptr;
                El *chunk = data_;
                size_t sz = size_;
                size_t doubleSz = sz << 1;
                while (chunk)
                {
                    if (prevChunk)
                    {
                        prevChunk[doubleSz - 1].next = chunk;
                    }
                    for (size_t i = 0; i < sz - 1; ++i)
                        chunk[i].next = &chunk[i + 1];
                    chunk[sz - 1].next = nullptr; // did not change
                    prevChunk = chunk;
                    chunk = chunk[sz].next;
                    doubleSz = sz;
                    sz = sz >> 1;
                }
                free_ = data_;
                forceClear = false;
            }
        }
};
