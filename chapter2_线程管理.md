
本章主要内容：

- 启动新线程
- 等待线程与分离线程
- 线程唯一标识符

### 2.1 线程管理基础

每个程序至少有一个线程: 执行 main（） 函数的线程，其余线程有各自的入口函数。 线程执行完入口函数后，线程也会推出。

####2.1.1 启动线程

启动线程可以使用如下语法：

    void do_some_work();
    std::thread thread_name(do_some_work);

也可以使用可调用类型构造，将带有函数调用符类型的实例传入 srd::thread 类中，替换默认的构造函数。

    class MyClass
    {
    public:
    	void operator()() const
    	{
    		do_some_thing();
    		do_some_thing_else();
    	}
    };
    
    MyClass f;
    std::thread thread_name(f);

**注意事项：** 这里如果你传入的是一个临时变量，而不是一个命名的变量，C++编译器会将其解析为函数声明，而不是类型对象的定义。

如： 

    std::thread thread_name(Myclass());

这里相当与声明了一个名为thread_name的函数，这个函数带有一个参数(函数指针指向没有参
数并返回 Myclass 对象的函数)，返回一个 std::thread 对象的函数，而非启动了一个线程。

上述问题解决办法：

- 使用多组括号或者使用新的初始化方法
 
 <pre name=code>
   std::thread thread_name ((Myclass()));
   std::thread thread_name{Myclass()};
</pre>
- 使用 lamda 表达式

lamda 表达式允许使用一个可以捕获局部变量的局部函数（可以避免传递参数）

    std::thread thread_name([]{
					do_something();
				    do_something_else();
					});


启动线程以后需要注意的问题是： 你需要明确是要等待线程结束（joined），还是让其自主运行（detached）。 若是选择自主运行，则必须保证线程结束前，可访问数据的有效性。 

反例见下：

    struct func
    {
    	int &i;
    	func(int &i_) : i(i_){}
    
    	void operator()()
    	{
    		for (unsigned j = 0; j < 100000; j++)
    		{
    			do_something(i);
    		}
    	}
    };
    
    void oops()
    {
    	int locate_state = 0;
    	func my_func(locate_state);
    	std::thread thread_name(my_func);
    	thread_name.detach();  // 不等待线程结束
    }

这里选择不等待线程结束，当新的线程 thread_name 启动以后，oops（）函数继续运行，后面不进行其他操作，所以很快就会结束，而 thread_name 线程里通过多次调用  `do_something(i)` 可能还在运行，这时局部变量 `locate_state` 就会被销毁，再掉用可能就会出错。

####2.1.2 等待线程完成

这里就是调用` thread_name.join（）`,就可以确保局部变量在线程完成后，才被销毁。 其是简单粗暴的等待线程完成或不等待，当需要对等待中的线程有更灵活的控制时，例如，查看某个线程是否结束，或者只等待一段时间，就需要其他机制来完成，比如 条件变量和期待（future）。只能对抑恶个线程使用一次 `join()`

####2.1.3 特殊情况下的等待

####2.1.4 后台运行线程

使用 detach() 会让线程在后台运行，这就意味着主线程不能与之产生直接交互，所以分离线程不能被加入(再调用 join())。所以可用同样的方式进行检查——当 std::thread 对象使用t.joinable()返回的是true，就可以使用t.detach()。

试想如何能让一个文字处理应用同时编辑多个文档。无论是用户界面，还是在内部应用内部
进行，都有很多的解决方法。虽然，这些窗口看起来是完全独立的，每个窗口都有自己独立
的菜单选项，但他们却运行在同一个应用实例中。一种内部处理方式是，让每个文档处理窗
口拥有自己的线程；每个线程运行同样的的代码，并隔离不同窗口处理的数据。如此这般，
打开一个文档就要启动一个新线程。因为是对独立的文档进行操作，所以没有必要等待其他
线程完成。因此，这里就可以让文档处理窗口运行在分离的线程上。

代码展示上述文档处理方法:

    void edit_document(std::string const& filename)
    {
    	open_document_and_display_gui(filename);
    	while (!done_editing())
    	{
    		user_command cmd = get_user_input();
    		if (cmd.type == open_new_document)
    		{
    			std::string const new_name = get_filename_from_user();
    			std::thread t(edit_document, new_name);  // 1 
    			t.detach();  // 2
    		}
    		else
    		{
    			process_user_input(cmd);
    		}
    	}
    }

如果用户选择打开一个新文档，需要启动一个新线程去打开新文档(1 处)，并分离线程（2 处）。与当前线程做出的操作一样，新线程只不过是打开另一个文件而已。所以，edit_document函数可以复用，通过传参的形式打开新的文件。

### 2.2 向线程函数传递参数

向 std::thread 构造函数中的可调用对象，或函数传递一个参数很简单，默认参数要拷贝到线程独立内存中，即使参数是引用的形式，也可以在新线程中进行访问。
    void f(int i, std::string const& s);
    std::thread t(f, 3, "hello");

这里使用的是 `"hello"` 字面值，是 `char const* ` 类型，在线程上下文中完成字面值向 string 对象的转化。 一定要保证在 std::thread 构造函数之前就将传入的数据转成理想格式。


std::ref 可以将参数直接转化成引用的形式，因为 std::thread 构造函数和 std::bind 的操作都在标准库中定义好了，可以传递一个成员函数指针作为线程函数，并提供一个合适的对象指针作为第一个参数。 （这里 std::bind 要额外查阅资料）


