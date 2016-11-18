#! /bin/python3


import GPOF.strategies as gs
import GPOF.runset as gr
import GPOF.display as gd

sets = gr.open_runset_from_google_benchmark('./fast_raw_bench.txt')

v1 = sets['vpp_raw'].view(('range_y', 'real_time'))
v2 = sets['vpp_opencv_raw'].view(('range_y', 'real_time'))

data=[]
for r1, r2 in zip(v1.data, v2.data):
    data.append([r1[0],  r2[1] / r1[1]])

v=gr.RunSetView(('th', 'speedup'), data);

gd.display_2d_points(v)

