import numpy as np

# Internal state
_pos = np.zeros(3)
_vel = np.zeros(3)
_dt = 0.01
_g = np.array([0, 0, 9.81])

def get_position(acc, gyro, mag):
    """
    Computes the new position based on IMU input.
    acc, gyro, mag should be tuples/lists of (x, y, z)
    """
    global _pos, _vel

    acc_arr = np.array(acc)
    
    # Minimal physics: subtract gravity 
    # (Note: This assumes the device stays level)
    a_lin = acc_arr - _g

    # Euler integration
    _vel += a_lin * _dt
    _pos += _vel * _dt

    # Damping to minimize drift
    _vel *= 0.98

    # Return current x, y, z
    return _pos.tolist()