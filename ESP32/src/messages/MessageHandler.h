#ifndef _MESSAGE_HANDLER_H_
#define _MESSAGE_HANDLER_H_

#include <memory>
#include <vector>

#include "utils.h"
#include "logging/TagHandler.h"
#include "logging/LogHandler.h"

namespace Messages
{
    // A message structure for inter-task communication
    struct message_t
    {
        uint32_t tag_mask;
        union
        {
            /* data */
            uint32_t u_data;
            int32_t s_data;
            float f_data;
        };
        const char *message;
    };

    // A message sink buffers and processes messages
    constexpr size_t MESSAGE_SINK_DEFAULT_BUFFER_SIZE = 10;
    class MessageSink
    {
    private:
        uint32_t _tags_include;
        uint32_t _tags_exclude;
        Ringbuffer<message_t, MESSAGE_SINK_DEFAULT_BUFFER_SIZE> _message_buffer;
        message_t _current_message;
    public:
        MessageSink(uint32_t include_tags = Tags::ALL_TAGS, uint32_t exclude_tags = 0) : _tags_include(include_tags), _tags_exclude(exclude_tags) {}

        void handle_msg(uint32_t tags, message_t msg)
        {
            if ((_tags_include & tags) && !(_tags_exclude & tags))
            {
                if (!_message_buffer.push(msg))
                {
                    const char* message = msg.message ? msg.message : "(null)";
                    LogHandler::error(Tags::MessageQueue, "Sink message buffer full, dropping message: %s", message);
                }
            }
        }

        message_t* next_message()
        {
            if (_message_buffer.pop(_current_message))
            {
                return &_current_message;
            }
            return nullptr;
        }
    };

    // Routes messages to registered sinks
    class MessageRouter
    {
    private:
        inline static MessageRouter* _singleton = nullptr;
        std::vector<MessageSink*> _sinks;
    public:
        MessageRouter() {
            if (!_singleton) {
                _singleton = this;
            }
        }

        void push_sink(MessageSink* sink)
        {
            _sinks.push_back(sink);
        }

        void send(uint32_t tags, const char *msg)
        {
            message_t _msg{
                .tag_mask = tags,
                .s_data = -1,
                .message = msg,
            };
            send(_msg);
        }

        void send(message_t msg)
        {
            for (auto &sink : _sinks)
            {
                if (sink)
                {
                    sink->handle_msg(msg.tag_mask, msg);
                }
            }
        }

        static MessageRouter *getInstance()
        {
            if (!_singleton) {
                _singleton = new MessageRouter();
            }
            return _singleton;
        }
    };
};

#endif // _MESSAGE_HANDLER_H_