提供的参数可以移动，但不能拷贝，详见 std::move 的用法。


### 2.3 转移线程所有权

    void some_function();
    void some_other_function();
    
    std::thread t1(some_function);
    std::thread t2 = std::move(t1); // t1 线程执行的函数与 t2 关联，与 t1 无关
    t1 = std::thread(some_other_function); // 临时对象的线程启动，移动操作会隐式调用
    std::thread t3;  // 默认方式构造 t3
    t3 = std::move(t2); // t2 是一个命名对象，需要显示调用 move()
	
	// 此时，t1 与 some_other_function 线程关联，
	//t2 不与任何线程关联， 
	//t3与 some_function 线程关联

    t1 = std::move(t3);

上述最后一步会使程序直接停止运行，但不会**抛出**异常，**不能通过赋一个新值给 std::thread 对象的方式来"丢弃"一个线程**。

std::thread 支持移动，就意味着线程的所有权可以在函数外进行转移，说明 Sth::thread 可以作为函数的返回类型，以及进行类似于 push进 std::vector 中，实现线程自动化管理等操作。

    std::thread f()
    {
    	void func1();
    	return std::thread(func1);
    }
    
    std::thread g()
    {
    	void func2(int i);
    	std::thread t(func2, 42);
    	return t;
    }
    
还有：

    void do_work(int i)；
    
    void h()
    {
    	std::vector<std::thread> threads;
    	for (int i = 0; i < 20; i++)
    	{
    		threads.push_back(std::thread(do_work, i));
    	}
    	// 对每个线程调用 join()
    	std::for_each(threads.begin(),threads.end(),std::mem_fn(&std::thread::join()) 
    }

### 2.4 运行时决定线程数量

`std::thread::hardware_concurrency()` 在新版C++标准库中是一个很有用的函数。这个函数将返回能同时并发在一个程序中的线程数量。

    template< typename Iterator, typename T>
    struct accumulate_block
    {
    	void operator()(Iterator first, Iterator last, T& result)
    	{
    		result = std::accumulate(first, last, result);
    	}
    };
    
    template<typename Iterator, typename T>
    T parallel_accumulate(Iterator first, Iterator last, T init)
    {
    	unsigned long const length = std::distance(first, last);
		// 如果输入的范围为空，就返回得到的 init 值
    	if (!length)  // 1
    	{
    		return init;
    	}
    	// 设置最小任务数，避免产生线程太多
    	unsigned long const min_per_thread = 25;
		// 确定需要启动的线程最大数量
    	unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;  // 2
		// 返回能同时并发在一个程序中的线程数量
    	unsigned long const hardware_threads = std::thread::hardware_concurrency();
    	unsigned long const num_threads =     // 3  
  		  	std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    	// 每个进程中处理的元素数量
		unsigned long const block_size = length / num_threads; // 4
    	// 创建容器存放中间结果
    	std::vector<T> results(num_threads);
		// 启动线程数比总数少1，是减去了主线程
    	std::vector<std::thread> threads(num_threads - 1);  // 5
    	
    	Iterator block_start = first;
    	for (unsigned long  i = 0; i < (num_threads - 1); ++i)
    	{
    		Iterator block_end = block_start;
    		std::advance(block_end, block_size);  // 6
			// 不断累加中间结果
    		threads[i] = std::thread(  // 7
    			accumulate_block<Iterator, T>(),
    			block_start, block_end, std::ref(results[i]));
    		block_start = block_end;  // 8
    	}
    
    	accumulate_block<Iterator, T>()(
    		block_start, last, results[num_threads - 1]);  // 9
    	std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));  // 10
    
    	return std::accumulate(results.begin(), results.end(), init); // 11
    }

### 2.5 识别线程

线程标志类型是 std::thread::id，第一种检索方式，可以通过调
用 std::thread 对象的成员函数 `get_id()` 来直接获取。如果 std::thread 对象没有与任何执行线程相关联， `get_id()` 将返回 std::thread::type 默认构造值。这个值表示“没有线程”。第二种，当前线程中调用 `std::this_thread::get_id()` (这个函数定义在 <thread> 头文件中)也可以获得线程标识。

std::thread::id 对象可以自由的拷贝和对比,因为标识符就可以复用。如果两个对象
的 std::thread::id 相等，那它们就是同一个线程，或者都“没有线程”。如果不等，那么就代
表了两个不同线程，或者一个有线程，另一没有。

    std::thread::id master_thread;
    void some_core_part_of_algrithm()
    {
    	if (std::this_thread::get_id() == master_thread)
    	{
    		do_master_thread_work();
    	}
    	do_common_work();
    }

如上述示例，std::thread::id 实例常用作检测线程是否需要进行一些操作，比如：当用线程来分割一项工作，主线程可能要做一些与其他线程不同的工作。这种情况下，启动其他线程前，它可以将自己的线程ID通过`std::this_thread::get_id()` 得到，并进行存储。就是算法核心部分(所有线程都一样的),每个线程都要检查一下，其拥有的线程ID是否与初始线程的ID相同。另外，当前线程的std::thread::id 将存储到一个数据结构中。之后在这个结构体中对当前线程的ID与存储的线程ID做对比，来决定操作是被“允许”，还是“需要”(permitted/required)。

