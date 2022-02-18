//---------------------------- upgrade_mutex.h ---------------------------------
//
// This software is in the public domain.  The only restriction on its use is
// that no one can remove it from the public domain by claiming ownership of it,
// including the original authors.
//
// There is no warranty of correctness on the software contained herein.  Use
// at your own risk.
//
//------------------------------------------------------------------------------

#include "upgrade_mutex.h"
#include <thread>
#include <cassert>

#include <iostream>

enum {reading, writing};
int state = reading;

void sub_print() {}

template <class A0, class ...Args>
void
sub_print(const A0& a0, const Args& ...args)
{
    std::cout << a0;
    sub_print(args...);
}

std::mutex&
cout_mut()
{
    static std::mutex m;
    return m;
}

template <class ...Args>
void
print(const Args& ...args)
{
    std::lock_guard<std::mutex> _(cout_mut());
    sub_print(args...);
}

namespace S
{

std::shared_timed_mutex mut;

void reader()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        mut.lock_shared();
        assert(state == reading);
        ++count;
        mut.unlock_shared();
    }
    print("reader = ", count, '\n');
}

void writer()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        mut.lock();
        state = writing;
        assert(state == writing);
        state = reading;
        ++count;
        mut.unlock();
    }
    print("writer = ", count, '\n');
}

void try_reader()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_shared())
        {
            assert(state == reading);
            ++count;
            mut.unlock_shared();
        }
    }
    print("try_reader = ", count, '\n');
}

void try_writer()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock())
        {
            state = writing;
            assert(state == writing);
            state = reading;
            ++count;
            mut.unlock();
        }
    }
    print("try_writer = ", count, '\n');
}

void try_for_reader()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_shared_for(std::chrono::microseconds(5)))
        {
            assert(state == reading);
            ++count;
            mut.unlock_shared();
        }
    }
    print("try_for_reader = ", count, '\n');
}

void try_for_writer()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_for(std::chrono::microseconds(5)))
        {
            state = writing;
            assert(state == writing);
            state = reading;
            ++count;
            mut.unlock();
        }
    }
    print("try_for_writer = ", count, '\n');
}

void
test_shared_mutex()
{
    {
        std::thread t1(reader);
        std::thread t2(writer);
        std::thread t3(reader);
        t1.join();
        t2.join();
        t3.join();
    }
    {
        std::thread t1(try_reader);
        std::thread t2(try_writer);
        std::thread t3(try_reader);
        t1.join();
        t2.join();
        t3.join();
    }
    {
        std::thread t1(try_for_reader);
        std::thread t2(try_for_writer);
        std::thread t3(try_for_reader);
        t1.join();
        t2.join();
        t3.join();
    }
}

}

namespace U
{

acme::upgrade_mutex mut;

void reader()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        mut.lock_shared();
        assert(state == reading);
        ++count;
        mut.unlock_shared();
    }
    print("reader = ", count, '\n');
}

void writer()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        mut.lock();
        state = writing;
        assert(state == writing);
        state = reading;
        ++count;
        mut.unlock();
    }
    print("writer = ", count, '\n');
}

void try_reader()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_shared())
        {
            assert(state == reading);
            ++count;
            mut.unlock_shared();
        }
    }
    print("try_reader = ", count, '\n');
}

void try_writer()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock())
        {
            state = writing;
            assert(state == writing);
            state = reading;
            ++count;
            mut.unlock();
        }
    }
    print("try_writer = ", count, '\n');
}

void try_for_reader()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_shared_for(std::chrono::microseconds(5)))
        {
            assert(state == reading);
            ++count;
            mut.unlock_shared();
        }
    }
    print("try_for_reader = ", count, '\n');
}

void try_for_writer()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_for(std::chrono::microseconds(5)))
        {
            state = writing;
            assert(state == writing);
            state = reading;
            ++count;
            mut.unlock();
        }
    }
    print("try_for_writer = ", count, '\n');
}

void upgradable()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        mut.lock_upgrade();
        assert(state == reading);
        ++count;
        mut.unlock_upgrade();
    }
    print("upgradable = ", count, '\n');
}

