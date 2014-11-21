    jsoncons::json::any

`any` can contain any value that supports copy construction and assignment.

### Constructors

    any()

    any(const any& val)

    any(any&& val)

    template<typename T>
    explicit any(T val, typename std::enable_if<!std::is_same<any, typename std::decay<T>::type>::value,int>::type* = 0)

### Destructor

    ~any()

### Assignment operator

    any& operator=(any rhs)
Supports copy and move assignment

### Accessors
    
    template <typename T>
    const T& cast() const

    template <typename T>
    T& cast() 


