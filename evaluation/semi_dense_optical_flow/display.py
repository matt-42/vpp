import sys
import numpy as np

import gpof

from gpof.runset import RunSet


rs=gpof.open_runset('./pareto.rs');
gpof.display.display_2d_colorpoints(rs.view(('errors', 'runtime', 'nkeypoints')))

