
#include "ChunkedList.hpp"

#ifdef DEBUGGING
#include <iostream>
#define DEBUG_LINE std::cout << std::endl;
#define DEBUG_LOG(value) std::cout << value;
#define DEBUG_EXECUTE(source) source
#else
#define DEBUG_LINE
#define DEBUG_LOG(value)
#define DEBUG_EXECUTE(source)
#endif

// ChunkedList::Chunk

template<typename T, size_t ChunkSize>
ChunkedList<T, ChunkSize>::Chunk::Chunk(Chunk *nextChunk, Chunk *prevChunk) : nextChunk(nextChunk),
                                                                              prevChunk(prevChunk) {}

template<typename T, size_t ChunkSize>
ChunkedList<T, ChunkSize>::Chunk::Chunk(const T *pointerStart, int range, Chunk *nextChunk, Chunk *prevChunk)
: nextChunk(nextChunk), prevChunk(prevChunk), nextIndex(range) {
  DEBUG_LOG("range: " << range << std::endl)
  for (int index = 0; index < range; ++index) {
    DEBUG_LOG(pointerStart[index] << ' ')
    data[index] = shouldCopy<T>() ? std::move(pointerStart[index]) : pointerStart[index];
  }
  DEBUG_LINE
}

template<typename T, size_t ChunkSize>
ChunkedList<T, ChunkSize>::Chunk::Chunk(const std::initializer_list<T> &initializerList) : nextIndex(
initializerList.size()) {
  for (int index = 0; index < initializerList.size(); ++index) {
    DEBUG_LOG(initializerList.begin()[index] << ' ')
    data[index] = shouldCopy<T>() ? std::move(initializerList.begin()[index]) : initializerList.begin()[index];
  }
  DEBUG_LINE
}

template<typename T, size_t ChunkSize>
ChunkedList<T, ChunkSize>::Chunk::Chunk() = default;

template<typename T, size_t ChunkSize>
ChunkedList<T, ChunkSize>::Chunk::~Chunk() = default;

template<typename T, size_t ChunkSize>
inline T &ChunkedList<T, ChunkSize>::Chunk::operator[](int index) {
  return data[index];
}

template<typename T, size_t ChunkSize>
inline const T &ChunkedList<T, ChunkSize>::Chunk::operator[](int index) const {
  return data[index];
}

// ChunkedList::Iterator

template<typename T, size_t ChunkSize>
typename ChunkedList<T, ChunkSize>::Iterator &ChunkedList<T, ChunkSize>::Iterator::operator++() {
  if (index == ChunkSize) {
    chunk = chunk->nextChunk;
    index = 1;
  } else {
    ++index;
  }
  return *this;
}

template<typename T, size_t ChunkSize>
typename ChunkedList<T, ChunkSize>::Iterator ChunkedList<T, ChunkSize>::Iterator::operator++(int) {
  Iterator original = *this;
  if (index == ChunkSize) {
    index = 1;
    chunk = chunk->nextChunk;
  } else {
    ++index;
  }
  return original;
}

template<typename T, size_t ChunkSize>
typename ChunkedList<T, ChunkSize>::Iterator &ChunkedList<T, ChunkSize>::Iterator::operator--() {
  if (index == 0) {
    index = ChunkSize - 1;
    chunk = chunk->prevChunk;
  } else {
    --index;
  }
  return *this;
}

template<typename T, size_t ChunkSize>
typename ChunkedList<T, ChunkSize>::Iterator ChunkedList<T, ChunkSize>::Iterator::operator--(int) {
  Iterator original = *this;
  if (index == 0) {
    index = ChunkSize - 1;
    chunk = chunk->prevChunk;
  } else {
    --index;
  }
  return original;
}

template<typename T, size_t ChunkSize>
inline T &ChunkedList<T, ChunkSize>::Iterator::operator*() {
  return (*chunk)[index];
}

template<typename T, size_t ChunkSize>
inline bool
operator==(typename ChunkedList<T, ChunkSize>::Iterator &x, typename ChunkedList<T, ChunkSize>::Iterator &y) {
  return x.chunk == y.chunk && x.index == y.index;
}

// ChunkedList

template<typename T, size_t ChunkSize>
void ChunkedList<T, ChunkSize>::pushChunk(ChunkedList::Chunk *chunk) {
  back->nextChunk = chunk;
  chunk->prevChunk = back;
  back = chunk;
}

template<typename T, size_t ChunkSize>
ChunkedList<T, ChunkSize>::ChunkedList() {
  front = back = new Chunk();
}

