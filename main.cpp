#include <functional>  // for std::function and std::bind
#include <iostream>

#include <functional>
#include <map>
#include <vector>
#include <utility> // for std::forward
#include <thread>
#include <chrono>
#include <future>
#include <queue>
#include <condition_variable>

template <typename T>
class Queue
{
 public :

  T pop()
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    auto val = queue_.front();
    queue_.pop();
    return val;
  }

  void pop(T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    item = queue_.front();
    queue_.pop();
  }

  void push(const T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(item);
    mlock.unlock();
    cond_.notify_one();
  }
  Queue()=default;
  Queue(const Queue&) = delete;            // disable copying
  Queue& operator=(const Queue&) = delete; // disable assignment

 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

enum class EventType { GREEN, RED, BLUE, YELLOW, ORANGE };

template <typename Event>
class Subject
{
 public:
  Subject()=default;
  template <typename Observer>
  void registerObserver(const Event& event, Observer&& observer)
  {
    observers_[event].push_back(std::forward<Observer>(observer));
  }
  template <typename Observer>
  void registerObserver(Event&& event, Observer&& observer)
  {
    observers_[std::move(event)].push_back(std::forward<Observer>(observer));
  }

  void notify(const Event& event) const
  {
    for (const auto& obs : observers_.at(event)) obs();
  }
  // disallow copying and assigning
  Subject(const Subject&)=delete;
  Subject& operator=(const Subject&)=delete;

 private:
  std::map<Event, std::vector<std::function<void()>>> observers_;
};

class ThreadFuncStop
{
    std::promise<void> exitSignal;
    std::future<void> futureObj;
public:
    ThreadFuncStop() :
            futureObj(exitSignal.get_future())
    {

    }
    ThreadFuncStop(ThreadFuncStop && obj) : exitSignal(std::move(obj.exitSignal)), futureObj(std::move(obj.futureObj))
    {
        std::cout << "Move Constructor is called" << std::endl;
    }

    ThreadFuncStop & operator=(ThreadFuncStop && obj)
    {
        std::cout << "Move Assignment is called" << std::endl;
        exitSignal = std::move(obj.exitSignal);
        futureObj = std::move(obj.futureObj);
        return *this;
    }

    // Task need to provide defination  for this function
    // It will be called by thread function
    virtual void run() = 0;


    // Thread function to be executed by thread
    void operator()()
    {
        run();
    }

    //Checks if thread is requested to stop
    bool stopRequested()
    {
        // checks if value in future object is available
        if (futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
            return false;
        return true;
    }
    // Request the thread to stop by setting value in promise object
    void stop()
    {
        exitSignal.set_value();
    }
};

class Baz : public ThreadFuncStop {
    int _n;
public:
    Baz() = default;
    void myprint(int n, Queue<int>& q) noexcept{
        Queue<int> & q_ = const_cast<Queue<int>&>(q);
        q_.push(_n);
        std::clog << "baz( " <<  n <<  " ) " << "Private counter: " << _n << "\n";
    }
    void run(){
        _n = 0;
        while(stopRequested() == false){
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::clog<< "baz thread()\n";
            _n++;
        }
    }
    int getNn() const{return _n;}
};

class Foo: public ThreadFuncStop {
    int _n;
public:
    Foo()= default;
    void eventFoo(int n, Queue<int>& q) noexcept{
        Queue<int> & q_ = const_cast<Queue<int> &>(q);
        q_.push(_n);
        std::clog << "foo( " <<  n <<  " ) " << "Private counter: " << _n << "\n";
    }
    void run(){
        _n = 0;
        while(stopRequested() == false){
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::clog << "foo thread\n";
            _n++;
        }
    }
    int getNn() const{return _n;}
};

class Bar: public ThreadFuncStop {
    int _n;
public:
    Bar()= default;
    void eventBar(int n, Queue<int>& q) noexcept{
        Queue<int> & q_ = const_cast<Queue<int> &>(q);
        q_.push(_n);
        std::clog << "bar( " <<  n <<  " ) " << "Private counter: " << _n << "\n";
    }

    void run(){
        _n = 0;
        while(stopRequested() == false){
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::clog << "bar thread\n";
            _n++;
        }
    }
    int getNn() const{return _n;}
};

class Qux {

    int _n;
public:
    Qux()= default;
    void run(Subject<EventType>& s, Queue<int>& q) {
        _n = 0;
        while(true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::clog << "Qux thread\n";
            s.notify(EventType::RED);
            s.notify(EventType::ORANGE);
            s.notify(EventType::YELLOW);
            _n++;
            std::clog << "Qux from queue "<< std::to_string(q.pop()) << "\n";
            std::clog << "Qux from queue "<< std::to_string(q.pop()) << "\n";
            std::clog << "Qux from queue "<< std::to_string(q.pop()) << "\n";
        }
    }
    int getNn() const{return _n;}

};


int main()
{
  Queue<int> q;
  Subject<EventType> s;
  Baz baz;
  Foo foo;
  Bar bar;
  Qux qux;

  int n = 46;
  tryt {
  s.registerObserver(EventType::YELLOW, std::bind(&Baz::myprint, std::ref(baz), n, std::ref(q)));
  s.registerObserver(EventType::ORANGE, std::bind(&Foo::eventFoo, std::ref(foo), n, std::ref(q)));
  s.registerObserver(EventType::RED, std::bind(&Bar::eventBar, std::ref(bar), n, std::ref(q)));

  std::thread thr_foo(&Foo::run, std::ref(foo));
  std::thread thr_bar(&Bar::run, std::ref(bar));
  std::thread thr_baz(&Baz::run, std::ref(baz));
  std::thread thr_qux(&Qux::run, std::ref(qux), std::ref(s), std::ref(q));
  }
  catch (const std::overflow_error& e) {
      std::cout << "[ERROR][RUN] " << e.what() << " <==========" << std::endl;
  }
  catch (const std::runtime_error& e) {
      std::cout << "[ERROR][RUN] " << e.what() << " <==========" << std::endl;
  }
  catch(const std::exception &e) {
      std::cout << "[ERROR][RUN] " << e.what() << " <==========" << std::endl;
  }
  catch (...) {
      std::cout << "[ERROR][RUN] Unexpected Exception! Something wrong it's going here! <==========" << std::endl;
  }
 
  auto count = 0;

  while(true) {

      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      count++;
      if(count > 10)
          break;
  }

  baz.stop();
  foo.stop();
  bar.stop();

  thr_bar.join();
  thr_baz.join();
  thr_foo.join();
}

