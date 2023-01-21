#pragma once
#include <memory>
#include <utility>
#include <type_traits>
#include <map>

/**
 * Provides a pool of objects that are each identified by a key. Requesting an
 * object of a certain key will either return a handle to the existing object,
 * or should there be no existing object, to a newly constructed one. Once
 * there are no more handles in use the object is destroyed. The storage for
 * the objects must be provided by the user (note this means duplicate keys can
 * exist globally, being associated with different storage).
 */

/**
 * Storage class for objects
 * K - key type
 * T - stored or mapped type
 */
template<class K, class T>
using KeyedSharedStorage = std::map<K,std::weak_ptr<T>>;

/**
 * Handle to object
 */
template<class T>
using KeyedSharedHdl = std::shared_ptr<T>;

/**
 * Deleter for objects - responsible for deleting the object from
 * the storage (this will occur when the last handle to the object
 * is destoryed)
 */
template<class K, class T>
struct KeyedSharedClean {
  
  KeyedSharedStorage<K,T>* storage;
  typename KeyedSharedStorage<K,T>::const_iterator element;
 
  void operator ()(T* value) const {
    storage->erase(element);
    delete value;
  }
};
// 
/**
 * Returns the handle to the object identified by key in storage if it already exists,
 * else creates an object of type T from val, using key to identify it, adds it to storage
 * and finally returns a handle to it.
 * XXX SHALL: if equiv(k,k2) then equiv(k(k2),k2); SHOULD: comparing k with k2 is efficient
 * K2 - Type convertible to K
 * T2 - Type convertible to T
 */
template<class K2, class T2, class K, class T>
[[nodiscard]]
KeyedSharedHdl<T> KeyedSharedInsert(K2&& key, T2&& val, KeyedSharedStorage<K,T>& storage) {

  auto it = storage.lower_bound(key);

  if (it == storage.end() || storage.key_comp()(key, it->first)) {

    auto ptr = std::make_unique<T>(std::forward<T2>(val));
    it = storage.emplace_hint(it, std::forward<K2>(key), std::weak_ptr<T>{}); 
    auto deleter = KeyedSharedClean<K,T>{ &storage, it };
    auto shared  = std::shared_ptr<T>{ ptr.release(), deleter };
    it->second = shared;
    return shared;
  }
  else {
    return it->second.lock();
  }
}