template<typename T, size_t ChunkSize>
ChunkedList<T, ChunkSize>::ChunkedList(const std::initializer_list<T> &initializerList) {
  if (initializerList.size() == 0) {
    front = back = new Chunk{};
    return;
  }
  
  DEBUG_LOG("Initializer list size: " << initializerList.size() << std::endl)
  
  DEBUG_EXECUTE({
                  for (int i = 0; i < initializerList.size(); ++i)
                    DEBUG_LOG(*(initializerList.begin() + i) << ' ');
                  DEBUG_LINE
                })
  
  if (ChunkSize >= initializerList.size()) {
    front = back = new Chunk{initializerList.begin(), static_cast<int>(initializerList.size())};
    return;
  }
  
  const int initializerListSizeModChunkSize = initializerList.size() % ChunkSize;
  chunkCount =
  initializerListSizeModChunkSize == 0 ? initializerList.size() / ChunkSize : initializerList.size() / ChunkSize + 1;
  front = back = new Chunk{initializerList.begin(), static_cast<int>(ChunkSize)};
  
  for (int index = 1; index < chunkCount - 1; ++index) {
    pushChunk(new Chunk{initializerList.begin() + index * ChunkSize, static_cast<int>(ChunkSize)});
  }
  
  pushChunk(new Chunk{
  initializerList.begin() + (chunkCount - 1) * ChunkSize,
  ChunkSize > initializerListSizeModChunkSize ? initializerListSizeModChunkSize : static_cast<int>(ChunkSize)
  });
}

template<typename T, size_t ChunkSize>
ChunkedList<T, ChunkSize>::~ChunkedList() {
  while (back)
    popChunk();
}

template<typename T, size_t ChunkSize>
T &ChunkedList<T, ChunkSize>::operator[](int index) {
  const int chunkIndex = index / ChunkSize;
  Chunk *chunk = front;
  
  for (int i = 0; i < chunkIndex; ++i)
    chunk = chunk->nextChunk;
  
  return chunk->operator[](index % ChunkSize);
}

template<typename T, size_t ChunkSize>
const T &ChunkedList<T, ChunkSize>::operator[](int index) const {
  const int chunkIndex = index / ChunkSize;
  Chunk *chunk = front;
  
  for (int i = 0; i < chunkIndex; ++i)
    chunk = chunk->nextChunk;
  
  return chunk->operator[](index % ChunkSize);
}

template<typename T, size_t ChunkSize>
typename ChunkedList<T, ChunkSize>::Iterator ChunkedList<T, ChunkSize>::begin() {
  return {back};
}

template<typename T, size_t ChunkSize>
typename ChunkedList<T, ChunkSize>::Iterator ChunkedList<T, ChunkSize>::end() {
  return {front, front->nextIndex - 1};
}

template<typename T, size_t ChunkSize>
typename ChunkedList<T, ChunkSize>::ChunkIterator ChunkedList<T, ChunkSize>::beginChunk() {
  return {back};
}

template<typename T, size_t ChunkSize>
typename ChunkedList<T, ChunkSize>::ChunkIterator ChunkedList<T, ChunkSize>::endChunk() {
  return {front + 1};
}

template<typename T, size_t ChunkSize>
void ChunkedList<T, ChunkSize>::pushBack(T value) {
  if (back->nextIndex == ChunkSize) {
    auto *nextChunk = new Chunk{value};
    nextChunk->prevChunk = back;
    back->nextChunk = nextChunk;
    back = nextChunk;
  } else {
    (*back)[back->nextIndex] = value;
    ++back->nextIndex;
  }
}

template<typename T, size_t ChunkSize>
void ChunkedList<T, ChunkSize>::emplace(const T &value) {
  if (back->nextIndex == ChunkSize) {
    auto *nextChunk = new Chunk{value};
    nextChunk->prevChunk = back;
    back->nextChunk = nextChunk;
    back = nextChunk;
  } else {
    back[back->nextIndex] = std::move(value);
    ++back->nextIndex;
  }
}

template<typename T, size_t ChunkSize>
void ChunkedList<T, ChunkSize>::pop() {
  if (back->nextIndex == 0)
    popChunk();
  else
    --back->nextIndex;
}

template<typename T, size_t ChunkSize>
void ChunkedList<T, ChunkSize>::popChunk() {
  Chunk* newBack = back->prevChunk;
  delete back;
  back = newBack;
}

#define RET_SIZE return (chunkCount * ChunkSize) + back->nextIndex - 1;

template<typename T, size_t ChunkSize>
size_t ChunkedList<T, ChunkSize>::length() const { RET_SIZE }

template<typename T, size_t ChunkSize>
size_t ChunkedList<T, ChunkSize>::size() const { RET_SIZE }

#undef RET_SIZE

template<typename T, size_t ChunkSize>
std::ostream &operator<<(std::ostream &os, ChunkedList<T, ChunkSize> &chunkedList) {
  DEBUG_EXECUTE(os << "Chunked List: ";)
  os << '[';
  DEBUG_EXECUTE({ os << '\n'; })
  auto *chunk = chunkedList.front;
  
  while (chunk != nullptr) {
    DEBUG_LOG("Next index: " << chunk->nextIndex << '\n')
    
    for (int i = 0; i < chunk->nextIndex; ++i)
      os << ' ' << (*chunk)[i] << ',';
    
    chunk = chunk->nextChunk;
    DEBUG_EXECUTE({ os << '\n'; })
  }
  
  os << " ]";
  
  return os;
}
