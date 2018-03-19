
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
//        rgOptionalRef<Item>  GetItem(int i) {
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
class rgOptionalRef
{
public:
    rgOptionalRef()       : m_good(false), m_ref(ms_emptyObj) {}
    rgOptionalRef(bool b) : m_good(false), m_ref(ms_emptyObj) { assert(b == false); }
    rgOptionalRef(T& ref) : m_ref(ref), m_good(true) {}
    inline rgOptionalRef& operator=(T& ref) { m_ref = ref; m_good = true; return *this; }
    inline operator bool() { return m_good; }
    inline T& operator*()  { return m_ref.get(); }
    inline T* operator->() { return &m_ref.get(); }
    inline T& get()        { return m_ref.get(); }
private:
    std::reference_wrapper<T>  m_ref;
    bool                       m_good;
    static T                   ms_emptyObj;
};

template<typename T> WEAK_LINK
T rgOptionalRef<T>::ms_emptyObj;
