#include<iostream>
using namespace std;

// 定长内存池（类，用ObjectPool创建的变量叫对象） , 定长对象的内存分配器
// 要有自由链表把申请的内存对象管理起来 , 有对象回收分配销毁功能
// 比如说这个链表指向一块回收的内存，又有内存还回来了，就用链表的形式管理(前一个对象把后一个对象的地址纯起来，就像next指针)  
// 这个思路和ijkplayer的对象池很像，只不过一个是任意对象的内存池，一个是AVPacket和AVFrame的对象池，实现对象的复用
template<class T>
class ObjectPool
{
public:
    ObjectPool()
    // : _memory(nullptr)           // 初始化列表, 在对象构造之前执行，是直接构造(初始化)
    // , _freelist(nullptr)
    {
        // _memory = nullptr;           // 构造函数体内赋值,在对象构造之后执行,先构造，再赋值
        // _freelist = nullptr;
        cout << "ObjectPool()" << endl;
    }
	T* New()
    {
        T* obj = nullptr;
        // 新申请的话优先从自由链表中取重复利于内存块
        if(_freelist != nullptr) // 头删
        {
            void *next = *(void**)_freelist;
            obj = _freelist;
            _freelist = next;
            return obj;
        }

        else 
        {
            // 剩余空间不够一个对象大小时，则重新开块大空间
            if (_remainBytes < sizeof(T))
            {
                _remainBytes = 1024 * 128;
                _memory = (char*)malloc(_remainBytes);// 在堆上分配内存空间
                if(_memory == nullptr) // 如果分配失败  抛出异常
                    throw bad_alloc(); // 抛出异常
            }
            obj = (T*)_memory;
            size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T); // 确保objSize足够存储下一个对象的地址
            _memory += objSize;
            _remainBytes -= objSize;
        }
        // 定位new, 在obj上构造对象,显式调用T的构造函数初始化对象
        new(obj) T();
        return obj;
    }
    void Delete(T* obj)
    {
        // 显式调用T的析构函数清理对象
        obj->~T();
        // 将对象加入自由链表（头插）
        if(_freelist == nullptr) // 如果自由链表为空
        {
            _freelist = obj; // 将对象加入自由链表
            //*(int*)obj = nullptr;
            // 关键操作：在对象的内存上直接存储指针// 直接用对象内存存储next// 不需要额外分配节点// 零额外开销！
            *(void**)obj = nullptr;  // 将对象的前4/8字节当作指针使用, 用来存储下一个对象的地址 (这和多态中的vptr很像即虚函数表指针)
        }
        else    // 如果自由链表不为空就头插
        {
            // 头插步骤如下，就是链表的头插步骤
            *(void**)obj = _freelist; 
            _freelist = obj;
        }
    }
    ~ObjectPool()
    {
        cout << "~ObjectPool()" << endl;
    }
private:
    char* _memory = nullptr; // 指向大块内存的指针
    size_t _remainBytes = 0; // 剩余的字节数
    void* _freelist = nullptr; // 还回来过程中链接回收对象的自由链表的头指针, 回收是头插，再利用是出队头
};