void try_upgradable()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_upgrade())
        {
            assert(state == reading);
            ++count;
            mut.unlock_upgrade();
        }
    }
    print("try_upgradable = ", count, '\n');
}

void try_for_upgradable()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_upgrade_for(std::chrono::microseconds(5)))
        {
            assert(state == reading);
            ++count;
            mut.unlock_upgrade();
        }
    }
    print("try_for_upgradable = ", count, '\n');
}

void clockwise()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        mut.lock_shared();
        assert(state == reading);
        if (mut.try_unlock_shared_and_lock())
        {
            state = writing;
        }
        else if (mut.try_unlock_shared_and_lock_upgrade())
        {
            assert(state == reading);
            mut.unlock_upgrade_and_lock();
            state = writing;
        }
        else
        {
            mut.unlock_shared();
            continue;
        }
        assert(state == writing);
        state = reading;
        mut.unlock_and_lock_upgrade();
        assert(state == reading);
        mut.unlock_upgrade_and_lock_shared();
        assert(state == reading);
        mut.unlock_shared();
        ++count;
    }
    print("clockwise = ", count, '\n');
}

void counter_clockwise()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        mut.lock_upgrade();
        assert(state == reading);
        mut.unlock_upgrade_and_lock();
        assert(state == reading);
        state = writing;
        assert(state == writing);
        state = reading;
        mut.unlock_and_lock_shared();
        assert(state == reading);
        mut.unlock_shared();
        ++count;
    }
    print("counter_clockwise = ", count, '\n');
}

void try_clockwise()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_shared())
        {
            assert(state == reading);
            if (mut.try_unlock_shared_and_lock())
            {
                state = writing;
            }
            else if (mut.try_unlock_shared_and_lock_upgrade())
            {
                assert(state == reading);
                mut.unlock_upgrade_and_lock();
                state = writing;
            }
            else
            {
                mut.unlock_shared();
                continue;
            }
            assert(state == writing);
            state = reading;
            mut.unlock_and_lock_upgrade();
            assert(state == reading);
            mut.unlock_upgrade_and_lock_shared();
            assert(state == reading);
            mut.unlock_shared();
            ++count;
        }
    }
    print("try_clockwise = ", count, '\n');
}

/*
void try_for_clockwise()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_shared_for(std::chrono::microseconds(5)))
        {
            assert(state == reading);
            if (mut.try_unlock_shared_and_lock_for(std::chrono::microseconds(5)))
            {
                state = writing;
            }
            else if (mut.try_unlock_shared_and_lock_upgrade_for(std::chrono::microseconds(5)))
            {
                assert(state == reading);
                mut.unlock_upgrade_and_lock();
                state = writing;
            }
            else
            {
                mut.unlock_shared();
                continue;
            }
            assert(state == writing);
            state = reading;
            mut.unlock_and_lock_upgrade();
            assert(state == reading);
            mut.unlock_upgrade_and_lock_shared();
            assert(state == reading);
            mut.unlock_shared();
            ++count;
        }
    }
    print("try_for_clockwise = ", count, '\n');
}
*/

void try_counter_clockwise()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_upgrade())
        {
            assert(state == reading);
            if (mut.try_unlock_upgrade_and_lock())
            {
                assert(state == reading);
                state = writing;
                assert(state == writing);
                state = reading;
                mut.unlock_and_lock_shared();
                assert(state == reading);
                mut.unlock_shared();
                ++count;
            }
            else
            {
                mut.unlock_upgrade();
            }
        }
    }
    print("try_counter_clockwise = ", count, '\n');
}

void try_for_counter_clockwise()
{
    typedef std::chrono::steady_clock Clock;
    unsigned count = 0;
    Clock::time_point until = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < until)
    {
        if (mut.try_lock_upgrade_for(std::chrono::microseconds(5)))
        {
            assert(state == reading);
            if (mut.try_unlock_upgrade_and_lock_for(std::chrono::microseconds(5)))
            {
                assert(state == reading);
                state = writing;
                assert(state == writing);
                state = reading;
                mut.unlock_and_lock_shared();
                assert(state == reading);
                mut.unlock_shared();
                ++count;
            }
            else
            {
                mut.unlock_upgrade();
            }
        }
    }
    print("try_for_counter_clockwise = ", count, '\n');
}

