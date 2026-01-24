#ifndef _MESSAGE_HANDLER_H_
#define _MESSAGE_HANDLER_H_

#include <memory>

#include "TagHandler.h"

namespace Messages {
struct message_t
{
    uint8_t id;
    const char* message;
    union 
    {
        /* data */
        uint32_t u_data;
        int32_t s_data;
        float f_data;
    };
};

class MessageSink
{
    protected:
        virtual void sink(message_t) = 0;
    private:
        uint32_t _tags;
    public:
        MessageSink(uint32_t tags = ALL_TAGS) : _tags(tags) {}

        void handle_msg(uint32_t tags, message_t msg)
        {
            if (_tags & tags)
            {
                this->sink(msg);
            }
        }
};

class MessageHandler
{
    private:
        std::vector<std::unique_ptr<MessageSink>> _sinks;
    public:
        void push(std::unique_ptr<MessageSink> sink)
        {
            _sinks.push_back(std::move(sink));
        }

        void send(uint32_t tags, const char* msg)
        {
            message_t _msg{msg, {{0}}};
            send(tags, _msg);
        }

        void send(uint32_t tags, message_t msg)
        {
            for(auto&& sink : _sinks)
            {
                sink->handle_msg(tags, msg);
            }
        }

        MessageHandler* getInstance()
        {
            static MessageHandler instance;
            return &instance;
        }
};
};

#endif // _MESSAGE_HANDLER_H_
