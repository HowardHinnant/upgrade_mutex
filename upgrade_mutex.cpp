//--------------------------- upgrade_mutex.cpp --------------------------------
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

namespace acme
{

upgrade_mutex::upgrade_mutex()
    : state_(0)
{
}

upgrade_mutex::~upgrade_mutex() = default;

// Exclusive ownership

void
upgrade_mutex::lock()
{
    std::unique_lock<std::mutex> lk(mut_);
    while (state_ & (write_entered_ | upgradable_entered_))
        gate1_.wait(lk);
    state_ |= write_entered_;
    while (state_ & n_readers_)
        gate2_.wait(lk);
}

bool
upgrade_mutex::try_lock()
{
    std::unique_lock<std::mutex> lk(mut_);
    if (state_ == 0)
    {
        state_ = write_entered_;
        return true;
    }
    return false;
}

void
upgrade_mutex::unlock()
{
    std::lock_guard<std::mutex> _(mut_);
    state_ = 0;
    gate1_.notify_all();
}

// Shared ownership

void
upgrade_mutex::lock_shared()
{
    std::unique_lock<std::mutex> lk(mut_);
    while ((state_ & write_entered_) || (state_ & n_readers_) == n_readers_)
        gate1_.wait(lk);
    unsigned num_readers = (state_ & n_readers_) + 1;
    state_ &= ~n_readers_;
    state_ |= num_readers;
}

bool
upgrade_mutex::try_lock_shared()
{
    std::unique_lock<std::mutex> lk(mut_);
    unsigned num_readers = state_ & n_readers_;
    if (!(state_ & write_entered_) && num_readers != n_readers_)
    {
        ++num_readers;
        state_ &= ~n_readers_;
        state_ |= num_readers;
        return true;
    }
    return false;
}

void
upgrade_mutex::unlock_shared()
{
    std::lock_guard<std::mutex> _(mut_);
    unsigned num_readers = (state_ & n_readers_) - 1;
    state_ &= ~n_readers_;
    state_ |= num_readers;
    if (state_ & write_entered_)
    {
        if (num_readers == 0)
            gate2_.notify_one();
    }
    else
    {
        if (num_readers == n_readers_ - 1)
            gate1_.notify_one();
    }
}

// Upgrade ownership

void
upgrade_mutex::lock_upgrade()
{
    std::unique_lock<std::mutex> lk(mut_);
    while ((state_ & (write_entered_ | upgradable_entered_)) || 
           (state_ & n_readers_) == n_readers_)
        gate1_.wait(lk);
    unsigned num_readers = (state_ & n_readers_) + 1;
    state_ &= ~n_readers_;
    state_ |= upgradable_entered_ | num_readers;
}

bool
upgrade_mutex::try_lock_upgrade()
{
    std::unique_lock<std::mutex> lk(mut_);
    unsigned num_readers = state_ & n_readers_;
    if (!(state_ & (write_entered_ | upgradable_entered_))
        && num_readers != n_readers_)
    {
        ++num_readers;
        state_ &= ~n_readers_;
        state_ |= upgradable_entered_ | num_readers;
        return true;
    }
    return false;
}

void
upgrade_mutex::unlock_upgrade()
{
    {
        std::lock_guard<std::mutex> _(mut_);
        unsigned num_readers = (state_ & n_readers_) - 1;
        state_ &= ~(upgradable_entered_ | n_readers_);
        state_ |= num_readers;
    }
    gate1_.notify_all();
}

// Shared <-> Exclusive

bool
upgrade_mutex::try_unlock_shared_and_lock()
{
    std::unique_lock<std::mutex> lk(mut_);
    if (state_ == 1)
    {
        state_ = write_entered_;
        return true;
    }
    return false;
}

void
upgrade_mutex::unlock_and_lock_shared()
{
    {
        std::lock_guard<std::mutex> _(mut_);
        state_ = 1;
    }
    gate1_.notify_all();
}

// Shared <-> Upgrade

bool
upgrade_mutex::try_unlock_shared_and_lock_upgrade()
{
    std::unique_lock<std::mutex> lk(mut_);
    if (!(state_ & (write_entered_ | upgradable_entered_)))
    {
        state_ |= upgradable_entered_;
        return true;
    }
    return false;
}

void
upgrade_mutex::unlock_upgrade_and_lock_shared()
{
    {
        std::lock_guard<std::mutex> _(mut_);
        state_ &= ~upgradable_entered_;
    }
    gate1_.notify_all();
}

// Upgrade <-> Exclusive

void
upgrade_mutex::unlock_upgrade_and_lock()
{
    std::unique_lock<std::mutex> lk(mut_);
    unsigned num_readers = (state_ & n_readers_) - 1;
    state_ &= ~(upgradable_entered_ | n_readers_);
    state_ |= write_entered_ | num_readers;
    while (state_ & n_readers_)
        gate2_.wait(lk);
}

bool
upgrade_mutex::try_unlock_upgrade_and_lock()
{
    std::unique_lock<std::mutex> lk(mut_);
    if (state_ == (upgradable_entered_ | 1))
    {
        state_ = write_entered_;
        return true;
    }
    return false;
}

void
upgrade_mutex::unlock_and_lock_upgrade()
{
    {
        std::lock_guard<std::mutex> _(mut_);
        state_ = upgradable_entered_ | 1;
    }
    gate1_.notify_all();
}

}  // acme
