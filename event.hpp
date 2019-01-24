#ifndef __PD_EVENT__
#define __PD_EVENT__

#define PD_EVENT_VER 8

#include <functional>
#include <iostream>
#include <stdint.h>
#include <unordered_map>

using namespace std::placeholders;

//
// @brief  Describes additional parameters for handler binding.
//
enum class EventFlag {
    DEFAULT = 0,
    ONLY_UNIQUE = 1
};

//
// @version 8
// @brief  Defines means for delegate function subscription and calling on demand.
//         Maximum allowed number of event arguments is 4.
// @param
//
template<class... Args>
class Event{

    //
    // @brief  Address representation for use as identifier in container mapping.
    //
    using Address = uint64_t;

    //
    // @brief  Function object with user-defined variadic template arguments.
    //
    using Handler = std::function<void(Args...)>;

    //
    // @brief  Type that provides a unique memory-based integral definition for given objects.
    //         Concept: should be collision-free.
    //
    using Identifier = uint64_t;

    //
    // @brief  Maximum supported event handler arguments.
    //
    static constexpr auto _MAX_EVENT_ARGS = 4;
    static_assert(sizeof...(Args) <= _MAX_EVENT_ARGS, "Too many arguments");

public:
    //
    // @brief  Subscribes a lambda expression with a matching argument list.
    // @param  lambda - the lambda expression to bind.
    //
    void operator+=(Handler lambda) {
        // TODO find a way to unbind
        _addToList(_identify(&lambda, 0), lambda, EventFlag::DEFAULT);
    }

    //
    // @brief  Doubles as a fire function.
    //
    void operator()(Args... e) {
        fire(e...);
    }

    //
    // @brief  Calls every subscriber in container with given arguments.
    // @param  e... - arguments, that the defined event accepts.
    //         Contract: [0-4] arguments
    //
    void fire(Args... e) {
        for (auto subscriber : subscribers)
            subscriber.second(e...);
    }

    //
    // @brief  Checks if there's a subscriber with a matching address in the container.
    //
    bool hasSubscriber(Address a) const {
        return subscribers.find(a) != subscribers.end();
    }

#pragma mark - bind(...) overloads

    //
    // @brief  Subscribes a member function with no arguments.
    // @param  member - member function pointer &C::M.
    // @param  instance - instance of structure housing the member function.
    // @usage  <i>event.bind(&MyClass::member, myClassInstance)</i>.
    //
    template<class C, class M, class T>
    void bind(C (M::*member)(), T *instance, EventFlag flag = EventFlag::DEFAULT) {
        _addToList(_identify(instance, member), std::bind(member, instance), flag);
    }

    //
    // @brief  Subscribes a member function with 1 argument.
    //         Event handler argument A1.
    //
    template<class C, class M, class T, typename A1>
    void bind(C (M::*member)(A1), T *instance, EventFlag flag = EventFlag::DEFAULT) {
        _addToList(_identify(instance, member), std::bind(member, instance, _1), flag);
    }

    //
    // @brief  Subscribes a member function with 2 arguments.
    //         Event handler arguments: A1, A2.
    //
    template<class C, class M, class T, typename A1, typename A2>
    void bind(C (M::*member)(A1, A2), T *instance, EventFlag flag = EventFlag::DEFAULT) {
        _addToList(_identify(instance, member), std::bind(member, instance, _1, _2), flag);
    }

    //
    // @brief  Subscribes a member function with 3 arguments.
    //         Event handler arguments: A1, A2, A3.
    //
    template<class C, class M, class T, typename A1, typename A2, typename A3>
    void bind(C (M::*member)(A1, A2, A3), T *instance, EventFlag flag = EventFlag::DEFAULT) {
        _addToList(_identify(instance, member), std::bind(member, instance, _1, _2, _3), flag);
    }

    //
    // @brief  Subscribes a member function with 4 arguments.
    //         Event handler arguments: A1, A2, A3, A4.
    //
    template<class C, class M, class T, typename A1, typename A2, typename A3, typename A4>
    void bind(C (M::*member)(A1, A2, A3, A4), T *instance, EventFlag flag = EventFlag::DEFAULT) {
        _addToList(_identify(instance, member), std::bind(member, instance, _1, _2, _3, _4), flag);
    }

#pragma mark - unbind(...) overloads

    //
    // @brief  Unsubscribes a member function by its function pointer address as key.
    // @param  member - member function pointer &C::M.
    // @param  instance - instance of structure housing the member function.
    // @usage  <i>event.unbind(&MyClass::member, myClassInstance)</i>.
    //
    template<typename C, typename M, typename T>
    void unbind(C (M::*member)(), T *instance) {
        _removeFromList(_identify(instance, member));
    }

    //
    // @brief  Unsubscribes a member function by its function pointer address as key.
    //         Event handler argument A1.
    //
    template<class C, class M, class T, typename A1>
    void unbind(C (M::*member)(A1), T *instance) {
        _removeFromList(_identify(instance, member));
    }

    //
    // @brief  Unsubscribes a member function by its function pointer address as key.
    //         Event handler arguments: A1, A2.
    //
    template<class C, class M, class T, typename A1, typename A2>
    void unbind(C (M::*member)(A1, A2), T *instance) {
        _removeFromList(_identify(instance, member));
    }

    //
    // @brief  Unsubscribes a member function by its function pointer address as key.
    //         Event handler arguments: A1, A2, A3.
    //
    template<class C, class M, class T, typename A1, typename A2, typename A3>
    void unbind(C (M::*member)(A1, A2, A3), T *instance) {
        _removeFromList(_identify(instance, member));
    }

    //
    // @brief  Unsubscribes a member function by its function pointer address as key.
    //         Event handler arguments: A1, A2, A3, A4.
    //
    template<class C, class M, class T, typename A1, typename A2, typename A3, typename A4>
    void unbind(C (M::*member)(A1, A2, A3, A4), T *instance) {
        _removeFromList(_identify(instance, member));
    }

private:

#pragma mark - Address conversion

    //
    // @brief  Unions a generic lvalue object address and an integral identifier.
    //
    template<typename T>
    union AddressCast {
        explicit AddressCast(T _type) : type(_type) { }
        T type;
        Identifier address;
    };

    //
    // @brief  Returns a unique identifier for a given member function pointer and instance pointer.
    //
    template<class C, class M>
    inline static Identifier _identify(C _class, M _member) {
        return AddressCast<C>(_class).address * 10 + AddressCast<M>(_member).address;
    }

#pragma mark - Private inline functions

    //
    // @brief  Validates prepared function object and adds it to the container.
    //         Contract: handlers with matching identifiers present in the container are silently ignored.
    //
    inline void _addToList(Address a, Handler h, EventFlag flag) {
        // disallow non-unique handlers
        if (flag == EventFlag::ONLY_UNIQUE && hasSubscriber(a)) return;

        subscribers.insert(std::make_pair(a, h));
    }

    //
    // @brief  Removes bound function object from the container by its address.
    //         Contract: non-existent member identifier is silently ignored.
    //
    inline void _removeFromList(Address a) {
        auto it = subscribers.find(a);
        if (it != subscribers.end())
            subscribers.erase(it);
        else {
            // TODO handle
        }
    }

    //
    // @brief  Pairs bound subscribers with their unique memory address.
    //         Container has to be unordered for the handlers to be called sequentially.
    //
    std::unordered_map<Identifier, Handler> subscribers;
};

#endif
