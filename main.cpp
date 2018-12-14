#include <functional>  // for std::function and std::bind
#include <iostream>

#include <functional>
#include <map>
#include <vector>
#include <utility> // for std::forward
#include <thread>
#include <chrono>
#include <future>

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


class bazClass {
    int nn;
    std::promise<void> exitSignal;
    std::future<void> futureObj;
public:
    bazClass():
        futureObj(exitSignal.get_future()){}
    void myprint(int n){
        std::cout << "baz( " <<  n << " )\n";
    }
    void endlessloop(){
        nn = 0;
        while(true){
            std::this_thread::sleep_for(std::chrono::milliseconds(110));
            nn++;
        }
    }
    int getNn() const{return nn;}
    void stop()
    {
        exitSignal.set_value();
    }
};

class fooClass {
    int nn;
public:
    fooClass()= default;
    void myprint(int n){
        std::cout << "foo( " <<  n << " )\n";
    }
    void endlessloop(){
        nn = 0;
        while(true){
            std::this_thread::sleep_for(std::chrono::milliseconds(120));
            nn++;
        }
    }
    int getNn() const{return nn;}
};

class barClass {
    int nn;
public:
    barClass()= default;
    void myprint(int n){
        std::cout << "bar( " <<  n << " )\n";
    }

    void endlessloop(){
        nn = 0;
        while(true){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            nn++;
        }
    }
    int getNn() const{return nn;}
};


void foo(int i)
{
  std::cout << "foo( " <<  i << " )\n";
}

void bar()
{
  std::cout << "bar()\n";
}

int main()
{

  Subject<EventType> s;
  bazClass baz;
  fooClass foo;
  barClass bar;
  int n = 46;
//  s.registerObserver(EventType::GREEN, bar);
//  s.registerObserver(EventType::ORANGE, std::bind(foo, 42));
//  s.registerObserver(EventType::RED, std::bind(foo, 12345));
  s.registerObserver(EventType::YELLOW, std::bind(&bazClass::myprint, std::ref(baz), n));
  s.registerObserver(EventType::ORANGE, std::bind(&fooClass::myprint, std::ref(foo), n));
  s.registerObserver(EventType::RED, std::bind(&barClass::myprint, std::ref(bar), n));

  std::thread thr_foo(&fooClass::endlessloop, std::ref(foo));
  std::thread thr_bar(&barClass::endlessloop, std::ref(bar));
  std::thread thr_baz(&bazClass::endlessloop, std::ref(baz));

  //s.notify(EventType::GREEN);
  s.notify(EventType::RED);
  s.notify(EventType::ORANGE);
  s.notify(EventType::YELLOW);

  thr_bar.

}

