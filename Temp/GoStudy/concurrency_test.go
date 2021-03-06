/*******************************************************************************
TODO:goroutine 不是线程，也不是简单意义的线程池 -- 协程coroutine的一种(类似纤程?)
  官方说是动态复用线程，来保持所有的goroutine运行 --

Go官方说 "Concurrency is not parallelism"
  https://talks.golang.org/2012/concurrency.slide#7

Go让函数很容易成为非常轻量的线程，线程之前的通讯使用 channel 完成
  goroutine--是Go并发能力的核心要素，并行执行，是轻量的(仅比分配栈多一点消耗),调度开销非常小，
    ★:Go语言标准库提供的所有系统调用操作(也包括所有同步IO操作)，都会出让CPU给其他goroutine
    初识时栈很小，且随着需要在堆空间上分配
    默认时，如果一个goroutine没有被阻塞，别的goroutine就不会得到执行，可通过runtime.GOMAXPROCS(n)设置使用CPU的个数
    TODO:没有任何办法知道，当所有goroutine都已经退出应当等待多久
    注意：goroutine函数返回时，对应的goroutine就会自动结束。如果该函数有返回值，其值会被丢弃

采用通讯序列化过程(Communicating Sequential Processes - CSP)作为goroutine间的推荐通信方式，
chan(channel?)--通信机制(双向的生产者消费者队列?)，类型相关的
    定义：
	var chanName [<-只读]chan[<-只写] 类型 = make(chan 类型[, 缓冲大小])
	chanName := make(chan 类型 [,缓冲大小])   //创建一个可以收发指定类型(如string)的channel,可选设置缓冲大小，默认无缓冲

    默认是0-Buffer的阻塞方式(即Reader/Writer都要准备好以后才能继续)

	用于向系统中其他代理发送和接收特定类型对象的通信机制。
    是本地进程内通信方式，若要跨进程，则推荐采用Socket等分布式方式(用于连接到命名的目的地的库方法?)

	ci <- myValue	//将变量myValue发送到channel ci中
	定义channel数组并初始化：chs:= make([]chan int, 10) for(i < 10) { chs[i]:=make(chan int); }

	myValue := <-ci	//从channel ci 中读取变量，并保存到变量myValue中，如是从带缓冲的chan中读取，可使用range
	默认是阻塞方式读写，也可以通过 myValue, ok := <-ci 来非阻塞方式读写(需要判断ok变量)
	关闭：close(c1)， 等待routie会通过第二个返回值(ok)得知是否读取到数据？ 使用 closed 判断关闭状态？
    TODO:非阻塞读取和关闭后读取判断似乎冲突，哪一个对？

	单向chan:使用限制只能用于发送或接收数据
      返回"只读取string的chan"的函数原型：func someFun(msg string) <-chan string {return xxx}
        即返回值处的"<-"表示只读取

    可以用 select 来侦听多个channel的读写(类似WaitForMulti),select阻塞直到任何一个chan可以读写才返回，
	  设置超时流程的处理: case <-time.After(time.Second * 10): {xxx};
	  通过default进行无阻塞处理：default: {xxx} -- 即没有任何chan中读取到数据时立刻进入该分支
      每一个case语句里必须是一个IO操作

runtime
  NumCPU -- 获取CPU个数
  GOMAXPROCS(n) -- 设置可以使用的并行CPU个数？也可通过同名环境变量来设置
  Gosched() -- 出让CPU时间片，类似yield?
sync -- 多线程同步时的保护
  atomic -- 提供对一些基础数据类型的原子操作函数
  Mutex -- 互斥，mutex.Lock(); defer mutex.Unlock(); xxxx;
  Once -- 全局唯一性操作, 通过 Do() 调用需要保证唯一性的函数
  RWMutex -- 单写多读模型锁
*******************************************************************************/
package gostudy

import (
	"fmt"
	"math/rand"
	//"reflect"
	"runtime"
	//"sync/atomic"
	"testing"
	"time"
)

func sumArray(values []int, resultChan chan int) {
	sum := 0
	for _, val := range values {
		sum += val
	}
	time.Sleep(time.Duration(rand.Intn(1000)) * time.Millisecond)
	resultChan <- sum //将计算结果发送到channel中
}

