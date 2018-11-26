
###[`https://github.com/anlongstory/C-_Concurrency_in_Action_reading_notes`](https://github.com/anlongstory/C-_Concurrency_in_Action_reading_notes)

本章主要内容：

- 共享数据带来的问题
- 使用互斥量保护数据
- 数据保护的代替方案

&emsp;&emsp; 想象一下，你和你的朋友合租一个公寓，公寓中只有一个厨房和一个卫生间。当你的朋友在卫生间时，你就会不能使用了(除非你们特别好，好到可以在同时使用一个房间)。这个问题也会出现在厨房，假如:厨房里有一个组合式烤箱，当在烤香肠的时候，也在做蛋糕，就可能得到我们不想要的食物(香肠味的蛋糕)。此外，在公共空间将一件事做到一半时，发现某些需要的东西被别人借走，或是当离开的一段时间内有些东西被变动了地方，这都会令我们不爽。同样的问题，也困扰着线程。当线程在访问共享数据的时候，必须定一些规矩，用来限定线程可访问的数据位。还有，一个线程更新了共享数据，需要对其他线程进行通知。从易用性的角度，同一进程中的多个线程进行数据共享，有利有弊。错误的共享数据使用是产生并发bug的一个主要原因，并且后果要比香肠味的蛋糕更加严重。

### 3.1 共享数据带来的问题

<div align=center><img src="https://i.imgur.com/EV9MKrp.png" width=50%></div>

&emsp;&emsp;如上图，一个双链表每一个节点有两个指针分别指向前一个和后一个节点，这两个节点称为**不变量**。当涉及到某一个线程需要修改共享数据时，例如删除操作，当只有其中一边更新，不变量就被破坏了，直到另一边也完成更新，不变量就又稳定了，当一个线程只完成其中一边更新，另一个线程刚好需要访问这个被删除的节点，或者另一个线程也在尝试要删除这个节点，就会有问题出现，这是常见的错误，**条件竞争。**

##### 3.1.1 条件竞争

&emsp;&emsp;上面例子可以看出，当不变量遭到破坏时，才会产生条件竞争，当系统负载增加时，随着执行数量的增加，执行序列的问题复现的概率也在增加，条件竞争通常是时间敏感的，所以程序以调试模式运行时，它们常会完全消失，因为调试模式会影响程序的执行时间(即使影响不多)。

##### 3.1.2 避免恶性条件竞争

- 最简单的方法就是对数据结构采用某种保护机制，确保只有进行修改的线程才能看到不变量被破坏时的中间状态。从其他访问线程的角度来看，修改不是已经完成了，就是还没开始。
- 对数据结构和不变量的设计进行修改，修改完的结构必须能完成一系列不可分割的变化，也就是保证每个不变量保持稳定的状态，即**无锁编程。**
- 另一种处理条件竞争的方式是，使用事务的方式去处理数据结构的更新(这里的"处理"就如同对数据库进行更新一样)

<font color=#DC143C>保护共享数据结构的最基本的方式，是使用C++标准库提供的互斥量。</font>

### 3.2 使用互斥量保护共享数据

&emsp;&emsp;当一个线程使用特定互斥量锁住共享数据时，其他的线程想要访问锁住的数据，都必须等到之前那个线程对数据进行解锁后，才能进行访问。这就保证了所有线程能看到共享数据，而不破坏不变量。

##### 3.2.1 C++中使用互斥量

&emsp;&emsp;C++中通过实例化 std::mutex 创建互斥量，通过调用成员函数lock()进行上锁，unlock()进行解锁。不过，不推荐实践中直接去调用成员函数，因为调用成员函数就意味着，必须记住在每个函数出口都要去调用unlock()，也包括异常的情况。
&emsp;&emsp;C++中提供了模板类 `std::lock_guard`，其会在构造的时候提供已锁的互斥量，并在析构的时候进行解锁，从而保证了一个已锁的互斥量总是会被正确的解锁。std::mutex 和 `std::lock_guard` 都在 `<mutex>` 头文件中声明。

    #include<iostream>
    #include<list>
    #include<mutex>
    #include <algorithm>
    
    std::list<int> some_list;  // 全局变量
    std::mutex some_mutex;  // 全局互斥锁
    
    void add_to_list(int new_value)
    {
    	std::lock_guard<std::mutex> guard(some_mutex);  // 1
    	some_list.push_back(new_value);
    }
    
    bool list_contains(int value_to_find)
    {
    	std::lock_guard<std::mutex> guard(some_mutex);  // 2
    	return
    		std::find(some_list.begin(), some_list.end(), value_to_find) != some_list.end();
    }

