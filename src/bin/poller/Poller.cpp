#include "Poller.h"
#include "../channel__/Channel.h"
#include "../times/timestamp.h"
#include <poll.h>

TimeStamp Poller::poll(int timeOutMs, ChannelList *channelContainer) {
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeOutMs);
    TimeStamp now(TimeStamp::now());
    if (numEvents > 0) {
        std::cout << numEvents << " events happened";
    } else if (numEvents == 0) {

    } else {
        std::cerr << "Poller::poll() error!";
    }
    return now;
}

void
Poller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    for (PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd) {
        if(pfd->revents <= 0)
            continue;
        --numEvents;
        auto&& chanIter = channels_.find(pfd->fd);
        assert(chanIter != channels_.end());
        auto&& channel = chanIter->second;
        channel->set_revents(pfd->revents);
        activeChannels->push_back(channel);
    }
}