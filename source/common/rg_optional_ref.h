
#ifdef _WIN32
#define WEAK_LINK __declspec(selectany)
#else
#define WEAK_LINK __attribute__((weak))
#endif

// This is a "poor man's" implementation of c++ "optional" concept for references.
// It is useful when you need to return a reference that can be "empty".
// Example:
//
//    class Item { public: void bar() {} };
//
//    class Vector
//    {
//    public:
//        RgOptionalRef<Item>  GetItem(int i) {
//            if (i >= m_items.size())
//                return false;
//            else
//                return m_items[i];
//        }
//    private:
//        std::vector<Item>  m_items;
//    };
//
//    void foo(Vector& v, int n)
//    {
//        if (auto item = v.GetItem(n)) {
//            item->bar();
//        }
//    }
//
// Notes:
//   1. This class uses static "empty" object of class T, so use it carefully when sizeof(T) is huge.
//   2. If your compiler supports c++17, consider using std::optional instead.
//
template<typename T>
class RgOptionalRef
{
public:
    RgOptionalRef()       : good_(false), ref_(ms_empty_obj_) {}
    RgOptionalRef(bool b) : good_(false), ref_(ms_empty_obj_) { assert(b == false); }
    RgOptionalRef(T& ref) : ref_(ref), good_(true) {}
    inline RgOptionalRef& operator=(T& ref) { ref_ = ref; good_ = true; return *this; }
    inline operator bool() { return good_; }
    inline T& operator*()  { return ref_.get(); }
    inline T* operator->() { return &ref_.get(); }
    inline T& get()        { return ref_.get(); }
private:
    std::reference_wrapper<T>  ref_;
    bool                       good_;
    static T                   ms_empty_obj_;
};

template<typename T> WEAK_LINK
T RgOptionalRef<T>::ms_empty_obj_;