在上面 1 和 2 两个函数处使用了 std::lock_guard<std::mutex>，使得这两个函数对数据的访问是互斥的：2的函数不能看到正在被 1 函数修改的列表。一般的，可以将其封装成类，**互斥量和要保护的数据，在类中都需要定义为private成员**，这会让访问数据的代码变的清晰，并且容易看出在什么时候对互斥量上锁。

##### 3.2.2 精心组织代码来保护共享数据

切勿将受保护数据的指针或引用传递到互斥锁作用域之外，无论是函数返回值，还是存储在外部可见内存，亦或是以参数的形式传递到用户提供的函数中去。

##### 3.2.3 发现接口内在的条件竞争

##### 3.2.4 死锁：问题描述及解决方案

死锁是指不同的两个线程会互相等待，从而什么都没做的情况。避免死锁的一般建议，就是让两个互斥量总以相同的顺序上锁：总在互斥量B之前锁住互斥量A，就永远不会死锁。不过，选择一个固定的顺序(例如，实例提供的第一互斥量作为第一个参数，提供的第二个互斥量为第二个参数)，可能会适得其反：在参数交换了之后，两个线程试图在相同的两个实例间进行数据交换时，程序又死锁了！很幸运，C++标准库有办法解决这个问题， `std::lock` ——可以一次性锁住多个(两个以上)的互斥量，并且没有副作用(死锁风险)。

    class some_big_object;
    void swap(some_big_object& lhs, some_big_object& rhs);
    
    class X
    {
    private:
    	some_big_object some_detail;
    	std::mutex m;
    public:
    	X(some_big_object const & sd) :some_detail(sd) {}
    
    	friend void swap(X& lhs, X& rhs)
    	{
    		if (&lhs == &rhs) // 检查参数是否为不同的实例
    			return;
    		std::lock(lhs.m, rhs.m);  // 调用 lock()锁住两个互斥量
			
			// 提供 std::adopt_lock 参数除了表示 std::lock_guard 对象可获取锁之外，还将锁交由 
			// std::lock_guard 对象管理，而不需要 std::lock_guard 对象再去构建新的锁。
    		std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock);
    		std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);
    		swap(lhs.some_detail, ths.some_detail);
    	}
    };

std::lock 要么将两个锁都锁住，要不一个都不锁。虽然 std::lock 可以在这情况下(获取两个以上的锁)避免死锁，但它没办法帮助你获取其中一个锁。

##### 3.2.5 避免死锁的进阶指导
- 避免嵌套锁
- 避免在持有锁时调用用户提供的代码
- 使用固定顺序获取锁
- 使用锁的层次结构

##### 3.2.6 std::unique_lock——灵活的锁


`std::unqiue_lock` 使用更为自由的不变量，这样 `std::unique_lock` 实例不会总与互斥量的数据类型相关，使用起来要比 `std:lock_guard` 更加灵活。首先，可将 `std::adopt_lock` 作为第二个参数传入构造函数，对互斥量进行管理；也可以将 `std::defer_lock` 作为第二个参数传递进去，表明互斥量应保持解锁状态。这样，就可以被 `std::unique_lock` 对象(不是互斥量)的lock()函数的所获取，或传递 `std::unique_lock` 对象到 `std::lock()` 中。

    class some_big_object;
    void swap(some_big_object& lhs, some_big_object& rhs);
    
    class X
    {
    private:
    	some_big_object some_detail;
    	std::mutex m;
    public:
    	X(some_big_object const & sd) :some_detail(sd) {}
    
    	friend void swap(X& lhs, X& rhs)
    	{
    		if (&lhs == &rhs)
    			return;
    		// std::def_lock 留下未上锁的互斥量
    		std::unique_lock<std::mutex> lock_a(lhs.m, std::defer_lock);
    		std::unique_lock<std::mutex> lock_b(rhs.m, std::defer_lock);
    		std::lock(lock_a, lock_b); // 互斥量在这里上锁
    		swap(lhs.some_detail, ths.some_detail);
    	}
    };

