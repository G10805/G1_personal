#ifndef SINGLETON_H
#define SINGLETON_H
template <typename T>
class Singleton {
 private:
    static T* sInstance;

 public:
    static T* getInstance() { return sInstance; }
    Singleton(const T&) = delete;
    Singleton(T&&) = delete;
    void operator=(const T&) = delete;

 protected:
    Singleton() = default;
};
template <typename T>
T* Singleton<T>::sInstance = new T;
#endif
