#ifndef OCEAH_PID
#define OCEAH_PID

namespace oceah
{

    // 位置式 PID
    class PositionalPID
    {
    public:
        constexpr PositionalPID();
        constexpr PositionalPID(float kp, float ki, float kd);

        void reset();

        /// @return 控制量
        /// @param error 目标值减测量值
        float update(float error);

    private:
        float _kp;
        float _ki;
        float _kd;
        float _integral          = 0.0f;
        float _previous_error    = 0.0f;
        bool _has_previous_error = false;
    };

    // 增量式 PID
    class IncrementalPID
    {
    public:
        constexpr IncrementalPID();
        constexpr IncrementalPID(float kp, float ki, float kd);

        void reset();

        /// @return 控制量增量
        /// @param error 目标值减测量值
        float update(float error);

    private:
        float _kp;
        float _ki;
        float _kd;
        float _previous_error       = 0.0f;
        float _older_error          = 0.0f;
        bool _has_previous_error    = false;
        bool _has_two_error_samples = false;
    };

#pragma region detail

    constexpr PositionalPID::PositionalPID()
        : PositionalPID(1.0f, 0.001f, 10.0f)
    {
    }

    constexpr PositionalPID::PositionalPID(float kp, float ki, float kd)
        : _kp(kp), _ki(ki), _kd(kd)
    {
    }

    void PositionalPID::reset()
    {
        _integral           = 0.0f;
        _previous_error     = 0.0f;
        _has_previous_error = false;
    }

    float PositionalPID::update(float error)
    {
        _integral += error;

        float output = _kp * error + _ki * _integral;
        if (_has_previous_error)
            output += _kd * (error - _previous_error);

        _previous_error     = error;
        _has_previous_error = true;
        return output;
    }

    constexpr IncrementalPID::IncrementalPID()
        : IncrementalPID(1.0f, 0.001f, 10.0f)
    {
    }

    constexpr IncrementalPID::IncrementalPID(float kp, float ki, float kd)
        : _kp(kp), _ki(ki), _kd(kd)
    {
    }

    void IncrementalPID::reset()
    {
        _previous_error        = 0.0f;
        _older_error           = 0.0f;
        _has_previous_error    = false;
        _has_two_error_samples = false;
    }

    float IncrementalPID::update(float error)
    {
        float output;
        if (!_has_previous_error) {
            output = _kp * error;
        } else if (!_has_two_error_samples) {
            output = _kp * (error - _previous_error) + _ki * error;
        } else {
            output = _kp * (error - _previous_error) + _ki * error + _kd * (error - 2.0f * _previous_error + _older_error);
        }

        _older_error           = _previous_error;
        _previous_error        = error;
        _has_two_error_samples = _has_previous_error;
        _has_previous_error    = true;
        return output;
    }

#pragma endregion

} // namespace oceah

#endif // OCEAH_PID
