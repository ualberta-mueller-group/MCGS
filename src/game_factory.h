// This file will probably get deleted soon


//    #pragma once
//    #include "game.h"
//    #include <memory>
//
//    // Interface of instantiated game factory
//    struct game_factory
//    {
//        virtual game* new_game() const = 0;
//
//        virtual ~game_factory()
//        {}
//    };
//
//
//    // Messy hidden internals of game factory
//    template <class T, class ...Ts>
//    struct _game_factory_impl : public game_factory
//    {
//        // T is type of game
//        // ...Ts is types of arguments to game constructor
//
//        _game_factory_impl(Ts... args) : data(args...)
//        { }
//
//        virtual ~_game_factory_impl()
//        { }
//
//
//        game* new_game() const override
//        {
//            return std::apply(_make_new, data);
//        }
//
//    private:
//
//        // this is just here so we can call std::apply to unpack the tuple
//        static game* _make_new(Ts... args)
//        {
//            return new T(args...);
//        }
//
//
//        std::tuple<Ts...> data;
//    };
//
//    /*  i.e.
//        make_factory<clobber_1xn>("XO.OX");
//        make_factory<switch_game>(5, 3);
//
//    */
//    typedef std::shared_ptr<game_factory> game_factory_ptr;
//
//    template <class T, class ...Ts>
//    game_factory_ptr make_factory(Ts... data)
//    {
//        _game_factory_impl<T, Ts...>* ptr = new _game_factory_impl<T, Ts...>(data...);
//
//        return std::shared_ptr<game_factory>(ptr);
//
//    }
//