`std::unique_lock` 对象的体积通常要比 `std::lock_guard` 对象大，当使用 `std::unique_lock` 替代 `std::lock_guard` ，因为会对标志进行适当的更新或检查，就会做些轻微的性能惩罚。当 `std::lock_guard` 已经能够满足你的需求，那么还是建议你继续使用它。当需要更加灵活的锁时，最好选择 `std::unique_lock`。

##### 3.2.7 不同域中互斥量所有权的传递

##### 3.2.8 锁的粒度
锁的粒度是用来描述一个锁保护着的数据量大小。细粒度锁表示能够保护较小的数据量，粗粒度锁表示能够保护较多的数据量。如果很多线程正在等待同一个资源(等待收银员对自己拿到的商品进行清点)，当有线程持有锁的时间过长，这就会增加等待的时间(别等到结账的时候，才想起来蔓越莓酱没拿)。在可能的情况下，锁住互斥量的同时只能对共享数据进行访问；试图对锁外数据进行处理。特别是做一些费时的动作，比如：对文件的输入/输出操作进行上锁。文件输入/输出通常要比从内存中读或写同样长度的数据慢成百上千倍，所以除非锁已经打算去保护对文件的访问，要么执行输入/输出操作将会将延迟其他线程执行的时间，这很没有必要(因为文件锁阻塞住了很多操作)，这样多线程带来的性能效益会被抵消。锁不仅是能锁住合适粒度的数据，还要控制锁的持有时间，以及什么操作在执行的同时能够拥有锁。

### 3.3 保护共享数据的替代设施

互斥量是最通用的机制，但其并非保护共享数据的唯一方式。这里有很多替代方式可以在特定情况下，提供更加合适的保护。

##### 3.3.1 保护共享数据的初始化过程

C++标准库提供了 `std::once_flag` 和 `std::call_once` 来处理这种情况。比起锁住互斥量，并显式的检查指针，每个线程只需要使用 `std::call_once` ，在 `std::call_once` 的结束时，就能安全的知道指针已经被其他的线程初始化了。使用 `std::call_once` 比显式使用互斥量消耗的资源更少，特别是当初始化完成后。

    std::shared_ptr<some_resource> resource_ptr;
    std::once_flag resource_flag;
    
    void init_resource()
    {
    	resource_ptr.reset(new some_resource);
    }
    
    void foo()
    {
    	std::call_once(resource_flag, init_resource); // 可以完整的进行一次初始化
    	resource_ptr->do_something();
    }

在这个例子中， `std::once_flag` 和初始化好的数据都是命名空间区域的对象，但是 `std::call_once() `可仅作为延迟初始化的类型成员。

##### 3.3.2 保护很少更新的数据结构
比起使用 std::mutex 实例进行同步，不如使用 `boost::shared_mutex` 来做同步。对于更新操作，可以使用 `std::lock_guard<boost::shared_mutex> `和 `std::unique_lock<boost::shared_mutex>` 上锁。作为 std::mutex 的替代方案，与 std::mutex 所做的一样，这就能保证更新线程的独占访问。因为其他线程不需要去修改数据结构，所以其可以使用 `boost::shared_lock<boost::shared_mutex>` 获取访问权。这与使用 `std::unique_lock` 一样，除非多线程要在同时获取同一个` boost::shared_mutex` 上有共享锁。唯一的限制：当任一线程拥有一个共享锁时，这个线程就会尝试获取一个独占锁，直到其他线程放弃他们的锁；同样的，当任一线程拥有一个独占锁时，其他线程就无法获得共享锁或独占锁，直到第一个线程放弃其拥有的锁。


##### 3.3.3 嵌套锁
当一个线程已经获取一个 std::mutex 时(已经上锁)，并对其再次上锁，这个操作就是错误的，并且继续尝试这样做的话，就会产生未定义行为。然而，在某些情况下，一个线程尝试获取同一个互斥量多次，而没有对其进行一次释放是可以的。之所以可以，是因为 C++ 标准库提供了 `std::recursive_mutex` 类。其功能与 std::mutex 类似，除了你可以从同一线程的单个实例上获取多个锁。互斥量锁住其他线程前，你必须释放你拥有的所有锁，所以当你调用lock()三次时，你也必须调用unlock()三次。正确使用 `std::lock_guard<std::recursive_mutex>` 和 `std::unique_lock<std::recursive_mutex>` 可以帮你处理这些问题。

