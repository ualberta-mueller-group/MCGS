#pragma once

#include <atomic>
#include <memory>
#include <utility>

////////////////////////////////////////////////// class mcgs_stop_token
class mcgs_stop_token
{
public:
    mcgs_stop_token(std::shared_ptr<std::atomic<bool>> should_stop);
    bool stop_requested() const;

private:
    std::shared_ptr<std::atomic<bool>> _should_stop;
};

////////////////////////////////////////////////// class mcgs_stop_source
class mcgs_stop_source
{
public:
    mcgs_stop_source();
    bool stop_requested() const;
    mcgs_stop_token get_token();

private:
    std::shared_ptr<std::atomic<bool>> _should_stop;
};

////////////////////////////////////////////////// mcgs_stop_token methods
inline mcgs_stop_token::mcgs_stop_token(std::shared_ptr<std::atomic<bool>> should_stop)
    : _should_stop(std::move(should_stop))
{

}

inline bool mcgs_stop_token::stop_requested() const
{
    return *_should_stop;
}

////////////////////////////////////////////////// mcgs_stop_source methods
inline mcgs_stop_source::mcgs_stop_source()
    : _should_stop(new std::atomic<bool>(false))
{
}

inline bool mcgs_stop_source::stop_requested() const
{
    return *_should_stop;
}

inline mcgs_stop_token mcgs_stop_source::get_token()
{
    return mcgs_stop_token(_should_stop);
}