void
test_upgrade_mutex()
{
    {
        std::thread t1(reader);
        std::thread t2(writer);
        std::thread t3(reader);
        t1.join();
        t2.join();
        t3.join();
    }
    {
        std::thread t1(try_reader);
        std::thread t2(try_writer);
        std::thread t3(try_reader);
        t1.join();
        t2.join();
        t3.join();
    }
    {
        std::thread t1(try_for_reader);
        std::thread t2(try_for_writer);
        std::thread t3(try_for_reader);
        t1.join();
        t2.join();
        t3.join();
    }
    {
        std::thread t1(reader);
        std::thread t2(writer);
        std::thread t3(upgradable);
        t1.join();
        t2.join();
        t3.join();
    }
    {
        std::thread t1(reader);
        std::thread t2(writer);
        std::thread t3(try_upgradable);
        t1.join();
        t2.join();
        t3.join();
    }
    {
        std::thread t1(reader);
        std::thread t2(writer);
        std::thread t3(try_for_upgradable);
        t1.join();
        t2.join();
        t3.join();
    }
    {
        state = reading;
        std::thread t1(clockwise);
        std::thread t2(counter_clockwise);
        std::thread t3(clockwise);
        std::thread t4(counter_clockwise);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
    }
    {
        state = reading;
        std::thread t1(try_clockwise);
        std::thread t2(try_counter_clockwise);
        t1.join();
        t2.join();
    }
    {
        state = reading;
        //std::thread t1(try_for_clockwise);
        std::thread t2(try_for_counter_clockwise);
        //t1.join();
        t2.join();
    }
}

}

#include <vector>

namespace Assignment
{

class A
{
    typedef acme::upgrade_mutex            mutex_type;
    typedef std::shared_lock<mutex_type>   SharedLock;
    typedef acme::upgrade_lock<mutex_type> UpgradeLock;
    typedef std::unique_lock<mutex_type>   Lock;

    mutable mutex_type  mut_;
    std::vector<double> data_;

public:

    A(const A& a)
    {
        SharedLock _(a.mut_);
        data_ = a.data_;
    }

    A& operator=(const A& a)
    {
        if (this != &a)
        {
            Lock       this_lock(mut_, std::defer_lock);
            SharedLock that_lock(a.mut_, std::defer_lock);
            std::lock(this_lock, that_lock);
            data_ = a.data_;
        }
        return *this;
    }

    void swap(A& a)
    {
        Lock this_lock(mut_, std::defer_lock);
        Lock that_lock(a.mut_, std::defer_lock);
        std::lock(this_lock, that_lock);
        data_.swap(a.data_);
    }

    void average(A& a)
    {
        assert(data_.size() == a.data_.size());
        assert(this != &a);

        Lock        this_lock(mut_, std::defer_lock);
        UpgradeLock share_that_lock(a.mut_, std::defer_lock);
        std::lock(this_lock, share_that_lock);

        for (unsigned i = 0; i < data_.size(); ++i)
            data_[i] = (data_[i] + a.data_[i]) / 2;

#if 0
        //  Not possible without committee help
        SharedLock share_this_lock(std::move(this_lock));
#else
        //  Ugly workaraound
        SharedLock share_this_lock(*this_lock.release(), std::adopt_lock);
        share_this_lock.mutex()->unlock_and_lock_shared();
#endif
        Lock that_lock(std::move(share_that_lock));
        a.data_ = data_;
    }
};

}  // Assignment

void temp()
{
    using namespace acme;
    using namespace std;
    static upgrade_mutex mut;
    unique_lock<upgrade_mutex> ul(mut);
    shared_lock<upgrade_mutex> sl;
#if 0
    //  Not possible without committee help
    sl = shared_lock<upgrade_mutex>(std::move(ul));
#else
    //  Ugly workaraound
    sl = shared_lock<upgrade_mutex>(*ul.release(), std::adopt_lock);
    sl.mutex()->unlock_and_lock_shared();
#endif
}

int main()
{
    S::test_shared_mutex();
    U::test_upgrade_mutex();
}
