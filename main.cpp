#include <iostream>

using namespace std;

// This is a trick to print out the deduced type of T
template<typename T>
void f(T&& parameter);  // purposefully not defined

template<typename T>
void f2(const T& parameter);  // purposefully not defined


class Any
{
private:
    void* data_;
    //const std::type_info& type_; // we can store the type of data using the type_info. But type_info is non-copyable / non-moveable, so it's not enough

    // Syntax for function ptr: return type (* func_name)(params)
    const std::type_info& (*getType_)(); // function ptr to get the data type. The return type of typeid has to be const type_info& 
    void* (*clone_)(void* data); // copy the data to data_, when we call copy constructor 
    void (*destroy_)(void* data); // delete the data in the destructor, but we don't really know the 


public:

    // Deal with lvalue. But cannot have const T& otherwise the compiler will try to use the T&& constructor instead...
    template <typename T>
    Any(T& value)
        : data_(new T(value)) // when we have universal reference, we should use perfect forwarding
        , getType_([]()-> const std::type_info& {return typeid(T); }) // has to specify the return type of the lambda 
        , clone_([](void* data)->void* {return new T(*static_cast<T*>(data)); }) // cast incoming data to our type T
        , destroy_([](void* data)->void {delete static_cast<T*>(data); }) // call destructor to destory the data. You can call static_cast<T*>(data)->~T() as well. What's the difference? 
    {
    }

    // We can only take rvalue here. lvalue won't work as T will be deduced to T&, and new T& is not allowed 
    template <typename T>
    Any(T&& value)
        : data_(new T(std::forward<T>(value))) // whne we have universal reference, we should use perfect forwarding
        , getType_([]()-> const std::type_info& {return typeid(T); }) // has to specify the return type of the lambda 
        , clone_([](void* data)->void* {return new T(*static_cast<T*>(data)); }) // cast incoming void* data to our type T* data
        , destroy_([](void* data)->void {delete static_cast<T*>(data); })
    {
    }

    ~Any()
    {
        destroy_(data_);
    }

    template <typename T>
    T& get()
    {
        // Check if the type is the same, and cast it back to its real type 
        if (typeid(T) == getType_())
        {
            return *static_cast<T*>(data_);
        }
        else
        {
            std::cout << "bad cast \n";
            throw std::bad_cast();
        }
    }

    // Copy constructor with template specification. Otherwise, Any is also a template T, so it will go into the regular constructor and recursive forever
    template <>
    Any(Any& a)
        : data_(a.clone_(a.data_)) // new a copy of a's data and cast it into void, then copy to our data_
        , getType_(a.getType_)
        , clone_(a.clone_)
        , destroy_(a.destroy_)
    {
    }

    // Same thing for the move constructor 
    template <>
    Any(Any&& a)
        : data_(a.data_) // Just take the value from a. Don't forget to assign a's data_ to nullptr
        , getType_(a.getType_)
        , clone_(a.clone_)
        , destroy_(a.destroy_)
    {
        a.data_ = nullptr;
        std::cout << "MOVE \n";
    }

    Any& operator=(Any& a)
    {
        data_ = a.clone_(a.data_); // new a copy of a's data and cast it into void, then copy to our data_
        getType_ = a.getType_;
        clone_ = a.clone_;
        destroy_ = a.destroy_;

        return *this;
    }

    Any& operator=(Any&& a)
    {
        data_ = a.data_; // Take the value from a
        a.data_ = nullptr;
        getType_ = a.getType_;
        clone_ = a.clone_;
        destroy_ = a.destroy_;

        return *this;
    }
};

struct Test
{
    Test(int i, int j) : x(i), y(j){};
    int x = 0;
    int y = 0;
};

int main()
{
    Any b{5};
    Any a{ std::move(b) };

    std::cout << a.get<int>() << "\n";

    string s = "hello";
    Any c{ s };
    a = c;

    Any d{ Test(1,2) };

    Any e = d;

    std::cout << a.get<string>() << "\n";
    std::cout << e.get<Test>().x << "\n";


	return 0;
}