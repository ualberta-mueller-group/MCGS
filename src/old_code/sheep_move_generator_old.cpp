


//namespace {
///*
//    Uses move3 representation:
//    1. Target herd value (signed)
//    2. Herd start index
//    3. Herd end index
//*/
//class sheep_move_generator: public move_generator
//{
//public:
//    sheep_move_generator(const sheep& g, bw to_play);
//
//    void operator++() override;
//    operator bool() const override;
//    ::move gen_move() const override;
//
//private:
//    bool _increment(bool init);
//
//    bool _increment_herd_start(bool init);
//    bool _increment_target_dir(bool init);
//    bool _increment_target_size(bool init);
//
//    bool _increment_size_precondition() const;
//
//    const sheep& _g;
//    const int _player_step;
//
//    bool _has_move;
//
//    // Part 1
//    grid_location _herd_start;
//    int _herd_start_idx;
//    int _herd_start_size;
//
//    // Part 2
//    size_t _target_dir_idx;
//    grid_location _target_end;
//    int _target_end_idx;
//
//    // Part 3
//    int _target_size;
//};
//
//sheep_move_generator::sheep_move_generator(const sheep& g, bw to_play)
//    : move_generator(to_play),
//      _g(g),
//      _player_step(to_play == BLACK ? 1 : -1),
//      _herd_start(g.shape()),
//      _target_end(g.shape())
//{
//    _increment(true);
//}
//
//void sheep_move_generator::operator++()
//{
//    assert(*this);
//    _increment(false);
//}
//
//sheep_move_generator::operator bool() const
//{
//    return _has_move;
//}
//
//::move sheep_move_generator::gen_move() const
//{
//    assert(*this);
//
//    /*
//       3 part move:
//       _target_size (signed)
//       _herd_start_idx
//       _target_end_idx
//    */
//    assert(_herd_start_idx >= 0 && //
//           _target_end_idx >= 0    //
//    );
//
//    return cgt_move::move3_create(_target_size, _herd_start_idx,
//                                      _target_end_idx);
//}
//
//bool sheep_move_generator::_increment(bool init)
//{
//    assert(init || *this);
//
//    if (init)
//    {
//        _has_move = false;
//
//        if (_g.size() == 0)
//            return false;
//    }
//
//    bool has_herd_start = _has_move;
//    bool has_target_dir = _has_move;
//    bool has_target_size = _has_move;
//
//    while (true)
//    {
//        // Try to increment target size
//        if (has_target_dir && _increment_target_size(!has_target_size))
//        {
//            _has_move = true;
//            return true;
//        }
//
//        has_target_size = false;
//
//        // Try to increment target dir
//        if (has_herd_start && _increment_target_dir(!has_target_dir))
//        {
//            has_target_dir = true;
//            continue;
//        }
//
//        has_target_dir = false;
//
//        // Try to increment herd start
//        if (_increment_herd_start(!has_herd_start))
//        {
//            has_herd_start = true;
//            continue;
//        }
//
//        _has_move = false;
//        return false;
//    }
//}
//
//bool sheep_move_generator::_increment_herd_start(bool init)
//{
//    if (init)
//    {
//        _herd_start.set_coord({0, 0});
//        _herd_start_idx = 0;
//        _herd_start_size = 0;
//    }
//
//    assert(_herd_start.valid());
//
//    if (!init)
//        _herd_start.increment_position();
//
//    if (!_herd_start.valid())
//        return false;
//
//    const bw player = to_play();
//    assert(is_black_white(player));
//
//    while (_herd_start.valid())
//    {
//        _herd_start_idx = _herd_start.get_point();
//        _herd_start_size = _g.at(_herd_start_idx);
//
//        if (herd_movable_and_belongs_to_player(_herd_start_size, player))
//            return true;
//
//        _herd_start.increment_position();
//    }
//
//    return false;
//}
//
//bool sheep_move_generator::_increment_target_dir(bool init)
//{
//    if (init)
//        _target_dir_idx = 0;
//    else
//        _target_dir_idx++;
//
//    for (; _target_dir_idx < GRID_DIRS_HEX.size(); _target_dir_idx++)
//    {
//        const grid_dir dir = GRID_DIRS_HEX[_target_dir_idx];
//
//        _target_end = _herd_start;
//        assert(_target_end.valid());
//
//        // Find real target location by moving 1 step at a time
//        grid_location neighbor = _target_end;
//
//        while (true)
//        {
//            // Get next step
//            if (!neighbor.move(dir))
//                break;
//
//            // Is this step pathable?
//            const int neighbor_point = neighbor.get_point();
//            const int neighbor_val = _g.at(neighbor_point);
//
//            if (neighbor_val != 0)
//                break;
//
//            // Allow this step
//            _target_end = neighbor;
//        }
//
//        // Accept IFF target moved from the start
//        // if (_target_end == _herd_start) // TODO equality operator?
//        assert(_target_end.get_shape() == _herd_start.get_shape());
//        if (_target_end.get_coord() == _herd_start.get_coord())
//            continue;
//
//        // Accept
//        _target_end_idx = _target_end.get_point();
//        assert(_g.at(_target_end_idx) == 0);
//        return true;
//    }
//
//    return false;
//}
//
//bool sheep_move_generator::_increment_target_size(bool init)
//{
//    assert(_increment_size_precondition());
//
//    const bw player = to_play();
//    assert(is_black_white(player) &&                  //
//           _player_step == (player == BLACK ? 1 : -1) //
//    );
//
//    if (init)
//        _target_size = _player_step;
//    else
//    {
//        const int abs_target_prev = abs(_target_size);
//        assert(abs_target_prev < abs(_herd_start_size));
//
//        _target_size += _player_step;
//        assert(abs(_target_size) > abs_target_prev);
//    }
//
//    return _target_size != _herd_start_size;
//}
//
//// For debugging. NOTE: doesn't check pathing
//bool sheep_move_generator::_increment_size_precondition() const
//{
//    // Valid start point
//    if (!_herd_start.valid())
//        return false;
//
//    assert(_herd_start.get_point() == _herd_start_idx);
//    const int herd = _g.at(_herd_start_idx);
//
//    assert(herd == _herd_start_size);
//
//    if (!herd_movable_and_belongs_to_player(herd, to_play()))
//        return false;
//
//    // Valid end point
//    assert(_target_end.valid() &&                     //
//           _target_end.get_point() == _target_end_idx //
//    );
//
//    return _g.at(_target_end_idx) == 0;
//}
//} // namespace

