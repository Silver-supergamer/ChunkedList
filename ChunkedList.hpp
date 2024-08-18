#pragma once

#include <vector>
#include <utility>
#include <ostream>
#include <initializer_list>

template<typename T>
inline constexpr bool shouldCopy = sizeof(T) > sizeof(void *);

template<typename T, size_t ChunkSize = 16>
class ChunkedList {
    static_assert(ChunkSize > 0, "Chunk Size must be greater than 0");
  private:
    size_t chunkCount = 1;
    
    class Chunk {
      private:
        T data[ChunkSize];
      public:
        Chunk(Chunk *nextChunk, Chunk *prevChunk);
        
        explicit Chunk(const T *pointerStart, int range, Chunk *nextChunk = nullptr, Chunk *prevChunk = nullptr);
        
        Chunk(const std::initializer_list<T> &initializerList);
        
        Chunk();
        
        ~Chunk();
        
        int nextIndex = 0;
        
        Chunk *nextChunk = nullptr;
        Chunk *prevChunk = nullptr;
        
        inline T &operator[](int index);
        
        inline const T &operator[](int index) const;
    };
    
    class Iterator {
      private:
        Chunk *chunk = nullptr;
        int index = 0;
      public:
        ~Iterator() = default;
        
        Iterator &operator++();
        
        Iterator operator++(int);
        
        Iterator &operator--();
        
        Iterator operator--(int);
        
        inline T &operator*();
        
        template<typename, size_t>
        friend inline bool operator==(Iterator &x, Iterator &y);
    };
    
    using ChunkIterator = const Chunk *;
    
    Chunk *front = nullptr;
    Chunk *back = nullptr;
    
    inline void pushChunk(Chunk *chunk);
  
  public:
    ChunkedList();
    
    ChunkedList(const std::initializer_list<T> &initializerList);
    
    ~ChunkedList();
    
    T &operator[](int index);
    
    const T &operator[](int index) const;
    
    Iterator begin();
    
    Iterator end();
    
    ChunkIterator beginChunk();
    
    ChunkIterator endChunk();
    
    void pushBack(T value);
    
    void emplace(const T &value);
    
    void pop();
    
    void popChunk();
    
    [[nodiscard]] size_t length() const;
    
    [[nodiscard]] size_t size() const;
    
    template<typename, size_t>
    friend std::ostream &operator<<(std::ostream &os, ChunkedList &chunkedList);
};
