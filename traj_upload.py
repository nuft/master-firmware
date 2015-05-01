import cvra_rpc.message
import math

from collections import namedtuple

TrajectoryPoint = namedtuple("TrajectoryPoint",
                             ['x', 'y', 'theta', 'speed', 'omega']
                             )

Trajectory = namedtuple("Trajectory", ["start_time", "sampling_time",
                                       "points"])


def prepare_for_sending(traj):
    res = [int(traj.start_time),
           int(1e6 * (traj.start_time - int(traj.start_time))),
           int(1e6 * traj.sampling_time),
           [[p.x, p.y, p.theta, p.speed, p.omega] for p in traj.points]
           ]
    return res


def send_traj(host, traj):
    cvra_rpc.message.send(host, 'traj', prepare_for_sending(traj))


def convert_from_molly(traj, start_time, sampling_time):
    res = Trajectory(start_time=start_time,
                     sampling_time=sampling_time,
                     points=[])
    for t in traj:
        pos, spd, acc, ts = t

        # Equation 8 of tracy's paper
        speed = math.sqrt(spd.pos_x ** 2 + spd.pos_y ** 2)
        if speed > 1e-3:
            omega = (spd.pos_x * acc.pos_y - spd.pos_y * acc.pos_x)
            omega = omega / speed ** 2
        else:
            omega = 0

        point = TrajectoryPoint(x=pos.pos_x,
                                y=pos.pos_y,
                                theta=math.atan2(spd.pos_y, spd.pos_x),
                                speed=speed,
                                omega=omega,
                                )

        res.points.append(point)

    return res


if __name__ == '__main__':
    import time
    now = time.time()
    SAMPLE_INTERVAL = 0.1

    traj = [TrajectoryPoint(x=0., y=0., theta=0., speed=i*0.02, omega=i*0.4,
                            timestamp=now + i * SAMPLE_INTERVAL)
            for i in range(10)]

    send_traj(('192.168.2.20', 20000), traj)