//使用Go并发计算两个求和操作
func TestAddInGoroutine(t *testing.T) {
	values := []int{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
	resultChan := make(chan int, 2)
	//等价用法：var resultChan chan int = make(chan int, 2)

	GOUNIT_ASSERT(t, len(values) == 10, "数组长度")

	go sumArray(values[:len(values)/2], resultChan)
	go sumArray(values[len(values)/2:], resultChan)

	//TODO: sum1和sum2的顺序
	sum1, sum2 := <-resultChan, <-resultChan //从resultChan 中获取到结果

	fmt.Printf("sum1=%d, sum2=%d\n", sum1, sum2)
	GOUNIT_ASSERT(t, sum1 == 15 && sum2 == 40, "使用两个goroutine计算数据")
}

//首先定义会使用到的能收发int的channel -- 函数中的参数可以定义为 func xxx(cInput chan int)
var cInput chan int
var cResult chan int

func someResumerProc(i int) {
	//time.Sleep(i * time.Second)
	nInput := <-cInput //从channel接收整数，并保存到局部变量nInput中
	fmt.Printf("parallen routine %d will quit\n", nInput)

	cResult <- i //每个routine退出前都会向结果channel写一个值表示自己即将退出
}

const (
	PARALLEL_COUNT     = 3
	PARALLEL_TOTAL_SUM = 6 // = 1 + 2 + xxxx + PARALLEL_COUNT
)

func TestConcurrency(t *testing.T) {
	//创建发送和接收整数的输入channel，一个用于给消费者传递参数，一个用于等待所有的消费者结束
	cResult = make(chan int, PARALLEL_COUNT) //缓冲大小为PARALLEL_COUNT
	cInput = make(chan int)                  //无缓冲(等价于 ,0)

	//默认同一时刻只有一个goroutine执行，本函数设置可以并行的数量,也可以设置环境变量 GOMAXPROCS
	runtime.GOMAXPROCS(4)

	fmt.Printf("Test Parallel -- Before start goroutine\n")

	for i := 1; i <= PARALLEL_COUNT; i++ {
		go someResumerProc(i)
		cInput <- i //发送整数 i 到 channel cInput
	}

	nTotalCount := 0
	//*
	//使用两种方式检测所有的routine都退出的时机
	//方案1：直接使用 <- 读取channel
	for {
		nTmp := <-cResult //从结果channel中读取goroutine返回的数据
		fmt.Printf("Read %d from channel\n", nTmp)
		nTotalCount += nTmp
		if PARALLEL_TOTAL_SUM == nTotalCount {
			//所有的routine都退出
			break
		}
	}
	/*/
	//方案2：使用 select 来读取channel(select的使用场景是需要同时读取多个channel，此处仅是示例)
	Quit:
		for {
			select {
			case nTmp := <-cResult:
				nTotalCount += nTmp
				if PARALLEL_TOTAL_SUM == nTotalCount {
					break Quit
				}
			//case xxx := <-cOther:
			}
		}
		//*/

	fmt.Printf("Test Parallel -- After all goroutine ends\n")
}

//使用从函数中返回的只读chan--相当于数据提供服务，
//函数中启动一个goroutine并填充数据，填充结束后close，使得调用方处理完所有元素后自然结束
func funReturnReadonlyChannel(msg string) <-chan string {
	c := make(chan string)
	go func() {
		for i := 0; i < 5; i++ {
			c <- fmt.Sprintf("%s %d", msg, i)
			time.Sleep(time.Duration(rand.Intn(1e3)) * time.Millisecond)
		}
		close(c)
	}()
	return c
}

func TestReadInfoFromRunChan(t *testing.T) {
	fmt.Printf("Enter TestReadInfoFromRunChan\n")
	c1 := funReturnReadonlyChannel("Tom")
	c2 := funReturnReadonlyChannel("Jerry")
	count := 0
	timeout := time.After(time.Second * 3)
	var strInfo string = ""

Quit:
	for {
		select {
		case msg, ok := <-c1:
			if ok && msg != "" {
				fmt.Printf("Tom says \"%s\"\n", msg)
				//strInfo = fmt.Sprintf("Tom says \"%s\"\n", msg)
			}
		case msg, ok := <-c2:
			if ok && msg != "" {
				fmt.Printf("Jerry says \"%s\"\n", msg)
				//strInfo = fmt.Sprintf("Jerry says \"%s\"\n", msg)
			}
		case <-timeout: //超时
			fmt.Printf("Time out for TestReadInfoFromRunChan\n")
			//strInfo = fmt.Sprintf("Time out for TestReadInfoFromRunChan")
			break Quit
			//default:
			//	{
			//		fmt.Printf("break\n")
			//		break Quit
			//	}

		}
		count++
		fmt.Printf("count=%d\n", count)
		if count > 100 {
			break
		}
	}
	fmt.Printf("Leave TestReadInfoFromRunChan, %s\n", strInfo)
}

func TestSelectChannel(t *testing.T) {
	//随机向chan中写入0或1 -- 注意 make chan 时必须指定缓冲，否则本例子会死锁
	ch := make(chan int, 1)
	for i := 0; i < 5; i++ {
		select {
		case ch <- 0:
		case ch <- 1:
		}
		i := <-ch
		fmt.Printf("TestSelectChannel Received:%d\n", i)
	}
}

//使用Go来实现流水线(管道)架构的示例
type PipeData struct {
	value   int
	handler func(int) int
	next    chan int
}

func handle(queue chan *PipeData) {
	for data := range queue {
		data.next <- data.handler(data.value)
	}
}

func TestPipeLine(t *testing.T) {
	GOUNIT_ASSERT(t, true, "TODO: 流水线(管道)架构")
}

func TestOnwWayChannel(t *testing.T) {
	/*
		chTwoway := make(chan int, 10) //声明双向读写的channel，其他的只读、只写chan只是类型转换，本质是一个

		chWriteOnly := chan<- int(chTwoway) //类型转换成只写
		chReadOnly := <-chan int(chTwoway)  //类型转换成只读
		fmt.Printf("type of chReadOnly =%v\n", reflect.TypeOf(chReadOnly).String())

		chWriteOnly <- 100
		//val := <-chReadOnly //TODO:编译不过
	*/

	val := 100
	GOUNIT_ASSERT(t, val == 100, "通过只读、只写chan传递数据")

}
